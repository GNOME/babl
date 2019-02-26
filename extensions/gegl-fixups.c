/*
 * This file was part of gggl, it implements a variety of pixel conversion
 * functions that are usable with babl, the file needs more cleanup, but the
 * conversion functions that are usable gets used by babl.
 *
 *    GGGL is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    GGGL is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with GGGL; if not, see <https://www.gnu.org/licenses/>.
 *
 *    Rights are granted to use this shared object in libraries covered by
 *    LGPL. (exception added, during import into babl CVS.)
 *
 *  Copyright 2003, 2004, 2005, 2007 Øyvind Kolås <pippin@gimp.org>
 */

/*
 * Implemented according to information read from:
 *
 * http://www.cinenet.net/~spitzak/conversion/sketches_0265.pdf
 *
 * initially ignoring any diffusion, to keep the implementation
 * smaller, and interchangeable with the non optimized version.
 *
 * due to ability to be able to relicence gggl under a different
 * licence than GPL, I avoided the temptation to look at the
 * source files in the same location, in case I was going to
 * need this piece of code for projects where GPL compatibility
 * was a must.
 *
 * TODO: error diffusion,
 */

#include <stdlib.h>
#include <stdint.h>

#include "config.h"
#include "babl.h"

#include "base/util.h"
#include "extensions/util.h"


/* lookup tables used in conversion */

static float         table_8_F[1 << 8];
static float         table_8g_F[1 << 8];
static unsigned char table_F_8[1 << 17];
static unsigned char table_F_8g[1 << 17];

static int           table_inited = 0;

static void
table_init (void)
{
  if (table_inited)
    return;
  table_inited = 1;

  /* fill tables for conversion from integer to float */
  {
    int i;
    for (i = 0; i < 1 << 8; i++)
      {
        float direct = i / 255.0;
        table_8_F[i]  = direct;
        table_8g_F[i] = gamma_2_2_to_linear (direct);
      }
  }
  /* fill tables for conversion from float to integer */
  {
    union
    {
      float    f;
      uint32_t s;
    } u;
    u.f = 0.0;

    //u.s[0] = 0;

    for (u.s = 0; u.s < 4294900000U; u.s += 32768)
      {
        int c;
        int cg;

        if (u.f <= 0.0)
          {
            c  = 0;
            cg = 0;
          }
        else
          {
            c  = (u.f * 255.1619) + 0.5;
            cg = (linear_to_gamma_2_2 (u.f) * 255.1619) + 0.5;
            if (cg > 255) cg = 255;
            if (c > 255) c = 255;
          }

        table_F_8[(u.s >> 15) & ((1 << 17)-1)]  = c;
        table_F_8g[(u.s >> 15) & ((1 << 17)-1)] = cg;
      }
  }
#if 0
  /* fix tables to ensure 1:1 conversions back and forth */
  if (0)
    {
      int i;
      for (i = 0; i < 256; i++)
        {
          float           f;
          unsigned short *hi = ((unsigned short *) (void *) &f);
          unsigned short *lo = ((unsigned short *) (void *) &f);

          f = table_8_F[i];
          *lo              = 0;
          table_F_8[*hi] = i;
          f  = table_8g_F[i];
          *lo              = 0;
          table_F_8g[*hi] = i;
        }
    }
#endif
}

/* function to find the index in table for a float */
static inline unsigned int
gggl_float_to_index16 (float f)
{
  union
  {
    float          f;
    uint32_t       s;
  } u;
  u.f = f;
  return (u.s >> 15) & ((1 << 17)-1);
}

static inline void
conv_F_8 (const Babl    *conversion,
          unsigned char *src, 
          unsigned char *dst, 
          long           samples)
{
  long n = samples;

  if (!table_inited)
    table_init ();
  while (n--)
    {
      register float f = (*(float *) src);
      *(unsigned char *) dst = table_F_8[gggl_float_to_index16 (f)];
      dst                   += 1;
      src                   += 4;
    }
}


