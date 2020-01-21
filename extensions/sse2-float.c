/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2013 Massimo Valentini
 * Copyright (C) 2013 Daniel Sabo
 * Copyright (C) 2013 Loren Merritt
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
#include "base/util.h"
#include "extensions/util.h"

#define Q(a) { a, a, a, a }

static const float BABL_ALPHA_FLOOR_FLOAT = (float)BABL_ALPHA_FLOOR;

static void
conv_rgbaF_linear_rgbAF_linear (const Babl  *conversion,
                                const float *src,
                                float       *dst,
                                long         samples)
{
  long i = 0;
  long remainder;

  if (((uintptr_t)src % 16) + ((uintptr_t)dst % 16) == 0)
    {
      const long    n = (samples / 2) * 2;
      const __m128 *s = (const __m128*) src;
            __m128 *d = (__m128*)dst;

      for ( ; i < n; i += 2)
        {
          float alpha0 = ((float *)s)[3];
          float alpha1 = ((float *)s)[7];
          float used_alpha0 = babl_epsilon_for_zero_float (alpha0);
          float used_alpha1 = babl_epsilon_for_zero_float (alpha1);

         {
          __m128 rbaa0, rbaa1;
        
          __m128 rgba0 = *s++;
          __m128 rgba1 = *s++;


          /* Expand alpha */
          __m128 aaaa0 = _mm_set1_ps(used_alpha0);
          __m128 aaaa1 = _mm_set1_ps(used_alpha1);
          
          /* Premultiply */
          rgba0 = _mm_mul_ps(rgba0, aaaa0);
          rgba1 = _mm_mul_ps(rgba1, aaaa1);
    
          aaaa0 = _mm_set1_ps(alpha0);
          aaaa1 = _mm_set1_ps(alpha1);

          /* Shuffle the original alpha value back in */
          rbaa0 = _mm_shuffle_ps(rgba0, aaaa0, _MM_SHUFFLE(0, 0, 2, 0));
          rbaa1 = _mm_shuffle_ps(rgba1, aaaa1, _MM_SHUFFLE(0, 0, 2, 0));
          
          rgba0 = _mm_shuffle_ps(rgba0, rbaa0, _MM_SHUFFLE(2, 1, 1, 0));
          rgba1 = _mm_shuffle_ps(rgba1, rbaa1, _MM_SHUFFLE(2, 1, 1, 0));
          
          *d++ = rgba0;
          *d++ = rgba1;
         }
        }

#if !defined (_MSC_VER) || (!defined (_M_X64) && !defined  (_M_AMD64))
      _mm_empty ();
#endif
    }

  dst += i * 4;
  src += i * 4;
  remainder = samples - i;
  while (remainder--)
  {
    float a = src[3];
    float used_alpha = babl_epsilon_for_zero_float (a);
    dst[0] = src[0] * used_alpha;
    dst[1] = src[1] * used_alpha;
    dst[2] = src[2] * used_alpha;
    dst[3] = a;
    
    src += 4;
    dst += 4;
  }
}

static void
conv_rgbAF_linear_rgbaF_linear_shuffle (const Babl  *conversion,
                                        const float *src,  
                                        float       *dst, 
                                        long         samples)
{
  long i = 0;
  long remainder;

  if (((uintptr_t)src % 16) + ((uintptr_t)dst % 16) == 0)
    {
      const long    n = samples;
      const __m128 *s = (const __m128*) src;
            __m128 *d = (__m128*)dst;

      for ( ; i < n; i += 1)
        {
          __m128 pre_rgba0, rgba0, rbaa0, raaaa0;
          
          float alpha0 = ((float *)s)[3];
          float used_alpha0 = babl_epsilon_for_zero_float (alpha0);
          pre_rgba0 = *s;
          
          {
            float recip0 = 1.0f/used_alpha0;
            
            /* Expand reciprocal */
            raaaa0 = _mm_load1_ps(&recip0);
            
            /* Un-Premultiply */
            rgba0 = _mm_mul_ps(pre_rgba0, raaaa0);
          }
            
          /* Shuffle the original alpha value back in */
          rbaa0 = _mm_shuffle_ps(rgba0, pre_rgba0, _MM_SHUFFLE(3, 3, 2, 0));
          rgba0 = _mm_shuffle_ps(rgba0, rbaa0, _MM_SHUFFLE(2, 1, 1, 0));

          s++;
          *d++ = rgba0;
        }

#if !defined (_MSC_VER) || (!defined (_M_X64) && !defined  (_M_AMD64))
      _mm_empty ();
#endif
    }

  dst += i * 4;
  src += i * 4;
  remainder = samples - i;
  while (remainder--)
    {
      float alpha = src[3];
      float recip;
      if (alpha <= 0.0f)
        recip = 0.0f;
      else
        recip = 1.0f/alpha;
      dst[0] = src[0] * recip;
      dst[1] = src[1] * recip;
      dst[2] = src[2] * recip;
      dst[3] = alpha;
      
      src   += 4;
      dst   += 4;
    }
}

