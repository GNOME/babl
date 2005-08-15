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

#include "babl.h"
#include "util.h"
#include "rgb-constants.h"
#include "math.h"

static void components  (void);
static void models      (void);
static void conversions (void);

void babl_base_model_grayscale (void)
{
  components  ();
  models      ();
  conversions ();
}

static void
components (void)
{
  babl_component_new (
   "luminance", 
   "id",    BABL_LUMINANCE,
   "luma",
   NULL);
}

static void
models (void)
{
  babl_model_new (
    "grayscale",
    "id", BABL_GRAYSCALE,
    babl_component_id (BABL_LUMINANCE),
    NULL);

  babl_model_new (
    "grayscale-gamma2.2",
    "id", BABL_GRAYSCALE_GAMMA_2_2,
    babl_component_id (BABL_LUMINANCE),
    NULL);

  babl_model_new (
    "grayscale-alpha",
    "id", BABL_GRAYSCALE_ALPHA,
    babl_component_id (BABL_LUMINANCE),
    babl_component_id (BABL_ALPHA),
    NULL);

  babl_model_new (
    "grayscale-alpha-premultiplied",
    "id", BABL_GRAYSCALE_ALPHA_PREMULTIPLIED,
    babl_component_id (BABL_LUMINANCE_MUL_ALPHA),
    babl_component_id (BABL_ALPHA),
    NULL);

}


static void
rgb_to_grayscale (int    src_bands,
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
      double red, green, blue;
      double luminance, alpha;

      red   = *(double *)src[0];
      green = *(double *)src[1];
      blue  = *(double *)src[2];
      if (src_bands>3)
        alpha = *(double *)src[3];
      else
        alpha = 1.0;

      luminance  = red   * RGB_LUMINANCE_RED +
                   green * RGB_LUMINANCE_GREEN +
                   blue  * RGB_LUMINANCE_BLUE;
      *(double*)dst[0] = luminance;

      if (dst_bands==2)
        *(double*)dst[1] = alpha;

      BABL_PLANAR_STEP
    }
}


static void
rgb_to_grayscale_2_2 (int    src_bands,
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
      double red, green, blue;
      double luminance, alpha;

      red   = *(double *) src[0];
      green = *(double *) src[1];
      blue  = *(double *) src[2];
      if (src_bands>3)
        alpha = *(double *)src[3];
      else
        alpha = 1.0;

      luminance  = red   * RGB_LUMINANCE_RED +
                   green * RGB_LUMINANCE_GREEN +
                   blue  * RGB_LUMINANCE_BLUE;
      *(double*)dst[0] = pow (luminance, 2.2);

      if (dst_bands==2)
        *(double*)dst[1] = alpha;

      BABL_PLANAR_STEP
    }
}


static void
grayscale_2_2_to_rgb (int    src_bands,
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
      double luminance;
      double red, green, blue;
      double alpha;

      luminance = pow (*(double *)src[0], (1.0F/2.2F));
      red       = luminance;
      green     = luminance;
      blue      = luminance;
      if (src_bands > 1)
        alpha = *(double *)src[1];
      else
        alpha     = 1.0;

      *(double*)dst[0] = red;
      *(double*)dst[1] = green;
      *(double*)dst[2] = blue;

      if (dst_bands>3)
        *(double*)dst[3] = alpha;

      BABL_PLANAR_STEP
    }
}



static void
grayscale_to_rgb (int    src_bands,
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
      double luminance;
      double red, green, blue;
      double alpha;

      luminance = *(double *)src[0];
      red       = luminance;
      green     = luminance;
      blue      = luminance;
      if (src_bands > 1)
        alpha = *(double *)src[1];
      else
        alpha     = 1.0;

      *(double*)dst[0] = red;
      *(double*)dst[1] = green;
      *(double*)dst[2] = blue;

      if (dst_bands>3)
        *(double*)dst[3] = alpha;

      BABL_PLANAR_STEP
    }
}

static void
grayscale_alpha_premultiplied_to_rgba (int    src_bands,
                                       void **src,
                                       int   *src_pitch,
                                       int    dst_bands,
                                       void **dst,
                                       int   *dst_pitch,
                                       int    n)
{
  BABL_PLANAR_SANITY
  assert (src_bands == 2);
  assert (dst_bands == 4);

  while (n--)
    {
      double luminance = *(double *)src[0];
      double alpha     = *(double *)src[1];

      if (alpha > 0.001)
        {
          luminance = luminance / alpha;
        }
      else
        {
          luminance = 0.0;
        }
      
      *(double*)dst[0] = luminance;
      *(double*)dst[1] = luminance;
      *(double*)dst[2] = luminance;
      *(double*)dst[3] = alpha;
      BABL_PLANAR_STEP
    }
}


static void
rgba_to_grayscale_alpha_premultiplied (int    src_bands,
                                       void **src,
                                       int   *src_pitch,
                                       int    dst_bands,
                                       void **dst,
                                       int   *dst_pitch,
                                       int    n)
{
  BABL_PLANAR_SANITY;
  assert (src_bands == 4);
  assert (dst_bands == 2);

  while (n--)
    {
      double red       = *(double *)src[0];
      double green     = *(double *)src[1];
      double blue      = *(double *)src[2];
      double alpha     = *(double *)src[3];
      double luminance;

      luminance  = red   * RGB_LUMINANCE_RED +
                   green * RGB_LUMINANCE_GREEN +
                   blue  * RGB_LUMINANCE_BLUE;

      luminance *= alpha;
      
      *(double*)dst[0] = luminance;
      *(double*)dst[2] = alpha;
      BABL_PLANAR_STEP
    }
}

