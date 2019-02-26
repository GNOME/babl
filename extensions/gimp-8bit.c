/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2005, Øyvind Kolås.
 *
 * Optimized 8bit conversion routines as used by legacy GIMP code.
 * Copyright (C) 2008  Sven Neumann
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
#include <stdio.h>

#include "babl-internal.h"

#include "base/util.h"
#include "extensions/util.h"


/* lookup tables used in conversion */

#define MAX_SPACES 32
static const Babl *spaces[MAX_SPACES]={NULL,};

static float lut_linear[1 << 8];
static float lut_gamma_2_2[MAX_SPACES][1 << 8];


static int
tables_init (const Babl *space)
{
  int i, j;

  for (j = 0; spaces[j]; j++)
  {
    if (spaces[j] == space)
      return j;
  }
  spaces[j] = space;

  /* fill tables for conversion from 8 bit integer to float */
  if (j == 0)
  for (i = 0; i < 1 << 8; i++)
    {
      double value = i / 255.0;
      lut_linear[i]    = value;
    }

  /* fill tables for conversion from 8 bit integer to float */
  for (i = 0; i < 1 << 8; i++)
    {
      double value = i / 255.0;
      lut_gamma_2_2[j][i] = babl_trc_to_linear (space->space.trc[0], value);
    }

  return j;
}

static inline void
u8_linear_to_float_linear (const Babl    *conversion,
                           unsigned char *src,
                           unsigned char *dst,
                           long           samples)
{
  float *d = (float *) dst;
  long   n = samples;

  while (n--)
    *d++ = lut_linear[*src++];
}


static void
u8_linear_to_float_linear_premul (const Babl    *conversion,
                                  unsigned char *src,
                                  unsigned char *dst,
                                  long           samples)
{
  float *d = (float *) dst;
  long   n = samples;

  while (n--)
    {
      float alpha = lut_linear[src[3]];
      d[0] = lut_linear[src[0]] * alpha;
      d[1] = lut_linear[src[1]] * alpha;
      d[2] = lut_linear[src[2]] * alpha;
      d[3] = alpha;
      src += 4;
      d += 4;
    }
}

static inline void
u8_gamma_2_2_to_float_linear (const Babl    *conversion,
                              unsigned char *src,
                              unsigned char *dst,
                              long           samples)
{
  int   space_no = tables_init (conversion->conversion.source->format.space);
  float *d = (float *) dst;
  long   n = samples;

  while (n--)
    *d++ = lut_gamma_2_2[space_no][*src++];
}

static void
conv_rgba8_linear_rgbaF_linear (const Babl    *conversion,
                                unsigned char *src,
                                unsigned char *dst,
                                long           samples)
{
  u8_linear_to_float_linear (conversion, src, dst, samples * 4);
}

static void
conv_rgba8_linear_ragabaaF_linear (const Babl    *conversion,
                                   unsigned char *src,
                                   unsigned char *dst,
                                   long           samples)
{
  u8_linear_to_float_linear_premul (conversion, src, dst, samples);
}


static void
conv_rgba8_gamma_2_2_rgbaF_linear (const Babl    *conversion,
                                   unsigned char *src,
                                   unsigned char *dst,
                                   long           samples)
{
  int   space_no = tables_init (conversion->conversion.source->format.space);
  float *d = (float *) dst;
  long   n = samples;

  while (n--)
    {
      *d++ = lut_gamma_2_2[space_no][*src++];
      *d++ = lut_gamma_2_2[space_no][*src++];
      *d++ = lut_gamma_2_2[space_no][*src++];
      *d++ = lut_linear[*src++];
    }
}

static void
conv_rgb8_linear_rgbF_linear (const Babl    *conversion,
                              unsigned char *src,
                              unsigned char *dst,
                              long           samples)
{
  u8_linear_to_float_linear (conversion, src, dst, samples * 3);
}

static void
conv_rgb8_gamma_2_2_rgbF_linear (const Babl    *conversion,
                                 unsigned char *src,
                                 unsigned char *dst,
                                 long           samples)
{
  u8_gamma_2_2_to_float_linear (conversion, src, dst, samples * 3);
}

static void
conv_rgb8_linear_rgbaF_linear (const Babl    *conversion,
                               unsigned char *src,
                               unsigned char *dst,
                               long           samples)
{
  float *d = (float *) dst;
  long   n = samples;

  while (n--)
    {
      *d++ = lut_linear[*src++];
      *d++ = lut_linear[*src++];
      *d++ = lut_linear[*src++];
      *d++ = 1.0;
    }
}

#define conv_rgb8_linear_ragabaaF_linear conv_rgb8_linear_rgbaF_linear

