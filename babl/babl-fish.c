/* babl - dynamically extendable universal pixel fish library.
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
 * <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#define NEEDS_BABL_DB
#include "babl-internal.h"
#include "babl-db.h"
#include <stddef.h>
#include <string.h>
#include <stdarg.h>

#define BABL_LEGAL_ERROR           0.000001
static double legal_error (void)
{
  static double error = 0.0;
  const char   *env;

  if (error != 0.0)
    return error;

  env = getenv ("BABL_TOLERANCE");
  if (env && env[0] != '\0')
    error = atof (env);
  else
    error = BABL_LEGAL_ERROR;
  return error;
}

typedef struct _BablFindFish BablFindFish;

typedef struct _BablFindFish
{
  Babl       *fish_path_list;
  Babl       *fish_ref;
  Babl       *fish_fish;
  int        fishes;
  const Babl *source;
  const Babl *destination;
} _BablFishFish;


static int
match_conversion (Babl *conversion,
                  void *inout);

static int
find_fish_path (Babl *item,
                void *data);

static int
find_memcpy_fish (Babl *item,
                  void *data);

static int
find_fish_path (Babl *item,
                void *data)
{
  BablFindFish *ffish = (BablFindFish *) data;
  if ((item->fish.source == ffish->source) &&
      (item->fish.destination == ffish->destination))
    {
      if (item->instance.class_type == BABL_FISH_REFERENCE)
        {
          ffish->fish_ref = item;
          ffish->fishes++;
        }
      else if (item->instance.class_type == BABL_FISH_PATH_LIST)
        {
          ffish->fish_path_list = item;
          ffish->fishes++;
        }
      else if (item->instance.class_type == BABL_FISH)
        {
          ffish->fish_fish = item;
          ffish->fishes++;
        }
      if (ffish->fishes == 3)
        return 1;
    }

  return 0;
}

static int
find_memcpy_fish (Babl *item,
                  void *data)
{
  BablFindFish *ffish = (BablFindFish *) data;
  if ((item->fish.source == ffish->source) &&
      (item->fish.destination == ffish->destination))
    {
      if ((item->fish.source == item->fish.destination) &&
          (item->instance.class_type == BABL_FISH_REFERENCE))
        {
          ffish->fish_ref = item;
          return 1;
        }
    }

  return 0;
}

static int
match_conversion (Babl *conversion,
                  void *inout)
{
  void **data = inout;

  if ((Babl *) conversion->conversion.destination == (Babl *) *data)
    {
      *data = (void *) conversion;
      return 1;
    }
  return 0;
}

Babl *
babl_conversion_find (const void *source,
                      const void *destination)
{
  void *data = (void*)destination;

  babl_list_each (BABL (source)->type.from_list, match_conversion, &data);
  if (data == (void*)destination) /* didn't change */
    return NULL;
  return data;
}

int
babl_fish_get_id (const Babl *source,
                  const Babl *destination)
{
  /* value of 'id' will be used as argument for hash function,
   * substraction serves as simple combination of
   * source/destination values. */
  ptrdiff_t id = source - destination;
  /* instances with id 0 won't be inserted into database */
  if (id == 0)
    id = 1;
  return id;
}

const Babl *
babl_fish (const void *source,
           const void *destination)
{
  return babl_fish_with_tolerance (source, destination, legal_error());
}

const Babl *
babl_fish_with_tolerance (const void *source,
                          const void *destination,
                          double tolerance)
{
  const Babl *source_format      = NULL;
  const Babl *destination_format = NULL;

  babl_assert (source);
  babl_assert (destination);

  if (BABL_IS_BABL (source))
    source_format = source;

  if (!source_format)
    source_format = babl_format ((char *) source);

  if (!source_format)
    {
      babl_log ("args=(%p, %p) source format invalid", source, destination);
      return NULL;
    }

  if (BABL_IS_BABL (destination))
    destination_format = destination;

  if (!destination_format)
    destination_format = babl_format ((char *) destination);

  if (!destination_format)
    {
      babl_log ("args=(%p, %p) destination format invalid", source, destination);
      return NULL;
    }

  {
    int            hashval;
    BablHashTable *id_htable;
    BablFindFish   ffish = {(Babl *) NULL,
                            (Babl *) NULL,
                            (Babl *) NULL,
                            0,
                            (Babl *) NULL,
                            (Babl *) NULL};

    /* some vendor compilers can't compile non-constant elements of
     * compound struct initializers
     */
    ffish.source = source_format;
    ffish.destination = destination_format;

    id_htable = (babl_fish_db ())->id_hash;
    hashval = babl_hash_by_int (id_htable, babl_fish_get_id (source_format, destination_format));

    if (source_format == destination_format)
      {
        /* In the case of equal source and destination formats
         * we will search through the fish database for reference fish
         * to handle the memcpy */
        babl_hash_table_find (id_htable, hashval, find_memcpy_fish, (void *) &ffish);
      }
    else
      {
        /* In the case of different source and destination formats
         * we will search through the fish database for appropriate fish path
         * to handle the conversion. In the case that preexistent
         * fish path is found, we'll return it. In the case BABL_FISH
         * instance with the same source/destination is found, we'll
         * return reference fish.
         * In the case neither fish path nor BABL_FISH path are found,
         * we'll try to construct new fish path for requested
         * source/destination. In the case new fish path is found, we'll
         * return it, otherwise we'll create dummy BABL_FISH instance and
         * insert it into the fish database to indicate non-existent fish
         * path.
         */
        Babl *conversion_list = NULL;
        Babl *fish_path = NULL;
        int i;
        
        babl_hash_table_find (id_htable, hashval, find_fish_path, (void *) &ffish);
        
        if (ffish.fish_path_list)
          {
            /* we have found suitable fish path in the database */
            conversion_list = ffish.fish_path_list;
          }
        else
          {
            /* we haven't tried to search for suitable path yet */
            conversion_list = babl_fish_path (source_format, destination_format);
          }

        /* find a conversion with an acceptable tolerance */
        for (i = 0; i < babl_list_size(conversion_list->fish_path_list.path_list); ++i) {
          Babl *candidate_path = babl_list_get_n(conversion_list->fish_path_list.path_list, i);
          if (candidate_path->fish.error <= tolerance)
            fish_path = candidate_path;
          else
            break;
        }
        
        if (fish_path)
          return fish_path;
      }

    if (ffish.fish_ref)
      {
        /* we have already found suitable reference fish */
        return ffish.fish_ref;
      }
    else
      {
        /* we have to create new reference fish */
        return babl_fish_reference (source_format, destination_format);
      }
  }
}

BABL_CLASS_MINIMAL_IMPLEMENT (fish);
