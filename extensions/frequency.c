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
 * This file contains a pseudo model and format for storing frequency domain
 * buffers,
 * the main purpose is registering a fully decorated introspectable format.
 * It currently provides nop conversions when registering when conversions to
 * or from this conversion is requested nothing is done.
 *
 * Added as a potential aid for summer of code project adding frequency domain
 * processing to GEGL
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
                  babl_component_from_name ("Rr"),
                  babl_component_from_name ("Gr"),
                  babl_component_from_name ("Br"),
                  babl_component_from_name ("Ar"),
                  babl_component_from_name ("Ri"),
                  babl_component_from_name ("Gi"),
                  babl_component_from_name ("Bi"),
                  babl_component_from_name ("Ai"),
                  NULL
                  );

  babl_conversion_new (
                       babl_model_from_name ("RGBA"),
                       babl_model_from_name ("frequency"),
                       "linear", rgba_to_frequency,
                       NULL
                       );

  babl_conversion_new (
                       babl_model_from_name ("frequency"),
                       babl_model_from_name ("RGBA"),
                       "linear", frequency_to_rgba,
                       NULL
                       );

  babl_format_new (
                   "name", "frequency float",
                   babl_model_from_name ("frequency"),
                   babl_component_from_name ("Rr"),
                   babl_component_from_name ("Gr"),
                   babl_component_from_name ("Br"),
                   babl_component_from_name ("Ar"),
                   babl_component_from_name ("Ri"),
                   babl_component_from_name ("Gi"),
                   babl_component_from_name ("Bi"),
                   babl_component_from_name ("Ai"),
                   NULL
                   );

  babl_format_new (
                   "name", "frequency double",
                   babl_model_from_name ("frequency"),
                   babl_type_from_name ("double"),
                   babl_component_from_name ("Rr"),
                   babl_component_from_name ("Gr"),
                   babl_component_from_name ("Br"),
                   babl_component_from_name ("Ar"),
                   babl_component_from_name ("Ri"),
                   babl_component_from_name ("Gi"),
                   babl_component_from_name ("Bi"),
                   babl_component_from_name ("Ai"),
                   NULL
                   );
    
  return 0;
}


static long
rgba_to_frequency (char *src,
                   char *dst,
                   long  n)
{
  /* we don't do any conversion, which will be registered as the only valid
   * conversion in babl to go to RGB, it won't work though and the buffer is
   * left untouched without complaints from babl.
   */
  return n;
}

static long
frequency_to_rgba (char *src,
                   char *dst,
                   long  n)
{
  while (n--)
    {
      double Rr = ((double *) src)[0];
      double Gr = ((double *) src)[1];
      double Br = ((double *) src)[2];
      double Ri = ((double *) src)[4];
      double Gi = ((double *) src)[5];
      double Bi = ((double *) src)[6];

      double red, green, blue;

      red = sqrt(Rr*Rr + Ri*Ri);
      green = sqrt(Gr*Gr + Gi*Gi);
      blue = sqrt(Br*Br + Bi*Bi);

      ((double *) dst)[0] = red;
      ((double *) dst)[1] = green;
      ((double *) dst)[2] = blue;
      ((double *) dst)[3] = 1;   

      src += sizeof (double) * 8;
      dst += sizeof (double) * 4;
    }
  return n;
}