static inline void
conv_F_8g (const Babl    *conversion,
           unsigned char *src, 
           unsigned char *dst, 
           long           samples)
{
  long n = samples;

  if (!table_inited)
    table_init ();
  while (n--)
    {
      register float f = (*(float *) src);
      *(unsigned char *) dst = table_F_8g[gggl_float_to_index16 (f)];
      dst                   += 1;
      src                   += 4;
    }
}


static inline void __attribute__((unused))
conv_8_F (const Babl    *conversion,
          unsigned char *src, 
          unsigned char *dst, 
          long           samples)
{
  long n = samples;

  if (!table_inited)
    table_init ();
  while (n--)
    {
      (*(float *) dst) = table_8_F[*(unsigned char *) src];
      dst             += 4;
      src             += 1;
    }
}


static void
conv_rgbaF_rgb8 (const Babl    *conversion,
                 unsigned char *src, 
                 unsigned char *dst, 
                 long           samples)
{
  long n = samples;

  while (n--)
    {
      register float f = (*(float *) src);
      *(unsigned char *) dst = table_F_8g[gggl_float_to_index16 (f)];
      src                   += 4;
      dst                   += 1;

      f                      = (*(float *) src);
      *(unsigned char *) dst = table_F_8g[gggl_float_to_index16 (f)];
      src                   += 4;
      dst                   += 1;

      f                      = (*(float *) src);
      *(unsigned char *) dst = table_F_8g[gggl_float_to_index16 (f)];
      src                   += 4;
      dst                   += 1;

      src += 4;
    }
}


static void __attribute__((unused))
conv_rgbaF_rgba8 (const Babl    *conversion,
                  unsigned char *src, 
                  unsigned char *dst, 
                  long           samples)
{
  long n = samples;

  while (n--)
    {
      register float f = (*(float *) src);
      *(unsigned char *) dst = table_F_8g[gggl_float_to_index16 (f)];
      src                   += 4;
      dst                   += 1;

      f                      = (*(float *) src);
      *(unsigned char *) dst = table_F_8g[gggl_float_to_index16 (f)];
      src                   += 4;
      dst                   += 1;

      f                      = (*(float *) src);
      *(unsigned char *) dst = table_F_8g[gggl_float_to_index16 (f)];
      src                   += 4;
      dst                   += 1;

      f                      = (*(float *) src);
      *(unsigned char *) dst = table_F_8[gggl_float_to_index16 (f)];
      src                   += 4;
      dst                   += 1;
    }
}

#define conv_rgbaF_rgbP8    conv_rgbaF_rgba8

static void __attribute__((unused))
conv_rgbF_rgb8 (const Babl    *conversion,
                unsigned char *src, 
                unsigned char *dst, 
                long           samples)
{
  conv_F_8g (conversion, src, dst, samples * 3);
}

static void __attribute__((unused))
conv_gaF_ga8 (const Babl    *conversion,
              unsigned char *src, 
              unsigned char *dst, 
              long           samples)
{
  conv_F_8 (conversion, src, dst, samples * 2);
}

#define conv_rgbAF_rgbA8    conv_rgbaF_rgba8
#define conv_gF_g8          conv_F_8
#define conv_gAF_gA8        conv_gaF_ga8


static void
conv_rgba8_rgbaF (const Babl    *conversion,
                  unsigned char *src, 
                  unsigned char *dst, 
                  long           samples)
{
  long n = samples;

  while (n--)
    {
      (*(float *) dst) = table_8g_F[*(unsigned char *) src];
      dst             += 4;
      src             += 1;

      (*(float *) dst) = table_8g_F[*(unsigned char *) src];
      dst             += 4;
      src             += 1;

      (*(float *) dst) = table_8g_F[*(unsigned char *) src];
      dst             += 4;
      src             += 1;

      (*(float *) dst) = table_8_F[*(unsigned char *) src];
      dst             += 4;
      src             += 1;
    }
}

