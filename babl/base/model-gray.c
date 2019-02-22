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
 * <https://www.gnu.org/licenses/>.
 */

#include "config.h"
#include <stdlib.h>

#include "babl-internal.h"
#include "babl-ids.h"
#include "math.h"
#include "babl-base.h"

static void components (void);
static void models (void);
static void conversions (void);
static void formats (void);
static void init_single_precision (void);

void 
babl_base_model_gray (void)
{
  components ();
  models ();
  conversions ();
  formats ();
  init_single_precision ();
}

static void
components (void)
{
  babl_component_new (
    "Y",
    "id", BABL_GRAY_LINEAR,
    "luma",
    NULL);

  babl_component_new (
    "Ya",
    "id", BABL_GRAY_LINEAR_MUL_ALPHA,
    "luma",
    NULL);

  babl_component_new (
    "Y'",
    "id", BABL_GRAY_NONLINEAR,
    "luma",
    NULL);

  babl_component_new (
    "Y'a",
    "id", BABL_GRAY_NONLINEAR_MUL_ALPHA,
    "luma",
    NULL);

  babl_component_new (
    "Y~",
    "id", BABL_GRAY_PERCEPTUAL,
    "luma",
    NULL);
}

static void
models (void)
{
  babl_model_new (
    "id", BABL_GRAY,
    babl_component_from_id (BABL_GRAY_LINEAR),
    "gray",
    "linear",
    NULL);


  babl_model_new (
    "id", BABL_GRAY_ALPHA,
    babl_component_from_id (BABL_GRAY_LINEAR),
    babl_component_from_id (BABL_ALPHA),
    "gray",
    "linear",
    "alpha",
    NULL);

  babl_model_new (
    "id", BABL_GRAY_ALPHA_PREMULTIPLIED,
    babl_component_from_id (BABL_GRAY_LINEAR_MUL_ALPHA),
    babl_component_from_id (BABL_ALPHA),
    "gray",
    "linear",
    "premultiplied",
    "alpha",
    NULL);

  babl_model_new (
    "id", BABL_MODEL_GRAY_NONLINEAR,
    babl_component_from_id (BABL_GRAY_NONLINEAR),
    "gray",
    "nonlinear",
    NULL);

  babl_model_new (
    "id", BABL_MODEL_GRAY_NONLINEAR_ALPHA,
    babl_component_from_id (BABL_GRAY_NONLINEAR),
    babl_component_from_id (BABL_ALPHA),
    "gray",
    "nonlinear",
    "alpha",
    NULL);

  babl_model_new (
    "id", BABL_MODEL_GRAY_NONLINEAR_ALPHA_PREMULTIPLIED,
    babl_component_from_id (BABL_GRAY_NONLINEAR_MUL_ALPHA),
    babl_component_from_id (BABL_ALPHA),
    "gray",
    "nonlinear",
    "premultiplied",
    "alpha",
    NULL);

  babl_model_new (
    "id", BABL_MODEL_GRAY_PERCEPTUAL,
    babl_component_from_id (BABL_GRAY_PERCEPTUAL),
    "gray",
    "perceptual",
    NULL);

  babl_model_new (
    "id", BABL_MODEL_GRAY_PERCEPTUAL_ALPHA,
    babl_component_from_id (BABL_GRAY_PERCEPTUAL),
    babl_component_from_id (BABL_ALPHA),
    "gray",
    "perceptual",
    "alpha",
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

static const Babl *perceptual_trc = NULL;

static void
rgb_to_gray_nonlinear (Babl  *conversion,
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

      luminance = red   * RGB_LUMINANCE_RED +
                  green * RGB_LUMINANCE_GREEN +
                  blue  * RGB_LUMINANCE_BLUE;
      *(double *) dst[0] = babl_trc_from_linear (trc, luminance);

      if (dst_bands == 2)
        *(double *) dst[1] = alpha;

      BABL_PLANAR_STEP
    }
}


static void
gray_nonlinear_to_rgb (Babl *conversion,
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
rgb_to_gray_perceptual (Babl  *conversion,
                        int    src_bands,
                        char **src,
                        int   *src_pitch,
                        int    dst_bands,
                        char **dst,
                        int   *dst_pitch,
                        long   n)
{
  const Babl *space = babl_conversion_get_destination_space (conversion);
  const Babl *trc = perceptual_trc;
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

      luminance = red   * RGB_LUMINANCE_RED +
                  green * RGB_LUMINANCE_GREEN +
                  blue  * RGB_LUMINANCE_BLUE;
      *(double *) dst[0] = babl_trc_from_linear (trc, luminance);

      if (dst_bands == 2)
        *(double *) dst[1] = alpha;

      BABL_PLANAR_STEP
    }
}

static void
gray_perceptual_to_rgb (Babl  *conversion,
                        int    src_bands,
                        char **src,
                        int   *src_pitch,
                        int    dst_bands,
                        char **dst,
                        int   *dst_pitch,
                        long   n)
{
  const Babl *trc = perceptual_trc;

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
gray_alpha_premultiplied_to_rgba (Babl  *conversion,
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
      double alpha;
      alpha = *(double *) src[1];
      if (alpha == 0)
        luminance = 0;
      else
      {
        luminance = luminance / alpha;
        if (alpha == BABL_ALPHA_FLOOR || alpha == -BABL_ALPHA_FLOOR)
          alpha = 0.0;
      }

      *(double *) dst[0] = luminance;
      *(double *) dst[1] = luminance;
      *(double *) dst[2] = luminance;
      *(double *) dst[3] = alpha;
      BABL_PLANAR_STEP
    }
}


static void
rgba_to_gray_alpha_premultiplied (Babl  *conversion,
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
      double luminance;
      double alpha = *(double *) src[3];
      if (alpha <= BABL_ALPHA_FLOOR)
      {
        if (alpha >= 0.0f)
          alpha = BABL_ALPHA_FLOOR;
         else if (alpha >= -BABL_ALPHA_FLOOR)
           alpha = -BABL_ALPHA_FLOOR;
      }

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
      int    band;
      double alpha = *(double *) src[src_bands-1];
      if (alpha < BABL_ALPHA_FLOOR)
      {
        if (alpha >= 0.0f)
          alpha = BABL_ALPHA_FLOOR;
        else if (alpha >= -BABL_ALPHA_FLOOR)
          alpha = -BABL_ALPHA_FLOOR;
      }

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
      int    band;
      double alpha;
      alpha = *(double *) src[src_bands-1];

      for (band = 0; band < src_bands - 1; band++)
        {
          if (alpha == 0.0)
            *(double *) dst[band] = 0;
          else
            *(double *) dst[band] = *(double *) src[band] / alpha;
        }
      if (alpha == BABL_ALPHA_FLOOR || alpha == -BABL_ALPHA_FLOOR)
        alpha = 0.0;
      *(double *) dst[dst_bands - 1] = alpha;

      BABL_PLANAR_STEP
    }
}

