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

static const char *class_names[] =
  {
    "BablInstance",
    "BablType",
    "BablTypeInteger",
    "BablTypeFloat",
    "BablSampling",
    "BablComponent",
    "BablModel",
    "BablFormat",
    "BablConversion",
    "BablConversionType",
    "BablConversionTypePlanar",
    "BablConversionModelPlanar",
    "BablConversionFormat",
    "BablConversionFormatPlanar",
    "BablFish",
    "BablFishReference",
    "BablImage",
    "BablExtenstion",
    "BablSky"
  };

const char *
babl_class_name (BablClassType klass)
{
  return class_names[klass-BABL_INSTANCE];
}

/* global variable, indicating whether name lookups
 * are frowned upon or not (they are frowned upon within BablBase,
 * since that leads to more strings than neccesary in the library.)
 */
int babl_hmpf_on_name_lookups = 0;

#include <sys/types.h>
#include <unistd.h>

void
babl_backtrack (void)
{
  char buf[512];

  sprintf (buf,"echo bt>/tmp/babl.gdb;"
               "gdb -q --batch -x /tmp/babl.gdb --pid=%i 2>/dev/null", getpid());
  system (buf);
}

void
babl_die (void)
{
  babl_backtrack ();
  exit (-1);
}

long   babl_conversion_process (Babl *conversion,
                                void *source,
                                void *destination,
                                long  n);
long
babl_process (Babl *babl,
              void *source,
              void *destination,
              long  n)
{
  babl_assert (babl);
  babl_assert (source);
  babl_assert (destination);
  babl_assert (BABL_IS_BABL (babl));
  babl_assert (n>0);

  /* these fields are common between conversions and fishes */
  babl->fish.processings++;
  babl->fish.pixels += n;

  /* matches all conversion classes */
  if (babl->class_type >= BABL_CONVERSION &&
      babl->class_type <= BABL_CONVERSION_FORMAT_PLANAR)
    return babl_conversion_process (babl, source, destination, n);
  
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

  babl_log ("eek");
  return -1;
}

void
babl_internal_init (void)
{
  babl_set_malloc (malloc);
  babl_set_free (free);
}

void
babl_internal_destroy (void)
{
}


const char *
babl_name (Babl *babl)
{
  babl_assert (BABL_IS_BABL (babl));
  return babl->instance.name;
}
