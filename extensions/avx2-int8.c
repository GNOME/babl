/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2019 Ell
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

#if defined(USE_AVX2)

/* AVX 2 */
#include <immintrin.h>

#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#include "babl.h"
#include "babl-cpuaccel.h"
#include "extensions/util.h"
#include "extensions/avx2-int8-tables.h"

#define TABLE_SIZE (sizeof (linear_to_gamma) / sizeof (linear_to_gamma[0]))
#define SCALE      ((float) (TABLE_SIZE - 1))

#define CVT1(src, dst)                                    \
  do                                                      \
    {                                                     \
      float x = *src;                                     \
                                                          \
      if (isnan (x) || x > 1.0f)                          \
        *dst = 255;                                       \
      else if (x < 0.0f)                                  \
        *dst = 0;                                         \
      else                                                \
        *dst = linear_to_gamma[(int) (SCALE * x + 0.5f)]; \
                                                          \
      src++;                                              \
      dst++;                                              \
    }                                                     \
  while (0)

#define CVTA1(src, dst)                                   \
  do                                                      \
    {                                                     \
      float x = *src;                                     \
                                                          \
      if (isnan (x) || x > 1.0f)                          \
        *dst = 255;                                       \
      else if (x < 0.0f)                                  \
        *dst = 0;                                         \
      else                                                \
        *dst = 255.0f * x + 0.5f;                         \
                                                          \
      src++;                                              \
      dst++;                                              \
    }                                                     \
  while (0)

static inline void
conv_yF_linear_y8_gamma (const Babl  *conversion,
                         const float *src,
                         uint8_t     *dst,
                         long         samples)
{
  const __v8sf *src_vec;
  __m256i      *dst_vec;
  const __v8sf  scale = _mm256_set1_ps (SCALE);
  const __v8sf  zero  = _mm256_setzero_ps ();
  const __v8sf  half  = _mm256_set1_ps (0.5f);

  while ((uintptr_t) src % 32 && samples > 0)
    {
      CVT1 (src, dst);

      samples--;
    }

  src_vec = (const __v8sf  *) src;
  dst_vec = (__m256i       *) dst;

  while (samples >= 32)
    {
      __m256i i32_0, i32_1, i32_2, i32_3;
      __m256i i16_01,       i16_23;
      __m256i i8_0123;

      #define CVT8(i)                                                        \
        do                                                                   \
          {                                                                  \
            __v8sf yyyyyyyy;                                                 \
                                                                             \
            yyyyyyyy = scale * src_vec[i] + half;                            \
            yyyyyyyy = _mm256_max_ps (yyyyyyyy, zero);                       \
            yyyyyyyy = _mm256_min_ps (yyyyyyyy, scale);                      \
            i32_##i  = _mm256_cvttps_epi32 (yyyyyyyy);                       \
            i32_##i  = _mm256_i32gather_epi32 (linear_to_gamma, i32_##i, 4); \
          }                                                                  \
        while (0)

      CVT8 (0);
      CVT8 (1);

      i16_01 = _mm256_packus_epi32 (i32_0, i32_1);

      CVT8 (2);
      CVT8 (3);

      i16_23 = _mm256_packus_epi32 (i32_2, i32_3);

      i8_0123 = _mm256_packus_epi16 (i16_01, i16_23);
      i8_0123 = _mm256_permutevar8x32_epi32 (
        i8_0123,
        _mm256_setr_epi32 (0, 4, 1, 5,
                           2, 6, 3, 7));

      _mm256_storeu_si256 (dst_vec, i8_0123);

      #undef CVT8

      src_vec += 4;
      dst_vec++;

      samples -= 32;
    }

  src = (const float *) src_vec;
  dst = (uint8_t     *) dst_vec;

  while (samples > 0)
    {
      CVT1 (src, dst);

      samples--;
    }
}