static void
rgba2gray_nonlinear_premultiplied (Babl *conversion,
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
      double luminance;
      double luma;
      double alpha = ((double *) src)[3];
      if (alpha < BABL_ALPHA_FLOOR)
      {
        if (alpha >= 0.0f)
          alpha = BABL_ALPHA_FLOOR;
        else if (alpha >= -BABL_ALPHA_FLOOR)
          alpha = -BABL_ALPHA_FLOOR;
      }

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
gray_nonlinear_premultiplied2rgba (Babl *conversion,
                                   char *src,
                                   char *dst,
                                   long  n)
{
  const Babl *space = babl_conversion_get_destination_space (conversion);
  const Babl *trc = space->space.trc[0];

  while (n--)
    {
      double luma  = ((double *) src)[0];
      double luminance;
      double alpha;
      alpha = ((double *) src)[1];
      if (alpha == 0.0)
        luma = 0.0;
      else
        luma = luma / alpha;

      luminance = babl_trc_to_linear (trc, luma);

      if (alpha == BABL_ALPHA_FLOOR || alpha == -BABL_ALPHA_FLOOR)
        alpha = 0.0;

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
  perceptual_trc = babl_trc ("sRGB");
  babl_conversion_new (
    babl_model_from_id (BABL_MODEL_GRAY_NONLINEAR),
    babl_model_from_id (BABL_RGBA),
    "planar", gray_nonlinear_to_rgb,
    NULL
  );

  babl_conversion_new (
    babl_model_from_id (BABL_RGBA),
    babl_model_from_id (BABL_MODEL_GRAY_NONLINEAR),
    "planar", rgb_to_gray_nonlinear,
    NULL
  );

  babl_conversion_new (
    babl_model_from_id (BABL_MODEL_GRAY_NONLINEAR_ALPHA),
    babl_model_from_id (BABL_RGBA),
    "planar", gray_nonlinear_to_rgb,
    NULL
  );

  babl_conversion_new (
    babl_model_from_id (BABL_RGBA),
    babl_model_from_id (BABL_MODEL_GRAY_NONLINEAR_ALPHA),
    "planar", rgb_to_gray_nonlinear,
    NULL
  );


  babl_conversion_new (
    babl_model_from_id (BABL_MODEL_GRAY_NONLINEAR_ALPHA_PREMULTIPLIED),
    babl_model_from_id (BABL_RGBA),
    "linear", gray_nonlinear_premultiplied2rgba,
    NULL
  );

  babl_conversion_new (
    babl_model_from_id (BABL_RGBA),
    babl_model_from_id (BABL_MODEL_GRAY_NONLINEAR_ALPHA_PREMULTIPLIED),
    "linear", rgba2gray_nonlinear_premultiplied,
    NULL
  );



  babl_conversion_new (
    babl_model_from_id (BABL_MODEL_GRAY_PERCEPTUAL),
    babl_model_from_id (BABL_RGBA),
    "planar", gray_perceptual_to_rgb,
    NULL
  );

  babl_conversion_new (
    babl_model_from_id (BABL_RGBA),
    babl_model_from_id (BABL_MODEL_GRAY_PERCEPTUAL),
    "planar", rgb_to_gray_perceptual,
    NULL
  );

  babl_conversion_new (
    babl_model_from_id (BABL_MODEL_GRAY_PERCEPTUAL_ALPHA),
    babl_model_from_id (BABL_RGBA),
    "planar", gray_perceptual_to_rgb,
    NULL
  );

  babl_conversion_new (
    babl_model_from_id (BABL_RGBA),
    babl_model_from_id (BABL_MODEL_GRAY_PERCEPTUAL_ALPHA),
    "planar", rgb_to_gray_perceptual,
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
    babl_component_from_id (BABL_GRAY_LINEAR),
    babl_component_from_id (BABL_ALPHA),
    NULL);
  babl_format_new (
    babl_model_from_id (BABL_GRAY_ALPHA_PREMULTIPLIED),
    babl_type_from_id (BABL_HALF),
    babl_component_from_id (BABL_GRAY_LINEAR_MUL_ALPHA),
    babl_component_from_id (BABL_ALPHA),
    NULL);
  babl_format_new (
    babl_model_from_id (BABL_GRAY),
    babl_type_from_id (BABL_HALF),
    babl_component_from_id (BABL_GRAY_LINEAR),
    NULL);
  babl_format_new (
    babl_model_from_id (BABL_MODEL_GRAY_NONLINEAR_ALPHA),
    babl_type_from_id (BABL_HALF),
    babl_component_from_id (BABL_GRAY_NONLINEAR),
    babl_component_from_id (BABL_ALPHA),
    NULL);
  babl_format_new (
    babl_model_from_id (BABL_MODEL_GRAY_NONLINEAR_ALPHA_PREMULTIPLIED),
    babl_type_from_id (BABL_HALF),
    babl_component_from_id (BABL_GRAY_NONLINEAR_MUL_ALPHA),
    babl_component_from_id (BABL_ALPHA),
    NULL);
  babl_format_new (
    babl_model_from_id (BABL_MODEL_GRAY_NONLINEAR),
    babl_type_from_id (BABL_HALF),
    babl_component_from_id (BABL_GRAY_NONLINEAR),
    NULL);
  /***********/

  babl_format_new (
    babl_model_from_id (BABL_GRAY_ALPHA),
    babl_type ("u15"),
    babl_component_from_id (BABL_GRAY_LINEAR),
    babl_component_from_id (BABL_ALPHA),
    NULL);
  babl_format_new (
    babl_model_from_id (BABL_GRAY_ALPHA_PREMULTIPLIED),
    babl_type ("u15"),
    babl_component_from_id (BABL_GRAY_LINEAR_MUL_ALPHA),
    babl_component_from_id (BABL_ALPHA),
    NULL);
  babl_format_new (
    babl_model_from_id (BABL_GRAY),
    babl_type ("u15"),
    babl_component_from_id (BABL_GRAY_LINEAR),
    NULL);
  babl_format_new (
    babl_model_from_id (BABL_MODEL_GRAY_NONLINEAR_ALPHA),
    babl_type ("u15"),
    babl_component_from_id (BABL_GRAY_NONLINEAR),
    babl_component_from_id (BABL_ALPHA),
    NULL);
  babl_format_new (
    babl_model_from_id (BABL_MODEL_GRAY_NONLINEAR_ALPHA_PREMULTIPLIED),
    babl_type ("u15"),
    babl_component_from_id (BABL_GRAY_NONLINEAR_MUL_ALPHA),
    babl_component_from_id (BABL_ALPHA),
    NULL);
  babl_format_new (
    babl_model_from_id (BABL_MODEL_GRAY_NONLINEAR),
    babl_type ("u15"),
    babl_component_from_id (BABL_GRAY_NONLINEAR),
    NULL);

  babl_format_new (
    babl_model_from_id (BABL_GRAY_ALPHA),
    babl_type_from_id (BABL_U32),
    babl_component_from_id (BABL_GRAY_LINEAR),
    babl_component_from_id (BABL_ALPHA),
    NULL);
  babl_format_new (
    babl_model_from_id (BABL_GRAY_ALPHA_PREMULTIPLIED),
    babl_type_from_id (BABL_U32),
    babl_component_from_id (BABL_GRAY_LINEAR_MUL_ALPHA),
    babl_component_from_id (BABL_ALPHA),
    NULL);
  babl_format_new (
    babl_model_from_id (BABL_GRAY),
    babl_type_from_id (BABL_U32),
    babl_component_from_id (BABL_GRAY_LINEAR),
    NULL);
  babl_format_new (
    babl_model_from_id (BABL_MODEL_GRAY_NONLINEAR_ALPHA),
    babl_type_from_id (BABL_U32),
    babl_component_from_id (BABL_GRAY_NONLINEAR),
    babl_component_from_id (BABL_ALPHA),
    NULL);
  babl_format_new (
    babl_model_from_id (BABL_MODEL_GRAY_NONLINEAR_ALPHA_PREMULTIPLIED),
    babl_type_from_id (BABL_U32),
    babl_component_from_id (BABL_GRAY_NONLINEAR_MUL_ALPHA),
    babl_component_from_id (BABL_ALPHA),
    NULL);
  babl_format_new (
    babl_model_from_id (BABL_MODEL_GRAY_NONLINEAR),
    babl_type_from_id (BABL_U32),
    babl_component_from_id (BABL_GRAY_NONLINEAR),
    NULL);
}

/********** float versions ***********/

static void
rgba_to_graya_float (Babl *conversion,
                     char *src,
                     char *dst,
                     long  n)
{
  const Babl *space = babl_conversion_get_source_space (conversion);
  float RGB_LUMINANCE_RED   = space->space.RGBtoXYZf[3];
  float RGB_LUMINANCE_GREEN = space->space.RGBtoXYZf[4];
  float RGB_LUMINANCE_BLUE  = space->space.RGBtoXYZf[5];

  while (n--)
    {
      float red, green, blue;
      float luminance, alpha;

      red   = ((float *) src)[0];
      green = ((float *) src)[1];
      blue  = ((float *) src)[2];
      alpha = ((float *) src)[3];

      luminance = red * RGB_LUMINANCE_RED +
                  green * RGB_LUMINANCE_GREEN +
                  blue * RGB_LUMINANCE_BLUE;

      ((float *) dst)[0] = luminance;
      ((float *) dst)[1] = alpha;

      src += sizeof (float) * 4;
      dst += sizeof (float) * 2;
    }
}

static void
rgba_to_gray_float (Babl *conversion,
                    char *src,
                    char *dst,
                    long  n)
{
  const Babl *space = babl_conversion_get_source_space (conversion);
  float RGB_LUMINANCE_RED   = space->space.RGBtoXYZf[3];
  float RGB_LUMINANCE_GREEN = space->space.RGBtoXYZf[4];
  float RGB_LUMINANCE_BLUE  = space->space.RGBtoXYZf[5];

  while (n--)
    {
      float red, green, blue;
      float luminance;

      red   = ((float *) src)[0];
      green = ((float *) src)[1];
      blue  = ((float *) src)[2];

      luminance = red * RGB_LUMINANCE_RED +
                  green * RGB_LUMINANCE_GREEN +
                  blue * RGB_LUMINANCE_BLUE;

      ((float *) dst)[0] = luminance;

      src += sizeof (float) * 4;
      dst += sizeof (float) * 1;
    }
}

static void
rgb_to_gray_nonlinear_float (Babl  *conversion,
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
  float RGB_LUMINANCE_RED   = space->space.RGBtoXYZf[3];
  float RGB_LUMINANCE_GREEN = space->space.RGBtoXYZf[4];
  float RGB_LUMINANCE_BLUE  = space->space.RGBtoXYZf[5];

  BABL_PLANAR_SANITY
  while (n--)
    {
      float red, green, blue;
      float luminance, alpha;

      red   = *(float *) src[0];
      green = *(float *) src[1];
      blue  = *(float *) src[2];
      if (src_bands > 3)
        alpha = *(float *) src[3];
      else
        alpha = 1.0f;

      luminance = red   * RGB_LUMINANCE_RED +
                  green * RGB_LUMINANCE_GREEN +
                  blue  * RGB_LUMINANCE_BLUE;
      *(float *) dst[0] = babl_trc_from_linear (trc, luminance);

      if (dst_bands == 2)
        *(float *) dst[1] = alpha;

      BABL_PLANAR_STEP
    }
}


static void
gray_nonlinear_to_rgb_float (Babl *conversion,
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
      float luminance;
      float red, green, blue;
      float alpha;

      luminance = babl_trc_to_linear (trc, *(float *) src[0]);
      red       = luminance;
      green     = luminance;
      blue      = luminance;
      if (src_bands > 1)
        alpha = *(float *) src[1];
      else
        alpha = 1.0f;

      *(float *) dst[0] = red;
      *(float *) dst[1] = green;
      *(float *) dst[2] = blue;

      if (dst_bands > 3)
        *(float *) dst[3] = alpha;

      BABL_PLANAR_STEP
    }
}

static void
rgb_to_gray_perceptual_float (Babl  *conversion,
                              int    src_bands,
                              char **src,
                              int   *src_pitch,
                              int    dst_bands,
                              char **dst,
                              int   *dst_pitch,
                              long   n)
{
  const Babl *space = babl_conversion_get_destination_space (conversion);
  const Babl *trc = perceptual_trc;
  float RGB_LUMINANCE_RED   = space->space.RGBtoXYZf[3];
  float RGB_LUMINANCE_GREEN = space->space.RGBtoXYZf[4];
  float RGB_LUMINANCE_BLUE  = space->space.RGBtoXYZf[5];

  BABL_PLANAR_SANITY
  while (n--)
    {
      float red, green, blue;
      float luminance, alpha;

      red   = *(float *) src[0];
      green = *(float *) src[1];
      blue  = *(float *) src[2];
      if (src_bands > 3)
        alpha = *(float *) src[3];
      else
        alpha = 1.0f;

      luminance = red   * RGB_LUMINANCE_RED +
                  green * RGB_LUMINANCE_GREEN +
                  blue  * RGB_LUMINANCE_BLUE;
      *(float *) dst[0] = babl_trc_from_linear (trc, luminance);

      if (dst_bands == 2)
        *(float *) dst[1] = alpha;

      BABL_PLANAR_STEP
    }
}

static void
gray_perceptual_to_rgb_float (Babl  *conversion,
                              int    src_bands,
                              char **src,
                              int   *src_pitch,
                              int    dst_bands,
                              char **dst,
                              int   *dst_pitch,
                              long   n)
{
  const Babl *trc = perceptual_trc;

  BABL_PLANAR_SANITY
  while (n--)
    {
      float luminance;
      float red, green, blue;
      float alpha;

      luminance = babl_trc_to_linear (trc, *(float *) src[0]);
      red       = luminance;
      green     = luminance;
      blue      = luminance;
      if (src_bands > 1)
        alpha = *(float *) src[1];
      else
        alpha = 1.0f;

      *(float *) dst[0] = red;
      *(float *) dst[1] = green;
      *(float *) dst[2] = blue;

      if (dst_bands > 3)
        *(float *) dst[3] = alpha;

      BABL_PLANAR_STEP
    }
}


static void
graya_to_rgba_float (Babl *conversion,
                     char *src,
                     char *dst,
                     long  n)
{
  while (n--)
    {
      float luminance;
      float red, green, blue;
      float alpha;

      luminance = ((float *) src)[0];
      alpha     = ((float *) src)[1];
      red       = luminance;
      green     = luminance;
      blue      = luminance;

      ((float *) dst)[0] = red;
      ((float *) dst)[1] = green;
      ((float *) dst)[2] = blue;
      ((float *) dst)[3] = alpha;

      src += sizeof (float) * 2;
      dst += sizeof (float) * 4;
    }
}


static void
gray_to_rgba_float (Babl *conversion,
                    char *src,
                    char *dst,
                    long  n)
{
  while (n--)
    {
      float luminance;
      float red, green, blue;

      luminance = ((float *) src)[0];
      red       = luminance;
      green     = luminance;
      blue      = luminance;

      ((float *) dst)[0] = red;
      ((float *) dst)[1] = green;
      ((float *) dst)[2] = blue;
      ((float *) dst)[3] = 1.0;

      src += sizeof (float) * 1;
      dst += sizeof (float) * 4;
    }
}

static void
gray_alpha_premultiplied_to_rgba_float (Babl  *conversion,
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
      float luminance = *(float *) src[0];
      float alpha;
      alpha = *(float *) src[1];
      if (alpha == 0)
        luminance = 0.0f;
      else
      {
        luminance = luminance / alpha;
        if (alpha == BABL_ALPHA_FLOOR || alpha == -BABL_ALPHA_FLOOR)
          alpha = 0.0f;
      }

      *(float *) dst[0] = luminance;
      *(float *) dst[1] = luminance;
      *(float *) dst[2] = luminance;
      *(float *) dst[3] = alpha;
      BABL_PLANAR_STEP
    }
}


