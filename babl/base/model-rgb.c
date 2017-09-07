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
#include <math.h>

#include "babl.h"
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
    "id", BABL_RED_GAMMA_2_2,
    "luma",
    "chroma",
    NULL);

  babl_component_new (
    "G'",
    "id", BABL_GREEN_GAMMA_2_2,
    "luma",
    "chroma",
    NULL);

  babl_component_new (
    "B'",
    "id", BABL_BLUE_GAMMA_2_2,
    "luma",
    "chroma",
    NULL);

  babl_component_new (
    "R'a",
    "id", BABL_RED_GAMMA_2_2_MUL_ALPHA,
    "luma",
    "chroma",
    NULL);

  babl_component_new (
    "G'a",
    "id", BABL_GREEN_GAMMA_2_2_MUL_ALPHA,
    "luma",
    "chroma",
    NULL);

  babl_component_new (
    "B'a",
    "id", BABL_BLUE_GAMMA_2_2_MUL_ALPHA,
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
    "id", BABL_RGB_GAMMA_2_2,
    babl_component_from_id (BABL_RED_GAMMA_2_2),
    babl_component_from_id (BABL_GREEN_GAMMA_2_2),
    babl_component_from_id (BABL_BLUE_GAMMA_2_2),
    NULL);

  babl_model_new (
    "id", BABL_RGBA_GAMMA_2_2,
    babl_component_from_id (BABL_RED_GAMMA_2_2),
    babl_component_from_id (BABL_GREEN_GAMMA_2_2),
    babl_component_from_id (BABL_BLUE_GAMMA_2_2),
    babl_component_from_id (BABL_ALPHA),
    NULL);

  babl_model_new (
    "id", BABL_RGBA_GAMMA_2_2_PREMULTIPLIED,
    babl_component_from_id (BABL_RED_GAMMA_2_2_MUL_ALPHA),
    babl_component_from_id (BABL_GREEN_GAMMA_2_2_MUL_ALPHA),
    babl_component_from_id (BABL_BLUE_GAMMA_2_2_MUL_ALPHA),
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
g3_gamma_2_2 (Babl  *conversion,
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
g3_inv_gamma_2_2 (Babl  *conversion,
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
                                    long   samples)
{
  long n = samples;

  BABL_PLANAR_SANITY
  while (n--)
    {
      double alpha;
      int    band;

      alpha = *(double *) src[src_bands - 1];
      if (alpha > BABL_ALPHA_THRESHOLD)
        {
          double recip_alpha = 1.0 / alpha;
          for (band = 0; band < src_bands - 1; band++)
            *(double *) dst[band] = *(double *) src[band] * recip_alpha;
        }
      else
        {
          for (band = 0; band < src_bands - 1; band++)
            *(double *) dst[band] = 0.0;
        }
      *(double *) dst[dst_bands - 1] = alpha;

      BABL_PLANAR_STEP
    }
}


static void
rgba2rgba_gamma_2_2_premultiplied (Babl *conversion,
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
      ((double *) dst)[0] = babl_trc_from_linear (trc[0], ((double *) src)[0]) * alpha;
      ((double *) dst)[1] = babl_trc_from_linear (trc[1], ((double *) src)[1]) * alpha;
      ((double *) dst)[2] = babl_trc_from_linear (trc[2], ((double *) src)[2]) * alpha;
      ((double *) dst)[3] = alpha;
      src                += 4 * sizeof (double);
      dst                += 4 * sizeof (double);
    }
}


static void
rgba_gamma_2_2_premultiplied2rgba (Babl *conversion,
                                   char           *src,
                                   char           *dst,
                                   long            samples)
{
  const Babl *space = babl_conversion_get_source_space (conversion);
  const Babl **trc  = (void*)space->space.trc;
  long n = samples;

  while (n--)
    {
      double alpha = ((double *) src)[3];
      if (alpha > BABL_ALPHA_THRESHOLD)
        {
          ((double *) dst)[0] = babl_trc_to_linear (trc[0], ((double *) src)[0] / alpha);
          ((double *) dst)[1] = babl_trc_to_linear (trc[1], ((double *) src)[1] / alpha);
          ((double *) dst)[2] = babl_trc_to_linear (trc[2], ((double *) src)[2] / alpha);
        }
      else
        {
          ((double *) dst)[0] = 0.0;
          ((double *) dst)[1] = 0.0;
          ((double *) dst)[2] = 0.0;
        }
      ((double *) dst)[3] = alpha;

      src += 4 * sizeof (double);
      dst += 4 * sizeof (double);
    }
}


static void
rgba2rgba_gamma_2_2 (Babl *conversion,
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
rgba_gamma_2_22rgba (Babl *conversion,
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

static void
conversions (void)
{
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
    babl_model_from_id (BABL_RGB_GAMMA_2_2),
    "planar", g3_gamma_2_2,
    NULL
  );
  babl_conversion_new (
    babl_model_from_id (BABL_RGB_GAMMA_2_2),
    babl_model_from_id (BABL_RGBA),
    "planar", g3_inv_gamma_2_2,
    NULL
  );

  babl_conversion_new (
    babl_model_from_id (BABL_RGBA),
    babl_model_from_id (BABL_RGBA_GAMMA_2_2),
    "linear", rgba2rgba_gamma_2_2,
    NULL);
  babl_conversion_new (
    babl_model_from_id (BABL_RGBA_GAMMA_2_2),
    babl_model_from_id (BABL_RGBA),
    "linear", rgba_gamma_2_22rgba,
    NULL);

  babl_conversion_new (
    babl_model_from_id (BABL_RGBA),
    babl_model_from_id (BABL_RGBA_GAMMA_2_2_PREMULTIPLIED),
    "linear", rgba2rgba_gamma_2_2_premultiplied,
    NULL);
  babl_conversion_new (
    babl_model_from_id (BABL_RGBA_GAMMA_2_2_PREMULTIPLIED),
    babl_model_from_id (BABL_RGBA),
    "linear", rgba_gamma_2_2_premultiplied2rgba,
    NULL);
}

static void
formats (void)
{
  babl_format_new (
    "id", BABL_SRGB,
    babl_model_from_id (BABL_RGB_GAMMA_2_2),
    babl_type_from_id (BABL_U8),
    babl_component_from_id (BABL_RED_GAMMA_2_2),
    babl_component_from_id (BABL_GREEN_GAMMA_2_2),
    babl_component_from_id (BABL_BLUE_GAMMA_2_2),
    NULL);

  babl_format_new (
    "id", BABL_SRGBA,
    babl_model_from_id (BABL_RGBA_GAMMA_2_2),
    babl_type_from_id (BABL_U8),
    babl_component_from_id (BABL_RED_GAMMA_2_2),
    babl_component_from_id (BABL_GREEN_GAMMA_2_2),
    babl_component_from_id (BABL_BLUE_GAMMA_2_2),
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
    babl_model_from_id (BABL_RGB_GAMMA_2_2),
    babl_type_from_id (BABL_HALF),
    babl_component_from_id (BABL_RED_GAMMA_2_2),
    babl_component_from_id (BABL_GREEN_GAMMA_2_2),
    babl_component_from_id (BABL_BLUE_GAMMA_2_2),
    NULL);

  babl_format_new (
    babl_model_from_id (BABL_RGBA_GAMMA_2_2),
    babl_type_from_id (BABL_HALF),
    babl_component_from_id (BABL_RED_GAMMA_2_2),
    babl_component_from_id (BABL_GREEN_GAMMA_2_2),
    babl_component_from_id (BABL_BLUE_GAMMA_2_2),
    babl_component_from_id (BABL_ALPHA),
    NULL);

  babl_format_new (
    babl_model_from_id (BABL_RGBA_GAMMA_2_2_PREMULTIPLIED),
    babl_type_from_id (BABL_HALF),
    babl_component_from_id (BABL_RED_GAMMA_2_2_MUL_ALPHA),
    babl_component_from_id (BABL_GREEN_GAMMA_2_2_MUL_ALPHA),
    babl_component_from_id (BABL_BLUE_GAMMA_2_2_MUL_ALPHA),
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
    babl_model_from_id (BABL_RGB_GAMMA_2_2),
    babl_type ("u15"),
    babl_component_from_id (BABL_RED_GAMMA_2_2),
    babl_component_from_id (BABL_GREEN_GAMMA_2_2),
    babl_component_from_id (BABL_BLUE_GAMMA_2_2),
    NULL);

  babl_format_new (
    babl_model_from_id (BABL_RGBA_GAMMA_2_2),
    babl_type ("u15"),
    babl_component_from_id (BABL_RED_GAMMA_2_2),
    babl_component_from_id (BABL_GREEN_GAMMA_2_2),
    babl_component_from_id (BABL_BLUE_GAMMA_2_2),
    babl_component_from_id (BABL_ALPHA),
    NULL);

  babl_format_new (
    babl_model_from_id (BABL_RGBA_GAMMA_2_2_PREMULTIPLIED),
    babl_type ("u15"),
    babl_component_from_id (BABL_RED_GAMMA_2_2_MUL_ALPHA),
    babl_component_from_id (BABL_GREEN_GAMMA_2_2_MUL_ALPHA),
    babl_component_from_id (BABL_BLUE_GAMMA_2_2_MUL_ALPHA),
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
    babl_model_from_id (BABL_RGB_GAMMA_2_2),
    babl_type_from_id (BABL_U32),
    babl_component_from_id (BABL_RED_GAMMA_2_2),
    babl_component_from_id (BABL_GREEN_GAMMA_2_2),
    babl_component_from_id (BABL_BLUE_GAMMA_2_2),
    NULL);

  babl_format_new (
    babl_model_from_id (BABL_RGBA_GAMMA_2_2),
    babl_type_from_id (BABL_U32),
    babl_component_from_id (BABL_RED_GAMMA_2_2),
    babl_component_from_id (BABL_GREEN_GAMMA_2_2),
    babl_component_from_id (BABL_BLUE_GAMMA_2_2),
    babl_component_from_id (BABL_ALPHA),
    NULL);

  babl_format_new (
    babl_model_from_id (BABL_RGBA_GAMMA_2_2_PREMULTIPLIED),
    babl_type_from_id (BABL_U32),
    babl_component_from_id (BABL_RED_GAMMA_2_2_MUL_ALPHA),
    babl_component_from_id (BABL_GREEN_GAMMA_2_2_MUL_ALPHA),
    babl_component_from_id (BABL_BLUE_GAMMA_2_2_MUL_ALPHA),
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

