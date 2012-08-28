/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2012, Maxime Nicco <maxime.nicco@gmail.fr>
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
 * Adding support for HSV colorspace
 */

#include "config.h"
#include <math.h>
#include <string.h>

#include "babl.h"

#define MIN(a,b) (a > b) ? b : a;
#define MAX(a,b) (a < b) ? b : a;

static long  rgba_to_hsva (char *src,
                           char *dst,
                           long  n);

static long  hsva_to_rgba (char *src,
                           char *dst,
                           long  n);

int init (void);

int
init (void)
{
  babl_component_new ("hue", NULL);
  babl_component_new ("saturation", NULL);
  babl_component_new ("value", NULL);
  babl_component_new ("alpha", NULL);

  babl_model_new (
    "name", "HSVA",
    babl_component ("hue"),
    babl_component ("saturation"),
    babl_component ("value"),
    babl_component ("alpha"),
    NULL
  );

  babl_conversion_new (
    babl_model ("RGBA"),
    babl_model ("HSVA"),
    "linear", rgba_to_hsva,
    NULL
  );

  babl_conversion_new (
    babl_model ("HSVA"),
    babl_model ("RGBA"),
    "linear", hsva_to_rgba,
    NULL
  );
  babl_format_new (
    "name", "HSVA float",
    babl_model ("HSVA"),
    babl_type ("float"),
    babl_component ("hue"),
    babl_component ("saturation"),
    babl_component ("value"),
    babl_component ("alpha"),
    NULL
  );

  return 0;
}


static long
rgba_to_hsva (char *src,
              char *dst,
              long  n)
{
  while (n--)
  {
    double red   = ((double *) src)[0];
    double green = ((double *) src)[1];
    double blue  = ((double *) src)[2];
    double alpha = ((double *) src)[3];

    double hue, saturation, value;

    double  min;
    double  chroma;

    if (red > green)
    {
      value = MAX (red, blue);
      min = MIN (green, blue);
    }
    else
    {
      value = MAX (green, blue);
      min = MIN (red, blue);
    }

    chroma = value - min;

    if (value == 0.0)
      saturation = 0.0;
    else
      saturation = chroma / value;

    if (saturation == 0.0)
    {
      hue = 0.0;
    }
    else
    {
      if (red == value)
      {
        hue = (green - blue) / chroma;

        if(hue < 0.0)
          hue += 6.0;
      }
      else if (green == value)
        hue = 2.0 + (blue - red) / chroma;
      else
        hue = 4.0 + (red - green) / chroma;

      hue *= 60.0;
    }

    ((double *) dst)[0] = hue;
    ((double *) dst)[1] = saturation;
    ((double *) dst)[2] = value;
    ((double *) dst)[3] = alpha;

    src += 4 * sizeof (double);
    dst += 4 * sizeof (double);
  }
  return n;
}

static long
hsva_to_rgba (char *src,
              char *dst,
              long  n)
{
  while (n--)
  {
    double hue        = ((double *) src)[0];
    double saturation = ((double *) src)[1];
    double value      = ((double *) src)[2];
    double alpha      = ((double *) src)[3];

    double red = 0, green = 0, blue = 0;

    double chroma, h_tmp, x, min;

    chroma = saturation * value;
    h_tmp = hue / 60.0;
    x = chroma * (1.0 - fabs(fmod(h_tmp, 2.0) - 1.0));

    if (h_tmp < 1.0)
    {
      red   = chroma;
      green = x;
    }
    else if (h_tmp < 2.0)
    {
      red   = x;
      green = chroma;
    }
    else if (h_tmp < 3.0)
    {
      green = chroma;
      blue  = x;
    }
    else if (h_tmp < 4.0)
    {
      green = x;
      blue  = chroma;
    }
    else if (h_tmp < 5.0)
    {
      red  = x;
      blue = chroma;
    }
    else if (h_tmp < 6.0)
    {
      red  = chroma;
      blue = x;
    }

    min = value-chroma;

    red += min;
    green += min;
    blue += min;

    ((double *) dst)[0] = red;
    ((double *) dst)[1] = green;
    ((double *) dst)[2] = blue;
    ((double *) dst)[3] = alpha;

    src += 4 * sizeof (double);
    dst += 4 * sizeof (double);
  }
  return n;
}