static void
rgba_to_gray_alpha_premultiplied_float (Babl  *conversion,
                                        int    src_bands,
                                        char **src,
                                        int   *src_pitch,
                                        int    dst_bands,
                                        char **dst,
                                        int   *dst_pitch,
                                        long   n)
{
  const Babl *space = babl_conversion_get_source_space (conversion);
  float RGB_LUMINANCE_RED   = space->space.RGBtoXYZf[3];
  float RGB_LUMINANCE_GREEN = space->space.RGBtoXYZf[4];
  float RGB_LUMINANCE_BLUE  = space->space.RGBtoXYZf[5];

  BABL_PLANAR_SANITY;
  assert (src_bands == 4);
  assert (dst_bands == 2);

  while (n--)
    {
      float red   = *(float *) src[0];
      float green = *(float *) src[1];
      float blue  = *(float *) src[2];
      float luminance;
      float alpha = *(float *) src[3];
      if (alpha <= BABL_ALPHA_FLOOR)
      {
        if (alpha >= 0.0f)
          alpha = BABL_ALPHA_FLOOR;
         else if (alpha >= -BABL_ALPHA_FLOOR)
           alpha = -BABL_ALPHA_FLOOR;
      }

      luminance = red * RGB_LUMINANCE_RED +
                  green * RGB_LUMINANCE_GREEN +
                  blue * RGB_LUMINANCE_BLUE;

      luminance *= alpha;

      *(float *) dst[0] = luminance;
      *(float *) dst[1] = alpha;
      BABL_PLANAR_STEP
    }
}

