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
#include <stdlib.h>

#include "babl-classes.h"
#include "babl.h"
#include "babl-ids.h"
#include "math.h"
#include "babl-base.h"

static void components (void);
static void models (void);
static void conversions (void);
static void formats (void);

void babl_base_model_gray (void)
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
    "Y",
    "id", BABL_LUMINANCE,
    "luma",
    NULL);

  babl_component_new (
    "Ya",
    "id", BABL_LUMINANCE_MUL_ALPHA,
    "luma",
    NULL);

  babl_component_new (
    "Y'",
    "id", BABL_LUMA,
    "luma",
    NULL);

  babl_component_new (
    "Y'a",
    "id", BABL_LUMA_MUL_ALPHA,
    "luma",
    NULL);
}

static void
models (void)
{
  babl_model_new (
    "id", BABL_GRAY,
    babl_component_from_id (BABL_LUMINANCE),
    NULL);


  babl_model_new (
    "id", BABL_GRAY_ALPHA,
    babl_component_from_id (BABL_LUMINANCE),
    babl_component_from_id (BABL_ALPHA),
    NULL);

  babl_model_new (
    "id", BABL_GRAY_ALPHA_PREMULTIPLIED,
    babl_component_from_id (BABL_LUMINANCE_MUL_ALPHA),
    babl_component_from_id (BABL_ALPHA),
    NULL);


  babl_model_new (
    "id", BABL_GRAY_GAMMA_2_2,
    babl_component_from_id (BABL_LUMA),
    NULL);

  babl_model_new (
    "id", BABL_GRAY_GAMMA_2_2_ALPHA,
    babl_component_from_id (BABL_LUMA),
    babl_component_from_id (BABL_ALPHA),
    NULL);

  babl_model_new (
    "id", BABL_GRAY_GAMMA_2_2_ALPHA_PREMULTIPLIED,
    babl_component_from_id (BABL_LUMA_MUL_ALPHA),
    babl_component_from_id (BABL_ALPHA),
    NULL);
}


static void
rgba_to_graya (Babl *conversion,
               char *src,
               char *dst,
               long  n)
{
  const Babl *space = babl_conversion_get_source_space (conversion);
  double RGB_LUMINANCE_RED   = space->space.RGBtoXYZ[3];
  double RGB_LUMINANCE_GREEN = space->space.RGBtoXYZ[4];
  double RGB_LUMINANCE_BLUE  = space->space.RGBtoXYZ[5];

  while (n--)
    {
      double red, green, blue;
      double luminance, alpha;

      red   = ((double *) src)[0];
      green = ((double *) src)[1];
      blue  = ((double *) src)[2];
      alpha = ((double *) src)[3];

      luminance = red * RGB_LUMINANCE_RED +
                  green * RGB_LUMINANCE_GREEN +
                  blue * RGB_LUMINANCE_BLUE;

      ((double *) dst)[0] = luminance;
      ((double *) dst)[1] = alpha;

      src += sizeof (double) * 4;
      dst += sizeof (double) * 2;
    }
}

static void
rgba_to_gray (Babl *conversion,
              char *src,
              char *dst,
              long  n)
{
  const Babl *space = babl_conversion_get_source_space (conversion);
  double RGB_LUMINANCE_RED   = space->space.RGBtoXYZ[3];
  double RGB_LUMINANCE_GREEN = space->space.RGBtoXYZ[4];
  double RGB_LUMINANCE_BLUE  = space->space.RGBtoXYZ[5];

  while (n--)
    {
      double red, green, blue;
      double luminance;

      red   = ((double *) src)[0];
      green = ((double *) src)[1];
      blue  = ((double *) src)[2];

      luminance = red * RGB_LUMINANCE_RED +
                  green * RGB_LUMINANCE_GREEN +
                  blue * RGB_LUMINANCE_BLUE;

      ((double *) dst)[0] = luminance;

      src += sizeof (double) * 4;
      dst += sizeof (double) * 1;
    }
}