static void
conv_rgbAF_linear_rgbaF_linear_spin (const Babl  *conversion,
                                     const float *src,  
                                     float       *dst, 
                                     long         samples)
{
  long i = 0;
  long remainder;
  // XXX : not ported to zero preserving alpha transforms
  if (((uintptr_t)src % 16) + ((uintptr_t)dst % 16) == 0)
    {
      const long    n = samples;
      const __m128 *s = (const __m128*) src;
            __m128 *d = (__m128*)dst;
      const __m128 zero = _mm_set_ss (BABL_ALPHA_FLOOR_FLOAT);
      const __m128 one = _mm_set_ss(1.0f);

      for ( ; i < n; i += 1)
        {
          __m128 pre_abgr0, abgr0, rgba0, raaaa0;
          
          
          rgba0 = *s;
          /* Rotate to ABGR */
          pre_abgr0 = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(rgba0),_MM_SHUFFLE(0, 1, 2, 3)));
          
          if (_mm_ucomile_ss(pre_abgr0, zero))
          {
            /* Zero RGB */
            abgr0 = zero;
          }
          else
          {
            /* Un-Premultiply */
            raaaa0 = _mm_div_ss(one, pre_abgr0);
            
            /* Expand reciprocal */
            raaaa0 = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(raaaa0), _MM_SHUFFLE(0, 0, 0, 0)));
            
            /* Un-Premultiply */
            abgr0 = _mm_mul_ps(pre_abgr0, raaaa0);
          }
          
          /* Move the original alpha value back in */
          abgr0 = _mm_move_ss(abgr0, pre_abgr0);
          
          /* Rotate to ABGR */
          rgba0 = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(abgr0), _MM_SHUFFLE(0, 1, 2, 3)));
          
          *d++ = rgba0;
          s++;
        }

#if !defined (_MSC_VER) || (!defined (_M_X64) && !defined  (_M_AMD64))
      _mm_empty ();
#endif
    }

  dst += i * 4;
  src += i * 4;
  remainder = samples - i;
  while (remainder--)
    {
      float alpha = src[3];
      float recip;
      if (alpha <= 0.0f)
        recip = 0.0f;
      else
        recip = 1.0f/alpha;
      dst[0] = src[0] * recip;
      dst[1] = src[1] * recip;
      dst[2] = src[2] * recip;
      dst[3] = alpha;
      
      src   += 4;
      dst   += 4;
    }
}

#define splat4f(x) ((__m128){x,x,x,x})
#define splat4i(x) ((__m128i){x,x,x,x})
#define FLT_ONE 0x3f800000 // ((union {float f; int i;}){1.0f}).i
#define FLT_MANTISSA (1<<23)

typedef union {
    __m128 v;    // SSE 4 x float vector
    float a[4];  // scalar array of 4 floats
} __m128_union;

static inline float
sse_max_component (__m128 x) {

  __m128 s;
  __m128_union m;

  /* m = [max (x[3], x[1]), max (x[2], x[0])] */
  s = _mm_castsi128_ps( _mm_shuffle_epi32 (_mm_castps_si128(x), _MM_SHUFFLE(0, 0, 3, 2)));
  m.v = _mm_max_ps (x, s);

  /* m = [max (m[1], m[0])] = [max (max (x[3], x[1]), max (x[2], x[0]))] */
  s = _mm_castsi128_ps( _mm_shuffle_epi32 (_mm_castps_si128(m.v), _MM_SHUFFLE(0, 0, 0, 1)));
  m.v = _mm_max_ps (m.v, s);

  return m.a[0];
}