static void
non_premultiplied_to_premultiplied_float (Babl  *conversion,
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
      int    band;
      float alpha = *(float *) src[src_bands-1];
      if (alpha < BABL_ALPHA_FLOOR)
      {
        int non_zero_components = 0;
        if (alpha >= 0.0f)
          alpha = BABL_ALPHA_FLOOR;
        else if (alpha >= -BABL_ALPHA_FLOOR)
          alpha = -BABL_ALPHA_FLOOR;

        for (band = 0; band < src_bands - 1; band++)
        {
          if (*(float *) src[band] != 0.0f)
            non_zero_components++;
        }
        if (non_zero_components)
          alpha = 0.0f;

      }

      for (band = 0; band < src_bands - 1; band++)
        {
          *(float *) dst[band] = *(float *) src[band] * alpha;
        }
      *(float *) dst[dst_bands - 1] = alpha;

      BABL_PLANAR_STEP
    }
}

static void
premultiplied_to_non_premultiplied_float (Babl  *conversion,
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
      int    band;
      float alpha;
      alpha = *(float *) src[src_bands-1];

      for (band = 0; band < src_bands - 1; band++)
        {
          if (alpha == 0.0f)
            *(float *) dst[band] = 0.0f;
          else
            *(float *) dst[band] = *(float *) src[band] / alpha;
        }
      if (alpha == BABL_ALPHA_FLOOR || alpha == -BABL_ALPHA_FLOOR)
        alpha = 0.0f;
      *(float *) dst[dst_bands - 1] = alpha;

      BABL_PLANAR_STEP
    }
}

