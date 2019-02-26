/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2015 Daniel Sabo
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

#if defined(USE_SSE4_1) && defined(USE_F16C) && defined(ARCH_X86_64)

#include <immintrin.h>

#include <stdint.h>
#include <stdlib.h>

#include "babl.h"
#include "babl-cpuaccel.h"
#include "extensions/util.h"

static inline void
conv_yHalf_yF (const Babl     *conversion,
               const uint16_t *src, 
               float          *dst, 
               long            samples)
{
  const uint64_t *s_vec;
  __v4sf         *d_vec;

  long n = samples;

  s_vec = (const uint64_t *)src;
  d_vec = (__v4sf *)dst;

  while (n >= 4)
    {
      __m128i in_val = _mm_insert_epi64((__m128i)_mm_setzero_ps(), *s_vec++, 0);
      __v4sf out_val = (__v4sf)_mm_cvtph_ps(in_val);
      _mm_storeu_ps((float *)d_vec++, out_val);
      n -= 4;
    }

  src = (const uint16_t *)s_vec;
  dst = (float *)d_vec;

  while (n)
    {
      __m128i in_val = _mm_insert_epi16((__m128i)_mm_setzero_ps(), *src++, 0);
      __v4sf out_val = (__v4sf)_mm_cvtph_ps(in_val);
      _mm_store_ss(dst++, out_val);
      n -= 1;
    }
}

static void
conv_yaHalf_yaF (const Babl     *conversion,
                 const uint16_t *src, 
                 float          *dst, 
                 long            samples)
{
  conv_yHalf_yF (conversion, src, dst, samples * 2);
}

static void
conv_rgbHalf_rgbF (const Babl     *conversion,
                   const uint16_t *src, 
                   float          *dst, 
                   long            samples)
{
  conv_yHalf_yF (conversion, src, dst, samples * 3);
}

static void
conv_rgbaHalf_rgbaF (const Babl     *conversion,
                     const uint16_t *src, 
                     float          *dst, 
                     long            samples)
{
  conv_yHalf_yF (conversion, src, dst, samples * 4);
}

static inline void
conv_yF_yHalf (const Babl  *conversion,
               const float *src, 
               uint16_t    *dst, 
               long         samples)
{
  const __v4sf *s_vec;
  uint64_t     *d_vec;

  long n = samples;

  s_vec = (const __v4sf *)src;
  d_vec = (uint64_t *)dst;

  while (n >= 4)
    {
      __m128 in_val = _mm_loadu_ps((float *)s_vec++);
      __m128i out_val = _mm_cvtps_ph(in_val, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
      _mm_storel_epi64((__m128i *)d_vec++, out_val);
      n -= 4;
    }

  src = (const float *)s_vec;
  dst = (uint16_t *)d_vec;

  while (n)
    {
      __m128 in_val = _mm_load_ss(src++);
      __m128i out_val = _mm_cvtps_ph(in_val, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
      *dst++ = _mm_extract_epi16(out_val, 0);
      n -= 1;
    }
}

static void
conv_yaF_yaHalf (const Babl  *conversion,
                 const float *src, 
                 uint16_t    *dst, 
                 long         samples)
{
  conv_yF_yHalf (conversion, src, dst, samples * 2);
}

static void
conv_rgbF_rgbHalf (const Babl  *conversion,
                   const float *src, 
                   uint16_t    *dst, 
                   long         samples)
{
  conv_yF_yHalf (conversion, src, dst, samples * 3);
}

static void
conv_rgbaF_rgbaHalf (const Babl  *conversion,
                     const float *src, 
                     uint16_t    *dst, 
                     long         samples)
{
  conv_yF_yHalf (conversion, src, dst, samples * 4);
}

#endif /* defined(USE_SSE4_1) && defined(USE_F16C) && defined(ARCH_X86_64) */

int init (void);

int
init (void)
{
#if defined(USE_SSE4_1) && defined(USE_F16C) && defined(ARCH_X86_64)
  const Babl *rgbaF_linear = babl_format_new (
    babl_model ("RGBA"),
    babl_type ("float"),
    babl_component ("R"),
    babl_component ("G"),
    babl_component ("B"),
    babl_component ("A"),
    NULL);
  const Babl *rgbaHalf_linear = babl_format_new (
    babl_model ("RGBA"),
    babl_type ("half"),
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
  const Babl *rgbaHalf_gamma = babl_format_new (
    babl_model ("R'G'B'A"),
    babl_type ("half"),
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
  const Babl *rgbHalf_linear = babl_format_new (
    babl_model ("RGB"),
    babl_type ("half"),
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
  const Babl *rgbHalf_gamma = babl_format_new (
    babl_model ("R'G'B'"),
    babl_type ("half"),
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
  const Babl *yaHalf_linear = babl_format_new (
    babl_model ("YA"),
    babl_type ("half"),
    babl_component ("Y"),
    babl_component ("A"),
    NULL);
  const Babl *yaF_gamma = babl_format_new (
    babl_model ("Y'A"),
    babl_type ("float"),
    babl_component ("Y'"),
    babl_component ("A"),
    NULL);
  const Babl *yaHalf_gamma = babl_format_new (
    babl_model ("Y'A"),
    babl_type ("half"),
    babl_component ("Y'"),
    babl_component ("A"),
    NULL);
  const Babl *yF_linear = babl_format_new (
    babl_model ("Y"),
    babl_type ("float"),
    babl_component ("Y"),
    NULL);
  const Babl *yHalf_linear = babl_format_new (
    babl_model ("Y"),
    babl_type ("half"),
    babl_component ("Y"),
    NULL);
  const Babl *yF_gamma = babl_format_new (
    babl_model ("Y'"),
    babl_type ("float"),
    babl_component ("Y'"),
    NULL);
  const Babl *yHalf_gamma = babl_format_new (
    babl_model ("Y'"),
    babl_type ("half"),
    babl_component ("Y'"),
    NULL);

#define CONV(src, dst) \
{ \
  babl_conversion_new (src ## _linear, dst ## _linear, "linear", conv_ ## src ## _ ## dst, NULL); \
  babl_conversion_new (src ## _gamma, dst ## _gamma, "linear", conv_ ## src ## _ ## dst, NULL); \
}

  if ((babl_cpu_accel_get_support () & BABL_CPU_ACCEL_X86_SSE4_1) &&
      (babl_cpu_accel_get_support () & BABL_CPU_ACCEL_X86_F16C))
    {
      CONV(rgbaHalf, rgbaF);
      CONV(rgbHalf,  rgbF);
      CONV(yaHalf,   yaF);
      CONV(yHalf,    yF);
      CONV(rgbaF,    rgbaHalf);
      CONV(rgbF,     rgbHalf);
      CONV(yaF,      yaHalf);
      CONV(yF,       yHalf);
    }

#endif /* defined(USE_SSE4_1) && defined(USE_F16C) && defined(ARCH_X86_64) */

  return 0;
}

