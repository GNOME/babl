/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2013 Massimo Valentini
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

#define Q(a) { a, a, a, a }
static const __v4sf  u16_float = Q (1.f / 65535);

static void
conv_rgba16_rgbaF (const Babl     *conversion,
                   const uint16_t *src,
                   float          *dst,
                   long            samples)
{
  long i = 0;

  if (((uintptr_t)src % 16) + ((uintptr_t)dst % 16) == 0)
    {
      long           n  = (samples / 2) * 2;
      const __m128i *s  = (const __m128i*) src;
            __v4sf  *d  = (__v4sf*) dst;

      for (; i < n / 2; i++)
        {
          /* Expand shorts to ints by loading zero in the high bits */
          const __m128i t0 = _mm_unpacklo_epi16 (s[i + 0], (__m128i)_mm_setzero_ps());
          const __m128i t1 = _mm_unpackhi_epi16 (s[i + 0], (__m128i)_mm_setzero_ps());

          /* Convert to float */
          const __m128  u0 = _mm_cvtepi32_ps (t0);
          const __m128  u1 = _mm_cvtepi32_ps (t1);

          const __v4sf rgba0 = u0 * u16_float;
          const __v4sf rgba1 = u1 * u16_float;

          d[2 * i + 0] = rgba0;
          d[2 * i + 1] = rgba1;
        }
      _mm_empty();
    }

  for (i *= 2 * 4; i != 4 * samples; i++)
    dst[i] = src[i] * (1.f / 65535);
}

static void
conv_rgba16_rgbAF (const Babl     *conversion,
                   const uint16_t *src,
                   float          *dst,
                   long samples)
{
  long i = 0;
  long remainder;

  if (((uintptr_t)src % 16) + ((uintptr_t)dst % 16) == 0)
    {
      long           n  = (samples / 2) * 2;
      const __m128i *s  = (const __m128i*) src;
            __v4sf  *d  = (__v4sf*) dst;

      const __v4sf  max_mask = { 0.0f, 0.0f, 0.0f, 1.0f };

      for (; i < n / 2; i++)
        {
          /* Expand shorts to ints by loading zero in the high bits */
          const __m128i t0 = _mm_unpacklo_epi16 (s[i + 0], (__m128i)_mm_setzero_ps());
          const __m128i t1 = _mm_unpackhi_epi16 (s[i + 0], (__m128i)_mm_setzero_ps());

          /* Convert to float */
          const __m128  u0 = _mm_cvtepi32_ps (t0);
          const __m128  u1 = _mm_cvtepi32_ps (t1);

          /* Multiply by 1 / 65535 */
          __v4sf rgba0 = u0 * u16_float;
          __v4sf rgba1 = u1 * u16_float;
          
          /* Expand alpha */
          __v4sf aaaa0 = (__v4sf)_mm_shuffle_epi32((__m128i)rgba0, _MM_SHUFFLE(3, 3, 3, 3));
          __v4sf aaaa1 = (__v4sf)_mm_shuffle_epi32((__m128i)rgba1, _MM_SHUFFLE(3, 3, 3, 3));
          
          /* Set the value in the alpha slot to 1.0, we know max is sufficent because alpha was a short */
          aaaa0 = _mm_max_ps(aaaa0, max_mask);
          aaaa1 = _mm_max_ps(aaaa1, max_mask);
          
          /* Premultiply */
          rgba0 = rgba0 * aaaa0;
          rgba1 = rgba1 * aaaa1;
          
          d[2 * i + 0] = rgba0;
          d[2 * i + 1] = rgba1;
        }
      _mm_empty();
    }

  dst += i * 2 * 4;
  src += i * 2 * 4;
  remainder = samples - (i * 2);
  while (remainder--)
  {
    const float a = src[3] / 65535.0f;
    const float a_term = a / 65535.0f;
    dst[0] = src[0] * a_term;
    dst[1] = src[1] * a_term;
    dst[2] = src[2] * a_term;
    dst[3] = a;
    
    src += 4;
    dst += 4;
  }
}

#endif /* defined(USE_SSE2) */

int init (void);

int
init (void)
{
#if defined(USE_SSE2)

  const Babl *rgbaF_linear = babl_format_new (
    babl_model ("RGBA"),
    babl_type ("float"),
    babl_component ("R"),
    babl_component ("G"),
    babl_component ("B"),
    babl_component ("A"),
    NULL);
  const Babl *rgbAF_linear = babl_format_new (
    babl_model ("RaGaBaA"),
    babl_type ("float"),
    babl_component ("Ra"),
    babl_component ("Ga"),
    babl_component ("Ba"),
    babl_component ("A"),
    NULL);
  const Babl *rgba16_linear = babl_format_new (
    babl_model ("RGBA"),
    babl_type ("u16"),
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
  const Babl *rgbAF_gamma = babl_format_new (
    babl_model ("R'aG'aB'aA"),
    babl_type ("float"),
    babl_component ("R'a"),
    babl_component ("G'a"),
    babl_component ("B'a"),
    babl_component ("A"),
    NULL);
  const Babl *rgba16_gamma = babl_format_new (
    babl_model ("R'G'B'A"),
    babl_type ("u16"),
    babl_component ("R'"),
    babl_component ("G'"),
    babl_component ("B'"),
    babl_component ("A"),
    NULL);

#define CONV(src, dst) \
{ \
  babl_conversion_new (src ## _linear, dst ## _linear, "linear", conv_ ## src ## _ ## dst, NULL); \
  babl_conversion_new (src ## _gamma, dst ## _gamma, "linear", conv_ ## src ## _ ## dst, NULL); \
}

  if ((babl_cpu_accel_get_support () & BABL_CPU_ACCEL_X86_SSE) &&
      (babl_cpu_accel_get_support () & BABL_CPU_ACCEL_X86_SSE2))
    {
      CONV (rgba16, rgbaF);
      CONV (rgba16, rgbAF);
    }

#endif /* defined(USE_SSE2) */

  return 0;
}

