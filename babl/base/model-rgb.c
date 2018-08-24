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
#include <math.h>

#include "babl-internal.h"
#include "babl-classes.h"
#include "babl-ids.h"
#include "babl-base.h"

static void models (void);
static void components (void);
static void conversions (void);
static void formats (void);

void
babl_base_model_rgb (void)
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
    "Ra",
    "id", BABL_RED_MUL_ALPHA,
    "luma",
    "chroma",
    "alpha",
    NULL);
  babl_component_new (
    "Ga",
    "id", BABL_GREEN_MUL_ALPHA,
    "luma",
    "chroma",
    "alpha",
    NULL);
  babl_component_new (
    "Ba",
    "id", BABL_BLUE_MUL_ALPHA,
    "luma",
    "chroma",
    "alpha",
    NULL);

  babl_component_new (
    "R'",
    "id", BABL_RED_NONLINEAR,
    "luma",
    "chroma",
    NULL);

  babl_component_new (
    "G'",
    "id", BABL_GREEN_NONLINEAR,
    "luma",
    "chroma",
    NULL);

  babl_component_new (
    "B'",
    "id", BABL_BLUE_NONLINEAR,
    "luma",
    "chroma",
    NULL);

  babl_component_new (
    "R~",
    "id", BABL_RED_PERCEPTUAL,
    "luma",
    "chroma",
    NULL);

  babl_component_new (
    "G~",
    "id", BABL_GREEN_PERCEPTUAL,
    "luma",
    "chroma",
    NULL);

  babl_component_new (
    "B~",
    "id", BABL_BLUE_PERCEPTUAL,
    "luma",
    "chroma",
    NULL);

  babl_component_new (
    "R'a",
    "id", BABL_RED_NONLINEAR_MUL_ALPHA,
    "luma",
    "chroma",
    NULL);

  babl_component_new (
    "G'a",
    "id", BABL_GREEN_NONLINEAR_MUL_ALPHA,
    "luma",
    "chroma",
    NULL);

  babl_component_new (
    "B'a",
    "id", BABL_BLUE_NONLINEAR_MUL_ALPHA,
    "luma",
    "chroma",
    NULL);

  babl_component_new (
    "R~a",
    "id", BABL_RED_PERCEPTUAL_MUL_ALPHA,
    "luma",
    "chroma",
    NULL);

  babl_component_new (
    "G~a",
    "id", BABL_GREEN_PERCEPTUAL_MUL_ALPHA,
    "luma",
    "chroma",
    NULL);

  babl_component_new (
    "B~a",
    "id", BABL_BLUE_PERCEPTUAL_MUL_ALPHA,
    "luma",
    "chroma",
    NULL);
}

static void
models (void)
{
  babl_model_new (
    "id", BABL_RGB,
    babl_component_from_id (BABL_RED),
    babl_component_from_id (BABL_GREEN),
    babl_component_from_id (BABL_BLUE),
    NULL);

  babl_model_new (
    "id", BABL_RGBA_PREMULTIPLIED,
    babl_component_from_id (BABL_RED_MUL_ALPHA),
    babl_component_from_id (BABL_GREEN_MUL_ALPHA),
    babl_component_from_id (BABL_BLUE_MUL_ALPHA),
    babl_component_from_id (BABL_ALPHA),
    NULL);

  babl_model_new (
    "id", BABL_RGB_NONLINEAR,
    babl_component_from_id (BABL_RED_NONLINEAR),
    babl_component_from_id (BABL_GREEN_NONLINEAR),
    babl_component_from_id (BABL_BLUE_NONLINEAR),
    NULL);

  babl_model_new (
    "id", BABL_RGB_PERCEPTUAL,
    babl_component_from_id (BABL_RED_PERCEPTUAL),
    babl_component_from_id (BABL_GREEN_PERCEPTUAL),
    babl_component_from_id (BABL_BLUE_PERCEPTUAL),
    NULL);

  babl_model_new (
    "id", BABL_RGBA_NONLINEAR,
    babl_component_from_id (BABL_RED_NONLINEAR),
    babl_component_from_id (BABL_GREEN_NONLINEAR),
    babl_component_from_id (BABL_BLUE_NONLINEAR),
    babl_component_from_id (BABL_ALPHA),
    NULL);

  babl_model_new (
    "id", BABL_RGBA_PERCEPTUAL,
    babl_component_from_id (BABL_RED_PERCEPTUAL),
    babl_component_from_id (BABL_GREEN_PERCEPTUAL),
    babl_component_from_id (BABL_BLUE_PERCEPTUAL),
    babl_component_from_id (BABL_ALPHA),
    NULL);

  babl_model_new (
    "id", BABL_RGBA_NONLINEAR_PREMULTIPLIED,
    babl_component_from_id (BABL_RED_NONLINEAR_MUL_ALPHA),
    babl_component_from_id (BABL_GREEN_NONLINEAR_MUL_ALPHA),
    babl_component_from_id (BABL_BLUE_NONLINEAR_MUL_ALPHA),
    babl_component_from_id (BABL_ALPHA),
    NULL);

  babl_model_new (
    "id", BABL_RGBA_PERCEPTUAL_PREMULTIPLIED,
    babl_component_from_id (BABL_RED_PERCEPTUAL_MUL_ALPHA),
    babl_component_from_id (BABL_GREEN_PERCEPTUAL_MUL_ALPHA),
    babl_component_from_id (BABL_BLUE_PERCEPTUAL_MUL_ALPHA),
    babl_component_from_id (BABL_ALPHA),
    NULL);
}

