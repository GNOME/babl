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
static void pixel_formats (void);

void
babl_base_model_cmyk (void)
{
  components    ();
  models        ();
  conversions   ();
  pixel_formats ();
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

#if 0
static void
rgb_to_and_from_cmy (int    src_bands,
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
      int band;
      for (band=0; band< 3; band++)
        {
          *(double*)dst[band] = 1.0F- (*(double*) src[band]);
        }
      for (;band<dst_bands && band<src_bands;band++)
        {
          *(double*)dst[band] = *(double*) src[band];
        }
      for (;band<dst_bands;band++)
        {
          *(double*)dst[band] = 1.0;  /* alpha */
        }
      BABL_PLANAR_STEP
    }
}
#endif

static void
conversions (void)
{
  babl_conversion_new (
    "babl-base: rgba to cmy",
    "source",      babl_model_id (BABL_RGBA),
    "destination", babl_model_id (BABL_CMYK),
    "planar",      rgb_to_cmyk,
    NULL
  );


  babl_conversion_new (
    "babl-base: cmy to rgba",
    "source",      babl_model_id (BABL_CMYK),
    "destination", babl_model_id (BABL_RGBA),
    "planar",      cmyk_to_rgb,
    NULL
  );
}

static void
pixel_formats (void)
{
}

