/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2005, Øyvind Kolås.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, see
 * <https://www.gnu.org/licenses/>.
 */


#include "config.h"
#include <string.h>
#include <stdarg.h>
#include <math.h>
#define NEEDS_BABL_DB
#include "babl-internal.h"
#include "babl-db.h"
#include "babl-ref-pixels.h"

static void
babl_conversion_plane_process (BablConversion *conversion,
                               const void     *source,
                               void           *destination,
                               int             src_pitch,
                               int             dst_pitch,
                               long            n,
                               void           *user_data)
{
  conversion->function.plane ((void*)conversion, source, destination,
                              src_pitch, dst_pitch,
                              n,
                              user_data);
}

static void
babl_conversion_planar_process (const Babl *babl,
                                const char *src,
                                char       *dst,
                                long        n,
                                void       *user_data)
{
  BablConversion *conversion = (void*)babl;
  const BablImage *source = (void*)src;
  BablImage       *destination = (void*)dst;
#ifdef USE_ALLOCA
  const char **src_data = alloca (sizeof (void *) * source->components);
  char **dst_data = alloca (sizeof (void *) * destination->components);
#else
  const char  *src_data[BABL_MAX_COMPONENTS];
  char  *dst_data[BABL_MAX_COMPONENTS];
#endif

  memcpy (src_data, source->data, sizeof (void *) * source->components);
  memcpy (dst_data, destination->data, sizeof (void *) * destination->components);
  conversion->function.planar ((void*)conversion,
                                      source->components,
                                      src_data,
                                      source->pitch,
                                      destination->components,
                                      dst_data,
                                      destination->pitch,
                                      n,
                                      user_data);
}

static void dispatch_plane (const Babl *babl,
                            const char *source,
                            char       *destination,
                            long        n,
                            void       *user_data)
{
  const BablConversion *conversion = &babl->conversion;
  const void *src_data  = NULL;
  void *dst_data  = NULL;
  int   src_pitch = 0;
  int   dst_pitch = 0;

  if (BABL_IS_BABL (source))
    {
      BablImage *img;

      img       = (BablImage *) source;
      src_data  = img->data[0];
      src_pitch = img->pitch[0];
    }
  if (BABL_IS_BABL (destination))
    {
      BablImage *img = (BablImage *) destination;

      dst_data  = img->data[0];
      dst_pitch = img->pitch[0];
    }

  if (!src_data)
    src_data = source;
  if (!src_pitch)
    src_pitch = BABL (conversion->source)->type.bits / 8;
  if (!dst_data)
    dst_data = destination;
  if (!dst_pitch)
    dst_pitch = BABL (conversion->destination)->type.bits / 8;

  babl_conversion_plane_process ((void*)conversion,
                                 src_data, dst_data,
                                 src_pitch, dst_pitch,
                                 n, user_data);
}

static inline void
babl_conversion_rig_dispatch (const Babl *babl)
{
  BablConversion *conversion = (BablConversion *) babl;
  switch (BABL (conversion)->class_type)
  {
    case BABL_CONVERSION_PLANE:
      conversion->dispatch = dispatch_plane;
      break;
    case BABL_CONVERSION_PLANAR:
      conversion->dispatch = babl_conversion_planar_process;
      break;
    case BABL_CONVERSION_LINEAR:
      conversion->dispatch = conversion->function.linear;
      break;
  }
}

Babl *
_conversion_new (const char    *name,
                 int            id,
                 const Babl    *source,
                 const Babl    *destination,
                 BablFuncLinear linear,
                 BablFuncPlane  plane,
                 BablFuncPlanar planar,
                 void          *user_data,
                 int            allow_collision)
{
  Babl *babl = NULL;

  babl_assert (source->class_type ==
               destination->class_type);

  babl                = babl_malloc (sizeof (BablConversion) + strlen (name) + 1);
  babl->instance.name = (char *) babl + sizeof (BablConversion);
  strcpy (babl->instance.name, name);

  if (linear)
    {
      babl->class_type                 = BABL_CONVERSION_LINEAR;
      babl->conversion.function.linear = linear;
    }
  else if (plane)
    {
      babl->class_type                = BABL_CONVERSION_PLANE;
      babl->conversion.function.plane = plane;
    }
  else if (planar)
    {
      babl->class_type                 = BABL_CONVERSION_PLANAR;
      babl->conversion.function.planar = planar;
    }
  switch (source->class_type)
    {
      case BABL_TYPE:
        if (linear) /* maybe linear could take a special argument, passed as an
                       additional key/value pair in the constructor. To cast it
                       as a generic N-element conversion, thus making it applicable
                       to being generic for any within model conversion of plain
                       buffers.
                     */
          {
            babl_fatal ("linear conversions not supported for %s",
                        babl_class_name (source->class_type));
          }
        else if (planar)
          {
            babl_fatal ("planar conversions not supported for %s",
                        babl_class_name (source->class_type));
          }
        break;

      case BABL_MODEL:
        if (plane)
          {
            babl_fatal ("plane conversions not supported for %s",
                        babl_class_name (source->class_type));
          }
        break;

      case BABL_FORMAT:
        break;

      default:
        babl_fatal ("%s unexpected", babl_class_name (babl->class_type));
        break;
    }

  babl->instance.id            = id;
  babl->conversion.source      = source;
  babl->conversion.destination = destination;
  babl->conversion.error       = -1.0;
  babl->conversion.cost        = 69L;

  babl->conversion.pixels      = 0;

  babl->conversion.data = user_data;

  if (babl->class_type == BABL_CONVERSION_LINEAR &&
      BABL (babl->conversion.source)->class_type == BABL_MODEL)
    {
      const Babl *src_format = NULL;
      const Babl *dst_format = NULL;

      src_format = babl_format_with_model_as_type (
        BABL (babl->conversion.source),
        babl_type_from_id (BABL_DOUBLE));
      dst_format = babl_format_with_model_as_type (
        BABL (babl->conversion.destination),
        babl_type_from_id (BABL_DOUBLE));

      if(allow_collision){
        const Babl *fish = babl_conversion_find (src_format, dst_format);
        if (fish)
          return (void*)fish;
      }
      babl_conversion_new (
        src_format,
        dst_format,
        "linear", linear,
        "data", user_data,
        allow_collision?"allow-collision":NULL,
        NULL);
      babl->conversion.error = 0.0;
    }

  babl_conversion_rig_dispatch (babl);
  return babl;
}