static void
copy_strip_1 (Babl  *conversion,
              int    src_bands,
              char **src,
              int   *src_pitch,
              int    dst_bands,
              char **dst,
              int   *dst_pitch,
              long   samples)
{
  long n = samples;

  BABL_PLANAR_SANITY
  while (n--)
    {
      int i;

      for (i = 0; i < dst_bands; i++)
        {
          double foo;
          if (i < src_bands)
            foo = *(double *) src[i];
          else
            foo = 1.0;
          *(double *) dst[i] = foo;
        }

      BABL_PLANAR_STEP
    }
}

static void
g3_nonlinear_from_linear (Babl  *conversion,
                          int    src_bands,
                          char **src,
                          int   *src_pitch,
                          int    dst_bands,
                          char **dst,
                          int   *dst_pitch,
                          long   samples)
{
  const Babl *space = babl_conversion_get_destination_space (conversion);
  const Babl **trc  = (void*)space->space.trc;

  long n = samples;

  BABL_PLANAR_SANITY
  while (n--)
    {
      int band;
      for (band = 0; band < 3; band++)
        *(double *) dst[band] = babl_trc_from_linear (trc[band], (*(double *) src[band]));
      for (; band < dst_bands; band++)
        *(double *) dst[band] = *(double *) src[band];

      BABL_PLANAR_STEP
    }
}

