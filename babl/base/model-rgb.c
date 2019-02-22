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
static void init_single_precision (void);

void
babl_base_model_rgb (void)
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
    "rgb",
    "linear",
    NULL);

  babl_model_new (
    "id", BABL_RGBA_PREMULTIPLIED,
    babl_component_from_id (BABL_RED_MUL_ALPHA),
    babl_component_from_id (BABL_GREEN_MUL_ALPHA),
    babl_component_from_id (BABL_BLUE_MUL_ALPHA),
    babl_component_from_id (BABL_ALPHA),
    "rgb",
    "linear",
    "premultiplied",
    "alpha",
    NULL);

  babl_model_new (
    "id", BABL_RGB_NONLINEAR,
    babl_component_from_id (BABL_RED_NONLINEAR),
    babl_component_from_id (BABL_GREEN_NONLINEAR),
    babl_component_from_id (BABL_BLUE_NONLINEAR),
    "rgb",
    "nonlinear",
    NULL);

  babl_model_new (
    "id", BABL_RGB_PERCEPTUAL,
    babl_component_from_id (BABL_RED_PERCEPTUAL),
    babl_component_from_id (BABL_GREEN_PERCEPTUAL),
    babl_component_from_id (BABL_BLUE_PERCEPTUAL),
    "rgb",
    "perceptual",
    NULL);

  babl_model_new (
    "id", BABL_RGBA_NONLINEAR,
    babl_component_from_id (BABL_RED_NONLINEAR),
    babl_component_from_id (BABL_GREEN_NONLINEAR),
    babl_component_from_id (BABL_BLUE_NONLINEAR),
    babl_component_from_id (BABL_ALPHA),
    "rgb",
    "nonlinear",
    "alpha",
    NULL);

  babl_model_new (
    "id", BABL_RGBA_PERCEPTUAL,
    babl_component_from_id (BABL_RED_PERCEPTUAL),
    babl_component_from_id (BABL_GREEN_PERCEPTUAL),
    babl_component_from_id (BABL_BLUE_PERCEPTUAL),
    babl_component_from_id (BABL_ALPHA),
    "rgb",
    "perceptual",
    "alpha",
    NULL);

  babl_model_new (
    "id", BABL_RGBA_NONLINEAR_PREMULTIPLIED,
    babl_component_from_id (BABL_RED_NONLINEAR_MUL_ALPHA),
    babl_component_from_id (BABL_GREEN_NONLINEAR_MUL_ALPHA),
    babl_component_from_id (BABL_BLUE_NONLINEAR_MUL_ALPHA),
    babl_component_from_id (BABL_ALPHA),
    "rgb",
    "nonlinear",
    "premultiplied",
    "alpha",
    NULL);

  babl_model_new (
    "id", BABL_RGBA_PERCEPTUAL_PREMULTIPLIED,
    babl_component_from_id (BABL_RED_PERCEPTUAL_MUL_ALPHA),
    babl_component_from_id (BABL_GREEN_PERCEPTUAL_MUL_ALPHA),
    babl_component_from_id (BABL_BLUE_PERCEPTUAL_MUL_ALPHA),
    babl_component_from_id (BABL_ALPHA),
    "rgb",
    "perceptual",
    "premultiplied",
    "alpha",
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
      double alpha = *(double *) src[src_bands - 1];
      int    band;

      if (alpha < BABL_ALPHA_FLOOR)
      {
        if (alpha >= 0.0)
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
      double alpha = ((double *) src)[3];
      if (alpha <= BABL_ALPHA_FLOOR)
      {
        if (alpha >= 0.0f)
           alpha = BABL_ALPHA_FLOOR;
         else if (alpha >= -BABL_ALPHA_FLOOR)
           alpha = -BABL_ALPHA_FLOOR;
      }

      ((double *) dst)[0] = babl_trc_from_linear (trc[0], ((double *) src)[0]) * alpha;
      ((double *) dst)[1] = babl_trc_from_linear (trc[1], ((double *) src)[1]) * alpha;
      ((double *) dst)[2] = babl_trc_from_linear (trc[2], ((double *) src)[2]) * alpha;
      ((double *) dst)[3] = alpha;
      src                += 4 * sizeof (double);
      dst                += 4 * sizeof (double);
    }
}