static void
conv_rgb8_rgbaF (const Babl    *conversion,
                 unsigned char *src, 
                 unsigned char *dst, 
                 long           samples)
{
  long n = samples;

  while (n--)
    {
      (*(float *) dst) = table_8g_F[*(unsigned char *) src];
      dst             += 4;
      src             += 1;

      (*(float *) dst) = table_8g_F[*(unsigned char *) src];
      dst             += 4;
      src             += 1;

      (*(float *) dst) = table_8g_F[*(unsigned char *) src];
      dst             += 4;
      src             += 1;

      (*(float *) dst) = 1.0;
      dst             += 4;
    }
}

static void
conv_rgbAF_rgb8 (const Babl    *conversion,
                 unsigned char *srcc,
                 unsigned char *dstc,
                 long           samples)
{
  float         *src = (void *) srcc;
  unsigned char *dst = (void *) dstc;
  long           n   = samples;

  while (n--)
    {
      float alpha = src[3];
      if (alpha == 0.0f)
        {
          dst[0] = 0.0f;
          dst[1] = 0.0f;
          dst[2] = 0.0f;
        }
      else
        {
          float alpha_recip = 1.0f / alpha;
          dst[0] = table_F_8g[gggl_float_to_index16 (src[0] * alpha_recip)];
          dst[1] = table_F_8g[gggl_float_to_index16 (src[1] * alpha_recip)];
          dst[2] = table_F_8g[gggl_float_to_index16 (src[2] * alpha_recip)];
        }
      src += 4;
      dst += 3;
    }
}

static void
conv_bgrA8_rgba8 (const Babl    *conversion,
                  unsigned char *srcc,
                  unsigned char *dstc,
                  long           samples)
{
  unsigned char *src = (void *) srcc;
  unsigned char *dst = (void *) dstc;
  long           n   = samples;

  while (n--)
    {
      unsigned char alpha = src[3];
      dst[0] = alpha ? (src[2] * 255 / alpha) : 0;
      dst[1] = alpha ? (src[1] * 255 / alpha) : 0;
      dst[2] = alpha ? (src[0] * 255 / alpha) : 0;
      dst[3] = alpha;
      src   += 4;
      dst   += 4;
    }
}

static void
conv_rgbaF_rgbAF (const Babl    *conversion,
                  unsigned char *srcc,
                  unsigned char *dstc,
                  long           samples)
{
  float *src = (void *) srcc;
  float *dst = (void *) dstc;
  long           n   = samples;

  while (n--)
    {
      float alpha = src[3];
      if (alpha < BABL_ALPHA_FLOOR)
      {
        if (alpha >= 0.0f)
          alpha = BABL_ALPHA_FLOOR;
        else if (alpha >= -BABL_ALPHA_FLOOR)
          alpha = -BABL_ALPHA_FLOOR;
      }
      dst[0] = src[0] * alpha;
      dst[1] = src[1] * alpha;
      dst[2] = src[2] * alpha;
      dst[3] = alpha;
      src   += 4;
      dst   += 4;
    }
}

static void
conv_rgbAF_rgbaF (const Babl    *conversion,
                  unsigned char *srcc,
                  unsigned char *dstc,
                  long           samples)
{
  float *src = (void *) srcc;
  float *dst = (void *) dstc;
  long           n   = samples;

  while (n--)
    {
      float alpha = src[3];
      float recip;
      if (alpha == 0.0f)
        recip = 0.0f;
      else
        recip = 1.0f/alpha;

      dst[0] = src[0] * recip;
      dst[1] = src[1] * recip;
      dst[2] = src[2] * recip;
      if (alpha == BABL_ALPHA_FLOOR || alpha == -BABL_ALPHA_FLOOR)
        alpha = 0.0f;
      dst[3] = alpha;
      src   += 4;
      dst   += 4;
    }
}


