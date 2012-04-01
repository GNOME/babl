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

#define  LSHIFT 2

typedef  float (* BablLookupFunction) (float  value,
                                       void  *data);
#define babl_LOOKUP_MAX_ENTRIES   (819200)

typedef struct BablLookup
{
  BablLookupFunction function;
  void              *data;
  int               shift;
  uint32_t            positive_min, positive_max, negative_min, negative_max;
  uint32_t            bitmask[babl_LOOKUP_MAX_ENTRIES/32];
  float             table[];
} BablLookup;


static BablLookup *babl_lookup_new (BablLookupFunction  function,
                                    void *              data,
                                    float               start,
                                    float               end,
                                    float               precision);
#if 0
static void        babl_lookup_free      (BablLookup         *lookup);
#endif


static inline float
babl_lookup (BablLookup *lookup,
             float      number)
{
  union
  {
    float   f;
    uint32_t i;
  } u;
  uint32_t i;

  u.f = number;
  i = (u.i << LSHIFT )>> lookup->shift;

  if (i > lookup->positive_min &&
      i < lookup->positive_max)
    i = i - lookup->positive_min;
  else if (i > lookup->negative_min &&
           i < lookup->negative_max)
    i = i - lookup->negative_min + (lookup->positive_max - lookup->positive_min);
  else
    return lookup->function (number, lookup->data);

  if (!(lookup->bitmask[i/32] & (1<<(i & 31))))
    {
      /* XXX: should look up the value in the middle of the range
       *      that yields a given value,
       *
       *      potentially even do linear interpolation between
       *      the two neighbour values to get away with a tiny
       *      lookup table.. 
       */
      lookup->table[i]= lookup->function (number, lookup->data);
      lookup->bitmask[i/32] |= (1<<(i & 31));
    }

  return lookup->table[i];
}

static BablLookup *
babl_lookup_new (BablLookupFunction function,
                 void *             data,
                 float              start,
                 float              end,
                 float              precision)
{
  BablLookup *lookup;
  union
  {
    float   f;
    uint32_t i;
  } u;
  int positive_min, positive_max, negative_min, negative_max;
  int shift;

  /* normalize input parameters */
  if (start > end)
    { /* swap */
      u.f = start;
      start = end;
      end = u.f;
    }

       if (precision <= 0.000005) shift =  0; /* checked for later */
  else if (precision <= 0.000010) shift =  8;
  else if (precision <= 0.000020) shift =  9;
  else if (precision <= 0.000040) shift = 10;
  else if (precision <= 0.000081) shift = 11;
  else if (precision <= 0.000161) shift = 12;
  else if (precision <= 0.000324) shift = 14;
  else if (precision <= 0.000649) shift = 15;
  else shift = 16; /* a bit better than 8bit sRGB quality */

  /* Adjust slightly away from 0.0, saving many entries close to 0, this
   * causes lookups very close to zero to be passed directly to the
   * function instead.
   */
  if (start == 0.0)
    start = precision;
  if (end == 0.0)
    end = -precision;

  /* Compute start and */

  if (start < 0.0 || end < 0.0)
    {
      if (end < 0.0)
        {
          u.f = start;
          positive_max = (u.i << LSHIFT) >> shift;
          u.f = end;
          positive_min = (u.i << LSHIFT) >> shift;
          negative_min = positive_max;
          negative_max = positive_max;
        }
      else
        {
          u.f = 0 - precision;
          positive_min = (u.i << LSHIFT) >> shift;
          u.f = start;
          positive_max = (u.i << LSHIFT) >> shift;

          u.f = 0 + precision;
          negative_min = (u.i << LSHIFT) >> shift;
          u.f = end;
          negative_max = (u.i << LSHIFT) >> shift;
        }
    }
  else
    {
      u.f = start;
      positive_min = (u.i << LSHIFT) >> shift;
      u.f = end;
      positive_max = (u.i << LSHIFT) >> shift;
      negative_min = positive_max;
      negative_max = positive_max;
    }

  if (shift == 0) /* short circuit, do not use ranges */
    {
      positive_min = positive_max = negative_min = negative_max = 0;
    }

  if ((positive_max-positive_min) + (negative_max-negative_min) > babl_LOOKUP_MAX_ENTRIES)
    {
      /* Reduce the size of the cache tables to fit within the bittable
       * budget (the maximum allocation is around 2.18mb of memory
       */

      int diff = (positive_max-positive_min) + (negative_max-negative_min) - babl_LOOKUP_MAX_ENTRIES;

      if (negative_max - negative_min > 0)
        {
          if (negative_max - negative_min >= diff)
            {
              negative_max -= diff;
              diff = 0;
            }
          else
            {
              diff -= negative_max - negative_min;
              negative_max = negative_min;
            }
        }
      if (diff)
        positive_max-=diff;
    }

  lookup = calloc (sizeof (BablLookup) + sizeof (float) *
                                                  ((positive_max-positive_min)+
                                                   (negative_max-negative_min)), 1);

  lookup->positive_min = positive_min;
  lookup->positive_max = positive_max;
  lookup->negative_min = negative_min;
  lookup->negative_max = negative_max;
  lookup->shift = shift;
  lookup->function = function;
  lookup->data = data;

  return lookup;
}

