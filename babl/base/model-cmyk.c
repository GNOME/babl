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
#include <string.h>
#include <math.h>
#include <assert.h>
#include "babl.h"

#include "util.h"

static void components (void);
static void models (void);
static void conversions (void);
static void formats (void);

void
babl_base_model_cmyk (void)
{
  components ();
  models ();
  conversions ();
  formats ();
}

static void
components (void)
{
  babl_component_new (
    "cyan",
    "id", BABL_CYAN,
    NULL);

  babl_component_new (
    "yellow",
    "id", BABL_YELLOW,
    NULL);

  babl_component_new (
    "magenta",
    "id", BABL_MAGENTA,
    NULL);

  babl_component_new (
    "key",
    "id", BABL_KEY,
    NULL);
}

static void
models (void)
{
  babl_model_new (
    "cmy",
    "id", BABL_CMY,
    babl_component_id (BABL_CYAN),
    babl_component_id (BABL_MAGENTA),
    babl_component_id (BABL_YELLOW),
    NULL
  );
  babl_model_new (
    "cmyk",
    "id", BABL_CMYK,
    babl_component_id (BABL_CYAN),
    babl_component_id (BABL_MAGENTA),
    babl_component_id (BABL_YELLOW),
    babl_component_id (BABL_KEY),
    NULL
  );

  babl_model_new (
    "cmyka",
    "id", BABL_CMYKA,
    babl_component_id (BABL_CYAN),
    babl_component_id (BABL_MAGENTA),
    babl_component_id (BABL_YELLOW),
    babl_component_id (BABL_KEY),
    babl_component_id (BABL_ALPHA),
    NULL
  );
}

static long
rgb_to_cmyk (void *src,
             void *dst,
             long  n)
{
  while (n--)
    {
      double red   = ((double *) src)[0];
      double green = ((double *) src)[1];
      double blue  = ((double *) src)[2];

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
      dst += 4 * sizeof (double)
    }
  return n;
}

static long
cmyk_to_rgb (void *src,
             void *dst,
             long  n)
{
  while (n--)
    {
      double cyan    = ((double *) src)[0];
      double yellow  = ((double *) src)[1];
      double magenta = ((double *) src)[2];
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

      ((double *) dst)[0] = red;
      ((double *) dst)[1] = green;
      ((double *) dst)[2] = blue;

      ((double *) dst)[3] = 1.0;

      src += 4 * sizeof (double);
      dst += 4 * sizeof (double)
    }
  return n;
}

static void
conversions (void)
{
  babl_conversion_new (
    "babl-base: rgba to cmyk",
    "source", babl_model_id (BABL_RGBA),
    "destination", babl_model_id (BABL_CMYK),
    "linear", rgb_to_cmyk,
    NULL
  );


  babl_conversion_new (
    "babl-base: cmyk to rgba",
    "source", babl_model_id (BABL_CMYK),
    "destination", babl_model_id (BABL_RGBA),
    "linear", cmyk_to_rgb,
    NULL
  );
}

static void
formats (void)
{
}
