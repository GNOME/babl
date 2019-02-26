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

#if defined(USE_SSE2)

/* SSE 2 */
#include <emmintrin.h>

#include <stdint.h>
#include <stdlib.h>

#include "babl.h"
#include "babl-cpuaccel.h"
#include "extensions/util.h"

static inline void
conv_yF_y8 (const Babl  *conversion,
            const float *src, 
            uint8_t     *dst, 
            long         samples)
{
  const __v4sf *s_vec;
  __m128i      *d_vec;
  uint32_t     *d_int;

  long n = samples;

  const __v4sf byte_fill = _mm_set_ps1(255.0f);
  const __v4sf half      = _mm_set_ps1(0.5);

  while (((uintptr_t)src % 16) && n > 0)
    {
      /* Work through the unaligned floats */
      float y = *src++;
      *dst++ = (y >= 1.0f) ? 0xFF : ((y <= 0.0f) ? 0x0 : 0xFF * y + 0.5f);

      n -= 1;
    }

  s_vec = (__v4sf *)src;
  d_vec = (__m128i *)dst;

  /* Aligned chunks */

  while (n > 16)
    {
      __v4sf  yyyy0, yyyy1, yyyy2, yyyy3;
      __m128i i32_0, i32_1, i32_2, i32_3;
      __m128i i16_01, i16_23;
      __m128i mm_ints;

      /* Add 0.5 and truncate, to match C rounding behavior.
       *
       * The _mm_min_ps is needed because _mm_packs_epi32 uses
       * signed saturation, the unsigned version wasn't added
       * until SSE4.
       */
      yyyy0 = *s_vec++ * byte_fill + half;
      yyyy0 = _mm_min_ps(yyyy0, byte_fill);
      i32_0 = _mm_cvttps_epi32 ((__m128)yyyy0);

      yyyy1 = *s_vec++ * byte_fill + half;
      yyyy1 = _mm_min_ps(yyyy1, byte_fill);
      i32_1 = _mm_cvttps_epi32 ((__m128)yyyy1);

      i16_01 = _mm_packs_epi32 (i32_0, i32_1);

      yyyy2 = *s_vec++ * byte_fill + half;
      yyyy2 = _mm_min_ps(yyyy2, byte_fill);
      i32_2 = _mm_cvttps_epi32 ((__m128)yyyy2);

      yyyy3 = *s_vec++ * byte_fill + half;
      yyyy3 = _mm_min_ps(yyyy3, byte_fill);
      i32_3 = _mm_cvttps_epi32 ((__m128)yyyy3);

      i16_23 = _mm_packs_epi32 (i32_2, i32_3);

      mm_ints = _mm_packus_epi16 (i16_01, i16_23);

      _mm_storeu_si128 (d_vec++, mm_ints);

      n -= 16;
    }

  d_int = (uint32_t *)d_vec;

  while (n > 4)
    {
      __v4sf  yyyy0;
      __m128i mm_ints;

      yyyy0 = *s_vec++ * byte_fill + half;
      yyyy0 = _mm_min_ps(yyyy0, byte_fill);
      mm_ints = _mm_cvttps_epi32 ((__m128)yyyy0);
      mm_ints = _mm_packs_epi32 (mm_ints, mm_ints);
      mm_ints = _mm_packus_epi16 (mm_ints, mm_ints);
      _mm_store_ss ((float *)d_int++, (__v4sf)mm_ints);

      n -= 4;
    }

  src = (float *)s_vec;
  dst = (uint8_t *)d_int;

  while (n > 0)
    {
      float y = *src++;
      *dst++ = (y >= 1.0f) ? 0xFF : ((y <= 0.0f) ? 0x0 : 0xFF * y + 0.5f);

      n -= 1;
    }
}

static void
conv_yaF_ya8 (const Babl  *conversion,
              const float *src, 
              uint8_t     *dst, 
              long         samples)
{
  conv_yF_y8 (conversion, src, dst, samples * 2);
}


static void
conv_rgbF_rgb8 (const Babl  *conversion,
                const float *src, 
                uint8_t     *dst, 
                long         samples)
{
  conv_yF_y8 (conversion, src, dst, samples * 3);
}

static void
conv_rgbaF_rgba8 (const Babl  *conversion,
                  const float *src, 
                  uint8_t     *dst, 
                  long         samples)
{
  conv_yF_y8 (conversion, src, dst, samples * 4);
}

static void
conv_rgbAF_rgbA8 (const Babl  *conversion,
                  const float *src, 
                  uint8_t     *dst, 
                  long         samples)
{
  conv_yF_y8 (conversion, src, dst, samples * 4);
}

#endif

int init (void);

int
init (void)
{
#if defined(USE_SSE2)

  const Babl *rgbAF_linear = babl_format_new (
    babl_model ("RaGaBaA"),
    babl_type ("float"),
    babl_component ("Ra"),
    babl_component ("Ga"),
    babl_component ("Ba"),
    babl_component ("A"),
    NULL);
  const Babl *rgbA8_linear = babl_format_new (
    babl_model ("RaGaBaA"),
    babl_type ("u8"),
    babl_component ("Ra"),
    babl_component ("Ga"),
    babl_component ("Ba"),
    babl_component ("A"),
    NULL);
  const Babl *rgbAF_gamma = babl_format_new (
    babl_model ("R'aG'aB'aA"),
    babl_type ("float"),
    babl_component ("R'a"),
    babl_component ("G'a"),
    babl_component ("B'a"),
    babl_component ("A"),
    NULL);
  const Babl *rgbA8_gamma = babl_format_new (
    babl_model ("R'aG'aB'aA"),
    babl_type ("u8"),
    babl_component ("R'a"),
    babl_component ("G'a"),
    babl_component ("B'a"),
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

  if ((babl_cpu_accel_get_support () & BABL_CPU_ACCEL_X86_SSE2))
    {
      CONV(rgbaF, rgba8);
      CONV(rgbAF, rgbA8);
      CONV(rgbF,  rgb8);
      CONV(yaF,   ya8);
      CONV(yF,    y8);
    }

#endif

  return 0;
}