static BablLookup *fast_pow = NULL;

static inline float core_lookup (float val, void *userdata)
{
  return linear_to_gamma_2_2 (val);
}

static float
linear_to_gamma_2_2_lut (float val)
{
  return babl_lookup (fast_pow, val);
}


static BablLookup *fast_rpow = NULL;

static inline float core_rlookup (float val, void *userdata)
{
  return gamma_2_2_to_linear (val);
}

static float
gamma_2_2_to_linear_lut (float val)
{
  return babl_lookup (fast_rpow, val);
}

#if 0
static void
babl_lookup_free (BablLookup *lookup)
{
  free (lookup);
}
#endif

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
       *fdst++ = linear_to_gamma_2_2_lut (*fsrc++) * alpha;
       *fdst++ = linear_to_gamma_2_2_lut (*fsrc++) * alpha;
       *fdst++ = linear_to_gamma_2_2_lut (*fsrc++) * alpha;
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
           *fdst++ = linear_to_gamma_2_2_lut (*fsrc++);
           *fdst++ = linear_to_gamma_2_2_lut (*fsrc++);
           *fdst++ = linear_to_gamma_2_2_lut (*fsrc++);
           *fdst++ = *fsrc++;
         }
       else
         {
           float alpha_recip = 1.0 / alpha;
           *fdst++ = linear_to_gamma_2_2_lut (*fsrc++ * alpha_recip) * alpha;
           *fdst++ = linear_to_gamma_2_2_lut (*fsrc++ * alpha_recip) * alpha;
           *fdst++ = linear_to_gamma_2_2_lut (*fsrc++ * alpha_recip) * alpha;
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
       *fdst++ = linear_to_gamma_2_2_lut (*fsrc++);
       *fdst++ = linear_to_gamma_2_2_lut (*fsrc++);
       *fdst++ = linear_to_gamma_2_2_lut (*fsrc++);
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
       *fdst++ = linear_to_gamma_2_2_lut (*fsrc++);
       *fdst++ = linear_to_gamma_2_2_lut (*fsrc++);
       *fdst++ = linear_to_gamma_2_2_lut (*fsrc++);
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
       *fdst++ = gamma_2_2_to_linear_lut (*fsrc++);
       *fdst++ = gamma_2_2_to_linear_lut (*fsrc++);
       *fdst++ = gamma_2_2_to_linear_lut (*fsrc++);
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
       *fdst++ = gamma_2_2_to_linear_lut (*fsrc++);
       *fdst++ = gamma_2_2_to_linear_lut (*fsrc++);
       *fdst++ = gamma_2_2_to_linear_lut (*fsrc++);
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

  {
    float f;
    float a;

    fast_pow = babl_lookup_new (core_lookup, NULL, 0.0, 1.0,   0.0001);
    fast_rpow = babl_lookup_new (core_rlookup, NULL, 0.0, 1.0, 0.0001);

    for (f = 0.0; f < 1.0; f+= 0.00001)
      {
        a = linear_to_gamma_2_2_lut (f);
        a = gamma_2_2_to_linear_lut (f);
      }
    if (a < -10)
      f = 2;

  }

  o (rgbAF_linear, rgbAF_gamma);
  o (rgbaF_linear, rgbAF_gamma);
  o (rgbaF_linear, rgbaF_gamma);
  o (rgbaF_gamma,  rgbaF_linear);
  o (rgbF_linear, rgbF_gamma);
  o (rgbF_gamma,  rgbF_linear);

  return 0;
}