static char buf[512] = "";
static int collisions = 0;

static char *
create_name (Babl *source, 
             Babl *destination, 
             int   type)
{
  if (babl_extender ())
    {
      snprintf (buf, sizeof (buf), "%s %i: %s%s to %s",
                BABL (babl_extender ())->instance.name,
                collisions,
                type == BABL_CONVERSION_LINEAR ? "" :
                type == BABL_CONVERSION_PLANE ? "plane " :
                type == BABL_CONVERSION_PLANAR ? "planar " : "Eeeek! ",
                source->instance.name,
                destination->instance.name);
    }
  else
    {
      snprintf (buf, sizeof (buf), "%s %s to %s %i",
                type == BABL_CONVERSION_LINEAR ? "" :
                type == BABL_CONVERSION_PLANE ? "plane " :
                type == BABL_CONVERSION_PLANAR ? "planar " : "Eeeek! ",
                source->instance.name,
                destination->instance.name,
                collisions);
    }
  return buf;
}

const char *
babl_conversion_create_name (Babl *source, 
                             Babl *destination, 
                             int   type,
                             int   allow_collision)
{
  Babl *babl;
  char *name;
  int id = 0;
  collisions = 0;
  name = create_name (source, destination, type);

  if (allow_collision == 0)
  {
  babl = babl_db_exist (db, id, name);
  while (babl)
    {
      /* we allow multiple conversions to be registered per extender, each
         of them ending up with their own unique name
       */
      collisions++;
      name = create_name (source, destination, type);
      babl = babl_db_exist (db, id, name);
    }
  }
  return name;
}

const Babl *
babl_conversion_new (const void *first_arg,
                     ...)
{
  va_list        varg;
  Babl          *babl;

  int            id       = 0;
  BablFuncLinear linear   = NULL;
  BablFuncPlane  plane    = NULL;
  BablFuncPlanar planar   = NULL;
  int            type     = 0;
  int            got_func = 0;
  const char    *arg      = first_arg;
  void          *user_data= NULL;

  Babl          *source;
  Babl          *destination;
  char          *name;
  int            allow_collision = 0;

  va_start (varg, first_arg);
  source      = (Babl *) arg;
  destination = va_arg (varg, Babl *);
  arg         = va_arg (varg, char *);

  assert (BABL_IS_BABL (source));
  assert (BABL_IS_BABL (destination));


  while (arg)
    {
      if (!strcmp (arg, "id"))
        {
          id = va_arg (varg, int);
        }

      else if (!strcmp (arg, "data"))
        {
          user_data = va_arg (varg, void*);
        }

      else if (!strcmp (arg, "allow-collision"))
        {
          allow_collision = 1;
        }
      else if (!strcmp (arg, "linear"))
        {
          if (got_func++)
            {
              babl_fatal ("already got a conversion func\n");
            }
          linear = va_arg (varg, BablFuncLinear);
        }

      else if (!strcmp (arg, "plane"))
        {
          if (got_func++)
            {
              babl_fatal ("already got a conversion func\n");
            }
          plane = va_arg (varg, BablFuncPlane);
        }

      else if (!strcmp (arg, "planar"))
        {
          if (got_func++)
            {
              babl_fatal ("already got a conversion func\n");
            }
          planar = va_arg (varg, BablFuncPlanar);
        }

      else
        {
          babl_fatal ("unhandled argument '%s'", arg);
        }

      arg = va_arg (varg, char *);
    }

  va_end (varg);

  assert (source);
  assert (destination);

  if (linear)
    {
      type = BABL_CONVERSION_LINEAR;
    }
  else if (plane)
    {
      type = BABL_CONVERSION_PLANE;
    }
  else if (planar)
    {
      type = BABL_CONVERSION_PLANAR;
    }

  name = (void*) babl_conversion_create_name (source, destination, type, allow_collision);

  babl = _conversion_new (name, id, source, destination, linear, plane, planar,
                          user_data, allow_collision);

  /* Since there is not an already registered instance by the required
   * id/name, inserting newly created class into database.
   */
  babl_db_insert (db, babl);
  if (!source->type.from_list)
    source->type.from_list = babl_list_init_with_size (BABL_CONVERSIONS);
  babl_list_insert_last (source->type.from_list, babl);
  return babl;
}


