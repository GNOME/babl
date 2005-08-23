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

  assert (source->class_type == BABL_PIXEL_FORMAT ||
          source->class_type == BABL_MODEL);
  assert (destination->class_type == BABL_PIXEL_FORMAT ||
          destination->class_type == BABL_MODEL);

  babl                   = babl_calloc (sizeof (BablFishReference), 1);
  babl->class_type       = BABL_FISH_REFERENCE;
  babl->instance.id      = 0;
  babl->instance.name    = NULL;
  babl->fish.source      = (union Babl*)source;
  babl->fish.destination = (union Babl*)destination;
 
  if (source->class_type == BABL_PIXEL_FORMAT)
    {
      babl->reference_fish.type_to_double =
         babl_conversion_find (
            source->pixel_format.type[0],
            babl_type_id (BABL_DOUBLE)
         );

      babl->reference_fish.model_to_rgba =
        babl_conversion_find (
            source->pixel_format.model,
            babl_model_id (BABL_RGBA)
        );

      babl->reference_fish.rgba_to_model =
        babl_conversion_find (
            babl_model_id (BABL_RGBA),
            destination->pixel_format.model
        );

      babl->reference_fish.double_to_type =
        babl_conversion_find (
            babl_type_id (BABL_DOUBLE),
            destination->pixel_format.type[0]
        );
    }
  else if (source->class_type == BABL_MODEL)
    { 
      babl->reference_fish.type_to_double = NULL;


      babl->reference_fish.model_to_rgba =
        babl_conversion_find (
            source->pixel_format.model,
            babl_model_id (BABL_RGBA)
        );

      babl->reference_fish.rgba_to_model =
        babl_conversion_find (
            babl_model_id (BABL_RGBA),
            destination->pixel_format.model
        );

      babl->reference_fish.double_to_type =
        babl_conversion_find (
            babl_type_id (BABL_DOUBLE),
            destination->pixel_format.type[0]
        );
    }

  if (db_insert (babl) == babl)
    {
      return babl;
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
    
    babl_add_ptr_to_list ((void ***)&(source->type.from), babl);
    babl_add_ptr_to_list ((void ***)&(destination->type.to), babl);
  */
  return babl;
}

Babl *
babl_fish (Babl *source,
           Babl *destination)
{
  return babl_fish_reference_new (source, destination);
}

void *fooA;
void *fooB;
void *fooC;

#define BABL_MAX_BANDS   32

int
babl_fish_process (Babl *babl,
                   void *source,
                   void *destination,
                   int   n)
{
  Babl *imageA;
  Babl *imageB;
  Babl *imageC;

  /* FIXME: assumptions made about memory requirements that 
   * are not good
   */
  fooA = babl_malloc(sizeof (double) * n * 4);
  fooB = babl_malloc(sizeof (double) * n * 4);

  assert (babl);
  assert (source);
  assert (destination);
  assert (babl->class_type == BABL_FISH ||
          babl->class_type == BABL_FISH_REFERENCE);

  if (BABL_IS_BABL (source) ||
      BABL_IS_BABL (destination))
    {
      babl_log ("%s(%p, %p, %p, %i): not handling BablImage yet",
                __FUNCTION__, babl_fish, source, destination, n);
      return -1;
    }
 
  babl_conversion_process (babl->reference_fish.type_to_double,
                           source, fooA,
                           n * BABL(babl->fish.source)->pixel_format.bands);

  /* calculate planar representation of fooA, and fooB */

  imageA = babl_image_new_from_linear (fooA, BABL(BABL((babl->fish.source)) -> pixel_format.model));
  imageB = babl_image_new_from_linear (fooB, babl_model_id (BABL_RGBA));
  /* transform fooA into fooB fooB is rgba double */

  babl_conversion_process (babl->reference_fish.model_to_rgba,
                           imageA, imageB,
                           n);
  
  babl_free (imageA);
  babl_free (imageB);

  /* calculate planar representation of fooC */
  /* transform fooB into fooC fooC is ???? double */

  imageB = babl_image_new_from_linear (
              fooB, babl_model_id (BABL_RGBA));
  imageC = babl_image_new_from_linear (
              fooA, BABL(BABL((babl->fish.destination))->pixel_format.model));

  babl_conversion_process (babl->reference_fish.rgba_to_model,
                           imageB, imageC,
                           n);

  /* working directly on linear buffers */
  babl_conversion_process (babl->reference_fish.double_to_type,
                           fooA, destination,
                           n * BABL(babl->fish.destination)->pixel_format.bands);

  babl_free (imageB);
  babl_free (imageC);

  babl_free (fooA);
  babl_free (fooB);
  return 0;
}

BABL_DEFINE_INIT    (babl_fish)
BABL_DEFINE_DESTROY (babl_fish)
BABL_DEFINE_EACH    (babl_fish)
