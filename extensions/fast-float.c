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
 * <https://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <stdint.h>
#include <stdlib.h>

#include "babl.h"
#include "babl-cpuaccel.h"
#include "extensions/util.h"
#include "base/util.h"

#define  LSHIFT 4

typedef  float (* BablLookupFunction) (float  value,
                                       void  *data);
#define babl_LOOKUP_MAX_ENTRIES   (819200)

typedef struct BablLookup
{
  BablLookupFunction function;
  void              *data;
  int                shift;
  uint32_t           positive_min, positive_max, negative_min, negative_max;
  uint32_t           bitmask[babl_LOOKUP_MAX_ENTRIES/32];
  int                entries;
  float              table[];
} BablLookup;


static BablLookup *babl_lookup_new (BablLookupFunction function,
                                    void *             data,
                                    float              start,
                                    float              end,
                                    float              precision);
#if 0
static void        babl_lookup_free      (BablLookup         *lookup);
#endif

#include <string.h>

static inline float
babl_lookup (BablLookup *lookup,
             float       number)
{
  union { float   f; uint32_t i; } u;
  union { float   f; uint32_t i; } ub;
  union { float   f; uint32_t i; } ua;

  uint32_t i;
  float dx = 0.0;

  u.f = number;
  i = (u.i << LSHIFT ) >> lookup->shift;

  if (i > lookup->positive_min && i < lookup->positive_max)
  {
    ua.i = ((i) << lookup->shift)    >> LSHIFT;
    ub.i = ((i+ 1) << lookup->shift) >> LSHIFT;

    i = i - lookup->positive_min;
  }
  else if (i > lookup->negative_min && i < lookup->negative_max)
  {

    ua.i = ((i) << lookup->shift)    >> LSHIFT;
    ub.i = ((i+ 1) << lookup->shift) >> LSHIFT;

    i = i - lookup->negative_min + (lookup->positive_max - lookup->positive_min);
  }
  else
  {
    return lookup->function (number, lookup->data);
  }

  {
    uint32_t bm =u.i & 0b11110000000000000000000000000000;
    ua.i |= bm;
    ub.i |= bm;
  }
  dx = (u.f-ua.f) / (ub.f - ua.f);

  {

  if (!(lookup->bitmask[i/32] & (1UL<<(i & 31))))
    {
      lookup->table[i]= lookup->function (ua.f, lookup->data);
      lookup->bitmask[i/32] |= (1UL<<(i & 31));
    }
  i++;
  if (i< lookup->entries-2)
  {
    if (!(lookup->bitmask[i/32] & (1UL<<(i & 31))))
    {
      lookup->table[i]= lookup->function (ub.f, lookup->data);
      lookup->bitmask[i/32] |= (1UL<<(i & 31));
    }

    return lookup->table[i-1] * (1.0f-dx) +
           lookup->table[i] * (dx);
  }
  else
  {
    return lookup->table[i-1];
  }
  }
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
  else if (precision <= 0.000200) shift = 13;
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

  lookup->entries = (positive_max-positive_min)+
                    (negative_max-negative_min);

  return lookup;
}

static BablLookup *fast_pow = NULL;

static inline float core_lookup (float val, void *userdata)
{
  return babl_linear_to_gamma_2_2f (val);
}

static float
linear_to_gamma_2_2_lut (float val)
{
  return babl_lookup (fast_pow, val);
}

static BablLookup *fast_rpow = NULL;

