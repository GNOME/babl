/* babl - dynamically extendable universal pixel fish library.
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

/*#include "babl-type.h"
#include "babl-model.h"
#include "babl-image.h"
#include "babl-component.h"
#include "babl-format.h"*/

static int 
each_babl_fish_destroy (Babl *babl,
                        void *data)
{
  babl_free (babl);
  return 0;  /* continue iterating */
}

static char buf[1024];
static char *
create_name (Babl *source,
             Babl *destination,
             int   is_reference)
{
  /* fish names are intentionally kept short */
  snprintf (buf, 1024, "%s %p %p",
                 is_reference?"ref "
                             :"",
                 source, destination);
  return buf;
}

typedef struct SearchData
{
  Babl           *source;
  Babl           *destination;
  BablConversion *result;
} SearchData;

Babl *babl_conversion_find2 (void *source,
                            void *destination)
{
  int i=0;
  Babl **conversion;

  conversion = (void*)BABL(source)->type.from;
  while (conversion && conversion[i])
    {
      if (conversion[i]->conversion.destination == destination)
        return (Babl*)conversion[i];
      i++;
    }
  babl_fatal ("failed, aborting");
  return NULL;
}


Babl *babl_conversion_find (void *source,
                             void *destination)
{
  int i=0;
  Babl **conversion;

  conversion = (void*)BABL(source)->type.from;
  while (conversion && conversion[i])
    {
      if (conversion[i]->conversion.destination == destination)
        return (Babl*)conversion[i];
      i++;
    }
  return NULL;
}



Babl *
babl_fish_reference (Babl *source,
                     Babl *destination)
{
  Babl *babl = NULL;
  char *name = create_name (source, destination, 1);

  babl_assert (BABL_IS_BABL (source));
  babl_assert (BABL_IS_BABL (destination));

  babl_assert (source->class_type == BABL_FORMAT);
  babl_assert (destination->class_type == BABL_FORMAT);

  babl                   = babl_malloc (sizeof (BablFishReference) +
                                        strlen (name) + 1);
  babl->class_type       = BABL_FISH_REFERENCE;
  babl->instance.id      = 0;
  babl->instance.name    = ((void *)babl) + sizeof(BablFishReference);
  strcpy (babl->instance.name, name);
  babl->fish.source      = (union Babl*)source;
  babl->fish.destination = (union Babl*)destination;

  babl->fish.processings = 0;
  babl->fish.pixels      = 0;

  { 
    Babl *ret = babl_db_insert (db, babl);
    if (ret!=babl)
        babl_free (babl);
    return ret;
  }
}

Babl *
babl_fish_simple (BablConversion *conversion)
{
  Babl *babl = NULL;
  char *name;

  babl_assert (BABL_IS_BABL (conversion));

  name  = create_name (BABL(conversion->source),
                       BABL(conversion->destination),
                       0);

  babl                   = babl_malloc (sizeof (BablFishSimple) +
                                        strlen (name) + 1);
  babl->class_type       = BABL_FISH_SIMPLE;
  babl->instance.id      = 0;
  babl->instance.name    = ((void *)babl) + sizeof(BablFishSimple);
  strcpy (babl->instance.name, name);
  babl->fish.source      = (union Babl*)conversion->source;
  babl->fish.destination = (union Babl*)conversion->destination;

  babl->fish.processings = 0;
  babl->fish.pixels      = 0;
  babl->fish_simple.conversion = conversion;

  { 
    Babl *ret = babl_db_insert (db, babl);
    if (ret!=babl)
        babl_free (babl);
    return ret;
  }
}

