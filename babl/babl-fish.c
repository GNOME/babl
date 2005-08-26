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
#include "assert.h"
#include <string.h>
#include <stdarg.h>

#include "babl-type.h"
#include "babl-model.h"
#include "babl-image.h"
#include "babl-component.h"
#include "babl-pixel-format.h"

static int 
each_babl_fish_destroy (Babl *babl,
                        void *data)
{
  babl_free (babl);
  return 0;  /* continue iterating */
}

BablFish *
babl_fish_new (Babl *source,
               Babl *destination)
{
  Babl *babl = NULL;

  assert (BABL_IS_BABL (source));
  assert (BABL_IS_BABL (destination));

  babl                   = babl_calloc (sizeof (BablFish), 1);
  babl->class_type       = BABL_FISH;
  babl->instance.id      = 0;
  babl->instance.name    = "Fishy";
  babl->fish.source      = (union Babl*)source;
  babl->fish.destination = (union Babl*)destination;

  if (db_insert (babl) == babl)
    {
      return (BablFish*)babl;
    }
  else
    {
      each_babl_fish_destroy (babl, NULL);
      return NULL;
    }

/*  Might make sense to allow a precalculated shortcut to
 *  participate in later checks for optimal conversions, then we
 *  should also have better generated names,.   model + datatype 
 *  is a possibility , or even full single line serialization of
 *  components with types.
 *
    babl_add_ptr_to_list ((void ***)&(source->type.from), babl);
    babl_add_ptr_to_list ((void ***)&(destination->type.to), babl);
  */
  return (BablFish*)babl;
}

typedef struct SearchData
{
  Babl           *source;
  Babl           *destination;
  BablConversion *result;
} SearchData;

static int
find_conversion (Babl *babl,
                 void *user_data)
{
  SearchData *sd     = user_data;

  if (BABL(babl->conversion.source)      == sd->source &&
      BABL(babl->conversion.destination) == sd->destination)
    {
      sd->result = (BablConversion*)babl;
      return 1;
    }
  return 0;
}

BablConversion *babl_conversion_find (void *source,
                                      void *destination)
{
  SearchData data;
  data.source      = BABL(source);
  data.destination = BABL(destination);
  data.result      = NULL;
  babl_conversion_each (find_conversion, &data);

  if (!data.result)
    {
      babl_log ("%s('%s', '%s'): failed, aborting", __FUNCTION__,
        data.source->instance.name, data.destination->instance.name);
      exit (-1);
      return NULL;
    }
  return data.result;
}

Babl *
babl_fish_reference_new (Babl *source,
                         Babl *destination)
{
  Babl *babl = NULL;

  assert (BABL_IS_BABL (source));
  assert (BABL_IS_BABL (destination));

  assert (source->class_type == BABL_FORMAT);
  assert (destination->class_type == BABL_FORMAT);

  babl                   = babl_calloc (sizeof (BablFishReference), 1);
  babl->class_type       = BABL_FISH_REFERENCE;
  babl->instance.id      = 0;
  babl->instance.name    = NULL;
  babl->fish.source      = (union Babl*)source;
  babl->fish.destination = (union Babl*)destination;
 
  if (db_insert (babl) == babl)
    {
      return babl;
    }
  else
    {
      each_babl_fish_destroy (babl, NULL);
      return NULL;
    }

  return babl;
}

Babl *
babl_fish (void *source,
           void *destination)
{
  Babl *source_format = NULL;
  Babl *destination_format = NULL;

  assert (source);
  assert (destination);

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
      babl_log ("%s(%p, %p) source format invalid", 
         __FUNCTION__, source, destination);
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
      babl_log ("%s(%p, %p) destination format invalid",
         __FUNCTION__, source, destination);
    }
  
  return babl_fish_reference_new (source_format, destination_format);
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

      babl_conversion_process (
           babl_conversion_find (src_img->type[0], dst_img->type[0]),
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

      babl_conversion_process (
           babl_conversion_find (src_img->type[0], dst_img->type[0]),
           src_img, dst_img,
           n);
      dst_img->data[0] += dst_img->type[0]->bits/8;
    }
  babl_free (src_img);
  babl_free (dst_img);
}

static int
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

  /* FIXME: assumptions made about memory requirements that 
   * are not good
   */
  source_double_buf      = babl_malloc(sizeof (double) * n * 4);
  rgba_double_buf        = babl_malloc(sizeof (double) * n * 4);
  destination_double_buf = babl_malloc(sizeof (double) * n * 4);
  
  source_image      = babl_image_from_linear (
                         source_double_buf,
                         BABL(BABL((babl->fish.source)) -> format.model));
  rgba_image        = babl_image_from_linear (
                         rgba_double_buf,
                         babl_model_id (BABL_RGBA));
  destination_image = babl_image_from_linear (
                         destination_double_buf,
                         BABL(BABL((babl->fish.destination))->format.model));

  if (BABL_IS_BABL (source) ||
      BABL_IS_BABL (destination))
    {
      babl_log ("%s(%p, %p, %p, %li): trying to handle BablImage (unconfirmed code)",
                __FUNCTION__, babl_fish, source, destination, n);
    }

  convert_to_double (
     (BablFormat*) BABL(babl->fish.source),
     BABL_IS_BABL(source)?source:NULL,
     BABL_IS_BABL(source)?NULL:source,
     source_double_buf,
     n
   );

  babl_conversion_process (
    babl_conversion_find (
        BABL(babl->fish.source)->format.model,
        babl_model_id (BABL_RGBA)
    ),
    source_image, rgba_image,
    n);

  babl_conversion_process (
    babl_conversion_find (
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

static int
babl_fish_process (Babl *babl,
                   void *source,
                   void *destination,
                   long  n)
{
  babl_log ("%s(): NYI", __FUNCTION__);
  return -1;
}


int
babl_process (Babl *babl,
              void *source,
              void *destination,
              long  n)
{
  assert (babl);
  assert (source);
  assert (destination);
  assert (BABL_IS_BABL (babl));
  assert (n>0);

  if (babl->class_type == BABL_FISH)
    return babl_fish_process (babl, source, destination, n);
  
  if (babl->class_type == BABL_FISH_REFERENCE)
    {
       BablImage *source_image      = NULL;
       BablImage *destination_image = NULL;

       if (BABL_IS_BABL (source))
         source_image = source;
       if (!source_image)
         source_image = (BablImage*) babl_image_from_linear (
                                        source, (Babl*)babl->fish.source);
       if (BABL_IS_BABL (destination))
         destination_image = destination;
       if (!destination_image)
         destination_image = (BablImage*) babl_image_from_linear (
                        destination, (Babl*)babl->fish.destination);

       babl_fish_reference_process (babl, source, destination, n);

       babl_free (source_image);
       babl_free (destination_image);

       return 0;
    }

  babl_log ("%s(): eek", __FUNCTION__);
  return -1;
}

BABL_DEFINE_INIT    (babl_fish)
BABL_DEFINE_DESTROY (babl_fish)
BABL_DEFINE_EACH    (babl_fish)
