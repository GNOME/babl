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
 * <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#if defined(__GNUC__) && (__GNUC__ >= 4) && defined(USE_SSE) && defined(USE_MMX)

#include <xmmintrin.h>

#include <stdint.h>
#include <stdlib.h>

#include "babl.h"
#include "babl-cpuaccel.h"
#include "extensions/util.h"

#define Q(a) { a, a, a, a }

static long
conv_rgbaF_linear_rgbAF_linear (const float *src, float *dst, long samples)
{
  long i = 0;
  long remainder;

  if (((uintptr_t)src % 16) + ((uintptr_t)dst % 16) == 0)
    {
      const long    n = (samples / 4) * 4;
      const __v4sf *s = (const __v4sf*) src;
            __v4sf *d = (__v4sf*)dst;

      for ( ; i < n; i += 4)
        {
          const __v4sf s0 = *s++;
          const __v4sf s1 = *s++;
          const __v4sf s2 = *s++;
          const __v4sf s3 = *s++;

          /* Shuffle the pixels into a planar layout */
          const __v4sf rg01 = _mm_unpacklo_ps (s0, s1);
          const __v4sf ba01 = _mm_unpackhi_ps (s0, s1);
          const __v4sf rg23 = _mm_unpacklo_ps (s2, s3);
          const __v4sf ba23 = _mm_unpackhi_ps (s2, s3);

          const __v4sf r0213 = _mm_unpacklo_ps (rg01, rg23);
          const __v4sf g0213 = _mm_unpackhi_ps (rg01, rg23);
          const __v4sf b0213 = _mm_unpacklo_ps (ba01, ba23);
          const __v4sf a0213 = _mm_unpackhi_ps (ba01, ba23);

          const __v4sf R0213 = r0213 * a0213;
          const __v4sf G0213 = g0213 * a0213;
          const __v4sf B0213 = b0213 * a0213;

          const __v4sf RB02 = _mm_unpacklo_ps (R0213, B0213);
          const __v4sf RB13 = _mm_unpackhi_ps (R0213, B0213);
          const __v4sf Ga02 = _mm_unpacklo_ps (G0213, a0213);
          const __v4sf Ga13 = _mm_unpackhi_ps (G0213, a0213);

          *d++ = _mm_unpacklo_ps (RB02, Ga02);
          *d++ = _mm_unpacklo_ps (RB13, Ga13);
          *d++ = _mm_unpackhi_ps (RB02, Ga02);
          *d++ = _mm_unpackhi_ps (RB13, Ga13);
        }
      _mm_empty ();
    }

  dst += i * 4;
  src += i * 4;
  remainder = samples - i;
  while (remainder--)
  {
    const float a = src[3];
    dst[0] = src[0] * a;
    dst[1] = src[1] * a;
    dst[2] = src[2] * a;
    dst[3] = a;
    
    src += 4;
    dst += 4;
  }

  return samples;
}

static long
conv_rgbAF_linear_rgbaF_linear (const float *src, float *dst, long samples)
{
  long i = 0;
  long remainder;

  if (((uintptr_t)src % 16) + ((uintptr_t)dst % 16) == 0)
    {
      const long    n = (samples / 4) * 4;
      const __v4sf *s = (const __v4sf*) src;
            __v4sf *d = (__v4sf*)dst;

      for ( ; i < n; i += 4)
        {
          const __v4sf s0 = *s++;
          const __v4sf s1 = *s++;
          const __v4sf s2 = *s++;
          const __v4sf s3 = *s++;

          /* Shuffle the pixels into a planar layout */
          const __v4sf rg01 = _mm_unpacklo_ps (s0, s1);
          const __v4sf ba01 = _mm_unpackhi_ps (s0, s1);
          const __v4sf rg23 = _mm_unpacklo_ps (s2, s3);
          const __v4sf ba23 = _mm_unpackhi_ps (s2, s3);

          const __v4sf r0213 = _mm_unpacklo_ps (rg01, rg23);
          const __v4sf g0213 = _mm_unpackhi_ps (rg01, rg23);
          const __v4sf b0213 = _mm_unpacklo_ps (ba01, ba23);
          const __v4sf a0213 = _mm_unpackhi_ps (ba01, ba23);

          const __v4sf R0213 = r0213 / a0213;
          const __v4sf G0213 = g0213 / a0213;
          const __v4sf B0213 = b0213 / a0213;

          const __v4sf RB02 = _mm_unpacklo_ps (R0213, B0213);
          const __v4sf RB13 = _mm_unpackhi_ps (R0213, B0213);
          const __v4sf Ga02 = _mm_unpacklo_ps (G0213, a0213);
          const __v4sf Ga13 = _mm_unpackhi_ps (G0213, a0213);

          *d++ = _mm_unpacklo_ps (RB02, Ga02);
          *d++ = _mm_unpacklo_ps (RB13, Ga13);
          *d++ = _mm_unpackhi_ps (RB02, Ga02);
          *d++ = _mm_unpackhi_ps (RB13, Ga13);
        }
      _mm_empty ();
    }

  dst += i * 4;
  src += i * 4;
  remainder = samples - i;
  while (remainder--)
  {
    const float a = src[3];
    const float a_term = 1.0f / a;
    dst[0] = src[0] * a_term;
    dst[1] = src[1] * a_term;
    dst[2] = src[2] * a_term;
    dst[3] = a;
    
    src += 4;
    dst += 4;
  }

  return samples;
}

#endif /* defined(__GNUC__) && (__GNUC__ >= 4) && defined(USE_SSE) && defined(USE_MMX) */

#define o(src, dst) \
  babl_conversion_new (src, dst, "linear", conv_ ## src ## _ ## dst, NULL)

int init (void);

int
init (void)
{
#if defined(__GNUC__) && (__GNUC__ >= 4) && defined(USE_SSE) && defined(USE_MMX)

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

  if ((babl_cpu_accel_get_support () & BABL_CPU_ACCEL_X86_MMX) &&
      (babl_cpu_accel_get_support () & BABL_CPU_ACCEL_X86_SSE))
    {
      o (rgbaF_linear, rgbAF_linear);
      o (rgbAF_linear, rgbaF_linear);
    }

#endif

  return 0;
}

