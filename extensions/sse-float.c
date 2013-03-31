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

#if defined(USE_SSE2)

/* SSE 2 */
#include <emmintrin.h>

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
      const long    n = (samples / 2) * 2;
      const __v4sf *s = (const __v4sf*) src;
            __v4sf *d = (__v4sf*)dst;

      for ( ; i < n; i += 2)
        {
          __v4sf rbaa0, rbaa1;
        
          __v4sf rgba0 = *s++;
          __v4sf rgba1 = *s++;

          /* Expand alpha */
          __v4sf aaaa0 = (__v4sf)_mm_shuffle_epi32((__m128i)rgba0, _MM_SHUFFLE(3, 3, 3, 3));
          __v4sf aaaa1 = (__v4sf)_mm_shuffle_epi32((__m128i)rgba1, _MM_SHUFFLE(3, 3, 3, 3));
          
          /* Premultiply */
          rgba0 = rgba0 * aaaa0;
          rgba1 = rgba1 * aaaa1;
          
          /* Shuffle the original alpha value back in */
          rbaa0 = _mm_shuffle_ps(rgba0, aaaa0, _MM_SHUFFLE(0, 0, 2, 0));
          rbaa1 = _mm_shuffle_ps(rgba1, aaaa1, _MM_SHUFFLE(0, 0, 2, 0));
          
          rgba0 = _mm_shuffle_ps(rgba0, rbaa0, _MM_SHUFFLE(2, 1, 1, 0));
          rgba1 = _mm_shuffle_ps(rgba1, rbaa1, _MM_SHUFFLE(2, 1, 1, 0));
          
          *d++ = rgba0;
          *d++ = rgba1;
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
      const long    n = (samples / 2) * 2;
      const __v4sf *s = (const __v4sf*) src;
            __v4sf *d = (__v4sf*)dst;

      for ( ; i < n; i += 2)
        {
          __v4sf rbaa0, rbaa1;
          
          __v4sf rgba0 = *s++;
          __v4sf rgba1 = *s++;

          /* Expand alpha */
          __v4sf aaaa0 = (__v4sf)_mm_shuffle_epi32((__m128i)rgba0, _MM_SHUFFLE(3, 3, 3, 3));
          __v4sf aaaa1 = (__v4sf)_mm_shuffle_epi32((__m128i)rgba1, _MM_SHUFFLE(3, 3, 3, 3));
          
          /* Premultiply */
          rgba0 = rgba0 / aaaa0;
          rgba1 = rgba1 / aaaa1;
          
          /* Shuffle the original alpha value back in */
          rbaa0 = _mm_shuffle_ps(rgba0, aaaa0, _MM_SHUFFLE(0, 0, 2, 0));
          rbaa1 = _mm_shuffle_ps(rgba1, aaaa1, _MM_SHUFFLE(0, 0, 2, 0));
          rgba0 = _mm_shuffle_ps(rgba0, rbaa0, _MM_SHUFFLE(2, 1, 1, 0));
          rgba1 = _mm_shuffle_ps(rgba1, rbaa1, _MM_SHUFFLE(2, 1, 1, 0));
          
          *d++ = rgba0;
          *d++ = rgba1;
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

#endif /* defined(USE_SSE2) */

#define o(src, dst) \
  babl_conversion_new (src, dst, "linear", conv_ ## src ## _ ## dst, NULL)

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

  if ((babl_cpu_accel_get_support () & BABL_CPU_ACCEL_X86_SSE) &&
      (babl_cpu_accel_get_support () & BABL_CPU_ACCEL_X86_SSE2))
      
    {
      o (rgbaF_linear, rgbAF_linear);
      o (rgbAF_linear, rgbaF_linear);
    }

#endif /* defined(USE_SSE2) */

  return 0;
}