static void
conv_rgb8_gamma_2_2_rgbaF_linear (const Babl    *conversion,
                                  unsigned char *src,
                                  unsigned char *dst,
                                  long           samples)
{
  int   space_no = tables_init (conversion->conversion.source->format.space);
  float *d = (float *) dst;
  long   n = samples;

  while (n--)
    {
      *d++ = lut_gamma_2_2[space_no][*src++];
      *d++ = lut_gamma_2_2[space_no][*src++];
      *d++ = lut_gamma_2_2[space_no][*src++];
      *d++ = 1.0;
    }
}

static void
conv_ga8_linear_gaF_linear (const Babl    *conversion,
                            unsigned char *src,
                            unsigned char *dst,
                            long           samples)
{
  u8_linear_to_float_linear (conversion, src, dst, samples * 2);
}

static void
conv_ga8_gamma_2_2_gaF_linear (const Babl    *conversion,
                               unsigned char *src,
                               unsigned char *dst,
                               long           samples)
{
  int   space_no = tables_init (conversion->conversion.source->format.space);
  float *d = (float *) dst;
  long   n = samples;

  while (n--)
    {
      *d++ = lut_gamma_2_2[space_no][*src++];
      *d++ = lut_linear[*src++];
    }
}

static void
conv_ga8_gamma_2_2_rgba8_gamma_2_2 (const Babl    *conversion,
                                    unsigned char *src,
                                    unsigned char *dst,
                                    long           samples)
{
  long   n = samples;

  while (n--)
    {
      *dst++ = *src;
      *dst++ = *src;
      *dst++ = *src++;
      *dst++ = *src++;
    }
}

static void
conv_ga8_linear_rgbaF_linear (const Babl    *conversion,
                              unsigned char *src,
                              unsigned char *dst,
                              long           samples)
{
  float *d = (float *) dst;
  long   n = samples;

  while (n--)
    {
      float value = lut_linear[*src++];

      *d++ = value;
      *d++ = value;
      *d++ = value;
      *d++ = lut_linear[*src++];
    }
}

static void
conv_ga8_gamma_2_2_rgbaF_linear (const Babl    *conversion,
                                 unsigned char *src,
                                 unsigned char *dst,
                                 long           samples)
{
  int   space_no = tables_init (conversion->conversion.source->format.space);
  float *d = (float *) dst;
  long   n = samples;

  while (n--)
    {
      float value = lut_gamma_2_2[space_no][*src++];

      *d++ = value;
      *d++ = value;
      *d++ = value;
      *d++ = lut_linear[*src++];
    }
}

static void
conv_g8_linear_gF_linear (const Babl    *conversion,
                          unsigned char *src,
                          unsigned char *dst,
                          long           samples)
{
  u8_linear_to_float_linear (conversion, src, dst, samples);
}

static void
conv_g8_gamma_2_2_gF_linear (const Babl    *conversion,
                             unsigned char *src,
                             unsigned char *dst,
                             long           samples)
{
  u8_gamma_2_2_to_float_linear (conversion, src, dst, samples);
}

static void
conv_g8_linear_rgbaF_linear (const Babl    *conversion,
                             unsigned char *src,
                             unsigned char *dst,
                             long           samples)
{
  float *d = (float *) dst;
  long   n = samples;

  while (n--)
    {
      float value = lut_linear[*src++];

      *d++ = value;
      *d++ = value;
      *d++ = value;
      *d++ = 1.0;
    }
}

static void
conv_g8_gamma_2_2_rgbaF_linear (const Babl    *conversion,
                                unsigned char *src,
                                unsigned char *dst,
                                long           samples)
{
  int   space_no = tables_init (conversion->conversion.source->format.space);
  float *d = (float *) dst;
  long   n = samples;

  while (n--)
    {
      float value = lut_gamma_2_2[space_no][*src++];

      *d++ = value;
      *d++ = value;
      *d++ = value;
      *d++ = 1.0;
    }
}

static void
conv_rgbaF_linear_rgb8_linear (const Babl    *conversion,
                               unsigned char *src,
                               unsigned char *dst,
                               long           samples)
{
  float *fsrc = (float *) src;
  long n = samples;
  long int v;

  while (n--)
    {
      v = rint (*fsrc++ * 255.0);
      *dst++ = (v < 0) ? 0 : ((v > 255) ? 255 : v);

      v = rint (*fsrc++ * 255.0);
      *dst++ = (v < 0) ? 0 : ((v > 255) ? 255 : v);

      v = rint (*fsrc++ * 255.0);
      *dst++ = (v < 0) ? 0 : ((v > 255) ? 255 : v);

      fsrc++;
    }
}

