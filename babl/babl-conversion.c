/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2005, Øyvind Kolås.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "babl-internal.h"
#include "babl-db.h"
#include <string.h>
#include <stdarg.h>

static int 
each_babl_conversion_destroy (Babl *babl,
                              void *data)
{
  babl_free (babl->instance.name);
  babl_free (babl);
  return 0;  /* continue iterating */
}

static Babl *
conversion_new (const char        *name,
                int                id,
                Babl              *source,
                Babl              *destination,
                int                time_cost,
                int                loss,
                BablFuncLinear     linear,
                BablFuncPlanar     planar,
                BablFuncPlanarBit  planar_bit)
{
  Babl *babl = NULL;

  /* destination is of same type as source */ 
  switch (source->class_type)
    {
      case BABL_TYPE:
        if (linear)
          {
            babl = babl_malloc (sizeof (BablConversionType));
            babl->class_type      = BABL_CONVERSION_TYPE;
            babl->conversion.function.linear = linear;
          }
        else if (planar)
          {
            babl = babl_malloc (sizeof (BablConversionTypePlanar));
            babl->class_type = BABL_CONVERSION_TYPE_PLANAR;
            babl->conversion.function.planar = planar;
          }
        else if (planar_bit)
          {
            babl_log ("planar_bit support not implemented yet");
          }
        break;
      case BABL_MODEL:
        if (linear)
          {
            babl_log ("linear support for model conversion not supported");
          }
        else if (planar)
          {
            babl = babl_malloc (sizeof (BablConversionModelPlanar));
            babl->class_type = BABL_CONVERSION_MODEL_PLANAR;
            babl->conversion.function.planar = planar;
          }
        else if (planar_bit)
          {
            babl_log ("planar_bit support for model conversion not supported");
          }
        break;
      case BABL_FORMAT:
        if (linear)
          {
            babl = babl_malloc (sizeof (BablConversionFormat));
            babl->class_type = BABL_CONVERSION_FORMAT;
            babl->conversion.function.linear = linear;
          }
        else if (planar)
          {
            babl = babl_malloc (sizeof (BablConversionFormatPlanar));
            babl->class_type = BABL_CONVERSION_FORMAT_PLANAR;
            babl->conversion.function.planar = planar;
          }
        else if (planar_bit)
          {
            babl_log ("planar_bit support for pixelformat conversion not supported");
          }
        break;
      default:
          babl_log ("%s unexpected", babl_class_name (babl->class_type));
        break;
    }
  if (!babl)
    {
      babl_log ("args=(name='%s', ...): creation failed",  name);
      return NULL;
    }

  babl->instance.id            = id;
  babl->instance.name          = babl_strdup (name);
  babl->conversion.source      = (union Babl*)source;
  babl->conversion.destination = (union Babl*)destination;
  babl->conversion.time_cost   = time_cost;
  babl->conversion.loss        = loss;

  babl->conversion.pixels      = 0;
  babl->conversion.processings = 0;

  babl_add_ptr_to_list ((void ***)&(source->type.from), babl);
  babl_add_ptr_to_list ((void ***)&(destination->type.to), babl);
  
  return babl;
}

static char buf[512]="";
static char *
create_name (Babl *source, Babl *destination)
{

  if (babl_extender ())
    {
      snprintf (buf, 512-1, "%s : %s to %s", BABL(babl_extender())->instance.name, source->instance.name, destination->instance.name);
      buf[511]='\0';
    }
  else
    {
      snprintf (buf, 512-1, "%s to %s", source->instance.name, destination->instance.name);
      buf[511]='\0';
    }
  return buf;
}

