/* babl - dynamically extendable universal pixel conversion library.
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
#include <stdlib.h>
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
  "BablConversionLinear",
  "BablConversionPlane",
  "BablConversionPlanar",
  "BablFish",
  "BablFishReference",
  "BablFishSimple",
  "BablFishPath",
  "BablImage",
  "BablExtenstion",
  "BablSky"
};

const char *
babl_class_name (BablClassType klass)
{
  return class_names[klass - BABL_INSTANCE];
}

/* global variable, indicating whether name lookups
 * are frowned upon or not (they are frowned upon within BablBase,
 * since that leads to more strings than neccesary in the library.)
 */
int babl_hmpf_on_name_lookups = 0;

#include <sys/types.h>
#include <unistd.h>

int
babl_backtrack (void)
{
  char buf[512];

  sprintf (buf, "echo bt>/tmp/babl.gdb;"
           "gdb -q --batch -x /tmp/babl.gdb --pid=%i 2>/dev/null", getpid ());
  return system (buf);
}

void
babl_die (void)
{
  babl_backtrack ();
  exit (-1);
}

long babl_total_usecs = 0;

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
  if (n == 0)
    return 0;
  babl_assert (n > 0);

  /* matches all conversion classes */
  if (babl->class_type >= BABL_CONVERSION &&
      babl->class_type <= BABL_CONVERSION_PLANAR)
    return babl_conversion_process (babl, source, destination, n);

  if (babl->class_type == BABL_FISH ||
      babl->class_type == BABL_FISH_REFERENCE ||
      babl->class_type == BABL_FISH_PATH ||
      babl->class_type == BABL_FISH_SIMPLE)
    {
      long ret;
      /* long ticks = babl_ticks (); */
      ret = babl_fish_process (babl, source, destination, n);

      /* XX:
      ticks -= babl_ticks ();
      ticks *= -1L;

      babl_total_usecs += ticks;
      babl->fish.usecs += ticks;
      babl->fish.processings++;
      babl->fish.pixels += ret;
      */
      return ret;
    }

  babl_fatal ("eek");
  return -1;
}

#if BABL_DEBUG_MEM
BablMutex *babl_debug_mutex;
#endif

void
babl_internal_init (void)
{
  babl_set_malloc (malloc);
  babl_set_free (free);
#if BABL_DEBUG_MEM
  babl_debug_mutex = babl_mutex_new ();
#endif
}

void
babl_internal_destroy (void)
{
#if BABL_DEBUG_MEM
  babl_mutex_destroy (babl_debug_mutex);
#endif
}


const char *
babl_get_name (const Babl *babl)
{
  babl_assert (BABL_IS_BABL (babl));
  return babl->instance.name;
}