static inline float core_rlookup (float val, void *userdata)
{
  return babl_gamma_2_2_to_linearf (val);
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

static void
conv_rgbaF_linear_rgbAF_gamma (const Babl    *conversion,
                               unsigned char *src, 
                               unsigned char *dst, 
                               long           samples)
{
   float *fsrc = (float *) src;
   float *fdst = (float *) dst;
   int n = samples;

   while (n--)
     {
       float red   = *fsrc++;
       float green = *fsrc++;
       float blue  = *fsrc++;
       float alpha = *fsrc++;
       if (alpha == 1.0)
       {
         *fdst++ = linear_to_gamma_2_2_lut (red);
         *fdst++ = linear_to_gamma_2_2_lut (green);
         *fdst++ = linear_to_gamma_2_2_lut (blue);
         *fdst++ = alpha;
       }
       else
       {
         if (alpha < BABL_ALPHA_FLOOR)
         {
           if (alpha >= 0.0f)
             alpha = BABL_ALPHA_FLOOR;
           else if (alpha >= -BABL_ALPHA_FLOOR)
             alpha = -BABL_ALPHA_FLOOR;
         }
         *fdst++ = linear_to_gamma_2_2_lut (red)   * alpha;
         *fdst++ = linear_to_gamma_2_2_lut (green) * alpha;
         *fdst++ = linear_to_gamma_2_2_lut (blue)  * alpha;
         *fdst++ = alpha;
       }
     }
}



static void
conv_rgbaF_linear_rgba8_gamma (const Babl    *conversion,
                               unsigned char *src, 
                               unsigned char *dst, 
                               long           samples)
{
   float *fsrc = (float *) src;
   uint8_t *cdst = (uint8_t *) dst;
   int n = samples;

   while (n--)
     {
       float red   = *fsrc++;
       float green = *fsrc++;
       float blue  = *fsrc++;
       float alpha = *fsrc++;
       if (alpha <= 0) /* XXX: we need to drop alpha!! ? */
       {
       *cdst++ = 0;
       *cdst++ = 0;
       *cdst++ = 0;
       *cdst++ = 0;
       }
       else
       {
       int val = linear_to_gamma_2_2_lut (red) * 0xff + 0.5f;
       *cdst++ = val >= 0xff ? 0xff : val <= 0 ? 0 : val;
       val = linear_to_gamma_2_2_lut (green) * 0xff + 0.5f;
       *cdst++ = val >= 0xff ? 0xff : val <= 0 ? 0 : val;
       val = linear_to_gamma_2_2_lut (blue) * 0xff + 0.5f;
       *cdst++ = val >= 0xff ? 0xff : val <= 0 ? 0 : val;
       val = alpha * 0xff + 0.5;
       *cdst++ = val >= 0xff ? 0xff : val <= 0 ? 0 : val;
       }
     }
}

static void
conv_rgbaF_linear_rgbA8_gamma (const Babl    *conversion,
                               unsigned char *src, 
                               unsigned char *dst, 
                               long           samples)
{
   float *fsrc = (float *) src;
   uint8_t *cdst = (uint8_t *) dst;
   int n = samples;

   while (n--)
     {
       float red   = *fsrc++;
       float green = *fsrc++;
       float blue  = *fsrc++;
       float alpha = *fsrc++;
       if (alpha >= 1.0)
       {
         int val = linear_to_gamma_2_2_lut (red) * 0xff + 0.5f;
         *cdst++ = val >= 0xff ? 0xff : val <= 0 ? 0 : val;
         val = linear_to_gamma_2_2_lut (green) * 0xff + 0.5f;
         *cdst++ = val >= 0xff ? 0xff : val <= 0 ? 0 : val;
         val = linear_to_gamma_2_2_lut (blue) * 0xff + 0.5f;
         *cdst++ = val >= 0xff ? 0xff : val <= 0 ? 0 : val;
         *cdst++ = 0xff;
       }
       else
       {
         float balpha = alpha * 0xff;
         int val = linear_to_gamma_2_2_lut (red) * balpha + 0.5f;
         *cdst++ = val >= 0xff ? 0xff : val <= 0 ? 0 : val;
         val = linear_to_gamma_2_2_lut (green) * balpha + 0.5f;
         *cdst++ = val >= 0xff ? 0xff : val <= 0 ? 0 : val;
         val = linear_to_gamma_2_2_lut (blue) * balpha + 0.5f;
         *cdst++ = val >= 0xff ? 0xff : val <= 0 ? 0 : val;
         *cdst++ = balpha + 0.5f;
       }
     }
}

static void
conv_yaF_linear_rgbA8_gamma (const Babl *conversion,unsigned char *src, 
                             unsigned char *dst, 
                             long           samples)
{
   float *fsrc = (float *) src;
   uint8_t *cdst = (uint8_t *) dst;
   int n = samples;

   while (n--)
     {
       float gray = *fsrc++;
       float alpha = *fsrc++;
       if (alpha >= 1.0)
       {
         int val = linear_to_gamma_2_2_lut (gray) * 0xff + 0.5f;
         *cdst++ = val >= 0xff ? 0xff : val <= 0 ? 0 : val;
         *cdst++ = val >= 0xff ? 0xff : val <= 0 ? 0 : val;
         *cdst++ = val >= 0xff ? 0xff : val <= 0 ? 0 : val;
         *cdst++ = 0xff;
       }
       else if (alpha <= 0.0)
       {
         *((uint32_t*)(cdst))=0;
	     cdst+=4;
       }
       else
       {
         float balpha = alpha * 0xff;
         int val = linear_to_gamma_2_2_lut (gray) * balpha + 0.5f;
         *cdst++ = val >= 0xff ? 0xff : val <= 0 ? 0 : val;
         *cdst++ = val >= 0xff ? 0xff : val <= 0 ? 0 : val;
         *cdst++ = val >= 0xff ? 0xff : val <= 0 ? 0 : val;
         *cdst++ = balpha + 0.5f;
       }
     }
}



static void
conv_rgbaF_linear_rgbA8_gamma_cairo (const Babl *conversion,unsigned char *src, 
                                     unsigned char *dst, 
                                     long           samples)
{
  float *fsrc = (float *) src;
  unsigned char *cdst = (unsigned char *) dst;
  int n = samples;

  while (n--)
    {
      float red   = *fsrc++;
      float green = *fsrc++;
      float blue  = *fsrc++;
      float alpha = *fsrc++;
      if (alpha >= 1.0)
      {
        int val = linear_to_gamma_2_2_lut (blue) * 0xff + 0.5f;
        *cdst++ = val >= 0xff ? 0xff : val <= 0 ? 0 : val;
        val = linear_to_gamma_2_2_lut (green) * 0xff + 0.5f;
        *cdst++ = val >= 0xff ? 0xff : val <= 0 ? 0 : val;
        val = linear_to_gamma_2_2_lut (red) * 0xff + 0.5f;
        *cdst++ = val >= 0xff ? 0xff : val <= 0 ? 0 : val;
        *cdst++ = 0xff;
      }
      else
      {
        float balpha = alpha * 0xff;
        int val = linear_to_gamma_2_2_lut (blue) * balpha + 0.5f;
        *cdst++ = val >= 0xff ? 0xff : val <= 0 ? 0 : val;
        val = linear_to_gamma_2_2_lut (green) * balpha + 0.5f;
        *cdst++ = val >= 0xff ? 0xff : val <= 0 ? 0 : val;
        val = linear_to_gamma_2_2_lut (red) * balpha + 0.5f;
        *cdst++ = val >= 0xff ? 0xff : val <= 0 ? 0 : val;
        *cdst++ = balpha + 0.5f;
      }
    }
}

static void
conv_rgbAF_linear_rgbAF_gamma (const Babl    *conversion,
                               unsigned char *src, 
                               unsigned char *dst, 
                               long           samples)
{
   float *fsrc = (float *) src;
   float *fdst = (float *) dst;
   int n = samples;

   while (n--)
     {
      float red   = *fsrc++;
      float green = *fsrc++;
      float blue  = *fsrc++;
      float alpha = *fsrc++;

      if (alpha == 1.0)
        {
          *fdst++ = linear_to_gamma_2_2_lut (red);
          *fdst++ = linear_to_gamma_2_2_lut (green);
          *fdst++ = linear_to_gamma_2_2_lut (blue);
          *fdst++ = *fsrc++;
        }
      else
        {
          float alpha_recip = 1.0 / alpha;
          *fdst++ = linear_to_gamma_2_2_lut (red   * alpha_recip) * alpha;
          *fdst++ = linear_to_gamma_2_2_lut (green * alpha_recip) * alpha;
          *fdst++ = linear_to_gamma_2_2_lut (blue  * alpha_recip) * alpha;
          *fdst++ = alpha;
        }
     }
}

static void
conv_rgbaF_linear_rgbaF_gamma (const Babl    *conversion,
                               unsigned char *src, 
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
}

static void
conv_rgbF_linear_rgbF_gamma (const Babl *conversion,unsigned char *src, 
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
}


static void
conv_rgbaF_gamma_rgbaF_linear (const Babl    *conversion,
                               unsigned char *src, 
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
}

static void
conv_rgbF_gamma_rgbF_linear (const Babl    *conversion,
                             unsigned char *src, 
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
}

#define o(src, dst) \
  babl_conversion_new (src, dst, "linear", conv_ ## src ## _ ## dst, NULL)

int init (void);

int
init (void)
{
  const Babl *yaF_linear = babl_format_new (
    babl_model ("YA"),
    babl_type ("float"),
    babl_component ("Y"),
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

  const Babl *rgbA8_gamma = babl_format_new (
    babl_model ("R'aG'aB'aA"),
    babl_type ("u8"),
    babl_component ("R'a"),
    babl_component ("G'a"),
    babl_component ("B'a"),
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
  const Babl *rgbF_gamma = babl_format_new (
    babl_model ("R'G'B'"),
    babl_type ("float"),
    babl_component ("R'"),
    babl_component ("G'"),
    babl_component ("B'"),
    NULL);

  return 0;  // XXX: the fast paths registered here doesn't correctly
             //      clamp negative values - disabling for now
  {
    float f;
    float a;

    /* tweaking the precision - does impact speed.. */
    fast_pow = babl_lookup_new (core_lookup, NULL, 0.0, 1.0,   0.000199);
    fast_rpow = babl_lookup_new (core_rlookup, NULL, 0.0, 1.0, 0.000250);

    for (f = 0.0; f < 1.0; f+= 0.0000001)
      {
        a = linear_to_gamma_2_2_lut (f);
        a = gamma_2_2_to_linear_lut (f);
      }
    if (a < -10)
      f = 2;

  }


  {
     const Babl *f32 = babl_format_new (
        "name", "cairo-ARGB32",
        babl_model ("R'aG'aB'aA"),
        babl_type ("u8"),
        babl_component ("B'a"),
        babl_component ("G'a"),
        babl_component ("R'a"),
        babl_component ("A"),
        NULL
      );


    babl_conversion_new (rgbaF_linear, f32, "linear", conv_rgbaF_linear_rgbA8_gamma_cairo, NULL);
  }

  o (rgbaF_linear, rgbA8_gamma);
  o (rgbAF_linear, rgbAF_gamma);
  o (rgbaF_linear, rgbAF_gamma);
  o (rgbaF_linear, rgbaF_gamma);
  o (rgbaF_linear, rgba8_gamma);
  o (rgbaF_gamma,  rgbaF_linear);
  o (rgbF_linear,  rgbF_gamma);
  o (rgbF_gamma,   rgbF_linear);
  o (yaF_linear,   rgbA8_gamma);
  return 0;
}

void destroy (void);

void
destroy (void)
{
  free (fast_rpow);
  free (fast_pow);
}

