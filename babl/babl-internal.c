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
babl_die (void)
{
  char buf[512];

  sprintf (buf,"echo bt>/tmp/babl.gdb;"
               "gdb -q --batch -x /tmp/babl.gdb --pid=%i 2>/dev/null", getpid());
  system (buf);
  exit (-1);
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