static void
rgb_to_gray_2_2 (Babl  *conversion,
                 int    src_bands,
                 char **src,
                 int   *src_pitch,
                 int    dst_bands,
                 char **dst,
                 int   *dst_pitch,
                 long   n)
{
  const Babl *space = babl_conversion_get_destination_space (conversion);
  const Babl *trc = space->space.trc[0];
  double RGB_LUMINANCE_RED   = space->space.RGBtoXYZ[3];
  double RGB_LUMINANCE_GREEN = space->space.RGBtoXYZ[4];
  double RGB_LUMINANCE_BLUE  = space->space.RGBtoXYZ[5];

  BABL_PLANAR_SANITY
  while (n--)
    {
      double red, green, blue;
      double luminance, alpha;

      red   = *(double *) src[0];
      green = *(double *) src[1];
      blue  = *(double *) src[2];
      if (src_bands > 3)
        alpha = *(double *) src[3];
      else
        alpha = 1.0;

      luminance = red   * RGB_LUMINANCE_RED +    // XXX: should be taken from BablSpace
                  green * RGB_LUMINANCE_GREEN +
                  blue  * RGB_LUMINANCE_BLUE;
      *(double *) dst[0] = babl_trc_from_linear (trc, luminance);

      if (dst_bands == 2)
        *(double *) dst[1] = alpha;

      BABL_PLANAR_STEP
    }
}


static void
gray_2_2_to_rgb (Babl *conversion,
                 int    src_bands,
                 char **src,
                 int   *src_pitch,
                 int    dst_bands,
                 char **dst,
                 int   *dst_pitch,
                 long   n)
{
  const Babl *space = babl_conversion_get_source_space (conversion);
  const Babl *trc = space->space.trc[0];

  BABL_PLANAR_SANITY
  while (n--)
    {
      double luminance;
      double red, green, blue;
      double alpha;

      luminance = babl_trc_to_linear (trc, *(double *) src[0]);
      red       = luminance;
      green     = luminance;
      blue      = luminance;
      if (src_bands > 1)
        alpha = *(double *) src[1];
      else
        alpha = 1.0;

      *(double *) dst[0] = red;
      *(double *) dst[1] = green;
      *(double *) dst[2] = blue;

      if (dst_bands > 3)
        *(double *) dst[3] = alpha;

      BABL_PLANAR_STEP
    }
}

static void
graya_to_rgba (Babl *conversion,
               char *src,
               char *dst,
               long  n)
{
  while (n--)
    {
      double luminance;
      double red, green, blue;
      double alpha;

      luminance = ((double *) src)[0];
      alpha     = ((double *) src)[1];
      red       = luminance;
      green     = luminance;
      blue      = luminance;

      ((double *) dst)[0] = red;
      ((double *) dst)[1] = green;
      ((double *) dst)[2] = blue;
      ((double *) dst)[3] = alpha;

      src += sizeof (double) * 2;
      dst += sizeof (double) * 4;
    }
}


static void
gray_to_rgba (Babl *conversion,
              char *src,
              char *dst,
              long  n)
{
  while (n--)
    {
      double luminance;
      double red, green, blue;

      luminance = ((double *) src)[0];
      red       = luminance;
      green     = luminance;
      blue      = luminance;

      ((double *) dst)[0] = red;
      ((double *) dst)[1] = green;
      ((double *) dst)[2] = blue;
      ((double *) dst)[3] = 1.0;

      src += sizeof (double) * 1;
      dst += sizeof (double) * 4;
    }
}

static void
gray_alpha_premultiplied_to_rgba (Babl   *conversion,
                                  int    src_bands,
                                  char **src,
                                  int   *src_pitch,
                                  int    dst_bands,
                                  char **dst,
                                  int   *dst_pitch,
                                  long   n)
{
  BABL_PLANAR_SANITY
  assert (src_bands == 2);
  assert (dst_bands == 4);

  while (n--)
    {
      double luminance = *(double *) src[0];
      double alpha     = *(double *) src[1];

      if (alpha > BABL_ALPHA_THRESHOLD)
        {
          luminance = luminance / alpha;
        }
      else
        {
          luminance = 0.0;
        }

      *(double *) dst[0] = luminance;
      *(double *) dst[1] = luminance;
      *(double *) dst[2] = luminance;
      *(double *) dst[3] = alpha;
      BABL_PLANAR_STEP
    }
}