static void
non_premultiplied_to_premultiplied (int    src_bands,
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
      double alpha;
      int band;

      alpha = *(double *)src[src_bands-1];
      for (band=0; band<src_bands-1;band++)
        {
          *(double*)dst[band] = *(double*) src[band] * alpha;
        }
      *(double*)dst[dst_bands-1] = alpha;

      BABL_PLANAR_STEP
    }
}

static void
premultiplied_to_non_premultiplied (int    src_bands,
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
      double alpha;
      int band;

      alpha = *(double *)src[src_bands-1];
      for (band=0; band<src_bands-1;band++)
        {
          if (alpha>0.001)
            {
              *(double*)dst[band] = *(double*) src[band] / alpha;
            }
          else
            {
              *(double*)dst[band] = 0.001;
            }
        }
      *(double*)dst[dst_bands-1] = alpha;

      BABL_PLANAR_STEP
    }
}

static void
conversions (void)
{
  babl_conversion_new (
    "babl-base: grayscale-gamma2.2 to rgba",
    "source",      babl_model_id (BABL_GRAYSCALE_GAMMA_2_2),
    "destination", babl_model_id (BABL_RGBA),
    "planar",      grayscale_2_2_to_rgb,
    NULL
  );

  babl_conversion_new (
    "babl-base: rgba to grayscale-gamma2.2",
    "source",      babl_model_id (BABL_RGBA),
    "destination", babl_model_id (BABL_GRAYSCALE_GAMMA_2_2),
    "planar",      rgb_to_grayscale_2_2,
    NULL
  );

  babl_conversion_new (
    "babl-base: grayscale to rgba",
    "source",      babl_model_id (BABL_GRAYSCALE),
    "destination", babl_model_id (BABL_RGBA),
    "planar",      grayscale_to_rgb,
    NULL
  );

  babl_conversion_new (
    "babl-base: grayscale to rgb",
    "source",      babl_model_id (BABL_GRAYSCALE),
    "destination", babl_model_id (BABL_RGB),
    "planar",      grayscale_to_rgb,
    NULL
  );

  babl_conversion_new (
    "babl-base: grayscale-alpha to rgba",
    "source",      babl_model_id (BABL_GRAYSCALE_ALPHA),
    "destination", babl_model_id (BABL_RGBA),
    "planar",      grayscale_to_rgb,
    NULL
  );

  babl_conversion_new (
    "babl-base: grayscale-alpha to rgb",
    "source",      babl_model_id (BABL_GRAYSCALE_ALPHA),
    "destination", babl_model_id (BABL_RGB),
    "planar",      grayscale_to_rgb,
    NULL
  );

  babl_conversion_new (
    "babl-base: rgba to grayscale-alpha",
    "source",      babl_model_id (BABL_RGBA),
    "destination", babl_model_id (BABL_GRAYSCALE_ALPHA),
    "planar",      rgb_to_grayscale,
    NULL
  );

  babl_conversion_new (
    "babl-base: rgba to grayscale",
    "source",      babl_model_id (BABL_RGBA),
    "destination", babl_model_id (BABL_GRAYSCALE),
    "planar",      rgb_to_grayscale,
    NULL
  );

  babl_conversion_new (
    "babl-base: rgb to grayscale-alpha",
    "source",      babl_model_id (BABL_RGB),
    "destination", babl_model_id (BABL_GRAYSCALE_ALPHA),
    "planar",      rgb_to_grayscale,
    NULL
  );

  babl_conversion_new (
    "babl-base: rgb to grayscale",
    "source",      babl_model_id (BABL_RGB),
    "destination", babl_model_id (BABL_GRAYSCALE),
    "planar",      rgb_to_grayscale,
    NULL
  );

  babl_conversion_new (
    "babl-base: grayscale-alpha to grayscale-alpha-premultiplied",
    "source",      babl_model_id (BABL_GRAYSCALE_ALPHA),
    "destination", babl_model_id (BABL_GRAYSCALE_ALPHA_PREMULTIPLIED),
    "planar",      non_premultiplied_to_premultiplied,
    NULL
  );

  babl_conversion_new (
    "babl-base: grayscale-alpha-premultuplied to grayscale-alpha",
    "source",      babl_model_id (BABL_GRAYSCALE_ALPHA_PREMULTIPLIED),
    "destination", babl_model_id (BABL_GRAYSCALE_ALPHA),
    "planar",      premultiplied_to_non_premultiplied,
    NULL
  );

    babl_conversion_new (
    "babl-base: grayscale-alpha-premultiplied to rgba",
    "source",      babl_model_id (BABL_GRAYSCALE_ALPHA_PREMULTIPLIED),
    "destination", babl_model_id (BABL_RGBA),
    "planar",      grayscale_alpha_premultiplied_to_rgba,
    NULL
  );

  babl_conversion_new (
    "babl-base: rgba to grayscale-alpha-premultiplied",
    "source",      babl_model_id (BABL_RGBA),
    "destination", babl_model_id (BABL_GRAYSCALE_ALPHA_PREMULTIPLIED),
    "planar",      rgba_to_grayscale_alpha_premultiplied,
    NULL
  );
}
