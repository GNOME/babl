/*
 * This file was part of gggl, it implements a variety of pixel conversion
 * functions that are usable with babl, the file needs more cleanup, and
 * doesn't return the number of samples processed as a long, like it's supposed to.
 *
 *    GGGL is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    GGGL is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with GGGL; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *    Rights are granted to use this shared object in libraries covered by
 *    LGPL. (exception added, during import into babl CVS.)
 *
 *  Copyright 2003, 2004, 2005 Øyvind Kolås <pippin@gimp.org>
 */

#include "babl.h"

#define INLINE inline

#include <math.h>
#include <stdio.h>
#include <string.h>

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
 *       gamma correction  (not really,. gamma correction belongs in seperate ops,.
 */


/* lookup tables used in conversion */

static float table_8_F[1 << 8];
static float table_8g_F[1 << 8];
static unsigned char table_F_8[1 << 16];
static unsigned char table_F_8g[1 << 16];


static int table_inited = 0;

static void
table_init (void)
{
  if (table_inited)
    return;
  table_inited = 1;

  /* fill tables for conversion from integer to float */
  {
    int       i;
    for (i = 0; i < 1 << 8; i++)
      {
        float direct = i/255.0;
        table_8_F[i] = direct;
        table_8g_F[i] = pow(direct, 2.2F);
      }
  }
  /* fill tables for conversion from float to integer */
  {
    union
    {
      float     f;
      unsigned short s[2];
    } u;
    u.f = 0.0;

    u.s[0] = 0.0;

    for (u.s[1] = 0; u.s[1] < 65535; u.s[1] += 1)
      {
        unsigned char c;
        unsigned char cg;

        if (u.f <= 0.0)
          {
            c = 0;
            cg = 0;
          }
        else if (u.f >= 1.0)
          {
            c = 255;
            cg = 255;
          }
        else
          {
            c  = rint (u.f * 255.0);
            cg = rint (pow(u.f,2.2F) * 255.0);
          }

        table_F_8[u.s[1]] = c;
        table_F_8g[u.s[1]] = cg;
      }
  }
  /* fix tables to ensure 1:1 conversions back and forth */
  if (0)
    {
      int       i;
      for (i = 0; i < 256; i++)
        {
          float     f = table_8_F[i];
          unsigned short *hi = ((unsigned short *)(void*) &f);
          unsigned short *lo = ((unsigned short *)(void*) &f);
          *lo = 0;
          table_F_8[(*hi)] = i;
        }
    }
}

/* function to find the index in table for a float */
static unsigned int
gggl_float_to_index16 (float f)
{
  union
  {
    float     f;
    unsigned short s[2];
  } u;
  u.f = f;
  return u.s[1];
}

static INLINE long
conv_F_8 (unsigned char *src, unsigned char *dst, long samples)
{
  if (!table_inited)
    table_init ();
  long n=samples;
  while (n--)
    {
      register float f = (*(float *) src);
      *(unsigned char *) dst = table_F_8[gggl_float_to_index16 (f)];
      dst += 1;
      src += 4;
    }
  return samples;
}

static INLINE long
conv_8_F (unsigned char *src, unsigned char *dst, long samples)
{
  if (!table_inited)
    table_init ();
  long n=samples;
  while (n--)
    {
      (*(float *) dst) = table_8_F[*(unsigned char *) src];
      dst += 4;
      src += 1;
    }
  return samples;
}


static INLINE long
conv_rgbaF_rgb8 (unsigned char *src, unsigned char *dst, long samples)
{
  long n=samples;
  while (n--)
  {
      register float f = (*(float *) src);
      *(unsigned char *) dst = table_F_8[gggl_float_to_index16 (f)];
      src += 4;
      dst += 1;

      f = (*(float *) src);
      *(unsigned char *) dst = table_F_8[gggl_float_to_index16 (f)];
      src += 4;
      dst += 1;

      f = (*(float *) src);
      *(unsigned char *) dst = table_F_8[gggl_float_to_index16 (f)];
      src += 4;
      dst += 1;

      src += 4;

  }
  return samples;
}


/*********/
static INLINE long
conv_rgbaF_rgba8 (unsigned char *src, unsigned char *dst, long samples)
{
  conv_F_8 (src, dst, samples * 4);
  return samples;
}

#define conv_rgbaF_rgbP8 conv_rgbaF_rgba8

static INLINE long
conv_rgbF_rgb8 (unsigned char *src, unsigned char *dst, long samples)
{
  conv_F_8 (src, dst, samples * 3);
  return samples;
}

static INLINE long
conv_gaF_ga8 (unsigned char *src, unsigned char *dst, long samples)
{
  conv_F_8 (src, dst, samples * 2);
  return samples;
}

#define conv_rgbAF_rgbA8 conv_rgbaF_rgba8
#define conv_gF_g8       conv_F_8
#define conv_gAF_gA8     conv_gaF_ga8


static INLINE long
conv_rgba8_rgbaF (unsigned char *src, unsigned char *dst, long samples)
{
  long n=samples;
  while (n--)
    {
      (*(float *) dst) = table_8g_F[*(unsigned char *) src];
      dst += 4;
      src += 1;

      (*(float *) dst) = table_8g_F[*(unsigned char *) src];
      dst += 4;
      src += 1;

      (*(float *) dst) = table_8g_F[*(unsigned char *) src];
      dst += 4;
      src += 1;

      (*(float *) dst) = table_8_F[*(unsigned char*) src];
      dst += 4;
      src += 1;
    }
  return samples;
}

static INLINE long
conv_rgb8_rgbaF (unsigned char *src, unsigned char *dst, long samples)
{
  long n=samples;
  while (n--)
    {
      (*(float *) dst) = table_8g_F[*(unsigned char *) src];
      dst += 4;
      src += 1;

      (*(float *) dst) = table_8g_F[*(unsigned char *) src];
      dst += 4;
      src += 1;

      (*(float *) dst) = table_8g_F[*(unsigned char *) src];
      dst += 4;
      src += 1;

      (*(float *) dst) = 1.0;
      dst += 4;
    }
  return samples;
}

static long
conv_rgbAF_sdl32 (unsigned char *srcc,
                  unsigned char *dstc,
                  long           samples)
{
  float         *src = (void*)srcc;
  unsigned char *dst = (void*)dstc;
  long n=samples;
  while (n--)
    {
      int i;
      float alpha = src[3];
      if (alpha < 0.0001)
        alpha = 0.0001;
      for (i=0;i<3;i++)
        {
          float ca=src[i];
          float c;
          int ret;
          c=ca/alpha;
          if (alpha==0.0)
            ret=0;
          else
            ret = table_F_8g[gggl_float_to_index16 (c)];
          if (ret<=0)
            dst[i]=0;
          else if (ret>255)
            dst[i]=255;
          else
            dst[i] = ret;
        }
      {
        int t=dst[0];
        dst[0]=dst[2];
        dst[2]=t;
        dst[3]=255;
      }
      src += 4;
      dst += 4;
    }
  return samples;
}


static long
conv_rgbaF_sdl32 (unsigned char *srcc,
                  unsigned char *dstc,
                  long           samples)
{
  float         *src = (void*)srcc;
  unsigned char *dst = (void*)dstc;
  long n=samples;
  while (n--)
    {
      dst[0] = table_F_8g[gggl_float_to_index16 (src[2])];
      dst[1] = table_F_8g[gggl_float_to_index16 (src[1])];
      dst[2] = table_F_8g[gggl_float_to_index16 (src[0])];
      src += 4;
      dst += 4;
    }
  return samples;
}

#define conv_rgb8_rgbAF conv_rgb8_rgbaF

int
init (void)
{
  Babl *rgbaF = babl_format_new (
      babl_model ("RGBA"),
      babl_type  ("float"),
      babl_component ("R"),
      babl_component ("G"),
      babl_component ("B"),
      babl_component ("A"),
      NULL);
  Babl *rgbAF = babl_format_new (
      babl_model ("RaGaBaA"),
      babl_type  ("float"),
      babl_component ("Ra"),
      babl_component ("Ga"),
      babl_component ("Ba"),
      babl_component ("A"),
      NULL);
  Babl *rgba8 = babl_format_new (
      babl_model ("R'G'B'A"),
      babl_type  ("u8"),
      babl_component ("R'"),
      babl_component ("G'"),
      babl_component ("B'"),
      babl_component ("A"),
      NULL);
  Babl *rgb8 = babl_format_new (
      babl_model ("R'G'B'"),
      babl_type  ("u8"),
      babl_component ("R'"),
      babl_component ("G'"),
      babl_component ("B'"),
      NULL);
  Babl *sdl32 = babl_format_new (
      babl_model ("R'G'B'"),
      babl_type  ("u8"),
      babl_component ("B'"),
      babl_component ("G'"),
      babl_component ("R'"),
      babl_component ("PAD"),
      NULL);

  table_init ();

#define o(src,dst) \
  babl_conversion_new (src, dst, "linear", conv_##src##_##dst, NULL)

  o (rgb8, rgbaF);
  o (rgb8, rgbAF);
  o (rgba8, rgbaF);
  o (rgbaF, sdl32);
  o (rgbAF, sdl32);

  return 0;
}
