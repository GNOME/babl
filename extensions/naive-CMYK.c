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
#include <math.h>
#include <string.h>

#include "babl.h"
#include "base/util.h"


static long  rgba_to_cmyk (char *src,
                           char *dst,
                           long  n);

static long  cmyk_to_rgba (char *src,
                           char *dst,
                           long  n);

static long  rgba_to_cmy  (char *src,
                           char *dst,
                           long  n);

static long  cmy_to_rgba  (char *src,
                           char *dst,
                           long  n);

int init (void);

int
init (void)
{
  babl_component_new ("cyan", NULL);
  babl_component_new ("yellow", NULL);
  babl_component_new ("magenta", NULL);
  babl_component_new ("key", NULL);

  babl_model_new (
    "name", "CMYK",
    babl_component ("cyan"),
    babl_component ("magenta"),
    babl_component ("yellow"),
    babl_component ("key"),
    NULL
  );
  babl_model_new (
    "name", "CMY",
    babl_component ("cyan"),
    babl_component ("magenta"),
    babl_component ("yellow"),
    NULL
  );

  babl_conversion_new (
    babl_model ("RGBA"),
    babl_model ("CMYK"),
    "linear", rgba_to_cmyk,
    NULL
  );
  babl_conversion_new (
    babl_model ("CMYK"),
    babl_model ("RGBA"),
    "linear", cmyk_to_rgba,
    NULL
  );
  babl_conversion_new (
    babl_model ("RGBA"),
    babl_model ("CMY"),
    "linear", rgba_to_cmy,
    NULL
  );
  babl_conversion_new (
    babl_model ("CMY"),
    babl_model ("RGBA"),
    "linear", cmy_to_rgba,
    NULL
  );

  babl_format_new (
    "name", "CMYK float",
    babl_model ("CMYK"),
    babl_type ("float"),
    babl_component ("cyan"),
    babl_component ("magenta"),
    babl_component ("yellow"),
    babl_component ("key"),
    NULL
  );
  babl_format_new (
    "name", "CMY float",
    babl_model ("CMY"),
    babl_type ("float"),
    babl_component ("cyan"),
    babl_component ("magenta"),
    babl_component ("yellow"),
    NULL
  );

  babl_format_new (
    "name", "CMYK u8",
    babl_model ("CMYK"),
    babl_type ("u8"),
    babl_component ("cyan"),
    babl_component ("magenta"),
    babl_component ("yellow"),
    babl_component ("key"),
    NULL
  );

  return 0;
}


static long
rgba_to_cmyk (char *src,
              char *dst,
              long  n)
{
  while (n--)
    {
      double red   = linear_to_gamma_2_2 (((double *) src)[0]);
      double green = linear_to_gamma_2_2 (((double *) src)[1]);
      double blue  = linear_to_gamma_2_2 (((double *) src)[2]);

      double cyan, magenta, yellow, key;

      double pullout = 1.0;

      cyan    = 1.0 - red;
      magenta = 1.0 - green;
      yellow  = 1.0 - blue;

      key = 1.0;
      if (cyan < key) key = cyan;
      if (magenta < key) key = magenta;
      if (yellow < key) key = yellow;

      key *= pullout;

      if (key < 1.0)
        {
          cyan    = (cyan - key) / (1.0 - key);
          magenta = (magenta - key) / (1.0 - key);
          yellow  = (yellow - key) / (1.0 - key);
        }
      else
        {
          cyan    = 0.0;
          magenta = 0.0;
          yellow  = 0.0;
        }

      ((double *) dst)[0] = cyan;
      ((double *) dst)[1] = magenta;
      ((double *) dst)[2] = yellow;
      ((double *) dst)[3] = key;

      src += 4 * sizeof (double);
      dst += 4 * sizeof (double);
    }
  return n;
}

static long
cmyk_to_rgba (char *src,
              char *dst,
              long  n)
{
  while (n--)
    {
      double cyan    = ((double *) src)[0];
      double magenta = ((double *) src)[1];
      double yellow  = ((double *) src)[2];
      double key     = ((double *) src)[3];

      double red, green, blue;

      if (key < 1.0)
        {
          cyan    = cyan * (1.0 - key) + key;
          magenta = magenta * (1.0 - key) + key;
          yellow  = yellow * (1.0 - key) + key;
        }
      else
        {
          cyan = magenta = yellow = 1.0;
        }

      red   = 1.0 - cyan;
      green = 1.0 - magenta;
      blue  = 1.0 - yellow;

      ((double *) dst)[0] = gamma_2_2_to_linear (red);
      ((double *) dst)[1] = gamma_2_2_to_linear (green);
      ((double *) dst)[2] = gamma_2_2_to_linear (blue);

      ((double *) dst)[3] = 1.0;

      src += 4 * sizeof (double);
      dst += 4 * sizeof (double);
    }
  return n;
}

static long
rgba_to_cmy (char *src,
             char *dst,
             long  n)
{
  while (n--)
    {
      double red   = linear_to_gamma_2_2 (((double *) src)[0]);
      double green = linear_to_gamma_2_2 (((double *) src)[1]);
      double blue  = linear_to_gamma_2_2 (((double *) src)[2]);

      double cyan, magenta, yellow;

      cyan    = 1.0 - red;
      magenta = 1.0 - green;
      yellow  = 1.0 - blue;

      ((double *) dst)[0] = cyan;
      ((double *) dst)[1] = magenta;
      ((double *) dst)[2] = yellow;

      src += 4 * sizeof (double);
      dst += 3 * sizeof (double);
    }
  return n;
}

static long
cmy_to_rgba (char *src,
              char *dst,
              long  n)
{
  while (n--)
    {
      double cyan    = ((double *) src)[0];
      double magenta = ((double *) src)[1];
      double yellow  = ((double *) src)[2];

      double red, green, blue;

      red   = 1.0 - cyan;
      green = 1.0 - magenta;
      blue  = 1.0 - yellow;

      ((double *) dst)[0] = gamma_2_2_to_linear (red);
      ((double *) dst)[1] = gamma_2_2_to_linear (green);
      ((double *) dst)[2] = gamma_2_2_to_linear (blue);

      ((double *) dst)[3] = 1.0;

      src += 3 * sizeof (double);
      dst += 4 * sizeof (double);
    }
  return n;
}