long
babl_conversion_cost (BablConversion *conversion)
{
  if (!conversion)
    return 100000000L;
  if (conversion->error == -1.0)
    babl_conversion_error (conversion);
  return conversion->cost;
}

double
babl_conversion_error (BablConversion *conversion)
{
  Babl *fmt_source;
  Babl *fmt_destination;

  const Babl *fmt_rgba_double = babl_format_with_space ("RGBA double",
                                                 conversion->destination->format.space);
  double  error       = 0.0;
  long    ticks_start = 0;
  long    ticks_end   = 0;

  const int test_pixels = babl_get_num_conversion_test_pixels ();
  const double *test = babl_get_conversion_test_pixels ();

  void   *source;
  void   *destination;
  double *destination_rgba_double;
  void   *ref_destination;
  double *ref_destination_rgba_double;

  Babl   *fish_rgba_to_source;
  Babl   *fish_reference;
  Babl   *fish_destination_to_rgba;

  if (!conversion)
    return 0.0;

  if (conversion->error != -1.0)  /* double conversion against a set value should work */
    {
      return conversion->error;
    }

  fmt_source      = BABL (conversion->source);
  fmt_destination = BABL (conversion->destination);

  fish_rgba_to_source      = babl_fish_reference (fmt_rgba_double, fmt_source);
  fish_reference           = babl_fish_reference (fmt_source, fmt_destination);
  fish_destination_to_rgba = babl_fish_reference (fmt_destination, fmt_rgba_double);

  if (fmt_source == fmt_destination)
    {
      conversion->error = 0.0;
      return 0.0;
    }

  if (!(fmt_source->instance.id != BABL_RGBA &&
        fmt_destination->instance.id != BABL_RGBA &&
        fmt_source->instance.id != BABL_DOUBLE &&
        fmt_destination->instance.id != BABL_DOUBLE &&
        fmt_source->class_type == BABL_FORMAT &&
        fmt_destination->class_type == BABL_FORMAT))
    {
      conversion->error = 0.0000042;
    }

  source                      = babl_calloc (test_pixels, fmt_source->format.bytes_per_pixel);
  destination                 = babl_calloc (test_pixels, fmt_destination->format.bytes_per_pixel);
  ref_destination             = babl_calloc (test_pixels, fmt_destination->format.bytes_per_pixel);
  destination_rgba_double     = babl_calloc (test_pixels, fmt_rgba_double->format.bytes_per_pixel);
  ref_destination_rgba_double = babl_calloc (test_pixels, fmt_rgba_double->format.bytes_per_pixel);

  babl_process (fish_rgba_to_source,
                test, source, test_pixels);

  if (BABL(conversion)->class_type == BABL_CONVERSION_LINEAR)
  {
    ticks_start = babl_ticks ();
    babl_process (babl_fish_simple (conversion),
                  source, destination, test_pixels);
    ticks_end = babl_ticks ();
  }
  else
  {
    /* we could still measure it, but for the paths we only really consider
     * the linear ones anyways */
    ticks_end = 1000;
  }

  babl_process (fish_reference,
                source, ref_destination, test_pixels);

  babl_process (fish_destination_to_rgba,
                ref_destination, ref_destination_rgba_double, test_pixels);
  babl_process (fish_destination_to_rgba,
                destination, destination_rgba_double, test_pixels);

  error = babl_rel_avg_error (destination_rgba_double,
                              ref_destination_rgba_double,
                              test_pixels * 4);

  fish_rgba_to_source->fish.pixels      -= test_pixels;
  fish_reference->fish.pixels           -= test_pixels;
  fish_destination_to_rgba->fish.pixels -= 2 * test_pixels;

  babl_free (source);
  babl_free (destination);
  babl_free (destination_rgba_double);
  babl_free (ref_destination);
  babl_free (ref_destination_rgba_double);

  conversion->error = error;
  conversion->cost  = ticks_end - ticks_start;

  return error;
}

const Babl *
babl_conversion_get_source_space (const Babl *conversion)
{
  return conversion->conversion.source->format.space;
}

const Babl *
babl_conversion_get_destination_space (const Babl *conversion)
{
  return conversion->conversion.destination->format.space;
}



BABL_CLASS_IMPLEMENT (conversion)
