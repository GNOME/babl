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
      babl_log ("%s(%p, %p, %p, %li): not handling BablImage yet",
                __FUNCTION__, babl_fish, source, destination, n);
      return -1;
    }

#if 0  /* draft code*/
  {
    int i;
    BablFormat *source_fmt      = (BablFormat*)BABL(babl->fish.source);
    BablFormat *destination_fmt = (BablFormat*)BABL(babl->fish.destination);

    BablImage *src_img = babl_image ("R", pr, 1, 0, NULL);
    BablImage *dst_img = babl_image ("R", pr, 1, 0, NULL);

    for (i=0 ; i< destination_fmt->components; i++)
      {
        int j;

        dst_img->type[0]   = destination_fmt->type[i];
        dst_img->pitch[0]  = destination_fmt->pitch[i];
        dst_img->stride[0] = destination_fmt->stride[i];
        dst_img->data[0]   = destination_fmt->data[i];

        for (j=0;j<source_fmt->components;j++)
          {
            if (source_fmt->component[j] == destination_fmt[i])
              {
                src_img->type[0]   = source_fmt->type[j];
                src_img->pitch[0]  = source_fmt->pitch[j];
                src_img->stride[0] = source_fmt->stride[j];
                src_img->data[0]   = source_fmt->data[j];
                break;
              }
            babl_log ("%s(): matching source component not found", __FUNCTION);
          }

        babl_conversion_process (
           babl_conversion_find (
              src_img->type[0],
              dst_img->type[0]
              /*babl_type_id (BABL_DOUBLE)*/
           ),
           source, source_double_buf,
           n);
      }
  }
#endif
#if 1 
  babl_conversion_process (
     babl_conversion_find (
        BABL(babl->fish.source)->format.type[0],
        babl_type_id (BABL_DOUBLE)
     ),
     source, source_double_buf,
     n * BABL(babl->fish.source)->format.components);
#endif

  /* calculate planar representation of source_double, and rgba_double_buf */
  /* transform source_double_buf into rgba_double_buf rgba_double_buf is rgba double */

  babl_conversion_process (
    babl_conversion_find (
        BABL(babl->fish.source)->format.model,
        babl_model_id (BABL_RGBA)
    ),
    source_image, rgba_image,
    n);

  /* calculate planar representation of destination_double_buf */
  /* transform rgba_double_buf into destination_double_buf destination_double_buf is ???? double */

  babl_conversion_process (
    babl_conversion_find (
        babl_model_id (BABL_RGBA),
        BABL(babl->fish.destination)->format.model
    ),
    rgba_image, destination_image,
    n);

  /* FIXME: working directly on linear buffers */
  babl_conversion_process (
    babl_conversion_find (
        babl_type_id (BABL_DOUBLE),
        BABL(babl->fish.destination)->format.type[0]
    ),
    destination_double_buf, destination,
    n * BABL(babl->fish.destination)->format.components);

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
