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
#include "babl-ids.h"

#include "util.h"

static void components    (void);
static void models        (void);
static void conversions   (void);
static void formats (void);

void
babl_base_model_ycbcr (void)
{
  components    ();
  models        ();
  conversions   ();
  formats       ();
}

static void
components (void)
{
  babl_component_new (
   "Cb",
   "id", BABL_CB,
   "chroma",
   NULL);

  babl_component_new (
   "Cr",
   "id", BABL_CR,
   "chroma",
   NULL);
}

static void
models (void)
{
  babl_model_new (
    "id", BABL_YCBCR,
    babl_component_id (BABL_LUMINANCE_GAMMA_2_2),
    babl_component_id (BABL_CB),
    babl_component_id (BABL_CR),
    NULL);

  babl_model_new (
    "id", BABL_YCBCR_ALPHA,
    babl_component_id (BABL_LUMINANCE_GAMMA_2_2),
    babl_component_id (BABL_CB),
    babl_component_id (BABL_CR),
    babl_component_id (BABL_ALPHA),
    NULL);
}

static void
rgb_to_ycbcr (int    src_bands,
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

      double luminance, cb, cr;

      red   = linear_to_gamma_2_2 (red);
      green = linear_to_gamma_2_2 (green);
      blue  = linear_to_gamma_2_2 (blue);
      
      luminance =  0.299    * red  +0.587    * green  +0.114    * blue;
      cb        = -0.168736 * red  -0.331264 * green  +0.5      * blue;
      cr        =  0.5      * red  -0.418688 * green  -0.081312 * blue;

      *(double*)dst[0] = luminance;
      *(double*)dst[1] = cb;
      *(double*)dst[2] = cr;

      if (dst_bands > 3)               /* alpha passthorugh */
        *(double*)dst[3] = (src_bands>3)?*(double*)src[3]:1.0;

      BABL_PLANAR_STEP
    }
}

static void
ycbcr_to_rgb (int    src_bands,
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
      double luminance = *(double*)src[0];
      double cb        = *(double*)src[1];
      double cr        = *(double*)src[2];

      double red, green, blue;

      red   = 1.0 * luminance  + 0.0      * cb  + 1.40200    * cr;
      green = 1.0 * luminance  - 0.344136 * cb  - 0.71414136 * cr;
      blue  = 1.0 * luminance  + 1.772    * cb  + 0.0        * cr;

      red   = gamma_2_2_to_linear (red);
      green = gamma_2_2_to_linear (green);
      blue  = gamma_2_2_to_linear (blue);

      *(double*)dst[0] = red;
      *(double*)dst[1] = green;
      *(double*)dst[2] = blue;

      if (dst_bands > 3)               /* alpha passthorugh */
        *(double*)dst[3] = (src_bands>3)?*(double*)src[3]:1.0;

      BABL_PLANAR_STEP
    }
}

static void
conversions (void)
{
  babl_conversion_new (
    babl_model_id (BABL_RGBA),
    babl_model_id (BABL_YCBCR),
    "planar",      rgb_to_ycbcr,
    NULL
  );
  babl_conversion_new (
    babl_model_id (BABL_YCBCR),
    babl_model_id (BABL_RGBA),
    "planar",      ycbcr_to_rgb,
    NULL
  );
  babl_conversion_new (
    babl_model_id (BABL_RGB),
    babl_model_id (BABL_YCBCR),
    "planar",      rgb_to_ycbcr,
    NULL
  );
  babl_conversion_new (
    babl_model_id (BABL_YCBCR),
    babl_model_id (BABL_RGB),
    "planar",      ycbcr_to_rgb,
    NULL
  );
  babl_conversion_new (
    babl_model_id (BABL_RGBA),
    babl_model_id (BABL_YCBCR_ALPHA),
    "planar",      rgb_to_ycbcr,
    NULL
  );
  babl_conversion_new (
    babl_model_id (BABL_YCBCR_ALPHA),
    babl_model_id (BABL_RGBA),
    "planar",      ycbcr_to_rgb,
    NULL
  );
}

static void
formats (void)
{
  babl_format_new (
    "name",        "y'cbcr420",
    "id",          BABL_YCBCR420,
    "planar",
    babl_model_id  (BABL_YCBCR),
    babl_type_id   (BABL_U8_LUMA),
    babl_sampling  (1, 1),
    babl_component_id (BABL_LUMINANCE_GAMMA_2_2),
    babl_type_id   (BABL_U8_CHROMA),
    babl_sampling  (2, 2),
    babl_component_id (BABL_CB), 
    babl_sampling  (2, 2),
    babl_component_id (BABL_CR),
    NULL);
  

  babl_format_new (
    "name",        "y'cbcr422",
    "id",          BABL_YCBCR422,
    "planar",
    babl_model_id  (BABL_YCBCR),
    babl_type_id   (BABL_U8_LUMA),
    babl_sampling  (1, 1),
    babl_component_id (BABL_LUMINANCE_GAMMA_2_2),
    babl_type_id   (BABL_U8_CHROMA),
    babl_sampling  (2, 1),
    babl_component_id (BABL_CB), 
    babl_sampling  (2, 1),
    babl_component_id (BABL_CR),
    NULL);

  babl_format_new (
    "name",        "y'cbcr411",
    "id",          BABL_YCBCR411,
    "planar",
    babl_model_id  (BABL_YCBCR),
    babl_type_id   (BABL_U8_LUMA),
    babl_sampling  (1, 1),
    babl_component_id (BABL_LUMINANCE_GAMMA_2_2),
    babl_type_id   (BABL_U8_CHROMA),
    babl_sampling  (4, 1),
    babl_component_id (BABL_CB), 
    babl_sampling  (4, 1),
    babl_component_id (BABL_CR),
    NULL);
}