Babl *
babl_fish (void *source,
           void *destination)
{
  Babl *source_format      = NULL;
  Babl *destination_format = NULL;

  babl_assert (source);
  babl_assert (destination);

  if (BABL_IS_BABL (source))
    {
      source_format = source;
    }

  if (!source_format)
    {
      source_format = babl_format ((char*)source);
    }

  if (!source_format)
    {
      babl_log ("args=(%p, %p) source format invalid", source, destination);
      return NULL;
    }

  if (BABL_IS_BABL (destination))
    {
      destination_format = destination;
    }

  if (!destination_format)
    {
      destination_format = babl_format ((char*)destination);
    }

  if (!destination_format)
    {
      babl_log ("args=(%p, %p) destination format invalid", source, destination);
      return NULL;
    }
 
 
  {
    Babl *shortcut_conversion = babl_conversion_find (source_format, destination_format);

    if (shortcut_conversion)
      {
        return babl_fish_simple (&(shortcut_conversion->conversion));
      }
  }
  return babl_fish_reference (source_format, destination_format);
}


static void
convert_to_double (BablFormat *source_fmt,
                   BablImage  *source,
                   void       *source_buf,
                   void       *source_double_buf,
                   int         n)
{
  int i;

  BablImage *src_img;
  BablImage *dst_img;

  src_img = (BablImage*) babl_image (
      babl_component_id (BABL_LUMINANCE), NULL, 1, 0, NULL);
  dst_img = (BablImage*) babl_image (
      babl_component_id (BABL_LUMINANCE), NULL, 1, 0, NULL);

  dst_img->type[0]   = (BablType*) babl_type_id (BABL_DOUBLE);
  dst_img->pitch[0]  =
             (dst_img->type[0]->bits/8) * source_fmt->model->components;
  dst_img->stride[0] = 0;


  src_img->data[0]   = source_buf;
  src_img->type[0]   = (BablType*) babl_type_id (BABL_DOUBLE);
  src_img->pitch[0]  = source_fmt->bytes_per_pixel;
  src_img->stride[0] = 0;

  /* i is source position */
  for (i=0 ; i<source_fmt->components; i++)
    {
      int j;

      src_img->type[0] = source_fmt->type[i];
      /* j is source position */
      for (j=0;j<source_fmt->model->components;j++)
        {
          if (source_fmt->component[i] ==
              source_fmt->model->component[j])
            {
              dst_img->data[0] =
                       source_double_buf + (dst_img->type[0]->bits/8) * j;
              break;
            }
        }

      babl_process (
           babl_conversion_find2 (src_img->type[0], dst_img->type[0]),
           src_img, dst_img,
           n);
      src_img->data[0] += src_img->type[0]->bits/8;
    }
  babl_free (src_img);
  babl_free (dst_img);
}


static void
convert_from_double (BablFormat *destination_fmt,
                     void       *destination_double_buf,
                     BablImage  *destination,
                     void       *destination_buf,
                     int         n)
{
  int i;

  BablImage *src_img;
  BablImage *dst_img;

  src_img = (BablImage*) babl_image (
      babl_component_id (BABL_LUMINANCE), NULL, 1, 0, NULL);
  dst_img = (BablImage*) babl_image (
      babl_component_id (BABL_LUMINANCE), NULL, 1, 0, NULL);

  src_img->type[0]   = (BablType*) babl_type_id (BABL_DOUBLE);
  src_img->pitch[0]  =
            (src_img->type[0]->bits/8) * destination_fmt->model->components;
  src_img->stride[0] = 0;

  dst_img->data[0]   = destination_buf;
  dst_img->type[0]   = (BablType*) babl_type_id (BABL_DOUBLE);
  dst_img->pitch[0]  = destination_fmt->bytes_per_pixel;
  dst_img->stride    = 0;

  for (i=0 ; i<destination_fmt->components; i++)
    {
      int j;

      dst_img->type[0] = destination_fmt->type[i];

      for (j=0;j<destination_fmt->model->components;j++)
        {
          if (destination_fmt->component[i] ==
              destination_fmt->model->component[j])
            {
              src_img->data[0] =
                  destination_double_buf + (src_img->type[0]->bits/8) * j;
              break;
            }
        }

      babl_process (
           babl_conversion_find2 (src_img->type[0], dst_img->type[0]),
           src_img, dst_img,
           n);
      dst_img->data[0] += dst_img->type[0]->bits/8;
    }
  babl_free (src_img);
  babl_free (dst_img);
}