Babl *
babl_conversion_new (void *first_arg,
                     ...)
{
  va_list            varg;
  Babl              *babl;

  int                id          = 0;
  int                time_cost   = 0;
  int                loss        = 0;
  BablFuncLinear     linear      = NULL;
  BablFuncPlanar     planar      = NULL;
  BablFuncPlanarBit  planar_bit  = NULL;

  int                got_func    = 0;
  const char        *arg         = first_arg;

  Babl *source;
  Babl *destination;

  va_start (varg, first_arg);
  source = (Babl*) arg;
  destination = va_arg (varg, Babl*);
  arg = va_arg (varg, char *);

  assert (BABL_IS_BABL(source));
  assert (BABL_IS_BABL(destination));
  
  while (arg)
    {
     
      if (!strcmp (arg, "id"))
        {
          id = va_arg (varg, int);
        }
      
      else if (!strcmp (arg, "linear"))
        {
          if (got_func++)
            {
              babl_log ("already got a conversion func, registration of multiple might be possible later\n"); 
            }
          linear = va_arg (varg, BablFuncLinear);
        }

      else if (!strcmp (arg, "planar"))
        {
          if (got_func++)
            {
              babl_log ("already got a conversion func, registration of multiple might be possible later\n"); 
            }
          planar = va_arg (varg, BablFuncPlanar);
        }

      else if (!strcmp (arg, "planar-bit"))
        {
          if (got_func++)
            {
              babl_log ("already got a conversion func, registration of multiple might be possible later\n"); 
            }
          planar_bit = va_arg (varg, BablFuncPlanarBit);
        }

      else if (!strcmp (arg, "time-cost"))
        {
          time_cost = va_arg (varg, int);
        }
      else if (!strcmp (arg, "loss"))
        {
          loss = va_arg (varg, int);
        }
      else
        {
          babl_fatal ("unhandled argument '%s'", arg);
        }

      arg = va_arg (varg, char *);
    }
    
  va_end   (varg);

  assert (source);
  assert (destination);

  babl = conversion_new (create_name (source, destination), id, source, destination, time_cost, loss, linear,
                         planar, planar_bit);

  { 
    Babl *ret = db_insert (babl);
    if (ret!=babl)
        babl_free (babl);
    return ret;
  }
}

static long
babl_conversion_linear_process (BablConversion *conversion,
                                void           *source,
                                void           *destination,
                                int             src_pitch,
                                int             dst_pitch,
                                long            n)
{
  conversion->function.linear (source, destination, src_pitch, dst_pitch, n);
  return n;
}

static long
babl_conversion_planar_process (BablConversion *conversion,
                                BablImage      *source,
                                BablImage      *destination,
                                long            n)
{
#ifdef USE_ALLOCA
  void **src_data = alloca (sizeof (void*) * source->components);
  void **dst_data = alloca (sizeof (void*) * destination->components);
#else 
  void *src_data[BABL_MAX_COMPONENTS];
  void *dst_data[BABL_MAX_COMPONENTS];
#endif

  memcpy (src_data, source->data, sizeof (void*) * source->components);
  memcpy (dst_data, destination->data, sizeof (void*) * destination->components);
  
  conversion->function.planar (source->components,
                               src_data,
                               source->pitch,
                               destination->components,
                               dst_data,
                               destination->pitch,
                               n);
  return n;
}

long
babl_conversion_process (BablConversion *conversion,
                         void           *source,
                         void           *destination,
                         long            n)
{
  babl_assert (BABL_IS_BABL (conversion));

  switch (BABL(conversion)->class_type)
  {
    case BABL_CONVERSION_TYPE:
      {
        void *src_data = NULL;
        void *dst_data = NULL;
        int   src_pitch = 0;
        int   dst_pitch = 0;

        if (BABL_IS_BABL(source))
          {
            BablImage *img;
           
            img       = (BablImage*)source;
            src_data  = img->data[0];
            src_pitch = img->pitch[0];
          }
        if (!src_data)
          src_data=source;
        if (!src_pitch)
          src_pitch=BABL(conversion->source)->type.bits/8;


        if (BABL_IS_BABL(destination))
          {
            BablImage *img;
           
            img       = (BablImage*)destination;
            dst_data  = img->data[0];
            dst_pitch = img->pitch[0];
          }
        if (!dst_data)
          dst_data=destination;
        if (!dst_pitch)
          dst_pitch=BABL(conversion->destination)->type.bits/8;

        babl_conversion_linear_process (conversion,
                                        src_data,  dst_data,
                                        src_pitch, dst_pitch,
                                        n);
      }
      break;
    case BABL_CONVERSION_MODEL_PLANAR:
      babl_assert (BABL_IS_BABL (source));
      babl_assert (BABL_IS_BABL (destination));

      babl_conversion_planar_process (                  conversion,
                                      (BablImage*)      source, 
                                      (BablImage*)      destination,
                                                        n);
      break;
    default:
      babl_log ("args=(%s, %p, %p, %li) unhandled conversion type: %s",
           conversion->instance.name, source, destination, n,
          babl_class_name (conversion->instance.class_type));
      return 0;
      break;
  }
  return n;
}

BABL_CLASS_TEMPLATE (conversion)