static void
rgba_to_gray_alpha_premultiplied (Babl   *conversion,
                                  int    src_bands,
                                  char **src,
                                  int   *src_pitch,
                                  int    dst_bands,
                                  char **dst,
                                  int   *dst_pitch,
                                  long   n)
{
  const Babl *space = babl_conversion_get_source_space (conversion);
  double RGB_LUMINANCE_RED   = space->space.RGBtoXYZ[3];
  double RGB_LUMINANCE_GREEN = space->space.RGBtoXYZ[4];
  double RGB_LUMINANCE_BLUE  = space->space.RGBtoXYZ[5];

  BABL_PLANAR_SANITY;
  assert (src_bands == 4);
  assert (dst_bands == 2);

  while (n--)
    {
      double red   = *(double *) src[0];
      double green = *(double *) src[1];
      double blue  = *(double *) src[2];
      double alpha = *(double *) src[3];
      double luminance;

      luminance = red * RGB_LUMINANCE_RED +
                  green * RGB_LUMINANCE_GREEN +
                  blue * RGB_LUMINANCE_BLUE;

      luminance *= alpha;

      *(double *) dst[0] = luminance;
      *(double *) dst[1] = alpha;
      BABL_PLANAR_STEP
    }
}

static void
non_premultiplied_to_premultiplied (Babl  *conversion,
                                    int    src_bands,
                                    char **src,
                                    int   *src_pitch,
                                    int    dst_bands,
                                    char **dst,
                                    int   *dst_pitch,
                                    long   n)
{
  BABL_PLANAR_SANITY

  while (n--)
    {
      double alpha;
      int    band;

      alpha = *(double *) src[src_bands - 1];
      for (band = 0; band < src_bands - 1; band++)
        {
          *(double *) dst[band] = *(double *) src[band] * alpha;
        }
      *(double *) dst[dst_bands - 1] = alpha;

      BABL_PLANAR_STEP
    }
}

static void
premultiplied_to_non_premultiplied (Babl  *conversion,
                                    int    src_bands,
                                    char **src,
                                    int   *src_pitch,
                                    int    dst_bands,
                                    char **dst,
                                    int   *dst_pitch,
                                    long   n)
{
  BABL_PLANAR_SANITY

  while (n--)
    {
      double alpha;
      int    band;

      alpha = *(double *) src[src_bands - 1];
      for (band = 0; band < src_bands - 1; band++)
        {
          if (alpha > BABL_ALPHA_THRESHOLD)
            {
              *(double *) dst[band] = *(double *) src[band] / alpha;
            }
          else
            {
              *(double *) dst[band] = 0.0;
            }
        }
      *(double *) dst[dst_bands - 1] = alpha;

      BABL_PLANAR_STEP
    }
}

static void
rgba2gray_gamma_2_2_premultiplied (Babl *conversion,
                                   char *src,
                                   char *dst,
                                   long  n)
{
  const Babl *space = babl_conversion_get_destination_space (conversion);
  const Babl *trc = space->space.trc[0];
  double RGB_LUMINANCE_RED   = space->space.RGBtoXYZ[3];
  double RGB_LUMINANCE_GREEN = space->space.RGBtoXYZ[4];
  double RGB_LUMINANCE_BLUE  = space->space.RGBtoXYZ[5];

  while (n--)
    {
      double red   = ((double *) src)[0];
      double green = ((double *) src)[1];
      double blue  = ((double *) src)[2];
      double alpha = ((double *) src)[3];

      double luminance;
      double luma;

      luminance = red * RGB_LUMINANCE_RED +
                  green * RGB_LUMINANCE_GREEN +
                  blue * RGB_LUMINANCE_BLUE;
      luma = babl_trc_from_linear (trc, luminance);

      ((double *) dst)[0] = luma * alpha;
      ((double *) dst)[1] = alpha;

      src += 4 * sizeof (double);
      dst += 2 * sizeof (double);
    }
}


static void
gray_gamma_2_2_premultiplied2rgba (Babl *conversion,
                                   char *src,
                                   char *dst,
                                   long  n)
{
  const Babl *space = babl_conversion_get_destination_space (conversion);
  const Babl *trc = space->space.trc[0];

  while (n--)
    {
      double luma  = ((double *) src)[0];
      double alpha = ((double *) src)[1];
      double luminance;

      if (alpha > BABL_ALPHA_THRESHOLD)
        luma      = luma / alpha;
      else
        luma      = 0.0;

      luminance = babl_trc_to_linear (trc, luma);

      ((double *) dst)[0] = luminance;
      ((double *) dst)[1] = luminance;
      ((double *) dst)[2] = luminance;
      ((double *) dst)[3] = alpha;

      src += 2 * sizeof (double);
      dst += 4 * sizeof (double);
    }
}