static inline __m128
sse_init_newton (__m128 x, double exponent, double c0, double c1, double c2)
{
    double norm = exponent*M_LN2/FLT_MANTISSA;
    __m128 y = _mm_cvtepi32_ps(_mm_sub_epi32(_mm_castps_si128(x), splat4i(FLT_ONE)));
    return _mm_add_ps(_mm_add_ps(splat4f(c0), _mm_mul_ps(splat4f(c1*norm), y)),
                      _mm_mul_ps(_mm_mul_ps(splat4f(c1*norm*norm), y), y));
}

#define _mm_square_ps(x) _mm_mul_ps(x, x)
#define _mm_cube_ps(x) _mm_mul_ps(_mm_mul_ps(x, x), x)
#define _mm_6pow_ps(x) _mm_mul_ps(_mm_cube_ps(x), _mm_cube_ps(x))
#define _mm_7pow_ps(x) _mm_mul_ps(_mm_mul_ps(_mm_square_ps(x), _mm_square_ps(x)), _mm_cube_ps(x))

static inline __m128
sse_pow_1_24 (__m128 x)
{
  __m128 y, z;
  __m128_union u;
  u.v = x;
  if (sse_max_component (u.v) > 1024.0f) {
    /* for large values, fall back to a slower but more accurate version */
    return _mm_set_ps (expf (logf (u.a[3]) * (1.0f / 2.4f)),
                       expf (logf (u.a[2]) * (1.0f / 2.4f)),
                       expf (logf (u.a[1]) * (1.0f / 2.4f)),
                       expf (logf (u.a[0]) * (1.0f / 2.4f)));
  }
  y = sse_init_newton (u.v, -1./12, 0.9976800269, 0.9885126933, 0.5908575383);
  u.v = _mm_sqrt_ps (u.v);
  /* newton's method for x^(-1/6) */
  z = _mm_mul_ps(splat4f (1.f/6.f), u.v);
  y = _mm_sub_ps(_mm_mul_ps(splat4f (7.f/6.f), y), _mm_mul_ps(z, _mm_7pow_ps(y)));
  y = _mm_sub_ps(_mm_mul_ps(splat4f (7.f/6.f), y), _mm_mul_ps(z, _mm_7pow_ps(y)));
  return _mm_mul_ps(u.v, y);
}

static inline __m128
sse_pow_24 (__m128 x)
{
  __m128 y, z;
  __m128_union u;
  u.v = x;
  if (sse_max_component (u.v) > 16.0f) {
    /* for large values, fall back to a slower but more accurate version */
    return _mm_set_ps (expf (logf (u.a[3]) * 2.4f),
                       expf (logf (u.a[2]) * 2.4f),
                       expf (logf (u.a[1]) * 2.4f),
                       expf (logf (u.a[0]) * 2.4f));
  }
  y = sse_init_newton (u.v, -1./5, 0.9953189663, 0.9594345146, 0.6742970332);
  /* newton's method for x^(-1/5) */
  z = _mm_mul_ps(splat4f (1.f/5.f), x);
  y = _mm_sub_ps(_mm_mul_ps(splat4f (6.f/5.f), y), _mm_mul_ps(z, _mm_6pow_ps(y)));
  y = _mm_sub_ps(_mm_mul_ps(splat4f (6.f/5.f), y), _mm_mul_ps(z, _mm_6pow_ps(y)));
  x = _mm_mul_ps(x, y);
  return _mm_cube_ps(x);
}

static inline __m128
linear_to_gamma_2_2_sse2 (__m128 x)
{
  __m128 curve = _mm_sub_ps(_mm_mul_ps(sse_pow_1_24 (x), splat4f (1.055f)),
                            splat4f (0.055f                     -
                            3.0f / (float) (1 << 24)));
                            /* ^ offset the result such that 1 maps to 1 */
  __m128 line = _mm_mul_ps(x, splat4f (12.92f));
  __m128 mask = _mm_cmpgt_ps (x, splat4f (0.003130804954f));
  return _mm_or_ps (_mm_and_ps (mask, curve), _mm_andnot_ps (mask, line));
}

