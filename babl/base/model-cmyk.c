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

static void
conversions (void)
{
  babl_conversion_new (
    "babl-base: rgba to cmy",
    "source",      babl_model_id (BABL_RGBA),
    "destination", babl_model_id (BABL_CMY),
    "planar",      rgb_to_and_from_cmy,
    NULL
  );


  babl_conversion_new (
    "babl-base: cmy to rgba",
    "source",      babl_model_id (BABL_CMY),
    "destination", babl_model_id (BABL_RGBA),
    "planar",      rgb_to_and_from_cmy,
    NULL
  );
}

static void
pixel_formats (void)
{
  babl_pixel_format_new (
    "cmyk-float",
    "id",             BABL_CMYK_FLOAT,
    babl_model_id     (BABL_CMYK),
    babl_type_id      (BABL_FLOAT),
    babl_component_id (BABL_CYAN), 
    babl_component_id (BABL_MAGENTA), 
    babl_component_id (BABL_YELLOW),
    babl_component_id (BABL_KEY),
    NULL);

  babl_pixel_format_new (
    "cmyka-float",
    "id",             BABL_CMYKA_FLOAT,
    babl_model_id     (BABL_CMYKA),
    babl_type_id      (BABL_FLOAT),
    babl_component_id (BABL_CYAN), 
    babl_component_id (BABL_MAGENTA), 
    babl_component_id (BABL_YELLOW),
    babl_component_id (BABL_KEY),
    babl_component_id (BABL_ALPHA),
    NULL);
}