static void
rgba2gray_nonlinear_premultiplied_float (Babl *conversion,
                                         char *src,
                                         char *dst,
                                         long  n)
{
  const Babl *space = babl_conversion_get_destination_space (conversion);
  const Babl *trc = space->space.trc[0];
  float RGB_LUMINANCE_RED   = space->space.RGBtoXYZf[3];
  float RGB_LUMINANCE_GREEN = space->space.RGBtoXYZf[4];
  float RGB_LUMINANCE_BLUE  = space->space.RGBtoXYZf[5];

  while (n--)
    {
      float red   = ((float *) src)[0];
      float green = ((float *) src)[1];
      float blue  = ((float *) src)[2];
      float luminance;
      float luma;
      float alpha = ((float *) src)[3];
      if (alpha < BABL_ALPHA_FLOOR)
      {
        if (alpha >= 0.0f)
          alpha = BABL_ALPHA_FLOOR;
        else if (alpha >= -BABL_ALPHA_FLOOR)
          alpha = -BABL_ALPHA_FLOOR;
      }

      luminance = red * RGB_LUMINANCE_RED +
                  green * RGB_LUMINANCE_GREEN +
                  blue * RGB_LUMINANCE_BLUE;
      luma = babl_trc_from_linear (trc, luminance);

      ((float *) dst)[0] = luma * alpha;
      ((float *) dst)[1] = alpha;

      src += 4 * sizeof (float);
      dst += 2 * sizeof (float);
    }
}