static inline __m128
gamma_2_2_to_linear_sse2 (__m128 x)
{
  __m128 curve = sse_pow_24 (_mm_mul_ps(_mm_add_ps(x, splat4f (0.055f)), splat4f (1/1.055f)));
  __m128 line = _mm_mul_ps(x, splat4f (1/12.92f));
  __m128 mask = _mm_cmpgt_ps (x, splat4f (0.04045f));
  return _mm_or_ps (_mm_and_ps (mask, curve), _mm_andnot_ps (mask, line));
}

#define GAMMA_RGBA(func, munge) \
static inline void \
func (const Babl *conversion,const float *src, float *dst, long samples)\
{\
  int i = samples;\
  if (((uintptr_t)src % 16) + ((uintptr_t)dst % 16) == 0)\
    {\
      for (; i > 3; i -= 4, src += 16, dst += 16)\
        {\
          /* Pack the rgb components from 4 pixels into 3 vectors, gammafy, unpack. */\
          __m128 x0 = _mm_load_ps (src);\
          __m128 x1 = _mm_load_ps (src+4);\
          __m128 x2 = _mm_load_ps (src+8);\
          __m128 x3 = _mm_load_ps (src+12);\
          __m128 y0 = _mm_movelh_ps (x0, x1);\
          __m128 y1 = _mm_movelh_ps (x2, x3);\
          __m128 z0 = _mm_unpackhi_ps (x0, x1);\
          __m128 z1 = _mm_unpackhi_ps (x2, x3);\
          __m128 y2 = _mm_movelh_ps (z0, z1);\
          __m128 y3 = _mm_movehl_ps (z1, z0);\
          y0 = munge (y0);\
          _mm_storel_pi ((__m64*)(dst), y0);\
          _mm_storeh_pi ((__m64*)(dst+4), y0);\
          y1 = munge (y1);\
          _mm_storel_pi ((__m64*)(dst+8), y1);\
          _mm_storeh_pi ((__m64*)(dst+12), y1);\
          y2 = munge (y2);\
          z0 = _mm_unpacklo_ps (y2, y3);\
          z1 = _mm_unpackhi_ps (y2, y3);\
          _mm_storel_pi ((__m64*)(dst+2), z0);\
          _mm_storeh_pi ((__m64*)(dst+6), z0);\
          _mm_storel_pi ((__m64*)(dst+10), z1);\
          _mm_storeh_pi ((__m64*)(dst+14), z1);\
        }\
      for (; i > 0; i--, src += 4, dst += 4)\
        {\
          __m128 x = munge (_mm_load_ps (src));\
          float a = src[3];\
          _mm_store_ps (dst, x);\
          dst[3] = a;\
        }\
    }\
  else\
    {\
      for (; i > 0; i--, src += 4, dst += 4)\
        {\
          __m128 x = munge (_mm_loadu_ps (src));\
          float a = src[3];\
          _mm_storeu_ps (dst, x);\
          dst[3] = a;\
        }\
    }\
}

GAMMA_RGBA(conv_rgbaF_linear_rgbaF_gamma, linear_to_gamma_2_2_sse2)
GAMMA_RGBA(conv_rgbaF_gamma_rgbaF_linear, gamma_2_2_to_linear_sse2)

static void conv_rgbaF_linear_rgbAF_gamma (const Babl  *conversion,
                                           const float *src,  
                                           float       *dst, 
                                           long         samples)
{
  float *tmp = alloca (sizeof(float)*4*samples);
  conv_rgbaF_linear_rgbaF_gamma (conversion, src, tmp, samples);
  conv_rgbaF_linear_rgbAF_linear (conversion, tmp, dst, samples);
}

#define YA_APPLY(load, store, convert) \
{ \
  __m128 yyaa0, yyaa1; \
  __m128 yaya0  = load ((float *)s++); \
  __m128 yaya1  = load ((float *)s++); \
  __m128 yyyy01 = _mm_shuffle_ps (yaya0, yaya1, _MM_SHUFFLE(0, 2, 0, 2)); \
\
  yyyy01 = convert (yyyy01); \
\
  yyaa0 = _mm_shuffle_ps (yyyy01, yaya0, _MM_SHUFFLE(3, 1, 0, 1)); \
  yaya0 = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(yyaa0), _MM_SHUFFLE(3, 1, 2, 0))); \
  yyaa1 = _mm_shuffle_ps (yyyy01, yaya1, _MM_SHUFFLE(3, 1, 2, 3)); \
  yaya1 = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(yyaa1), _MM_SHUFFLE(3, 1, 2, 0))); \
