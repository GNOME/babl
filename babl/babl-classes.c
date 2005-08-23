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

#include "babl-classes.h"

static const char *class_names[] =
  {
    "BablInstance",
    "BablType",
    "BablTypeInteger",
    "BablTypeFloat",
    "BablSampling",
    "BablComponent",
    "BablModel",
    "BablPixelFormat",
    "BablConversion",
    "BablConversionType",
    "BablConversionTypePlanar",
    "BablConversionModelPlanar",
    "BablConversionPixelFormat",
    "BablConversionPixelFormatPlanar",
    "BablFish",
    "BablFishReference",
    "BablImage",
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

