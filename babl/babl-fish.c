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
babl_conversion_find (const void *source,
                      const void *destination)
{
  void *data = (void*)destination;

  babl_list_each ((void *) BABL (source)->type.from, match_conversion, &data);
  if (data == (void*)destination) /* didn't change */
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

static inline Babl *
go_fishing (const Babl *source,
            const Babl *destination)
{
  BablDb *db = babl_fish_db ();
  int i;

  for (i = 0; i < db->babl_list->count; i++)
    {
      Babl *item = db->babl_list->items[i];
      if ((void *) source == (void *) item->fish.source &&
          (void *) destination == (void *) item->fish.destination &&
          (item->class_type == BABL_FISH_PATH || /* accept only paths */
           source == destination)                /* or memcpy */
          )
        {
          return item;
        }
    }
  return NULL;
}

Babl *
babl_fish (const void *source,
           const void *destination,
           ...)
{
  const Babl *source_format      = NULL;
  const Babl *destination_format = NULL;

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

  if (0) /* do not accept shortcut conversions, since there might be
            a faster path, besides the shortcut conversion might
            have a too large error, let's rely on the paths for
            error checking.
          */
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
  long       ret               = 0;

  switch (babl->class_type)
    {
      case BABL_FISH_REFERENCE:
      case BABL_FISH_SIMPLE:
      case BABL_FISH_PATH:
        if (babl->class_type == BABL_FISH_REFERENCE)
          {
            if (babl->fish.source == babl->fish.destination)
              { /* XXX: we're assuming linear buffers */
                memcpy (destination, source, n * babl->fish.source->format.bytes_per_pixel);
                ret = n;
              }
            else
              {
                ret = babl_fish_reference_process (babl, source, destination, n);
              }
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
                babl_assert (0);
              }
          }
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
