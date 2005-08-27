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

#include <string.h>
#include <math.h>
#include <assert.h>
#include "babl.h"
#include "util.h"

static void components    (void);
static void models        (void);
static void conversions   (void);
static void formats       (void);

int
init (void)
{
  components    ();
  models        ();
  conversions   ();
  formats       ();

  return 0;
}

static void
components (void)
{
  babl_component_new ("cyan",    NULL);
  babl_component_new ("yellow",  NULL);
  babl_component_new ("magenta", NULL);
  babl_component_new ("key",     NULL);
}

static void
models (void)
{
  babl_model_new (
    "name", "CMYK",
    babl_component ("cyan"),
    babl_component ("magenta"),
    babl_component ("yellow"),
    babl_component ("key"),
    NULL
  );
}

static void
rgb_to_cmyk (int    src_bands,
             void **src,
             int   *src_pitch,
             int    dst_bands,
             void **dst,
             int   *dst_pitch,
             int    n)
{
  BABL_PLANAR_SANITY

  while (n--)
    {
      double red   = *(double*)src[0];
      double green = *(double*)src[1];
      double blue  = *(double*)src[2];

      double cyan, magenta, yellow, key;

      double pullout = 1.0;

      cyan    = 1.0 - red;
      magenta = 1.0 - green;
      yellow  = 1.0 - blue;

      key = 1.0;
      if (cyan    < key) key = cyan;
      if (magenta < key) key = magenta;
      if (yellow  < key) key = yellow;

      key *= pullout;

      if (key < 1.0)
        {
          cyan    = (cyan - key)    / (1.0 -key);
          magenta = (magenta - key) / (1.0 -key);
          yellow  = (yellow - key)  / (1.0 -key);
        }
      else
        {
          cyan    = 0.0;
          magenta = 0.0;
          yellow  = 0.0;
        }

      *(double*)dst[0] = cyan;
      *(double*)dst[1] = magenta;
      *(double*)dst[2] = yellow;
      *(double*)dst[3] = key;

      if (dst_bands > 4)               /* alpha passthorugh */
        *(double*)dst[4] = (src_bands>3)?*(double*)src[3]:1.0;

      BABL_PLANAR_STEP
    }
}

static void
cmyk_to_rgb (int    src_bands,
             void **src,
             int   *src_pitch,
             int    dst_bands,
             void **dst,
             int   *dst_pitch,
             int    n)
{
  BABL_PLANAR_SANITY

  while (n--)
    {
      double cyan    = *(double*)src[0];
      double yellow  = *(double*)src[1];
      double magenta = *(double*)src[2];
      double key     = *(double*)src[3];

      double red, green, blue;

      if (key < 1.0)
        {
          cyan    = cyan    * (1.0 - key) + key;
          magenta = magenta * (1.0 - key) + key;
          yellow  = yellow  * (1.0 - key) + key;
        }
      else
        {
          cyan = magenta = yellow = 1.0;
        }

      red   = 1.0 - cyan;
      green = 1.0 - magenta;
      blue  = 1.0 - yellow;

      *(double*)dst[0] = red;
      *(double*)dst[1] = green;
      *(double*)dst[2] = blue;

      if (dst_bands > 3)               /* alpha passthorugh */
        *(double*)dst[3] = (src_bands>4)?*(double*)src[4]:1.0;

      BABL_PLANAR_STEP
    }
}

static void
conversions (void)
{
  babl_conversion_new (
    babl_model ("RGBA"),
    babl_model ("CMYK"),
    "planar",      rgb_to_cmyk,
    NULL
  );


  babl_conversion_new (
    babl_model ("CMYK"),
    babl_model ("RGBA"),
    "planar",      cmyk_to_rgb,
    NULL
  );
}

static void
formats (void)
{
  babl_format_new ("CMYK float",
      babl_model ("CMYK"),
      babl_type ("float"),
      babl_component ("cyan"),
      babl_component ("yellow"),
      babl_component ("magenta"),
      babl_component ("key"),
      NULL
  );
}