static void
conversions (void)
{
  babl_conversion_new (
    babl_model_from_id (BABL_GRAY_GAMMA_2_2),
    babl_model_from_id (BABL_RGBA),
    "planar", gray_2_2_to_rgb,
    NULL
  );

  babl_conversion_new (
    babl_model_from_id (BABL_RGBA),
    babl_model_from_id (BABL_GRAY_GAMMA_2_2),
    "planar", rgb_to_gray_2_2,
    NULL
  );

  babl_conversion_new (
    babl_model_from_id (BABL_GRAY_GAMMA_2_2_ALPHA),
    babl_model_from_id (BABL_RGBA),
    "planar", gray_2_2_to_rgb,
    NULL
  );

  babl_conversion_new (
    babl_model_from_id (BABL_RGBA),
    babl_model_from_id (BABL_GRAY_GAMMA_2_2_ALPHA),
    "planar", rgb_to_gray_2_2,
    NULL
  );


  babl_conversion_new (
    babl_model_from_id (BABL_GRAY_GAMMA_2_2_ALPHA_PREMULTIPLIED),
    babl_model_from_id (BABL_RGBA),
    "linear", gray_gamma_2_2_premultiplied2rgba,
    NULL
  );

  babl_conversion_new (
    babl_model_from_id (BABL_RGBA),
    babl_model_from_id (BABL_GRAY_GAMMA_2_2_ALPHA_PREMULTIPLIED),
    "linear", rgba2gray_gamma_2_2_premultiplied,
    NULL
  );

  babl_conversion_new (
    babl_model_from_id (BABL_GRAY),
    babl_model_from_id (BABL_RGBA),
    "linear", gray_to_rgba,
    NULL
  );

  babl_conversion_new (
    babl_model_from_id (BABL_GRAY_ALPHA),
    babl_model_from_id (BABL_RGBA),
    "linear", graya_to_rgba,
    NULL
  );

  babl_conversion_new (
    babl_model_from_id (BABL_RGBA),
    babl_model_from_id (BABL_GRAY_ALPHA),
    "linear", rgba_to_graya,
    NULL
  );

  babl_conversion_new (
    babl_model_from_id (BABL_RGBA),
    babl_model_from_id (BABL_GRAY),
    "linear", rgba_to_gray,
    NULL
  );

  babl_conversion_new (
    babl_model_from_id (BABL_GRAY_ALPHA),
    babl_model_from_id (BABL_GRAY_ALPHA_PREMULTIPLIED),
    "planar", non_premultiplied_to_premultiplied,
    NULL
  );

  babl_conversion_new (
    babl_model_from_id (BABL_GRAY_ALPHA_PREMULTIPLIED),
    babl_model_from_id (BABL_GRAY_ALPHA),
    "planar", premultiplied_to_non_premultiplied,
    NULL
  );

  babl_conversion_new (
    babl_model_from_id (BABL_GRAY_ALPHA_PREMULTIPLIED),
    babl_model_from_id (BABL_RGBA),
    "planar", gray_alpha_premultiplied_to_rgba,
    NULL
  );

  babl_conversion_new (
    babl_model_from_id (BABL_RGBA),
    babl_model_from_id (BABL_GRAY_ALPHA_PREMULTIPLIED),
    "planar", rgba_to_gray_alpha_premultiplied,
    NULL
  );
}

