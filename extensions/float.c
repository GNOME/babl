/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2012, Øyvind Kolås
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

#include <stdint.h>
#include <stdlib.h>

#include "babl.h"
#include "babl-cpuaccel.h"
#include "extensions/util.h"
#include "base/util.h"

#define INLINE inline


/*
   optimized powf from:

http://stackoverflow.com/questions/6475373/optimizations-for-pow-with-const-non-integer-exponent

   by David Hammen
*/

// Returns x^(5/12) for x in [1,2), to within 3e-8 (relative error).
// Want more precision? Add more Chebychev polynomial coefs.
static INLINE double pow512norm (
   double x)
{
   static const int N = 8;

   // Chebychev polynomial terms.
   // Non-zero terms calculated via
   //   integrate (2/pi)*ChebyshevT[n,u]/sqrt(1-u^2)*((u+3)/2)^(5/12)
   //   from -1 to 1
   // Zeroth term is similar except it uses 1/pi rather than 2/pi.
   static const double Cn[8] = { 
       1.1758200232996901923,
       0.16665763094889061230,
      -0.0083154894939042125035,
       0.00075187976780420279038,
      // Wolfram alpha doesn't want to compute the remaining terms
      // to more precision (it times out).
      -0.0000832402,
       0.0000102292,
      -1.3401e-6,
       1.83334e-7};

   double Tn[N];

   double u = 2.0*x - 3.0;
   int i;
   double y = 0.0;

   Tn[0] = 1.0;
   Tn[1] = u;
   for (i = 2; i < N; ++i) {
      Tn[i] = 2*u*Tn[i-1] - Tn[i-2];
   }   

   for (i = N-1; i >= 0; --i) {
      y += Cn[i]*Tn[i];
   }   

   return y;
}

// Returns x^(5/12) to within 3e-8 (relative error).
static INLINE double pow512 (
   double x)
{
   static const double pow2_512[12] = {
      1.0,
      pow(2.0, 5.0/12.0),
      pow(4.0, 5.0/12.0),
      pow(8.0, 5.0/12.0),
      pow(16.0, 5.0/12.0),
      pow(32.0, 5.0/12.0),
      pow(64.0, 5.0/12.0),
      pow(128.0, 5.0/12.0),
      pow(256.0, 5.0/12.0),
      pow(512.0, 5.0/12.0),
      pow(1024.0, 5.0/12.0),
      pow(2048.0, 5.0/12.0)
   };

   double s;
   int iexp;

   s = frexp (x, &iexp);
   s *= 2.0;
   iexp -= 1;

   div_t qr = div (iexp, 12);
   if (qr.rem < 0) {
      qr.quot -= 1;
      qr.rem += 12;
   }

   return ldexp (pow512norm(s)*pow2_512[qr.rem], 5*qr.quot);
}

static inline double
flinear_to_gamma_2_2 (double value)
{
  if (value > 0.0030402477F)
    return 1.055F * pow512 (value) - 0.055F;
  return 12.92F * value;
}

static INLINE long
conv_rgbaF_linear_rgbAF_gamma (unsigned char *src, 
                               unsigned char *dst, 
                               long           samples)
{
   float *fsrc = (float *) src;
   float *fdst = (float *) dst;
   int n = samples;

   while (n--)
     {
       float alpha = fsrc[3];
       *fdst++ = flinear_to_gamma_2_2 (*fsrc++) * alpha;
       *fdst++ = flinear_to_gamma_2_2 (*fsrc++) * alpha;
       *fdst++ = flinear_to_gamma_2_2 (*fsrc++) * alpha;
       *fdst++ = *fsrc++;
     }
  return samples;
}

static INLINE long
conv_rgbAF_linear_rgbAF_gamma (unsigned char *src, 
                               unsigned char *dst, 
                               long           samples)
{
   float *fsrc = (float *) src;
   float *fdst = (float *) dst;
   int n = samples;

   while (n--)
     {
       float alpha = fsrc[3];
       if (alpha < BABL_ALPHA_THRESHOLD)
         {
           *fdst++ = 0.0;
           *fdst++ = 0.0;
           *fdst++ = 0.0;
           *fdst++ = 0.0;
           fsrc+=4;
         }
       else if (alpha >= 1.0)
         {
           *fdst++ = flinear_to_gamma_2_2 (*fsrc++);
           *fdst++ = flinear_to_gamma_2_2 (*fsrc++);
           *fdst++ = flinear_to_gamma_2_2 (*fsrc++);
           *fdst++ = *fsrc++;
         }
       else
         {
           float alpha_recip = 1.0 / alpha;
           *fdst++ = flinear_to_gamma_2_2 (*fsrc++ * alpha_recip) * alpha;
           *fdst++ = flinear_to_gamma_2_2 (*fsrc++ * alpha_recip) * alpha;
           *fdst++ = flinear_to_gamma_2_2 (*fsrc++ * alpha_recip) * alpha;
           *fdst++ = *fsrc++;
         }
     }
  return samples;
}

static INLINE long
conv_rgbaF_linear_rgbaF_gamma (unsigned char *src, 
                               unsigned char *dst, 
                               long           samples)
{
   float *fsrc = (float *) src;
   float *fdst = (float *) dst;
   int n = samples;

   while (n--)
     {
       *fdst++ = flinear_to_gamma_2_2 (*fsrc++);
       *fdst++ = flinear_to_gamma_2_2 (*fsrc++);
       *fdst++ = flinear_to_gamma_2_2 (*fsrc++);
       *fdst++ = *fsrc++;
     }
  return samples;
}

static INLINE long
conv_rgbF_linear_rgbF_gamma (unsigned char *src, 
                             unsigned char *dst, 
                             long           samples)
{
   float *fsrc = (float *) src;
   float *fdst = (float *) dst;
   int n = samples;

   while (n--)
     {
       *fdst++ = flinear_to_gamma_2_2 (*fsrc++);
       *fdst++ = flinear_to_gamma_2_2 (*fsrc++);
       *fdst++ = flinear_to_gamma_2_2 (*fsrc++);
     }
  return samples;
}


static INLINE long
conv_rgbaF_gamma_rgbaF_linear (unsigned char *src, 
                               unsigned char *dst, 
                               long           samples)
{
   float *fsrc = (float *) src;
   float *fdst = (float *) dst;
   int n = samples;

   while (n--)
     {
       *fdst++ = gamma_2_2_to_linear (*fsrc++);
       *fdst++ = gamma_2_2_to_linear (*fsrc++);
       *fdst++ = gamma_2_2_to_linear (*fsrc++);
       *fdst++ = *fsrc++;
     }
  return samples;
}

static INLINE long
conv_rgbF_gamma_rgbF_linear (unsigned char *src, 
                             unsigned char *dst, 
                             long           samples)
{
   float *fsrc = (float *) src;
   float *fdst = (float *) dst;
   int n = samples;

   while (n--)
     {
       *fdst++ = gamma_2_2_to_linear (*fsrc++);
       *fdst++ = gamma_2_2_to_linear (*fsrc++);
       *fdst++ = gamma_2_2_to_linear (*fsrc++);
     }
  return samples;
}

#define o(src, dst) \
  babl_conversion_new (src, dst, "linear", conv_ ## src ## _ ## dst, NULL)

int init (void);

int
init (void)
{
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

  o (rgbAF_linear, rgbAF_gamma);
  o (rgbaF_linear, rgbAF_gamma);
  o (rgbaF_linear, rgbaF_gamma);
  o (rgbaF_gamma,  rgbaF_linear);
  o (rgbF_linear, rgbF_gamma);
  o (rgbF_gamma,  rgbF_linear);

  return 0;
}

