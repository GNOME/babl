/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2013 Daniel Sabo
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

#if defined(USE_SSE4_1)

/* SSE 4 */
#include <smmintrin.h>

#include <stdint.h>
#include <stdlib.h>

#include "babl.h"
#include "babl-cpuaccel.h"
#include "extensions/util.h"

static inline void
conv_y8_yF (const Babl    *conversion,
            const uint8_t *src, 
            float         *dst, 
            long           samples)
{
  const float     factor = 1.0f / 255.0f;
  const __v4sf    factor_vec = {1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f};
  const uint32_t *s_vec;
  __v4sf         *d_vec;

  long n = samples;

  s_vec = (const uint32_t *)src;
  d_vec = (__v4sf *)dst;

  while (n >= 4)
    {
      __m128i in_val;
      __v4sf out_val;
      in_val = _mm_insert_epi32 ((__m128i)_mm_setzero_ps(), *s_vec++, 0);
      in_val = _mm_cvtepu8_epi32 (in_val);
      out_val = _mm_cvtepi32_ps (in_val) * factor_vec;
      _mm_storeu_ps ((float *)d_vec++, out_val);
      n -= 4;
    }

  src = (const uint8_t *)s_vec;
  dst = (float *)d_vec;

  while (n)
    {
      *dst++ = (float)(*src++) * factor;
      n -= 1;
    }
}

static void
conv_ya8_yaF (const Babl    *conversion,
              const uint8_t *src, 
              float         *dst, 
              long           samples)
{
  conv_y8_yF (conversion, src, dst, samples * 2);
}

static void
conv_rgb8_rgbF (const Babl    *conversion,
                const uint8_t *src, 
                float         *dst, 
                long           samples)
{
  conv_y8_yF (conversion, src, dst, samples * 3);
}

static void
conv_rgba8_rgbaF (const Babl    *conversion,
                  const uint8_t *src, 
                  float         *dst, 
                  long           samples)
{
  conv_y8_yF (conversion, src, dst, samples * 4);
}

#endif

int init (void);

int
init (void)
{
#if defined(USE_SSE4_1)
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
  const Babl *rgbaF_gamma = babl_format_new (
    babl_model ("R'G'B'A"),
    babl_type ("float"),
    babl_component ("R'"),
    babl_component ("G'"),
    babl_component ("B'"),
    babl_component ("A"),
    NULL);
  const Babl *rgba8_gamma = babl_format_new (
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
  const Babl *rgbF_gamma = babl_format_new (
    babl_model ("R'G'B'"),
    babl_type ("float"),
    babl_component ("R'"),
    babl_component ("G'"),
    babl_component ("B'"),
    NULL);
  const Babl *rgb8_gamma = babl_format_new (
    babl_model ("R'G'B'"),
    babl_type ("u8"),
    babl_component ("R'"),
    babl_component ("G'"),
    babl_component ("B'"),
    NULL);
  const Babl *yaF_linear = babl_format_new (
    babl_model ("YA"),
    babl_type ("float"),
    babl_component ("Y"),
    babl_component ("A"),
    NULL);
  const Babl *ya8_linear = babl_format_new (
    babl_model ("YA"),
    babl_type ("u8"),
    babl_component ("Y"),
    babl_component ("A"),
    NULL);
  const Babl *yaF_gamma = babl_format_new (
    babl_model ("Y'A"),
    babl_type ("float"),
    babl_component ("Y'"),
    babl_component ("A"),
    NULL);
  const Babl *ya8_gamma = babl_format_new (
    babl_model ("Y'A"),
    babl_type ("u8"),
    babl_component ("Y'"),
    babl_component ("A"),
    NULL);
  const Babl *yF_linear = babl_format_new (
    babl_model ("Y"),
    babl_type ("float"),
    babl_component ("Y"),
    NULL);
  const Babl *y8_linear = babl_format_new (
    babl_model ("Y"),
    babl_type ("u8"),
    babl_component ("Y"),
    NULL);
  const Babl *yF_gamma = babl_format_new (
    babl_model ("Y'"),
    babl_type ("float"),
    babl_component ("Y'"),
    NULL);
  const Babl *y8_gamma = babl_format_new (
    babl_model ("Y'"),
    babl_type ("u8"),
    babl_component ("Y'"),
    NULL);

#define CONV(src, dst) \
{ \
  babl_conversion_new (src ## _linear, dst ## _linear, "linear", conv_ ## src ## _ ## dst, NULL); \
  babl_conversion_new (src ## _gamma, dst ## _gamma, "linear", conv_ ## src ## _ ## dst, NULL); \
}

  if ((babl_cpu_accel_get_support () & BABL_CPU_ACCEL_X86_SSE4_1))
    {
      CONV(rgba8, rgbaF);
      CONV(rgb8,  rgbF);
      CONV(ya8,   yaF);
      CONV(y8,    yF);
    }

#endif

  return 0;
}