static void
conv_rgbaF_linear_rgba8_linear (const Babl    *conversion,
                                unsigned char *src,
                                unsigned char *dst,
                                long           samples)
{
  float *fsrc = (float *) src;
  long n = samples;
  long int v;

  while (n--)
    {
      v = rint (*fsrc++ * 255.0);
      *dst++ = (v < 0) ? 0 : ((v > 255) ? 255 : v);

      v = rint (*fsrc++ * 255.0);
      *dst++ = (v < 0) ? 0 : ((v > 255) ? 255 : v);

      v = rint (*fsrc++ * 255.0);
      *dst++ = (v < 0) ? 0 : ((v > 255) ? 255 : v);

      v = rint (*fsrc++ * 255.0);
      *dst++ = (v < 0) ? 0 : ((v > 255) ? 255 : v);
    }
}

int init (void);

int
init (void)
{
  const Babl *ragabaaF_linear = babl_format_new (
    babl_model ("RaGaBaA"),
    babl_type ("float"),
    babl_component ("Ra"),
    babl_component ("Ga"),
    babl_component ("Ba"),
    babl_component ("A"),
    NULL);
  const Babl *rgbaF_linear = babl_format_new (
    babl_model ("RGBA"),
    babl_type ("float"),
    babl_component ("R"),
    babl_component ("G"),
    babl_component ("B"),
    babl_component ("A"),
    NULL);
  const Babl *rgba8_linear = babl_format_new (
    babl_model ("RGBA"),
    babl_type ("u8"),
    babl_component ("R"),
    babl_component ("G"),
    babl_component ("B"),
    babl_component ("A"),
    NULL);
  const Babl *rgba8_gamma_2_2 = babl_format_new (
    babl_model ("R'G'B'A"),
    babl_type ("u8"),
    babl_component ("R'"),
    babl_component ("G'"),
    babl_component ("B'"),
    babl_component ("A"),
    NULL);
  const Babl *rgbF_linear = babl_format_new (
    babl_model ("RGB"),
    babl_type ("float"),
    babl_component ("R"),
    babl_component ("G"),
    babl_component ("B"),
    NULL);
  const Babl *rgb8_linear = babl_format_new (
    babl_model ("RGB"),
    babl_type ("u8"),
    babl_component ("R"),
    babl_component ("G"),
    babl_component ("B"),
    NULL);
  const Babl *rgb8_gamma_2_2 = babl_format_new (
    babl_model ("R'G'B'"),
    babl_type ("u8"),
    babl_component ("R'"),
    babl_component ("G'"),
    babl_component ("B'"),
    NULL);
  const Babl *gaF_linear = babl_format_new (
    babl_model ("YA"),
    babl_type ("float"),
    babl_component ("Y"),
    babl_component ("A"),
    NULL);
  const Babl *ga8_linear = babl_format_new (
    babl_model ("YA"),
    babl_type ("u8"),
    babl_component ("Y"),
    babl_component ("A"),
    NULL);
  const Babl *ga8_gamma_2_2 = babl_format_new (
    babl_model ("Y'A"),
    babl_type ("u8"),
    babl_component ("Y'"),
    babl_component ("A"),
    NULL);
  const Babl *gF_linear = babl_format_new (
    babl_model ("Y"),
    babl_type ("float"),
    babl_component ("Y"),
    NULL);
  const Babl *g8_linear = babl_format_new (
    babl_model ("Y"),
    babl_type ("u8"),
    babl_component ("Y"),
    NULL);
  const Babl *g8_gamma_2_2 = babl_format_new (
    babl_model ("Y'"),
    babl_type ("u8"),
    babl_component ("Y'"),
    NULL);

  tables_init (babl_space("sRGB"));

#define o(src, dst) \
  babl_conversion_new (src, dst, "linear", conv_ ## src ## _ ## dst, NULL)

  o (rgba8_linear, ragabaaF_linear);
  o (rgba8_linear, rgbaF_linear);
  o (rgba8_gamma_2_2, rgbaF_linear);

  o (rgb8_linear, rgbF_linear);
  o (rgb8_gamma_2_2, rgbF_linear);
  o (rgb8_linear, rgbaF_linear);
  o (rgb8_linear, ragabaaF_linear);
  o (rgb8_gamma_2_2, rgbaF_linear);

  o (ga8_linear, gaF_linear);
  o (ga8_gamma_2_2, gaF_linear);
  o (ga8_linear, rgbaF_linear);
  o (ga8_gamma_2_2, rgbaF_linear);

  o (ga8_gamma_2_2, rgba8_gamma_2_2);

  o (g8_linear, gF_linear);
  o (g8_gamma_2_2, gF_linear);
  o (g8_linear, rgbaF_linear);
  o (g8_gamma_2_2, rgbaF_linear);

  o (rgbaF_linear, rgb8_linear);
  o (rgbaF_linear, rgba8_linear);

  return 0;
}