static inline void
conv_yaF_linear_ya8_gamma (const Babl  *conversion,
                           const float *src,
                           uint8_t     *dst,
                           long         samples)
{
  if ((uintptr_t) src % 8 == 0)
    {
      const __v8sf  *src_vec;
      __m256i       *dst_vec;
      const __v8sf   scale = _mm256_setr_ps (SCALE, 255.0f, SCALE, 255.0f,
                                             SCALE, 255.0f, SCALE, 255.0f);
      const __v8sf   zero  = _mm256_setzero_ps ();
      const __v8sf   half  = _mm256_set1_ps (0.5f);
      const __m256i  mask  = _mm256_setr_epi32 (-1, 0, -1, 0,
                                                -1, 0, -1, 0);

      while ((uintptr_t) src % 32 && samples > 0)
        {
          CVT1  (src, dst);
          CVTA1 (src, dst);

          samples--;
        }

      src_vec = (const __v8sf  *) src;
      dst_vec = (__m256i       *) dst;

      while (samples >= 16)
        {
          __m256i i32_0, i32_1, i32_2, i32_3;
          __m256i i16_01,       i16_23;
          __m256i i8_0123;

          #define CVT8(i)                                                  \
            do                                                             \
              {                                                            \
                __v8sf yayayaya;                                           \
                                                                           \
                yayayaya = scale * src_vec[i] + half;                      \
                yayayaya = _mm256_max_ps (yayayaya, zero);                 \
                yayayaya = _mm256_min_ps (yayayaya, scale);                \
                i32_##i  = _mm256_cvttps_epi32 (yayayaya);                 \
                i32_##i  = _mm256_mask_i32gather_epi32 (i32_##i,           \
                                                        linear_to_gamma,   \
                                                        i32_##i, mask, 4); \
              }                                                            \
            while (0)

          CVT8 (0);
          CVT8 (1);

          i16_01 = _mm256_packus_epi32 (i32_0, i32_1);

          CVT8 (2);
          CVT8 (3);

          i16_23 = _mm256_packus_epi32 (i32_2, i32_3);

          i8_0123 = _mm256_packus_epi16 (i16_01, i16_23);
          i8_0123 = _mm256_permutevar8x32_epi32 (
            i8_0123,
            _mm256_setr_epi32 (0, 4, 1, 5,
                               2, 6, 3, 7));

          _mm256_storeu_si256 (dst_vec, i8_0123);

          #undef CVT8

          src_vec += 4;
          dst_vec++;

          samples -= 16;
        }

      src = (const float *) src_vec;
      dst = (uint8_t     *) dst_vec;
    }

  while (samples > 0)
    {
      CVT1  (src, dst);
      CVTA1 (src, dst);

      samples--;
    }
}

static void
conv_rgbF_linear_rgb8_gamma (const Babl  *conversion,
                             const float *src,
                             uint8_t     *dst,
                             long         samples)
{
  conv_yF_linear_y8_gamma (conversion, src, dst, 3 * samples);
}

static inline void
conv_rgbaF_linear_rgba8_gamma (const Babl  *conversion,
                               const float *src,
                               uint8_t     *dst,
                               long         samples)
{
  if ((uintptr_t) src % 16 == 0)
    {
      const __v8sf  *src_vec;
      __m256i       *dst_vec;
      const __v8sf   scale = _mm256_setr_ps (SCALE, SCALE, SCALE, 255.0f,
                                             SCALE, SCALE, SCALE, 255.0f);
      const __v8sf   zero  = _mm256_setzero_ps ();
      const __v8sf   half  = _mm256_set1_ps (0.5f);
      const __m256i  mask  = _mm256_setr_epi32 (-1, -1, -1, 0,
                                                -1, -1, -1, 0);

      while ((uintptr_t) src % 32 && samples > 0)
        {
          CVT1  (src, dst);
          CVT1  (src, dst);
          CVT1  (src, dst);
          CVTA1 (src, dst);

          samples--;
        }

      src_vec = (const __v8sf  *) src;
      dst_vec = (__m256i       *) dst;

      while (samples >= 8)
        {
          __m256i i32_0, i32_1, i32_2, i32_3;
          __m256i i16_01,       i16_23;
          __m256i i8_0123;

          #define CVT8(i)                                                  \
            do                                                             \
              {                                                            \
                __v8sf rgbargba;                                           \
                                                                           \
                rgbargba = scale * src_vec[i] + half;                      \
                rgbargba = _mm256_max_ps (rgbargba, zero);                 \
                rgbargba = _mm256_min_ps (rgbargba, scale);                \
                i32_##i  = _mm256_cvttps_epi32 (rgbargba);                 \
                i32_##i  = _mm256_mask_i32gather_epi32 (i32_##i,           \
                                                        linear_to_gamma,   \
                                                        i32_##i, mask, 4); \
              }                                                            \
            while (0)

          CVT8 (0);
          CVT8 (1);

          i16_01 = _mm256_packus_epi32 (i32_0, i32_1);

          CVT8 (2);
          CVT8 (3);

          i16_23 = _mm256_packus_epi32 (i32_2, i32_3);

          i8_0123 = _mm256_packus_epi16 (i16_01, i16_23);
          i8_0123 = _mm256_permutevar8x32_epi32 (
            i8_0123,
            _mm256_setr_epi32 (0, 4, 1, 5,
                               2, 6, 3, 7));

          _mm256_storeu_si256 (dst_vec, i8_0123);

          #undef CVT8

          src_vec += 4;
          dst_vec++;

          samples -= 8;
        }

      src = (const float *) src_vec;
      dst = (uint8_t     *) dst_vec;
    }

  while (samples > 0)
    {
      CVT1  (src, dst);
      CVT1  (src, dst);
      CVT1  (src, dst);
      CVTA1 (src, dst);

      samples--;
    }
}