static void
rgba_nonlinear_premultiplied2rgba (Babl *conversion,
                                   char *src,
                                   char *dst,
                                   long  samples)
{
  const Babl *space = babl_conversion_get_source_space (conversion);
  const Babl **trc  = (void*)space->space.trc;
  long n = samples;

  while (n--)
    {
      double alpha, reciprocal;
      alpha = ((double *) src)[3];
      if (alpha == 0.0)
      {
        reciprocal= 0.0f;
      }
      else
      {
        reciprocal= 1.0 / alpha;
        if (alpha == BABL_ALPHA_FLOOR || alpha == -BABL_ALPHA_FLOOR)
          alpha = 0;
      }

      ((double *) dst)[0] = babl_trc_to_linear (trc[0], ((double *) src)[0] * reciprocal);
      ((double *) dst)[1] = babl_trc_to_linear (trc[1], ((double *) src)[1] * reciprocal);
      ((double *) dst)[2] = babl_trc_to_linear (trc[2], ((double *) src)[2] * reciprocal);
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
                          int     src_bands,
                          char  **src,
                          int    *src_pitch,
                          int     dst_bands,
                          char  **dst,
                          int    *dst_pitch,
                          long    samples)
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
                        int     src_bands,
                        char  **src,
                        int    *src_pitch,
                        int     dst_bands,
                        char  **dst,
                        int    *dst_pitch,
                        long    samples)
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
      double alpha = ((double *) src)[3];

      if (alpha <= BABL_ALPHA_FLOOR)
      {
        if (alpha >= 0.0f)
          alpha = BABL_ALPHA_FLOOR;
        else if (alpha >= -BABL_ALPHA_FLOOR)
           alpha = -BABL_ALPHA_FLOOR;
      }

      ((double *) dst)[0] = babl_trc_from_linear (trc, ((double *) src)[0]) * alpha;
      ((double *) dst)[1] = babl_trc_from_linear (trc, ((double *) src)[1]) * alpha;
      ((double *) dst)[2] = babl_trc_from_linear (trc, ((double *) src)[2]) * alpha;
      ((double *) dst)[3] = alpha;
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
      double alpha, reciprocal;
      alpha = ((double *) src)[3];
      if (alpha == 0)
      {
        reciprocal = 0.0;
      }
      else
      {
         reciprocal = 1.0/alpha;
         if(alpha == BABL_ALPHA_FLOOR || alpha == -BABL_ALPHA_FLOOR)
           alpha = 0.0;
      }
      ((double *) dst)[0] = babl_trc_to_linear (trc, ((double *) src)[0] * reciprocal);
      ((double *) dst)[1] = babl_trc_to_linear (trc, ((double *) src)[1] * reciprocal);
      ((double *) dst)[2] = babl_trc_to_linear (trc, ((double *) src)[2] * reciprocal);
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
    babl_model_from_id (BABL_RGB_NONLINEAR),
    babl_type ("float"),
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
    babl_model_from_id (BABL_RGBA_NONLINEAR_PREMULTIPLIED),
    babl_type_from_id (BABL_FLOAT),
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


static void
g3_nonlinear_from_linear_float (Babl  *conversion,
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
        *(float *) dst[band] = babl_trc_from_linear (trc[band], (*(float *) src[band]));
      for (; band < dst_bands; band++)
        *(float *) dst[band] = *(float *) src[band];

      BABL_PLANAR_STEP
    }
}