static void
conv_rgbAF_lrgba8 (const Babl    *conversion,
                   unsigned char *srcc,
                   unsigned char *dstc,
                   long           samples)
{
  float *src = (void *) srcc;
  unsigned char *dst = (void *) dstc;
  long           n   = samples;

  while (n--)
    {
      float alpha = src[3];
      if (alpha == 0.0f)
        {
          dst[0] = dst[1] = dst[2] = dst[3] = 0;
        }
      else
        {
          float recip = (1.0/alpha);
          if (alpha == BABL_ALPHA_FLOOR || alpha == -BABL_ALPHA_FLOOR)
            alpha = 0.0f;
          dst[0] = table_F_8[gggl_float_to_index16 (src[0] * recip)];
          dst[1] = table_F_8[gggl_float_to_index16 (src[1] * recip)];
          dst[2] = table_F_8[gggl_float_to_index16 (src[2] * recip)];
          dst[3] = table_F_8[gggl_float_to_index16 (alpha)];
        }
      src   += 4;
      dst   += 4;
    }
}

static void
conv_rgba8_rgb8 (const Babl    *conversion,
                 unsigned char *src, 
                 unsigned char *dst, 
                 long           samples)
{
  long n = samples - 1;

  while (n--)
    {
      *(unsigned int *) dst = (*(unsigned int *) src);
      src   += 4;
      dst   += 3;
    }
  dst[0] = src[0];
  dst[1] = src[1];
  dst[2] = src[2];
}

#define conv_rgb8_rgbAF    conv_rgb8_rgbaF
#define conv_gamma_rgbaF_gamma_rgbAF   conv_rgbaF_rgbAF
#define conv_gamma_rgbAF_gamma_rgbaF   conv_rgbAF_rgbaF

int init (void);

int
init (void)
{
  const Babl *rgbaF = babl_format_new (
    babl_model ("RGBA"),
    babl_type ("float"),
    babl_component ("R"),
    babl_component ("G"),
    babl_component ("B"),
    babl_component ("A"),
    NULL);
  const Babl *rgbAF = babl_format_new (
    babl_model ("RaGaBaA"),
    babl_type ("float"),
    babl_component ("Ra"),
    babl_component ("Ga"),
    babl_component ("Ba"),
    babl_component ("A"),
    NULL);
    
  const Babl *gamma_rgbaF = babl_format_new (
    babl_model ("R'G'B'A"),
    babl_type ("float"),
    babl_component ("R'"),
    babl_component ("G'"),
    babl_component ("B'"),
    babl_component ("A"),
    NULL);
  const Babl *gamma_rgbAF = babl_format_new (
    babl_model ("R'aG'aB'aA"),
    babl_type ("float"),
    babl_component ("R'a"),
    babl_component ("G'a"),
    babl_component ("B'a"),
    babl_component ("A"),
    NULL);

  const Babl *lrgba8 = babl_format_new (
    babl_model ("RGBA"),
    babl_type ("u8"),
    babl_component ("R"),
    babl_component ("G"),
    babl_component ("B"),
    babl_component ("A"),
    NULL);

  const Babl *rgba8 = babl_format_new (
    babl_model ("R'G'B'A"),
    babl_type ("u8"),
    babl_component ("R'"),
    babl_component ("G'"),
    babl_component ("B'"),
    babl_component ("A"),
    NULL);
  const Babl *bgrA8 = babl_format_new (
    "name", "B'aG'aR'aA u8",
    babl_model ("R'aG'aB'aA"),
    babl_type ("u8"),
    babl_component ("B'a"),
    babl_component ("G'a"),
    babl_component ("R'a"),
    babl_component ("A"),
    NULL);
  const Babl *rgb8 = babl_format_new (
    babl_model ("R'G'B'"),
    babl_type ("u8"),
    babl_component ("R'"),
    babl_component ("G'"),
    babl_component ("B'"),
    NULL);

  table_init ();

#define o(src, dst) \
  babl_conversion_new (src, dst, "linear", conv_ ## src ## _ ## dst, NULL)

  o (rgbaF, rgbAF);
  o (rgbAF, rgbaF);
  o (gamma_rgbaF, gamma_rgbAF);
  o (gamma_rgbAF, gamma_rgbaF);
  o (rgbAF, lrgba8);
  o (rgb8,  rgbaF);
  o (rgb8,  rgbAF);
  o (rgba8, rgbaF);
  o (rgbaF, rgb8);
  o (rgbAF, rgb8);
  o (bgrA8, rgba8);
  o (rgba8, rgb8);

  return 0;
}