#undef CVT1
#undef CVTA1

#define CVT1(src, dst) \
  (*dst++ = gamma_to_linear[*src++])

#define CVTA1(src, dst) \
  (*dst++ = gamma_to_linear[*src++ + 256])

static inline void
conv_y8_gamma_yF_linear (const Babl    *conversion,
                         const uint8_t *src,
                         float         *dst,
                         long           samples)
{
  const __m128i *src_vec;
  __v8sf        *dst_vec;

  while ((uintptr_t) dst % 32 && samples > 0)
    {
      CVT1 (src, dst);

      samples--;
    }

  src_vec = (const __m128i *) src;
  dst_vec = (__v8sf        *) dst;

  while (samples >= 16)
    {
      __m128i i8_01;
      __m256i i32_0;

      i8_01 = _mm_loadu_si128 (src_vec++);

      i32_0      = _mm256_cvtepu8_epi32 (i8_01);
      *dst_vec++ = _mm256_i32gather_ps (gamma_to_linear, i32_0, 4);

      i8_01 = _mm_shuffle_epi32 (i8_01, _MM_SHUFFLE (1, 0, 3, 2));

      i32_0      = _mm256_cvtepu8_epi32 (i8_01);
      *dst_vec++ = _mm256_i32gather_ps (gamma_to_linear, i32_0, 4);

      samples -= 16;
    }

  src = (const uint8_t *) src_vec;
  dst = (float         *) dst_vec;

  while (samples > 0)
    {
      CVT1 (src, dst);

      samples--;
    }
}

static inline void
conv_ya8_gamma_yaF_linear (const Babl    *conversion,
                           const uint8_t *src,
                           float         *dst,
                           long           samples)
{
  const __m128i *src_vec;
  __v8sf        *dst_vec;
  const __m256i  offset = _mm256_setr_epi32 (0, 256, 0, 256,
                                             0, 256, 0, 256);

  while ((uintptr_t) dst % 32 && samples > 0)
    {
      CVT1  (src, dst);
      CVTA1 (src, dst);

      samples--;
    }

  src_vec = (const __m128i *) src;
  dst_vec = (__v8sf        *) dst;

  while (samples >= 8)
    {
      __m128i i8_01;
      __m256i i32_0;

      i8_01 = _mm_loadu_si128 (src_vec++);

      i32_0       = _mm256_cvtepu8_epi32 (i8_01);
      i32_0      += offset;
      *dst_vec++  = _mm256_i32gather_ps (gamma_to_linear, i32_0, 4);

      i8_01 = _mm_shuffle_epi32 (i8_01, _MM_SHUFFLE (1, 0, 3, 2));

      i32_0       = _mm256_cvtepu8_epi32 (i8_01);
      i32_0      += offset;
      *dst_vec++  = _mm256_i32gather_ps (gamma_to_linear, i32_0, 4);

      samples -= 8;
    }

  src = (const uint8_t *) src_vec;
  dst = (float         *) dst_vec;

  while (samples > 0)
    {
      CVT1  (src, dst);
      CVTA1 (src, dst);

      samples--;
    }
}