static void
g3_nonlinear_to_linear_float (Babl  *conversion,
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
          *(float *) dst[band] = babl_trc_to_linear (trc[band], (*(float *) src[band]));
        }
      for (; band < dst_bands; band++)
        {
          if (band < src_bands)
            *(float *) dst[band] = *(float *) src[band];
          else
            *(float *) dst[band] = 1.0f;
        }
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
                                          long   samples)
{
  long n = samples;

  BABL_PLANAR_SANITY
  while (n--)
    {
      float alpha = *(float *) src[src_bands - 1];
      int    band;

      if (alpha < BABL_ALPHA_FLOOR)
      {
        int non_zero_components = 0;
        if (alpha >= 0.0f)
          alpha = BABL_ALPHA_FLOOR;
        else if (alpha >= -BABL_ALPHA_FLOOR)
          alpha = -BABL_ALPHA_FLOOR;
        for (band = 0 ; band< src_bands-1; band++)
          if (*(float *) src[band] != 0.0f)
            non_zero_components++;
        if (non_zero_components == 0)
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
                                          long   samples)
{
  long n = samples;

  BABL_PLANAR_SANITY
  while (n--)
    {
      float alpha;
      float recip_alpha;
      int    band;

      alpha = *(float *) src[src_bands - 1];
      if (alpha == 0.0f)
         recip_alpha = 0.0f;
      else
      {
        recip_alpha  = 1.0f / alpha;
        if (alpha == BABL_ALPHA_FLOOR)
          alpha = 0.0f; // making 0 round-trip to zero, causing discontinuity
      }

      for (band = 0; band < src_bands - 1; band++)
        *(float *) dst[band] = *(float *) src[band] * recip_alpha;
      *(float *) dst[dst_bands - 1] = alpha;

      BABL_PLANAR_STEP
    }
}


static void
rgba2rgba_nonlinear_premultiplied_float (Babl *conversion,
                                         char *src,
                                         char *dst,
                                         long  samples)
{
  const Babl *space = babl_conversion_get_destination_space (conversion);
  const Babl **trc  = (void*)space->space.trc;
  long n = samples;

  while (n--)
    {
      float alpha = ((float *) src)[3];
      if (alpha <= BABL_ALPHA_FLOOR)
      {
        if (alpha >= 0.0f)
           alpha = BABL_ALPHA_FLOOR;
         else if (alpha >= -BABL_ALPHA_FLOOR)
           alpha = -BABL_ALPHA_FLOOR;
        if (((float *) src)[0] == 0.0 &&
            ((float *) src)[1] == 0.0 &&
            ((float *) src)[2] == 0.0)
           alpha = 0.0;
      }

      ((float *) dst)[0] = babl_trc_from_linear (trc[0], ((float *) src)[0]) * alpha;
      ((float *) dst)[1] = babl_trc_from_linear (trc[1], ((float *) src)[1]) * alpha;
      ((float *) dst)[2] = babl_trc_from_linear (trc[2], ((float *) src)[2]) * alpha;
      ((float *) dst)[3] = alpha;
      src  += 4 * sizeof (float);
      dst  += 4 * sizeof (float);
    }
}


static void
rgba_nonlinear_premultiplied2rgba_float (Babl *conversion,
                                         char *src,
                                         char *dst,
                                         long  samples)
{
  const Babl *space = babl_conversion_get_source_space (conversion);
  const Babl **trc  = (void*)space->space.trc;
  long n = samples;

  while (n--)
    {
      float alpha, reciprocal;
      alpha = ((float *) src)[3];
      if (alpha == 0.0)
      {
        reciprocal= 0.0f;
      }
      else
      {
        reciprocal= 1.0 / alpha;
        if (alpha == BABL_ALPHA_FLOOR || alpha == -BABL_ALPHA_FLOOR)
          alpha = 0;
      }

      ((float *) dst)[0] = babl_trc_to_linear (trc[0], ((float *) src)[0] * reciprocal);
      ((float *) dst)[1] = babl_trc_to_linear (trc[1], ((float *) src)[1] * reciprocal);
      ((float *) dst)[2] = babl_trc_to_linear (trc[2], ((float *) src)[2] * reciprocal);
      ((float *) dst)[3] = alpha;

      src += 4 * sizeof (float);
      dst += 4 * sizeof (float);
    }
}


static void
rgba2rgba_float (Babl *conversion,
                 char *src,
                 char *dst,
                 long  samples)
{
  long n = samples;

  while (n--)
    {
      ((float *) dst)[0] = ((float *) src)[0];
      ((float *) dst)[1] = ((float *) src)[1];
      ((float *) dst)[2] = ((float *) src)[2];
      ((float *) dst)[3] = ((float *) src)[3];
      src                += 4 * sizeof (float);
      dst                += 4 * sizeof (float);
    }
}


static void
rgba2rgba_perceptual_float (Babl *conversion,
                            char *src,
                            char *dst,
                            long  samples)
{
  const Babl *trc   = perceptual_trc;
  long n = samples;

  while (n--)
    {
      float alpha = ((float *) src)[3];
      ((float *) dst)[0] = babl_trc_from_linear (trc, ((float *) src)[0]);
      ((float *) dst)[1] = babl_trc_from_linear (trc, ((float *) src)[1]);
      ((float *) dst)[2] = babl_trc_from_linear (trc, ((float *) src)[2]);
      ((float *) dst)[3] = alpha;
      src                += 4 * sizeof (float);
      dst                += 4 * sizeof (float);
    }
}

static void
rgba_perceptual2rgba_float (Babl *conversion,
                            char *src,
                            char *dst,
                            long  samples)
{
  const Babl *trc   = perceptual_trc;
  long n = samples;

  while (n--)
    {
      float alpha = ((float *) src)[3];
      ((float *) dst)[0] = babl_trc_to_linear (trc, ((float *) src)[0]);
      ((float *) dst)[1] = babl_trc_to_linear (trc, ((float *) src)[1]);
      ((float *) dst)[2] = babl_trc_to_linear (trc, ((float *) src)[2]);
      ((float *) dst)[3] = alpha;

      src += 4 * sizeof (float);
      dst += 4 * sizeof (float);
    }
}


static void
g3_perceptual_from_linear_float (Babl  *conversion,
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
        *(float *) dst[band] = babl_trc_from_linear (trc, (*(float *) src[band]));
      for (; band < dst_bands; band++)
        *(float *) dst[band] = *(float *) src[band];

      BABL_PLANAR_STEP
    }
}

static void
g3_perceptual_to_linear_float (Babl  *conversion,
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
          *(float *) dst[band] = babl_trc_to_linear (trc, (*(float *) src[band]));
        }
      for (; band < dst_bands; band++)
        {
          if (band < src_bands)
            *(float *) dst[band] = *(float *) src[band];
          else
            *(float *) dst[band] = 1.0;
        }
      BABL_PLANAR_STEP
    }
}