static void
formats (void)
{
  babl_format_new (
    babl_model_from_id (BABL_GRAY_ALPHA),
    babl_type_from_id (BABL_HALF),
    babl_component_from_id (BABL_LUMINANCE),
    babl_component_from_id (BABL_ALPHA),
    NULL);
  babl_format_new (
    babl_model_from_id (BABL_GRAY_ALPHA_PREMULTIPLIED),
    babl_type_from_id (BABL_HALF),
    babl_component_from_id (BABL_LUMINANCE_MUL_ALPHA),
    babl_component_from_id (BABL_ALPHA),
    NULL);
  babl_format_new (
    babl_model_from_id (BABL_GRAY),
    babl_type_from_id (BABL_HALF),
    babl_component_from_id (BABL_LUMINANCE),
    NULL);
  babl_format_new (
    babl_model_from_id (BABL_GRAY_GAMMA_2_2_ALPHA),
    babl_type_from_id (BABL_HALF),
    babl_component_from_id (BABL_LUMA),
    babl_component_from_id (BABL_ALPHA),
    NULL);
  babl_format_new (
    babl_model_from_id (BABL_GRAY_GAMMA_2_2_ALPHA_PREMULTIPLIED),
    babl_type_from_id (BABL_HALF),
    babl_component_from_id (BABL_LUMA_MUL_ALPHA),
    babl_component_from_id (BABL_ALPHA),
    NULL);
  babl_format_new (
    babl_model_from_id (BABL_GRAY_GAMMA_2_2),
    babl_type_from_id (BABL_HALF),
    babl_component_from_id (BABL_LUMA),
    NULL);
  /***********/

  babl_format_new (
    babl_model_from_id (BABL_GRAY_ALPHA),
    babl_type ("u15"),
    babl_component_from_id (BABL_LUMINANCE),
    babl_component_from_id (BABL_ALPHA),
    NULL);
  babl_format_new (
    babl_model_from_id (BABL_GRAY_ALPHA_PREMULTIPLIED),
    babl_type ("u15"),
    babl_component_from_id (BABL_LUMINANCE_MUL_ALPHA),
    babl_component_from_id (BABL_ALPHA),
    NULL);
  babl_format_new (
    babl_model_from_id (BABL_GRAY),
    babl_type ("u15"),
    babl_component_from_id (BABL_LUMINANCE),
    NULL);
  babl_format_new (
    babl_model_from_id (BABL_GRAY_GAMMA_2_2_ALPHA),
    babl_type ("u15"),
    babl_component_from_id (BABL_LUMA),
    babl_component_from_id (BABL_ALPHA),
    NULL);
  babl_format_new (
    babl_model_from_id (BABL_GRAY_GAMMA_2_2_ALPHA_PREMULTIPLIED),
    babl_type ("u15"),
    babl_component_from_id (BABL_LUMA_MUL_ALPHA),
    babl_component_from_id (BABL_ALPHA),
    NULL);
  babl_format_new (
    babl_model_from_id (BABL_GRAY_GAMMA_2_2),
    babl_type ("u15"),
    babl_component_from_id (BABL_LUMA),
    NULL);

  babl_format_new (
    babl_model_from_id (BABL_GRAY_ALPHA),
    babl_type_from_id (BABL_U32),
    babl_component_from_id (BABL_LUMINANCE),
    babl_component_from_id (BABL_ALPHA),
    NULL);
  babl_format_new (
    babl_model_from_id (BABL_GRAY_ALPHA_PREMULTIPLIED),
    babl_type_from_id (BABL_U32),
    babl_component_from_id (BABL_LUMINANCE_MUL_ALPHA),
    babl_component_from_id (BABL_ALPHA),
    NULL);
  babl_format_new (
    babl_model_from_id (BABL_GRAY),
    babl_type_from_id (BABL_U32),
    babl_component_from_id (BABL_LUMINANCE),
    NULL);
  babl_format_new (
    babl_model_from_id (BABL_GRAY_GAMMA_2_2_ALPHA),
    babl_type_from_id (BABL_U32),
    babl_component_from_id (BABL_LUMA),
    babl_component_from_id (BABL_ALPHA),
    NULL);
  babl_format_new (
    babl_model_from_id (BABL_GRAY_GAMMA_2_2_ALPHA_PREMULTIPLIED),
    babl_type_from_id (BABL_U32),
    babl_component_from_id (BABL_LUMA_MUL_ALPHA),
    babl_component_from_id (BABL_ALPHA),
    NULL);
  babl_format_new (
    babl_model_from_id (BABL_GRAY_GAMMA_2_2),
    babl_type_from_id (BABL_U32),
    babl_component_from_id (BABL_LUMA),
    NULL);
}
