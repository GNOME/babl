/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2005-2008 Øyvind Kolås.
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

/*
 * This file contains a pseudo model and format for storing frequency domain buffers,
 * the main purpose is registering a fully decorated introspectable format. It currently
 * provides nop conversions when registering when conversions to or from this conversion
 * is requested nothing is done.
 *
 * Added as a potential aid for summer of code project adding frequency domain processing
 * to GEGL
 */

#include "config.h"

#include <math.h>
#include <string.h>

#include "babl.h"


static long  rgba_to_frequency (char *src,
                                char *dst,
                                long  n);

static long  frequency_to_rgba (char *src,
                                char *dst,
                                long  n);

int init (void);

int
init (void)
{
  babl_component_new ("Rr", NULL);
  babl_component_new ("Gr", NULL);
  babl_component_new ("Br", NULL);
  babl_component_new ("Ar", NULL);
  babl_component_new ("Ri", NULL);
  babl_component_new ("Gi", NULL);
  babl_component_new ("Bi", NULL);
  babl_component_new ("Ai", NULL);

  babl_model_new (
    "name", "frequency",
    babl_component ("Rr"),
    babl_component ("Gr"),
    babl_component ("Br"),
    babl_component ("Ar"),
    babl_component ("Ri"),
    babl_component ("Gi"),
    babl_component ("Bi"),
    babl_component ("Ai"),
    NULL
  );

  babl_conversion_new (
    babl_model ("RGBA"),
    babl_model ("frequency"),
    "linear", rgba_to_frequency,
    NULL
  );

  babl_conversion_new (
    babl_model ("frequency"),
    babl_model ("RGBA"),
    "linear", frequency_to_rgba,
    NULL
  );

  babl_format_new (
    "name", "frequency float",
    babl_model ("frequency"),
    babl_type ("float"),
    babl_component ("Rr"),
    babl_component ("Gr"),
    babl_component ("Br"),
    babl_component ("Ar"),
    babl_component ("Ri"),
    babl_component ("Gi"),
    babl_component ("Bi"),
    babl_component ("Ai"),
    NULL
  );

  return 0;
}


static long
rgba_to_frequency (char *src,
                   char *dst,
                   long  n)
{
  /* we don't do any conversion, which will be registered as the only valid conversion in babl to go to RGB, it won't work though and
   * the buffer is left untouched without complaints from babl.
   */
  return n;
}

static long
frequency_to_rgba (char *src,
                   char *dst,
                   long  n)
{
  /* we don't do any conversion, which will be registered as the only valid conversion in babl to go to RGB, it won't work though and
   * the buffer is left untouched without complaints from babl.
   */
  return n;
}