static int
process_same_model (Babl      *babl,
                    BablImage *source,
                    BablImage *destination,
                    long n)
{
  void *double_buf;

  if (BABL_IS_BABL (source) ||
      BABL_IS_BABL (destination))
    {
      babl_log ("args=(%p, %p, %p, %li): trying to handle BablImage (unconfirmed code)",
                 babl_fish, source, destination, n);
    }

  double_buf      = babl_malloc(sizeof (double) * n * 
                              BABL(babl->fish.source)->format.model->components);

  convert_to_double (
     (BablFormat*) BABL(babl->fish.source),
     BABL_IS_BABL(source)?source:NULL,
     BABL_IS_BABL(source)?NULL:source,
     double_buf,
     n
   );

  convert_from_double (
     (BablFormat*) BABL(babl->fish.destination),
     double_buf,
     BABL_IS_BABL(destination)?destination:NULL,
     BABL_IS_BABL(destination)?NULL:destination,
     n
   );

  babl_free (double_buf);
  return 0;
}

int
babl_fish_reference_process (Babl      *babl,
                             BablImage *source,
                             BablImage *destination,
                             long n)
{
  void *source_double_buf;
  void *rgba_double_buf;
  void *destination_double_buf;
  Babl *source_image;
  Babl *rgba_image;
  Babl *destination_image;

  if (BABL(babl->fish.source)->format.model ==
      BABL(babl->fish.destination)->format.model)
        return process_same_model (babl, source, destination, n); 
  
  if (BABL_IS_BABL (source) ||
      BABL_IS_BABL (destination))
    {
      babl_log ("args=(%p, %p, %p, %li): trying to handle BablImage (unconfirmed code)",
                 babl_fish, source, destination, n);
    }

  source_double_buf      = babl_malloc(sizeof (double) * n * 
                              BABL(babl->fish.source)->format.model->components);
  rgba_double_buf        = babl_malloc(sizeof (double) * n * 4);
  destination_double_buf = babl_malloc(sizeof (double) * n *
                              BABL(babl->fish.destination)->format.model->components);

  source_image      = babl_image_from_linear (
      source_double_buf,BABL(BABL((babl->fish.source)) -> format.model));
  rgba_image        = babl_image_from_linear (
      rgba_double_buf, babl_model_id (BABL_RGBA));
  destination_image = babl_image_from_linear (
      destination_double_buf, BABL(BABL((babl->fish.destination))->format.model));

  convert_to_double (
     (BablFormat*) BABL(babl->fish.source),
     BABL_IS_BABL(source)?source:NULL,
     BABL_IS_BABL(source)?NULL:source,
     source_double_buf,
     n
   );

  babl_process (
    babl_conversion_find2 (
        BABL(babl->fish.source)->format.model,
        babl_model_id (BABL_RGBA)
    ),
    source_image, rgba_image,
    n);

  babl_process (
    babl_conversion_find2 (
        babl_model_id (BABL_RGBA),
        BABL(babl->fish.destination)->format.model
    ),
    rgba_image, destination_image,
    n);

  convert_from_double (
     (BablFormat*) BABL(babl->fish.destination),
     destination_double_buf,
     BABL_IS_BABL(destination)?destination:NULL,
     BABL_IS_BABL(destination)?NULL:destination,
     n
   );

  babl_free (source_image);
  babl_free (rgba_image);
  babl_free (destination_image);

  babl_free (destination_double_buf);
  babl_free (rgba_double_buf);
  babl_free (source_double_buf);
  return 0;
}

int
babl_fish_process (Babl *babl,
                   void *source,
                   void *destination,
                   long  n)
{
  babl_log ("NYI");
  return -1;
}



BABL_DEFINE_INIT    (fish)
BABL_DEFINE_DESTROY (fish)
BABL_DEFINE_EACH    (fish)
