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
#include "babl-internal.h"
#include <stddef.h>
#include <string.h>
#include <stdarg.h>


typedef struct _BablFindFish BablFindFish;

typedef struct _BablFindFish
{
  Babl       *fish_path;
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
each_babl_fish_destroy (Babl *babl,
                        void *data);


/* ====================================== */


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
      else if (item->instance.class_type == BABL_FISH_PATH)
        {
          ffish->fish_path = item;
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

BablDb *
babl_fish_db (void)
{
  if (!db)
    db = babl_db_init ();
  return db;
}

Babl *
babl_fish (const void *source,
           const void *destination)
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
        id_htable->find_func = find_memcpy_fish;
        babl_hash_table_find (id_htable, hashval, (void *) &ffish);
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
        id_htable->find_func = find_fish_path;
        babl_hash_table_find (id_htable, hashval, (void *) &ffish);

        if (ffish.fish_path)
          {
            /* we have found suitable fish path in the database */
            return ffish.fish_path;
          }
        if (!ffish.fish_fish)
          {
            /* we haven't tried to search for suitable path yet */
            Babl *fish_path = babl_fish_path (source_format, destination_format);

            if (fish_path)
              {
                return fish_path;
              }
            else
              {
                /* there isn't a suitable path for requested formats,
                 * let's create a dummy BABL_FISH instance and insert
                 * it into the fish database to indicate that such path
                 * does not exist.
                 */
                char *name = "X"; /* name does not matter */
                Babl *fish = babl_calloc (1, sizeof (BablFish) + strlen (name) + 1);

                fish->class_type                = BABL_FISH;
                fish->instance.id               = babl_fish_get_id (source_format, destination_format);
                fish->instance.name             = ((char *) fish) + sizeof (BablFish);
                strcpy (fish->instance.name, name);
                fish->fish.source               = source_format;
                fish->fish.destination          = destination_format;
                babl_db_insert (babl_fish_db (), fish);
              }
          }
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

long
babl_fish_process (Babl *babl,
                   void *source,
                   void *destination,
                   long  n)
{
  long ret = 0;

  switch (babl->class_type)
    {
      case BABL_FISH_REFERENCE:
        if (babl->fish.source == babl->fish.destination)
          { /* XXX: we're assuming linear buffers */
            memcpy (destination, source, n * babl->fish.source->format.bytes_per_pixel);
            ret = n;
          }
        else
          {
            ret = babl_fish_reference_process (babl, source, destination, n);
          }
        break;

      case BABL_FISH_SIMPLE:
        if (BABL (babl->fish_simple.conversion)->class_type == BABL_CONVERSION_LINEAR)
          {
            ret = babl_conversion_process (BABL (babl->fish_simple.conversion),
                                           source, destination, n);
          }
        else
          {
            babl_fatal ("Cannot use a simple fish to process without a linear conversion");
          }
        break;

      case BABL_FISH_PATH:
        ret = babl_fish_path_process (babl, source, destination, n);
        break;

      default:
        babl_log ("NYI");
        ret = -1;
        break;
    }

  return ret;
}



static int
each_babl_fish_destroy (Babl *babl,
                        void *data)
{
  if (babl->class_type == BABL_FISH_PATH)
    babl_list_destroy (babl->fish_path.conversion_list);
  babl_free (babl);
  return 0;  /* continue iterating */
}

BABL_CLASS_MINIMAL_IMPLEMENT (fish);
