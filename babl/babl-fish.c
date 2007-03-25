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
#include <string.h>
#include <stdarg.h>


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
babl_conversion_find (void *source,
                      void *destination)
{
  void *data = destination;

  babl_list_each ((void *) BABL (source)->type.from, match_conversion, &data);
  if (data == destination)
    return NULL;
  return data;
}

BablDb *
babl_fish_db (void)
{
  if (!db)
    db = babl_db_init ();
  return db;
}

typedef struct BablFishingData
{
  Babl *source;
  Babl *destination;
  Babl *ret;
} BablFishingData;

static int
fishing_result_examine (Babl *babl,
                        void *void_data)
{
  BablFishingData *data = void_data;

  if ((void *) data->source == (void *) babl->fish.source &&
      (void *) data->destination == (void *) babl->fish.destination)
    {
      data->ret = babl;
      return 1;     /* stop iterating */
    }
  return 0;  /* continue iterating */
}

static Babl *
go_fishing (Babl *source,
            Babl *destination)
{
  {
    BablFishingData data;

    data.source      = source;
    data.destination = destination;
    data.ret         = NULL;

    babl_db_each (db, fishing_result_examine, &data);
    return data.ret;
  }
}

Babl *
babl_fish (void *source,
           void *destination,
           ...)
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
      source_format = babl_format ((char *) source);
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
      destination_format = babl_format ((char *) destination);
    }

  if (!destination_format)
    {
      babl_log ("args=(%p, %p) destination format invalid", source, destination);
      return NULL;
    }

  {
    Babl *lucky;
    lucky = go_fishing (source_format, destination_format);
    if (lucky)
      return lucky;
  }

  if (0)
    {
      Babl *shortcut_conversion;

      shortcut_conversion = babl_conversion_find (
        source_format, destination_format);

      if (shortcut_conversion)
        {
          return babl_fish_simple (&(shortcut_conversion->conversion));
        }
    }

  {
    Babl *fish_path;

    fish_path = babl_fish_path (source_format, destination_format);

    if (fish_path)
      {
        return fish_path;
      }
  }

  return babl_fish_reference (source_format, destination_format);
}

long
babl_fish_process (Babl *babl,
                   void *source,
                   void *destination,
                   long  n)
{
  BablImage *source_image      = NULL;
  BablImage *destination_image = NULL;
  long       ret               = 0;

  switch (babl->class_type)
    {
      case BABL_FISH_REFERENCE:
      case BABL_FISH_SIMPLE:
      case BABL_FISH_PATH:

#if 0
        if (BABL_IS_BABL (source))
          source_image = source;
#endif
        if (!source_image)
          source_image = (BablImage *) babl_image_from_linear (
            source, (Babl *) babl->fish.source);
#if 0
        if (BABL_IS_BABL (destination))
          destination_image = destination;
#endif
        if (!destination_image)
          destination_image = (BablImage *) babl_image_from_linear (
            destination, (Babl *) babl->fish.destination);

        if (babl->class_type == BABL_FISH_REFERENCE)
          {
            ret = babl_fish_reference_process (babl, source, destination, n);
          }
        else if (babl->class_type == BABL_FISH_PATH)
          {
            ret = babl_fish_path_process (babl, source, destination, n);
          }
        else if (babl->class_type == BABL_FISH_SIMPLE)
          {
            if (BABL (babl->fish_simple.conversion)->class_type == BABL_CONVERSION_LINEAR)
              {
                ret = babl_conversion_process (BABL (babl->fish_simple.conversion),
                                               source, destination, n);
              }
            else
              {
                ret = babl_conversion_process (BABL (babl->fish_simple.conversion),
                                               (char *) source_image, (char *) destination_image, n);
              }
          }

        babl_free (source_image);
        babl_free (destination_image);
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
  babl_free (babl);
  return 0;  /* continue iterating */
}

BABL_DEFINE_INIT (fish)
BABL_DEFINE_DESTROY (fish)
BABL_DEFINE_EACH (fish)