\
  store ((float *)d++, yaya0); \
  store ((float *)d++, yaya1); \
}\

static void
conv_yaF_linear_yaF_gamma (const Babl  *conversion,
                           const float *src,  
                           float       *dst, 
                           long         samples)
{
  const __m128 *s = (const __m128*)src;
        __m128 *d = (__m128*)dst;

  if (((uintptr_t)src % 16) + ((uintptr_t)dst % 16) == 0)
    {
      while (samples > 4)
        {
          YA_APPLY (_mm_load_ps, _mm_store_ps, linear_to_gamma_2_2_sse2);
          samples -= 4;
        }
    }
  else
    {
      while (samples > 4)
        {
          YA_APPLY (_mm_loadu_ps, _mm_storeu_ps, linear_to_gamma_2_2_sse2);
          samples -= 4;
        }
    }

  src = (const float *)s;
  dst = (float *)d;

  while (samples--)
    {
      float y = *src++;
      __m128_union u;
      u.v = linear_to_gamma_2_2_sse2 (splat4f (y));
      *dst++ = u.a[0];
      *dst++ = *src++;
    }
}


static void
conv_yaF_gamma_yaF_linear (const Babl  *conversion,
                           const float *src,  
                           float       *dst, 
                           long         samples)
{
  const __m128 *s = (const __m128*)src;
        __m128 *d = (__m128*)dst;

  if (((uintptr_t)src % 16) + ((uintptr_t)dst % 16) == 0)
    {
      while (samples > 4)
        {
          YA_APPLY (_mm_load_ps, _mm_store_ps, gamma_2_2_to_linear_sse2);
          samples -= 4;
        }
    }
  else
    {
      while (samples > 4)
        {
          YA_APPLY (_mm_loadu_ps, _mm_storeu_ps, gamma_2_2_to_linear_sse2);
          samples -= 4;
        }
    }

  src = (const float *)s;
  dst = (float *)d;

  while (samples--)
    {
      float y = *src++;
      __m128_union u;
      u.v = gamma_2_2_to_linear_sse2 (splat4f (y));
      *dst++ = u.a[0];
      *dst++ = *src++;
    }
}

static inline void
conv_yF_linear_yF_gamma (const Babl  *conversion,
                         const float *src,  
                         float       *dst, 
                         long         samples)
{
  const __m128 *s = (const __m128*)src;
        __m128 *d = (__m128*)dst;

  if (((uintptr_t)src % 16) + ((uintptr_t)dst % 16) == 0)
    {
      while (samples > 4)
        {
          __m128 rgba0 = _mm_load_ps ((float *)s++);
          rgba0 = linear_to_gamma_2_2_sse2 (rgba0);
          _mm_store_ps ((float *)d++, rgba0);
          samples -= 4;
        }
    }
  else
    {
      while (samples > 4)
        {
          __m128 rgba0 = _mm_loadu_ps ((float *)s++);
          rgba0 = linear_to_gamma_2_2_sse2 (rgba0);
          _mm_storeu_ps ((float *)d++, rgba0);
          samples -= 4;
        }
    }

  src = (const float *)s;
  dst = (float *)d;

  while (samples--)
    {
      float y = *src++;
      __m128_union u;
      u.v = linear_to_gamma_2_2_sse2 (splat4f (y));
      *dst++ = u.a[0];
    }
}

