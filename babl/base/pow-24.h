/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2005, Øyvind Kolås.
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

#ifndef _BASE_POW_24_H
#define _BASE_POW_24_H

static double babl_pow_1_24 (double x);
static double babl_pow_24 (double x);
static float  babl_pow_1_24f (float x);
static float  babl_pow_24f (float x);


/* a^b = exp(b*log(a))
 *
 * Extracting the exponent from a float gives us an approximate log.
 * Or better yet, reinterpret the bitpattern of the whole float as an int.
 *
 * However, the output values of 12throot vary by less than a factor of 2
 * over the domain we care about, so we only get log() that way, not exp().
 *
 * Approximate exp() with a low-degree polynomial; not exactly equal to the
 * Taylor series since we're minimizing maximum error over a certain finite
 * domain. It's not worthwhile to use lots of terms, since Newton's method
 * has a better convergence rate once you get reasonably close to the answer.
 */
static inline double
init_newton (double x, double exponent, double c0, double c1, double c2)
{
    int iexp;
    double y = frexp(x, &iexp);
    y = 2*y+(iexp-2);
    c1 *= M_LN2*exponent;
    c2 *= M_LN2*M_LN2*exponent*exponent;
    return y = c0 + c1*y + c2*y*y;
}

/* Returns x^2.4 == (x*(x^(-1/5)))^3, using Newton's method for x^(-1/5).
 */
static inline double
babl_pow_24 (double x)
{
  double y;
  int i;
  if (x > 16.0) {
    /* for large values, fall back to a slower but more accurate version */
    return exp (log (x) * 2.4);
  }
  y = init_newton (x, -1./5, 0.9953189663, 0.9594345146, 0.6742970332);
  for (i = 0; i < 3; i++)
    y = (1.+1./5)*y - ((1./5)*x*(y*y))*((y*y)*(y*y));
  x *= y;
  return x*x*x;
}

/* Returns x^(1/2.4) == x*((x^(-1/6))^(1/2))^7, using Newton's method for x^(-1/6).
 */
static inline double
babl_pow_1_24 (double x)
{
  double y;
  int i;
  double z;
  if (x > 1024.0) {
    /* for large values, fall back to a slower but more accurate version */
    return exp (log (x) * (1.0 / 2.4));
  }
  y = init_newton (x, -1./12, 0.9976800269, 0.9885126933, 0.5908575383);
  x = sqrt (x);
  /* newton's method for x^(-1/6) */
  z = (1./6.) * x;
  for (i = 0; i < 3; i++)
    y = (7./6.) * y - z * ((y*y)*(y*y)*(y*y*y));
  return x*y;
}


#include <stdint.h>
/* frexpf copied from musl */
static inline float babl_frexpf(float x, int *e)
{
        union { float f; uint32_t i; } y = { x };
        int ee = y.i>>23 & 0xff;

        if (!ee) {
                if (x) {
                        x = babl_frexpf(x*18446744073709551616.0, e);
                        *e -= 64;
                } else *e = 0;
                return x;
        } else if (ee == 0xff) {
                return x;
        }

        *e = ee - 0x7e;
        y.i &= 0x807ffffful;
        y.i |= 0x3f000000ul;
        return y.f;
}


//////////////////////////////////////////////
/* a^b = exp(b*log(a))
 *
 * Extracting the exponent from a float gives us an approximate log.
 * Or better yet, reinterpret the bitpattern of the whole float as an int.
 *
 * However, the output values of 12throot vary by less than a factor of 2
 * over the domain we care about, so we only get log() that way, not exp().
 *
 * Approximate exp() with a low-degree polynomial; not exactly equal to the
 * Taylor series since we're minimizing maximum error over a certain finite
 * domain. It's not worthwhile to use lots of terms, since Newton's method
 * has a better convergence rate once you get reasonably close to the answer.
 */
static inline float
init_newtonf (float x, float exponent, float c0, float c1, float c2)
{
    int iexp = 0;
    float y = babl_frexpf(x, &iexp);
    y = 2*y+(iexp-2);
    c1 *= M_LN2*exponent;
    c2 *= M_LN2*M_LN2*exponent*exponent;
    return y = c0 + c1*y + c2*y*y;
}

/* Returns x^2.4 == (x*(x^(-1/5)))^3, using Newton's method for x^(-1/5).
 */
static inline float
babl_pow_24f (float x)
{
  float y;
  int i;
  if (x > 16.0f) {
    /* for large values, fall back to a slower but more accurate version */
    return expf (logf (x) * 2.4f);
  }
  y = init_newtonf (x, -1.f/5, 0.9953189663f, 0.9594345146f, 0.6742970332f);
  for (i = 0; i < 3; i++)
    y = (1.f+1.f/5)*y - ((1.f/5)*x*(y*y))*((y*y)*(y*y));
  x *= y;
  return x*x*x;
}

/* Returns x^(1/2.4) == x*((x^(-1/6))^(1/2))^7, using Newton's method for x^(-1/6).
 */
static inline float
babl_pow_1_24f (float x)
{
  float y;
  int i;
  float z;
  if (x > 1024.0f) {
    /* for large values, fall back to a slower but more accurate version */
    return expf (logf (x) * (1.0f / 2.4f));
  }
  y = init_newtonf (x, -1.f/12, 0.9976800269f, 0.9885126933f, 0.5908575383f);
  x = sqrtf (x);
  /* newton's method for x^(-1/6) */
  z = (1.f/6.f) * x;
  for (i = 0; i < 3; i++)
    y = (7.f/6.f) * y - z * ((y*y)*(y*y)*(y*y*y));
  return x*y;
}



#endif