static void
gray_nonlinear_premultiplied2rgba_float (Babl *conversion,
                                         char *src,
                                         char *dst,
                                         long  n)
{
  const Babl *space = babl_conversion_get_destination_space (conversion);
  const Babl *trc = space->space.trc[0];

  while (n--)
    {
      float luma  = ((float *) src)[0];
      float luminance;
      float alpha;
      alpha = ((float *) src)[1];
      if (alpha == 0.0f)
        luma = 0.0f;
      else
        luma = luma / alpha;

      luminance = babl_trc_to_linear (trc, luma);

      if (alpha == BABL_ALPHA_FLOOR || alpha == -BABL_ALPHA_FLOOR)
        alpha = 0.0f;

      ((float *) dst)[0] = luminance;
      ((float *) dst)[1] = luminance;
      ((float *) dst)[2] = luminance;
      ((float *) dst)[3] = alpha;

      src += 2 * sizeof (float);
      dst += 4 * sizeof (float);
    }
}

static void init_single_precision (void)
{
  babl_format_new (
    babl_model ("Y"),
    babl_type_from_id (BABL_FLOAT),
    babl_component ("Y"),
    NULL);
  babl_format_new (
    babl_model ("Y'"),
    babl_type_from_id (BABL_FLOAT),
    babl_component ("Y'"),
    NULL);
  babl_format_new (
    babl_model ("Y~"),
    babl_type_from_id (BABL_FLOAT),
    babl_component ("Y~"),
    NULL);


  babl_format_new (
    babl_model ("YA"),
    babl_type_from_id (BABL_FLOAT),
    babl_component ("Y"),
    babl_component ("A"),
    NULL);
  babl_format_new (
    babl_model ("Y'A"),
    babl_type_from_id (BABL_FLOAT),
    babl_component ("Y'"),
    babl_component ("A"),
    NULL);
  babl_format_new (
    babl_model ("Y~A"),
    babl_type_from_id (BABL_FLOAT),
    babl_component ("Y~"),
    babl_component ("A"),
    NULL);


  babl_format_new (
    babl_model ("YaA"),
    babl_type_from_id (BABL_FLOAT),
    babl_component ("Ya"),
    babl_component ("A"),
    NULL);
  babl_format_new (
    babl_model ("Y'aA"),
    babl_type_from_id (BABL_FLOAT),
    babl_component ("Y'a"),
    babl_component ("A"),
    NULL);

  babl_conversion_new (
    babl_format ("Y' float"),
    babl_format ("RGBA float"),
    "planar", gray_nonlinear_to_rgb_float,
    NULL
  );

  babl_conversion_new (
    babl_format ("RGBA float"),
    babl_format ("Y' float"),
    "planar", rgb_to_gray_nonlinear_float,
    NULL
  );

  babl_conversion_new (
    babl_format ("Y'A float"),
    babl_format ("RGBA float"),
    "planar", gray_nonlinear_to_rgb_float,
    NULL
  );

  babl_conversion_new (
    babl_format ("RGBA float"),
    babl_format ("Y'A float"),
    "planar", rgb_to_gray_nonlinear_float,
    NULL
  );


  babl_conversion_new (
    babl_format ("Y'aA float"),
    babl_format ("RGBA float"),
    "linear", gray_nonlinear_premultiplied2rgba_float,
    NULL
  );

  babl_conversion_new (
    babl_format ("RGBA float"),
    babl_format ("Y'aA float"),
    "linear", rgba2gray_nonlinear_premultiplied_float,
    NULL
  );



  babl_conversion_new (
    babl_format ("Y~ float"),
    babl_format ("RGBA float"),
    "planar", gray_perceptual_to_rgb_float,
    NULL
  );

  babl_conversion_new (
    babl_format ("RGBA float"),
    babl_format ("Y~ float"),
    "planar", rgb_to_gray_perceptual_float,
    NULL
  );

  babl_conversion_new (
    babl_format ("Y~A float"),
    babl_format ("RGBA float"),
    "planar", gray_perceptual_to_rgb_float,
    NULL
  );

  babl_conversion_new (
    babl_format ("RGBA float"),
    babl_format ("Y~A float"),
    "planar", rgb_to_gray_perceptual_float,
    NULL
  );


  babl_conversion_new (
    babl_format ("Y float"),
    babl_format ("RGBA float"),
    "linear", gray_to_rgba_float,
    NULL
  );

  babl_conversion_new (
    babl_format ("YA float"),
    babl_format ("RGBA float"),
    "linear", graya_to_rgba_float,
    NULL
  );

  babl_conversion_new (
    babl_format ("RGBA float"),
    babl_format ("YA float"),
    "linear", rgba_to_graya_float,
    NULL
  );

  babl_conversion_new (
    babl_format ("RGBA float"),
    babl_format ("Y float"),
    "linear", rgba_to_gray_float,
    NULL
  );

  babl_conversion_new (
    babl_format ("YA float"),
    babl_format ("YaA float"),
    "planar", non_premultiplied_to_premultiplied_float,
    NULL
  );

  babl_conversion_new (
    babl_format ("YaA float"),
    babl_format ("YA float"),
    "planar", premultiplied_to_non_premultiplied_float,
    NULL
  );

  babl_conversion_new (
    babl_format ("YaA float"),
    babl_format ("RGBA float"),
    "planar", gray_alpha_premultiplied_to_rgba_float,
    NULL
  );

  babl_conversion_new (
    babl_format ("RGBA float"),
    babl_format ("YaA float"),
    "planar", rgba_to_gray_alpha_premultiplied_float,
    NULL
  );
}