static inline void
conv_yF_gamma_yF_linear (const Babl  *conversion,
                         const float *src,  
                         float       *dst, 
                         long         samples)
{
  const __m128 *s = (const __m128*)src;
        __m128 *d = (__m128*)dst;

  if (((uintptr_t)src % 16) + ((uintptr_t)dst % 16) == 0)
    {
      while (samples > 4)
        {
          __m128 rgba0 = _mm_load_ps ((float *)s++);
          rgba0 = gamma_2_2_to_linear_sse2 (rgba0);
          _mm_store_ps ((float *)d++, rgba0);
          samples -= 4;
        }
    }
  else
    {
      while (samples > 4)
        {
          __m128 rgba0 = _mm_loadu_ps ((float *)s++);
          rgba0 = gamma_2_2_to_linear_sse2 (rgba0);
          _mm_storeu_ps ((float *)d++, rgba0);
          samples -= 4;
        }
    }

  src = (const float *)s;
  dst = (float *)d;

  while (samples--)
    {
      float y = *src++;
      __m128_union u;
      u.v = gamma_2_2_to_linear_sse2 (splat4f (y));
      *dst++ = u.a[0];
    }
}


static void
conv_rgbF_linear_rgbF_gamma (const Babl  *conversion,
                             const float *src,  
                             float       *dst, 
                             long         samples)
{
  conv_yF_linear_yF_gamma (conversion, src, dst, samples * 3);
}


static void
conv_rgbF_gamma_rgbF_linear (const Babl  *conversion,
                             const float *src,  
                             float       *dst, 
                             long         samples)
{
  conv_yF_gamma_yF_linear (conversion, src, dst, samples * 3);
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
  const Babl *rgbAF_gamma = babl_format_new (
    babl_model ("R'aG'aB'aA"),
    babl_type ("float"),
    babl_component ("R'a"),
    babl_component ("G'a"),
    babl_component ("B'a"),
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
  const Babl *rgbF_linear = babl_format_new (
    babl_model ("RGB"),
    babl_type ("float"),
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
  const Babl *yaF_linear = babl_format_new (
    babl_model ("YA"),
    babl_type ("float"),
    babl_component ("Y"),
    babl_component ("A"),
    NULL);
  const Babl *yaF_gamma = babl_format_new (
    babl_model ("Y'A"),
    babl_type ("float"),
    babl_component ("Y'"),
    babl_component ("A"),
    NULL);
  const Babl *yF_linear = babl_format_new (
    babl_model ("Y"),
    babl_type ("float"),
    babl_component ("Y"),
    NULL);
  const Babl *yF_gamma = babl_format_new (
    babl_model ("Y'"),
    babl_type ("float"),
    babl_component ("Y'"),
    NULL);

  if ((babl_cpu_accel_get_support () & BABL_CPU_ACCEL_X86_SSE) &&
      (babl_cpu_accel_get_support () & BABL_CPU_ACCEL_X86_SSE2))
      
    {
      babl_conversion_new(rgbaF_linear, 
                          rgbAF_linear,
                          "linear",
                          conv_rgbaF_linear_rgbAF_linear,
                          NULL);

      babl_conversion_new(rgbaF_gamma, 
                          rgbAF_gamma,
                          "linear",
                          conv_rgbaF_linear_rgbAF_linear,
                          NULL);
                          
      babl_conversion_new(rgbaF_linear, 
                          rgbAF_gamma,
                          "linear",
                          conv_rgbaF_linear_rgbAF_gamma,
                          NULL);

      /* Which of these is faster varies by CPU, and the difference
       * is big enough that it's worthwhile to include both and
       * let them fight it out in the babl benchmarks.
       */
      babl_conversion_new(rgbAF_linear, 
                          rgbaF_linear,
                          "linear",
                          conv_rgbAF_linear_rgbaF_linear_shuffle,
                          NULL);
      babl_conversion_new(rgbAF_gamma, 
                          rgbaF_gamma,
                          "linear",
                          conv_rgbAF_linear_rgbaF_linear_shuffle,
                          NULL);

      babl_conversion_new(rgbAF_linear, 
                          rgbaF_linear,
                          "linear",
                          conv_rgbAF_linear_rgbaF_linear_spin,
                          NULL);

      o (yF_linear, yF_gamma);
      o (yF_gamma,  yF_linear);

      o (yaF_linear, yaF_gamma);
      o (yaF_gamma,  yaF_linear);

      o (rgbF_linear, rgbF_gamma);
      o (rgbF_gamma,  rgbF_linear);

      o (rgbaF_linear, rgbaF_gamma);
      o (rgbaF_gamma, rgbaF_linear);
    }

#endif /* defined(USE_SSE2) */

  return 0;
}

