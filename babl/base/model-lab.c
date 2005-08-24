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
#include "babl.h"
#include "util.h"
#include "cpercep.h"

static void components    (void);
static void models        (void);
static void conversions   (void);
static void formats       (void);

void
babl_base_model_lab (void)
{
  cpercep_init  ();
  components    ();
  models        ();
  conversions   ();
  formats       ();
}

static void
components (void)
{
  babl_component_new (
   "CIE L",
   "id",    BABL_CIE_L,
   NULL);

  babl_component_new (
   "CIE a",
   "id",    BABL_CIE_A,
   "chroma",
   NULL);

  babl_component_new (
   "CIE b",
   "id",    BABL_CIE_B,
   "chroma",
   NULL);
}

static void
models (void)
{
  babl_model_new (
    "CIE Lab",
    "id", BABL_CIE_LAB,
    babl_component_id (BABL_CIE_L),
    babl_component_id (BABL_CIE_A),
    babl_component_id (BABL_CIE_B),
    NULL);

  babl_model_new (
    "CIE Lab alpha",
    "id", BABL_CIE_LAB_ALPHA,
    babl_component_id (BABL_CIE_L),
    babl_component_id (BABL_CIE_A),
    babl_component_id (BABL_CIE_B),
    babl_component_id (BABL_ALPHA),
    NULL);
}

static void
rgb_to_lab (int    src_bands,
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

      double L, a, b;

      cpercep_rgb_to_space (red, green, blue, &L, &a, &b);

      *(double*)dst[0] = L;
      *(double*)dst[1] = a;
      *(double*)dst[2] = b;

      if (dst_bands > 3)               /* alpha passthorugh */
        *(double*)dst[3] = (src_bands>3)?*(double*)src[3]:1.0;

      BABL_PLANAR_STEP
    }
}

static void
lab_to_rgb (int    src_bands,
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
      double L = *(double*)src[0];
      double a = *(double*)src[1];
      double b = *(double*)src[2];

      double red, green, blue;

      cpercep_space_to_rgb (L, a, b, &red, &green, &blue);

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
    "babl-base: rgba to cie-lab",
    "source",      babl_model_id (BABL_RGBA),
    "destination", babl_model_id (BABL_CIE_LAB),
    "planar",      rgb_to_lab,
    NULL
  );
  babl_conversion_new (
    "babl-base: cie-lab to rgba",
    "source",      babl_model_id (BABL_CIE_LAB),
    "destination", babl_model_id (BABL_RGBA),
    "planar",      lab_to_rgb,
    NULL
  );
  babl_conversion_new (
    "babl-base: rgb to cie-lab",
    "source",      babl_model_id (BABL_RGB),
    "destination", babl_model_id (BABL_CIE_LAB),
    "planar",      rgb_to_lab,
    NULL
  );
  babl_conversion_new (
    "babl-base: cie-lab to rgb",
    "source",      babl_model_id (BABL_CIE_LAB),
    "destination", babl_model_id (BABL_RGB),
    "planar",      lab_to_rgb,
    NULL
  );
  babl_conversion_new (
    "babl-base: rgba to cie-lab-float",
    "source",      babl_model_id (BABL_RGBA),
    "destination", babl_model_id (BABL_CIE_LAB_ALPHA),
    "planar",      rgb_to_lab,
    NULL
  );
  babl_conversion_new (
    "babl-base: cie-lab-float to rgba",
    "source",      babl_model_id (BABL_CIE_LAB_ALPHA),
    "destination", babl_model_id (BABL_RGBA),
    "planar",      lab_to_rgb,
    NULL
  );
}

static void
formats (void)
{
  babl_format_new (
    "cie-lab-float",
    "id", BABL_LAB_FLOAT,
    babl_model_id     (BABL_CIE_LAB),
    babl_type_id      (BABL_FLOAT),
    babl_component_id (BABL_CIE_L), 
    babl_component_id (BABL_CIE_A), 
    babl_component_id (BABL_CIE_B),
    NULL);

  babl_format_new (
    "cie-lab-u8",
    "id", BABL_LAB_U8,
    babl_model_id     (BABL_CIE_LAB),
    babl_type_id      (BABL_U8_CIE_L),
    babl_component_id (BABL_CIE_L),
    babl_type_id      (BABL_U8_CIE_AB),
    babl_component_id (BABL_CIE_A), 
    babl_type_id      (BABL_U8_CIE_AB),
    babl_component_id (BABL_CIE_B),
    NULL);

  babl_format_new (
    "cie-lab-u16",
    "id", BABL_LAB_U16,
    babl_model_id     (BABL_CIE_LAB),
    babl_type_id      (BABL_U16_CIE_L),
    babl_component_id (BABL_CIE_L),
    babl_type_id      (BABL_U16_CIE_AB),
    babl_component_id (BABL_CIE_A), 
    babl_type_id      (BABL_U16_CIE_AB),
    babl_component_id (BABL_CIE_B),
    NULL);
}