static void
g3_nonlinear_to_linear (Babl  *conversion,
                        int    src_bands,
                        char **src,
                        int   *src_pitch,
                        int    dst_bands,
                        char **dst,
                        int   *dst_pitch,
                        long   samples)
{
  const Babl *space = babl_conversion_get_source_space (conversion);
  const Babl **trc  = (void*)space->space.trc;
  long n = samples;

  BABL_PLANAR_SANITY
  while (n--)
    {
      int band;
      for (band = 0; band < 3; band++)
        {
          *(double *) dst[band] = babl_trc_to_linear (trc[band], (*(double *) src[band]));
        }
      for (; band < dst_bands; band++)
        {
          if (band < src_bands)
            *(double *) dst[band] = *(double *) src[band];
          else
            *(double *) dst[band] = 1.0;
        }
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
                                    long   samples)
{
  long n = samples;

  BABL_PLANAR_SANITY
  while (n--)
    {
      double alpha;
      double alpha_used;
      int    band;

      alpha_used = alpha = *(double *) src[src_bands - 1];
      if (alpha < BABL_ALPHA_FLOOR)
         alpha_used = BABL_ALPHA_FLOOR;

      for (band = 0; band < src_bands - 1; band++)
        {
          *(double *) dst[band] = *(double *) src[band] * alpha_used;
        }
      *(double *) dst[dst_bands - 1] = alpha_used;

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
                                    long   samples)
{
  long n = samples;

  BABL_PLANAR_SANITY
  while (n--)
    {
      double alpha;
      double recip_alpha;
      int    band;

      alpha = *(double *) src[src_bands - 1];
      if (alpha == 0.0)
         recip_alpha = 0.0;
      else
      {
        recip_alpha  = 1.0 / alpha;
        if (alpha == BABL_ALPHA_FLOOR)
          alpha = 0.0; // making 0 round-trip to zero, causing discontinuity
      }

      for (band = 0; band < src_bands - 1; band++)
        *(double *) dst[band] = *(double *) src[band] * recip_alpha;
      *(double *) dst[dst_bands - 1] = alpha;

      BABL_PLANAR_STEP
    }
}


static void
rgba2rgba_nonlinear_premultiplied (Babl *conversion,
                                   char *src,
                                   char *dst,
                                   long  samples)
{
  const Babl *space = babl_conversion_get_destination_space (conversion);
  const Babl **trc  = (void*)space->space.trc;
  long n = samples;

  while (n--)
    {
      double alpha, alpha_used;
      alpha_used = alpha = ((double *) src)[3];
      if (alpha < BABL_ALPHA_FLOOR)
         alpha_used = BABL_ALPHA_FLOOR;
      ((double *) dst)[0] = babl_trc_from_linear (trc[0], ((double *) src)[0]) * alpha_used;
      ((double *) dst)[1] = babl_trc_from_linear (trc[1], ((double *) src)[1]) * alpha_used;
      ((double *) dst)[2] = babl_trc_from_linear (trc[2], ((double *) src)[2]) * alpha_used;
      ((double *) dst)[3] = alpha_used;
      src                += 4 * sizeof (double);
      dst                += 4 * sizeof (double);
    }
}


static void
rgba_nonlinear_premultiplied2rgba (Babl *conversion,
                                   char           *src,
                                   char           *dst,
                                   long            samples)
{
  const Babl *space = babl_conversion_get_source_space (conversion);
  const Babl **trc  = (void*)space->space.trc;
  long n = samples;

  while (n--)
    {
      double alpha;
      alpha = ((double *) src)[3];
      if (alpha == 0)
      {
          ((double *) dst)[0] = 0;
          ((double *) dst)[1] = 0;
          ((double *) dst)[2] = 0;
      }
      else
      {
          ((double *) dst)[0] = babl_trc_to_linear (trc[0], ((double *) src)[0] / alpha);
          ((double *) dst)[1] = babl_trc_to_linear (trc[1], ((double *) src)[1] / alpha);
          ((double *) dst)[2] = babl_trc_to_linear (trc[2], ((double *) src)[2] / alpha);
          if (alpha == BABL_ALPHA_FLOOR)
            alpha = 0;
      }
      ((double *) dst)[3] = alpha;

      src += 4 * sizeof (double);
      dst += 4 * sizeof (double);
    }
}


static void
rgba2rgba_nonlinear (Babl *conversion,
                     char *src,
                     char *dst,
                     long  samples)
{
  const Babl *space = babl_conversion_get_destination_space (conversion);
  const Babl **trc  = (void*)space->space.trc;
  long n = samples;

  while (n--)
    {
      double alpha = ((double *) src)[3];
      ((double *) dst)[0] = babl_trc_from_linear (trc[0], ((double *) src)[0]);
      ((double *) dst)[1] = babl_trc_from_linear (trc[1], ((double *) src)[1]);
      ((double *) dst)[2] = babl_trc_from_linear (trc[2], ((double *) src)[2]);
      ((double *) dst)[3] = alpha;
      src                += 4 * sizeof (double);
      dst                += 4 * sizeof (double);
    }
}


static void
rgba_nonlinear2rgba (Babl *conversion,
                     char *src,
                     char *dst,
                     long  samples)
{
  const Babl *space = babl_conversion_get_source_space (conversion);
  const Babl **trc  = (void*)(space->space.trc);
  long n = samples;

  while (n--)
    {
      double alpha = ((double *) src)[3];
      ((double *) dst)[0] = babl_trc_to_linear (trc[0], ((double *) src)[0]);
      ((double *) dst)[1] = babl_trc_to_linear (trc[1], ((double *) src)[1]);
      ((double *) dst)[2] = babl_trc_to_linear (trc[2], ((double *) src)[2]);
      ((double *) dst)[3] = alpha;

      src += 4 * sizeof (double);
      dst += 4 * sizeof (double);
    }
}


static const Babl *perceptual_trc = NULL;

static void
g3_perceptual_from_linear (Babl  *conversion,
                          int    src_bands,
                          char **src,
                          int   *src_pitch,
                          int    dst_bands,
                          char **dst,
                          int   *dst_pitch,
                          long   samples)
{
  const Babl *trc  = perceptual_trc;

  long n = samples;

  BABL_PLANAR_SANITY
  while (n--)
    {
      int band;
      for (band = 0; band < 3; band++)
        *(double *) dst[band] = babl_trc_from_linear (trc, (*(double *) src[band]));
      for (; band < dst_bands; band++)
        *(double *) dst[band] = *(double *) src[band];

      BABL_PLANAR_STEP
    }
}

static void
g3_perceptual_to_linear (Babl  *conversion,
                        int    src_bands,
                        char **src,
                        int   *src_pitch,
                        int    dst_bands,
                        char **dst,
                        int   *dst_pitch,
                        long   samples)
{
  const Babl *trc  = perceptual_trc;
  long n = samples;

  BABL_PLANAR_SANITY
  while (n--)
    {
      int band;
      for (band = 0; band < 3; band++)
        {
          *(double *) dst[band] = babl_trc_to_linear (trc, (*(double *) src[band]));
        }
      for (; band < dst_bands; band++)
        {
          if (band < src_bands)
            *(double *) dst[band] = *(double *) src[band];
          else
            *(double *) dst[band] = 1.0;
        }
      BABL_PLANAR_STEP
    }
}

static void
rgba2rgba_perceptual_premultiplied (Babl *conversion,
                                   char *src,
                                   char *dst,
                                   long  samples)
{
  const Babl *trc  = perceptual_trc;
  long n = samples;

  while (n--)
    {
      double alpha, alpha_used;
      alpha_used = alpha = ((double *) src)[3];
      if (alpha < BABL_ALPHA_FLOOR)
         alpha_used = BABL_ALPHA_FLOOR;
      ((double *) dst)[0] = babl_trc_from_linear (trc, ((double *) src)[0]) * alpha_used;
      ((double *) dst)[1] = babl_trc_from_linear (trc, ((double *) src)[1]) * alpha_used;
      ((double *) dst)[2] = babl_trc_from_linear (trc, ((double *) src)[2]) * alpha_used;
      ((double *) dst)[3] = alpha_used;
      src                += 4 * sizeof (double);
      dst                += 4 * sizeof (double);
    }
}


static void
rgba_perceptual_premultiplied2rgba (Babl *conversion,
                                    char *src,
                                    char *dst,
                                    long  samples)
{
  const Babl *trc  = perceptual_trc;
  long n = samples;

  while (n--)
    {
      double alpha;
      alpha = ((double *) src)[3];
      if (alpha == 0)
      {
        ((double *) dst)[0] = 0;
        ((double *) dst)[1] = 0;
        ((double *) dst)[2] = 0;
      }
      else
      {
         ((double *) dst)[0] = babl_trc_to_linear (trc, ((double *) src)[0] / alpha);
         ((double *) dst)[1] = babl_trc_to_linear (trc, ((double *) src)[1] / alpha);
         ((double *) dst)[2] = babl_trc_to_linear (trc, ((double *) src)[2] / alpha);
         if (alpha == BABL_ALPHA_FLOOR)
           alpha = 0.0;
      }
      ((double *) dst)[3] = alpha;

      src += 4 * sizeof (double);
      dst += 4 * sizeof (double);
    }
}


static void
rgba2rgba_perceptual (Babl *conversion,
                     char *src,
                     char *dst,
                     long  samples)
{
  const Babl *trc   = perceptual_trc;
  long n = samples;

  while (n--)
    {
      double alpha = ((double *) src)[3];
      ((double *) dst)[0] = babl_trc_from_linear (trc, ((double *) src)[0]);
      ((double *) dst)[1] = babl_trc_from_linear (trc, ((double *) src)[1]);
      ((double *) dst)[2] = babl_trc_from_linear (trc, ((double *) src)[2]);
      ((double *) dst)[3] = alpha;
      src                += 4 * sizeof (double);
      dst                += 4 * sizeof (double);
    }
}

static void
rgba_perceptual2rgba (Babl *conversion,
                     char *src,
                     char *dst,
                     long  samples)
{
  const Babl *trc   = perceptual_trc;
  long n = samples;

  while (n--)
    {
      double alpha = ((double *) src)[3];
      ((double *) dst)[0] = babl_trc_to_linear (trc, ((double *) src)[0]);
      ((double *) dst)[1] = babl_trc_to_linear (trc, ((double *) src)[1]);
      ((double *) dst)[2] = babl_trc_to_linear (trc, ((double *) src)[2]);
      ((double *) dst)[3] = alpha;

      src += 4 * sizeof (double);
      dst += 4 * sizeof (double);
    }
}

static void
conversions (void)
{
  if (!perceptual_trc)
    perceptual_trc = babl_trc ("sRGB");

  babl_conversion_new (
    babl_model_from_id (BABL_RGBA),
    babl_model_from_id (BABL_RGBA),
    "planar", copy_strip_1,
    NULL
  );


  babl_conversion_new (
    babl_model_from_id (BABL_RGB),
    babl_model_from_id (BABL_RGBA),
    "planar", copy_strip_1,
    NULL
  );

  babl_conversion_new (
    babl_model_from_id (BABL_RGBA),
    babl_model_from_id (BABL_RGB),
    "planar", copy_strip_1,
    NULL
  );

  babl_conversion_new (
    babl_model_from_id (BABL_RGBA),
    babl_model_from_id (BABL_RGBA_PREMULTIPLIED),
    "planar", non_premultiplied_to_premultiplied,
    NULL
  );

  babl_conversion_new (
    babl_model_from_id (BABL_RGBA_PREMULTIPLIED),
    babl_model_from_id (BABL_RGBA),
    "planar", premultiplied_to_non_premultiplied,
    NULL
  );

  babl_conversion_new (
    babl_model_from_id (BABL_RGBA),
    babl_model_from_id (BABL_RGB_NONLINEAR),
    "planar", g3_nonlinear_from_linear,
    NULL
  );
  babl_conversion_new (
    babl_model_from_id (BABL_RGB_NONLINEAR),
    babl_model_from_id (BABL_RGBA),
    "planar", g3_nonlinear_to_linear,
    NULL
  );

  babl_conversion_new (
    babl_model_from_id (BABL_RGBA),
    babl_model_from_id (BABL_RGBA_NONLINEAR),
    "linear", rgba2rgba_nonlinear,
    NULL);
  babl_conversion_new (
    babl_model_from_id (BABL_RGBA_NONLINEAR),
    babl_model_from_id (BABL_RGBA),
    "linear", rgba_nonlinear2rgba,
    NULL);

  babl_conversion_new (
    babl_model_from_id (BABL_RGBA),
    babl_model_from_id (BABL_RGBA_NONLINEAR_PREMULTIPLIED),
    "linear", rgba2rgba_nonlinear_premultiplied,
    NULL);
  babl_conversion_new (
    babl_model_from_id (BABL_RGBA_NONLINEAR_PREMULTIPLIED),
    babl_model_from_id (BABL_RGBA),
    "linear", rgba_nonlinear_premultiplied2rgba,
    NULL);
//////////

  babl_conversion_new (
    babl_model_from_id (BABL_RGBA),
    babl_model_from_id (BABL_RGB_PERCEPTUAL),
    "planar", g3_perceptual_from_linear,
    NULL
  );
  babl_conversion_new (
    babl_model_from_id (BABL_RGB_PERCEPTUAL),
    babl_model_from_id (BABL_RGBA),
    "planar", g3_perceptual_to_linear,
    NULL
  );

  babl_conversion_new (
    babl_model_from_id (BABL_RGBA),
    babl_model_from_id (BABL_RGBA_PERCEPTUAL),
    "linear", rgba2rgba_perceptual,
    NULL);
  babl_conversion_new (
    babl_model_from_id (BABL_RGBA_PERCEPTUAL),
    babl_model_from_id (BABL_RGBA),
    "linear", rgba_perceptual2rgba,
    NULL);

  babl_conversion_new (
    babl_model_from_id (BABL_RGBA),
    babl_model_from_id (BABL_RGBA_PERCEPTUAL_PREMULTIPLIED),
    "linear", rgba2rgba_perceptual_premultiplied,
    NULL);
  babl_conversion_new (
    babl_model_from_id (BABL_RGBA_PERCEPTUAL_PREMULTIPLIED),
    babl_model_from_id (BABL_RGBA),
    "linear", rgba_perceptual_premultiplied2rgba,
    NULL);
}

static void
formats (void)
{
  babl_format_new (
    "id", BABL_SRGB,
    babl_model_from_id (BABL_RGB_PERCEPTUAL),
    babl_type_from_id (BABL_U8),
    babl_component_from_id (BABL_RED_PERCEPTUAL),
    babl_component_from_id (BABL_GREEN_PERCEPTUAL),
    babl_component_from_id (BABL_BLUE_PERCEPTUAL),
    NULL);

  babl_format_new (
    babl_model_from_id (BABL_RGB_NONLINEAR),
    babl_type_from_id (BABL_U8),
    babl_component_from_id (BABL_RED_NONLINEAR),
    babl_component_from_id (BABL_GREEN_NONLINEAR),
    babl_component_from_id (BABL_BLUE_NONLINEAR),
    NULL);

  babl_format_new (
    babl_model_from_id (BABL_RGBA_NONLINEAR),
    babl_type_from_id (BABL_U8),
    babl_component_from_id (BABL_RED_NONLINEAR),
    babl_component_from_id (BABL_GREEN_NONLINEAR),
    babl_component_from_id (BABL_BLUE_NONLINEAR),
    babl_component_from_id (BABL_ALPHA),
    NULL);

  babl_format_new (
    babl_model_from_id (BABL_RGBA_PERCEPTUAL),
    babl_type_from_id (BABL_U8),
    babl_component_from_id (BABL_RED_PERCEPTUAL),
    babl_component_from_id (BABL_GREEN_PERCEPTUAL),
    babl_component_from_id (BABL_BLUE_PERCEPTUAL),
    babl_component_from_id (BABL_ALPHA),
    NULL);

  babl_format_new (
    "id", BABL_RGBA_FLOAT,
    babl_model_from_id     (BABL_RGBA),
    babl_type_from_id      (BABL_FLOAT),
    babl_component_from_id (BABL_RED),
    babl_component_from_id (BABL_GREEN),
    babl_component_from_id (BABL_BLUE),
    babl_component_from_id (BABL_ALPHA),
    NULL);

  babl_format_new (
    "id", BABL_RGB_FLOAT,
    babl_model_from_id (BABL_RGB),
    babl_type_from_id (BABL_FLOAT),
    babl_component_from_id (BABL_RED),
    babl_component_from_id (BABL_GREEN),
    babl_component_from_id (BABL_BLUE),
    NULL);

  babl_format_new (
    "id", BABL_RGB_HALF,
    babl_model_from_id (BABL_RGB),
    babl_type_from_id (BABL_HALF),
    babl_component_from_id (BABL_RED),
    babl_component_from_id (BABL_GREEN),
    babl_component_from_id (BABL_BLUE),
    NULL);

  babl_format_new (
    "id", BABL_RGBA_HALF,
    babl_model_from_id (BABL_RGBA),
    babl_type_from_id (BABL_HALF),
    babl_component_from_id (BABL_RED),
    babl_component_from_id (BABL_GREEN),
    babl_component_from_id (BABL_BLUE),
    babl_component_from_id (BABL_ALPHA),
    NULL);

  babl_format_new (
    babl_model_from_id (BABL_RGBA_PREMULTIPLIED),
    babl_type_from_id (BABL_HALF),
    babl_component_from_id (BABL_RED_MUL_ALPHA),
    babl_component_from_id (BABL_GREEN_MUL_ALPHA),
    babl_component_from_id (BABL_BLUE_MUL_ALPHA),
    babl_component_from_id (BABL_ALPHA),
    NULL);

  babl_format_new (
    babl_model_from_id (BABL_RGB_NONLINEAR),
    babl_type_from_id (BABL_HALF),
    babl_component_from_id (BABL_RED_NONLINEAR),
    babl_component_from_id (BABL_GREEN_NONLINEAR),
    babl_component_from_id (BABL_BLUE_NONLINEAR),
    NULL);

  babl_format_new (
    babl_model_from_id (BABL_RGBA_NONLINEAR),
    babl_type_from_id (BABL_HALF),
    babl_component_from_id (BABL_RED_NONLINEAR),
    babl_component_from_id (BABL_GREEN_NONLINEAR),
    babl_component_from_id (BABL_BLUE_NONLINEAR),
    babl_component_from_id (BABL_ALPHA),
    NULL);

  babl_format_new (
    babl_model_from_id (BABL_RGBA_NONLINEAR_PREMULTIPLIED),
    babl_type_from_id (BABL_HALF),
    babl_component_from_id (BABL_RED_NONLINEAR_MUL_ALPHA),
    babl_component_from_id (BABL_GREEN_NONLINEAR_MUL_ALPHA),
    babl_component_from_id (BABL_BLUE_NONLINEAR_MUL_ALPHA),
    babl_component_from_id (BABL_ALPHA),
    NULL);

  babl_format_new (
    babl_model_from_id (BABL_RGB_PERCEPTUAL),
    babl_type_from_id (BABL_HALF),
    babl_component_from_id (BABL_RED_PERCEPTUAL),
    babl_component_from_id (BABL_GREEN_PERCEPTUAL),
    babl_component_from_id (BABL_BLUE_PERCEPTUAL),
    NULL);

  babl_format_new (
    babl_model_from_id (BABL_RGBA_PERCEPTUAL),
    babl_type_from_id (BABL_HALF),
    babl_component_from_id (BABL_RED_PERCEPTUAL),
    babl_component_from_id (BABL_GREEN_PERCEPTUAL),
    babl_component_from_id (BABL_BLUE_PERCEPTUAL),
    babl_component_from_id (BABL_ALPHA),
    NULL);

  babl_format_new (
    babl_model_from_id (BABL_RGBA_PERCEPTUAL_PREMULTIPLIED),
    babl_type_from_id (BABL_HALF),
    babl_component_from_id (BABL_RED_PERCEPTUAL_MUL_ALPHA),
    babl_component_from_id (BABL_GREEN_PERCEPTUAL_MUL_ALPHA),
    babl_component_from_id (BABL_BLUE_PERCEPTUAL_MUL_ALPHA),
    babl_component_from_id (BABL_ALPHA),
    NULL);

  /******/
  babl_format_new (
    babl_model_from_id (BABL_RGB),
    babl_type ("u15"),
    babl_component_from_id (BABL_RED),
    babl_component_from_id (BABL_GREEN),
    babl_component_from_id (BABL_BLUE),
    NULL);

  babl_format_new (
    babl_model_from_id (BABL_RGBA),
    babl_type ("u15"),
    babl_component_from_id (BABL_RED),
    babl_component_from_id (BABL_GREEN),
    babl_component_from_id (BABL_BLUE),
    babl_component_from_id (BABL_ALPHA),
    NULL);

  babl_format_new (
    babl_model_from_id (BABL_RGBA_PREMULTIPLIED),
    babl_type ("u15"),
    babl_component_from_id (BABL_RED_MUL_ALPHA),
    babl_component_from_id (BABL_GREEN_MUL_ALPHA),
    babl_component_from_id (BABL_BLUE_MUL_ALPHA),
    babl_component_from_id (BABL_ALPHA),
    NULL);

  babl_format_new (
    babl_model_from_id (BABL_RGB_NONLINEAR),
    babl_type ("u15"),
    babl_component_from_id (BABL_RED_NONLINEAR),
    babl_component_from_id (BABL_GREEN_NONLINEAR),
    babl_component_from_id (BABL_BLUE_NONLINEAR),
    NULL);

  babl_format_new (
    babl_model_from_id (BABL_RGBA_NONLINEAR),
    babl_type ("u15"),
    babl_component_from_id (BABL_RED_NONLINEAR),
    babl_component_from_id (BABL_GREEN_NONLINEAR),
    babl_component_from_id (BABL_BLUE_NONLINEAR),
    babl_component_from_id (BABL_ALPHA),
    NULL);

  babl_format_new (
    babl_model_from_id (BABL_RGBA_NONLINEAR_PREMULTIPLIED),
    babl_type ("u15"),
    babl_component_from_id (BABL_RED_NONLINEAR_MUL_ALPHA),
    babl_component_from_id (BABL_GREEN_NONLINEAR_MUL_ALPHA),
    babl_component_from_id (BABL_BLUE_NONLINEAR_MUL_ALPHA),
    babl_component_from_id (BABL_ALPHA),
    NULL);


  babl_format_new (
    babl_model_from_id (BABL_RGB),
    babl_type_from_id (BABL_U32),
    babl_component_from_id (BABL_RED),
    babl_component_from_id (BABL_GREEN),
    babl_component_from_id (BABL_BLUE),
    NULL);

  babl_format_new (
    babl_model_from_id (BABL_RGBA),
    babl_type_from_id (BABL_U32),
    babl_component_from_id (BABL_RED),
    babl_component_from_id (BABL_GREEN),
    babl_component_from_id (BABL_BLUE),
    babl_component_from_id (BABL_ALPHA),
    NULL);

  babl_format_new (
    babl_model_from_id (BABL_RGBA_PREMULTIPLIED),
    babl_type_from_id (BABL_U32),
    babl_component_from_id (BABL_RED_MUL_ALPHA),
    babl_component_from_id (BABL_GREEN_MUL_ALPHA),
    babl_component_from_id (BABL_BLUE_MUL_ALPHA),
    babl_component_from_id (BABL_ALPHA),
    NULL);

  babl_format_new (
    babl_model_from_id (BABL_RGB_NONLINEAR),
    babl_type_from_id (BABL_U32),
    babl_component_from_id (BABL_RED_NONLINEAR),
    babl_component_from_id (BABL_GREEN_NONLINEAR),
    babl_component_from_id (BABL_BLUE_NONLINEAR),
    NULL);

  babl_format_new (
    babl_model_from_id (BABL_RGBA_NONLINEAR),
    babl_type_from_id (BABL_U32),
    babl_component_from_id (BABL_RED_NONLINEAR),
    babl_component_from_id (BABL_GREEN_NONLINEAR),
    babl_component_from_id (BABL_BLUE_NONLINEAR),
    babl_component_from_id (BABL_ALPHA),
    NULL);

  babl_format_new (
    babl_model_from_id (BABL_RGBA_NONLINEAR_PREMULTIPLIED),
    babl_type_from_id (BABL_U32),
    babl_component_from_id (BABL_RED_NONLINEAR_MUL_ALPHA),
    babl_component_from_id (BABL_GREEN_NONLINEAR_MUL_ALPHA),
    babl_component_from_id (BABL_BLUE_NONLINEAR_MUL_ALPHA),
    babl_component_from_id (BABL_ALPHA),
    NULL);


  babl_format_new (
    babl_model_from_id (BABL_RGB_PERCEPTUAL),
    babl_type_from_id (BABL_U32),
    babl_component_from_id (BABL_RED_PERCEPTUAL),
    babl_component_from_id (BABL_GREEN_PERCEPTUAL),
    babl_component_from_id (BABL_BLUE_PERCEPTUAL),
    NULL);

  babl_format_new (
    babl_model_from_id (BABL_RGBA_PERCEPTUAL),
    babl_type_from_id (BABL_U32),
    babl_component_from_id (BABL_RED_PERCEPTUAL),
    babl_component_from_id (BABL_GREEN_PERCEPTUAL),
    babl_component_from_id (BABL_BLUE_PERCEPTUAL),
    babl_component_from_id (BABL_ALPHA),
    NULL);

  babl_format_new (
    babl_model_from_id (BABL_RGBA_PERCEPTUAL_PREMULTIPLIED),
    babl_type_from_id (BABL_U32),
    babl_component_from_id (BABL_RED_PERCEPTUAL_MUL_ALPHA),
    babl_component_from_id (BABL_GREEN_PERCEPTUAL_MUL_ALPHA),
    babl_component_from_id (BABL_BLUE_PERCEPTUAL_MUL_ALPHA),
    babl_component_from_id (BABL_ALPHA),
    NULL);

#ifdef XXXX
  babl_format_new (
    "id", BABL_RGB565,
    babl_model_from_id (BABL_RGB),
    babl_component_from_id (BABL_RED),
    babl_component_from_id (BABL_GREEN),
    babl_component_from_id (BABL_BLUE),

  );
#endif
}