static inline void
conv_rgb8_gamma_rgbF_linear (const Babl    *conversion,
                             const uint8_t *src,
                             float         *dst,
                             long           samples)
{
  conv_y8_gamma_yF_linear (conversion, src, dst, 3 * samples);
}

static inline void
conv_rgba8_gamma_rgbaF_linear (const Babl    *conversion,
                               const uint8_t *src,
                               float         *dst,
                               long           samples)
{
  const __m128i *src_vec;
  __v8sf        *dst_vec;
  const __m256i  offset = _mm256_setr_epi32 (0, 0, 0, 256,
                                             0, 0, 0, 256);

  while ((uintptr_t) dst % 32 && samples > 0)
    {
      CVT1  (src, dst);
      CVT1  (src, dst);
      CVT1  (src, dst);
      CVTA1 (src, dst);

      samples--;
    }

  src_vec = (const __m128i *) src;
  dst_vec = (__v8sf        *) dst;

  while (samples >= 4)
    {
      __m128i i8_01;
      __m256i i32_0;

      i8_01 = _mm_loadu_si128 (src_vec++);

      i32_0       = _mm256_cvtepu8_epi32 (i8_01);
      i32_0      += offset;
      *dst_vec++  = _mm256_i32gather_ps (gamma_to_linear, i32_0, 4);

      i8_01 = _mm_shuffle_epi32 (i8_01, _MM_SHUFFLE (1, 0, 3, 2));

      i32_0       = _mm256_cvtepu8_epi32 (i8_01);
      i32_0      += offset;
      *dst_vec++  = _mm256_i32gather_ps (gamma_to_linear, i32_0, 4);

      samples -= 4;
    }

  src = (const uint8_t *) src_vec;
  dst = (float         *) dst_vec;

  while (samples > 0)
    {
      CVT1  (src, dst);
      CVT1  (src, dst);
      CVT1  (src, dst);
      CVTA1 (src, dst);

      samples--;
    }
}

#undef CVT1
#undef CVTA1

#endif /* defined(USE_AVX2) */

int init (void);

int
init (void)
{
#if defined(USE_AVX2)

  const Babl *yF_linear = babl_format_new (
    babl_model ("Y"),
    babl_type ("float"),
    babl_component ("Y"),
    NULL);
  const Babl *y8_gamma = babl_format_new (
    babl_model ("Y'"),
    babl_type ("u8"),
    babl_component ("Y'"),
    NULL);
  const Babl *yaF_linear = babl_format_new (
    babl_model ("YA"),
    babl_type ("float"),
    babl_component ("Y"),
    babl_component ("A"),
    NULL);
  const Babl *ya8_gamma = babl_format_new (
    babl_model ("Y'A"),
    babl_type ("u8"),
    babl_component ("Y'"),
    babl_component ("A"),
    NULL);
  const Babl *rgbF_linear = babl_format_new (
    babl_model ("RGB"),
    babl_type ("float"),
    babl_component ("R"),
    babl_component ("G"),
    babl_component ("B"),
    NULL);
  const Babl *rgb8_gamma = babl_format_new (
    babl_model ("R'G'B'"),
    babl_type ("u8"),
    babl_component ("R'"),
    babl_component ("G'"),
    babl_component ("B'"),
    NULL);
  const Babl *rgbaF_linear = babl_format_new (
    babl_model ("RGBA"),
    babl_type ("float"),
    babl_component ("R"),
    babl_component ("G"),
    babl_component ("B"),
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

#define CONV(src, dst)                                                \
  do                                                                  \
    {                                                                 \
      babl_conversion_new (src ## _linear,                            \
                           dst ## _gamma,                             \
                           "linear",                                  \
                           conv_ ## src ## _linear_ ## dst ## _gamma, \
                           NULL);                                     \
                                                                      \
      babl_conversion_new (dst ## _gamma,                             \
                           src ## _linear,                            \
                           "linear",                                  \
                           conv_ ## dst ## _gamma_ ## src ## _linear, \
                           NULL);                                     \
    }                                                                 \
  while (0)

  if ((babl_cpu_accel_get_support () & BABL_CPU_ACCEL_X86_AVX2))
    {
      CONV (yF,    y8);
      CONV (yaF,   ya8);
      CONV (rgbF,  rgb8);
      CONV (rgbaF, rgba8);
    }

#endif /* defined(USE_AVX2) */

  return 0;
}

