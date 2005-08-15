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

  if ((Babl*)babl->conversion.source      == sd->source &&
      (Babl*)babl->conversion.destination == sd->destination)
    {
      sd->result = (BablConversion*)babl;
      return 1;
    }
  return 0;
}

BablConversion *babl_conversion_find (Babl *source,
                                      Babl *destination)
{
  SearchData data;
  data.source       = source;
  data.destination = destination;
  data.result = NULL;
  babl_conversion_each (find_conversion, &data);

  if (!data.result)
    {
      babl_log ("%s('%s', '%s'): failed", __FUNCTION__,
        source->instance.name, destination->instance.name);
      return NULL;
    }
  return data.result;
}

BablFish *
babl_fish_reference_new (Babl *source,
                         Babl *destination)
{
  Babl *babl = NULL;

  assert (BABL_IS_BABL (source));
  assert (BABL_IS_BABL (destination));

  babl                   = babl_calloc (sizeof (BablFishReference), 1);
  babl->class_type       = BABL_FISH_REFERENCE;
  babl->instance.id      = 0;
  babl->instance.name    = "Fishy";
  babl->fish.source      = (union Babl*)source;
  babl->fish.destination = (union Babl*)destination;

  babl->reference_fish.type_to_double =
     babl_conversion_find (
        (Babl*)source->pixel_format.type[0],
        (Babl*)babl_type_id (BABL_DOUBLE)
     );

  babl->reference_fish.model_to_rgba =
    babl_conversion_find (
        (Babl*)source->pixel_format.model[0],
        (Babl*)babl_model_id (BABL_RGBA)
    );

  babl->reference_fish.rgba_to_model =
    babl_conversion_find (
        (Babl*)babl_model_id (BABL_RGBA),
        (Babl*)destination->pixel_format.model[0]
    );

  babl->reference_fish.double_to_type =
    babl_conversion_find (
        (Babl*)babl_type_id (BABL_DOUBLE),
        (Babl*)destination->pixel_format.type[0]
    );

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
    
    babl_add_ptr_to_list ((void ***)&(source->type.from), babl);
    babl_add_ptr_to_list ((void ***)&(destination->type.to), babl);
  */
  return (BablFish*) babl;
}

BablFish *
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
babl_fish_process (BablFish *babl_fish,
                   void     *source,
                   void     *destination,
                   int       n)
{
  Babl *babl;
  BablImage *imageA;
  BablImage *imageB;
  BablImage *imageC;

  fooA = babl_malloc(sizeof (double) * n * 4); 
  fooB = babl_malloc(sizeof (double) * n * 4); 

  assert (source);
  assert (destination);

  babl = (Babl *)babl_fish;
  if (BABL_IS_BABL (source) ||
      BABL_IS_BABL (destination))
    {
      babl_log ("%s(%p, %p, %p, %i): not handling BablImage yet",
                __FUNCTION__, babl_fish, source, destination, n);
      return -1;
    }
  
  ((BablConversion*)(babl->reference_fish.type_to_double))->function.linear(
          source,
          fooA,
          n*  ((BablPixelFormat*)(babl_fish->source))->bands
          );

  /* calculate planar representation of fooA, and fooB */

  imageA = babl_image_new_from_linear (fooA,
      (Babl*) ((BablPixelFormat*) babl->fish.source)->model[0]);
  imageB = babl_image_new_from_linear (fooB, (Babl*)babl_model_id (BABL_RGBA));
  /* transform fooA into fooB fooB is rgba double */

  ((BablConversion*)(babl->reference_fish.model_to_rgba))->function.planar(
          imageA->bands, 
          imageA->data,
          imageA->pitch,
          imageB->bands, 
          imageB->data,
          imageB->pitch,
          n);
  babl_free (imageA);
  babl_free (imageB);

  /* calculate planar representation of fooC */
  /* transform fooB into fooC fooC is ???? double */

  imageB = babl_image_new_from_linear (fooB, (Babl*)babl_model_id (BABL_RGBA));
  imageC = babl_image_new_from_linear (fooA, (Babl*)((BablPixelFormat*)babl->fish.destination)->model[0]);

  ((BablConversion*)(babl->reference_fish.rgba_to_model))->function.planar(
          imageB->bands, 
          imageB->data,
          imageB->pitch,
          imageC->bands, 
          imageC->data,
          imageC->pitch,
          n);


  ((BablConversion*)(babl->reference_fish.double_to_type))->function.linear(
          fooA, destination, n * ((BablPixelFormat*)(babl_fish->destination))->bands
          );

  babl_free (imageB);
  babl_free (imageC);

  babl_free (fooA);
  babl_free (fooB);
  return 0;
}

/*BABL_CLASS_TEMPLATE(BablFish, babl_fish, "BablFish")*/
BABL_DEFINE_INIT(babl_fish)
BABL_DEFINE_DESTROY(babl_fish)
BABL_DEFINE_EACH(babl_fish)