static void 
init_single_precision (void)
{

  babl_format_new (
    babl_model_from_id (BABL_RGBA_PREMULTIPLIED),
    babl_type_from_id (BABL_FLOAT),
    babl_component_from_id (BABL_RED_MUL_ALPHA),
    babl_component_from_id (BABL_GREEN_MUL_ALPHA),
    babl_component_from_id (BABL_BLUE_MUL_ALPHA),
    babl_component_from_id (BABL_ALPHA),
    NULL);

  babl_format_new (
    babl_model_from_id (BABL_RGBA_NONLINEAR),
    babl_type_from_id (BABL_FLOAT),
    babl_component_from_id (BABL_RED_NONLINEAR),
    babl_component_from_id (BABL_GREEN_NONLINEAR),
    babl_component_from_id (BABL_BLUE_NONLINEAR),
    babl_component_from_id (BABL_ALPHA),
    NULL);

  babl_format_new (
    babl_model_from_id (BABL_RGBA_PERCEPTUAL),
    babl_type_from_id (BABL_FLOAT),
    babl_component_from_id (BABL_RED_PERCEPTUAL),
    babl_component_from_id (BABL_GREEN_PERCEPTUAL),
    babl_component_from_id (BABL_BLUE_PERCEPTUAL),
    babl_component_from_id (BABL_ALPHA),
    NULL);

  babl_format_new (
    babl_model_from_id (BABL_RGB_PERCEPTUAL),
    babl_type_from_id (BABL_FLOAT),
    babl_component_from_id (BABL_RED_PERCEPTUAL),
    babl_component_from_id (BABL_GREEN_PERCEPTUAL),
    babl_component_from_id (BABL_BLUE_PERCEPTUAL),
    NULL);


  babl_conversion_new (
    babl_format ("RGBA float"),
    babl_format ("RGBA float"),
    "linear", rgba2rgba_float,
    NULL
  );

  babl_conversion_new (
    babl_format ("R'G'B' float"),
    babl_format ("RGBA float"),
    "planar", g3_nonlinear_to_linear_float,
    NULL
  );

  babl_conversion_new (
    babl_format ("RGBA float"),
    babl_format ("R'G'B' float"),
    "planar", g3_nonlinear_from_linear_float,
    NULL
  );
  babl_conversion_new (
    babl_format ("R'G'B'A float"),
    babl_format ("RGBA float"),
    "planar", g3_nonlinear_to_linear_float,
    NULL
  );


  babl_conversion_new (
    babl_format ("R~G~B~ float"),
    babl_format ("RGBA float"),
    "planar", g3_perceptual_to_linear_float,
    NULL
  );

  babl_conversion_new (
    babl_format ("RGBA float"),
    babl_format ("R~G~B~ float"),
    "planar", g3_perceptual_from_linear_float,
    NULL
  );



  babl_conversion_new (
    babl_format ("RGBA float"),
    babl_format ("R~G~B~A float"),
    "linear", rgba2rgba_perceptual_float,
    NULL
  );
  babl_conversion_new (
    babl_format ("R~G~B~A float"),
    babl_format ("RGBA float"),
    "linear", rgba_perceptual2rgba_float,
    NULL
  );

  babl_conversion_new (
    babl_format ("RGBA float"),
    babl_format ("R'G'B'A float"),
    "planar", g3_nonlinear_from_linear_float,
    NULL
  );


  babl_conversion_new (
    babl_format ("RGBA float"),
    babl_format ("R'aG'aB'aA float"),
    "linear", rgba2rgba_nonlinear_premultiplied_float,
    NULL);
  babl_conversion_new (
    babl_format ("R'aG'aB'aA float"),
    babl_format ("RGBA float"),
    "linear", rgba_nonlinear_premultiplied2rgba_float,
    NULL);


  babl_conversion_new (
    babl_format ("RGBA float"),
    babl_format ("RaGaBaA float"),
    "planar", non_premultiplied_to_premultiplied_float,
    NULL
  );
  babl_conversion_new (
    babl_format ("RaGaBaA float"),
    babl_format ("RGBA float"),
    "planar", premultiplied_to_non_premultiplied_float,
    NULL
  );


}
