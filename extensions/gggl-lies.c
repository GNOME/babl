/*
 * This file was part of gggl, it implements a variety of pixel conversion
 * functions that are usable with babl, the file needs more cleanup, and
 * doesn't return the number of samples processed as a long, like it's
 * supposed to.
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
 *  Copyright 2003, 2004, 2005 Øyvind Kolås <pippin@gimp.org>
 */

#define _POSIX_C_SOURCE 200112L

#include "config.h"
#include <math.h>
#include <string.h>

#include "babl.h"
#include "extensions/util.h"

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
 */

static void
conv_F_8 (const Babl    *conversion,
          unsigned char *src, 
          unsigned char *dst, 
          long           samples)
{
  long n = samples;

  while (n--)
    {
      float f = ((*(float *) src));
      if (f < 0.0)
        {
          *(unsigned char *) dst = 0;
        }
      else if (f > 1.0)
        {
          *(unsigned char *) dst = 255;
        }
      else
        {
          *(unsigned char *) dst = lrint (f * 255.0);
        }
      dst += 1;
      src += 4;
    }
}

static void
conv_F_16 (const Babl    *conversion,
           unsigned char *src, 
           unsigned char *dst, 
           long           samples)
{
  long n = samples;

  while (n--)
    {
      float f = ((*(float *) src));
      if (f < 0.0)
        {
          *(unsigned short *) dst = 0;
        }
      else if (f > 1.0)
        {
          *(unsigned short *) dst = 65535;
        }
      else
        {
          *(unsigned short *) dst = lrint (f * 65535.0);
        }
      dst += 2;
      src += 4;
    }
}



static void
conv_8_F (const Babl    *conversion,
          unsigned char *src, 
          unsigned char *dst, 
          long           samples)
{
  long n = samples;

  while (n--)
    {
      (*(float *) dst) = (*(unsigned char *) src / 255.0);
      dst             += 4;
      src             += 1;
    }
}

static void
conv_16_F (const Babl    *conversion,
           unsigned char *src, 
           unsigned char *dst, 
           long           samples)
{
  long n = samples;

  while (n--)
    {
      (*(float *) dst) = *(unsigned short *) src / 65535.0;
      dst             += 4;
      src             += 2;
    }
}

static void
conv_F_D (const Babl    *conversion,
          unsigned char *src, 
          unsigned char *dst, 
          long           samples)
{
  long n = samples;

  while (n--)
    {
      *(double *) dst = ((*(float *) src));
      dst            += 8;
      src            += 4;
    }
}

static void
conv_D_F (const Babl    *conversion,
          unsigned char *src, 
          unsigned char *dst, 
          long           samples)
{
  long n = samples;

  while (n--)
    {
      *(float *) dst = ((*(double *) src));
      dst           += 4;
      src           += 8;
    }
}

static void
conv_16_8 (const Babl    *conversion,
           unsigned char *src, 
           unsigned char *dst, 
           long           samples)
{
  long n = samples;

  while (n>4)
    {
#define div_257(a) ((((a)+128)-(((a)+128)>>8))>>8)
      ((unsigned char *) dst)[0] = div_257 (((unsigned short *) src)[0]);
      ((unsigned char *) dst)[1] = div_257 (((unsigned short *) src)[1]);
      ((unsigned char *) dst)[2] = div_257 (((unsigned short *) src)[2]);
      ((unsigned char *) dst)[3] = div_257 (((unsigned short *) src)[3]);
      dst                     += 4;
      src                     += 8;
      n-=4;
    }

  while (n--)
    {
      (*(unsigned char *) dst) = div_257 (*(unsigned short *) src);
      dst                     += 1;
      src                     += 2;
    }
}

static void
conv_8_16 (const Babl    *conversion,
           unsigned char *src, 
           unsigned char *dst, 
           long           samples)
{
  long n = samples;

  while (n--)
    {
      (*(unsigned short *) dst) = ((*(unsigned char *) src) << 8) | *src;
      dst                      += 2;
      src                      += 1;
    }
}

/*********/
static void
conv_rgbaF_rgba8 (const Babl    *conversion,
                  unsigned char *src, 
                  unsigned char *dst, 
                  long           samples)
{
  conv_F_8 (conversion, src, dst, samples * 4);
}

#define conv_rgbaF_rgbP8    conv_rgbaF_rgba8

static void
conv_rgbF_rgb8 (const Babl    *conversion,
                unsigned char *src, 
                unsigned char *dst, 
                long           samples)
{
  conv_F_8 (conversion, src, dst, samples * 3);
}

static void
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
conv_rgbaF_rgba16 (const Babl    *conversion,
                   unsigned char *src, 
                   unsigned char *dst, 
                   long           samples)
{
  conv_F_16 (conversion, src, dst, samples * 4);
}

static void
conv_rgbaF_rgbaD (const Babl    *conversion,
                  unsigned char *src, 
                  unsigned char *dst, 
                  long           samples)
{
  conv_F_D (conversion, src, dst, samples * 4);
}

static void
conv_rgbaD_rgbaF (const Babl    *conversion,
                  unsigned char *src, 
                  unsigned char *dst, 
                  long           samples)
{
  conv_D_F (conversion, src, dst, samples * 4);
}

static void
conv_rgbF_rgb16 (const Babl    *conversion,
                 unsigned char *src, 
                 unsigned char *dst, 
                 long           samples)
{
  conv_F_16 (conversion, src, dst, samples * 3);
}

static void
conv_gaF_ga16 (const Babl    *conversion,
               unsigned char *src, 
               unsigned char *dst, 
               long           samples)
{
  conv_F_16 (conversion, src, dst, samples * 2);
}

#define conv_rgbAF_rgbAD     conv_rgbaF_rgbaD
#define conv_rgbAD_rgbAF     conv_rgbaD_rgbaF
#define conv_rgbAF_rgbA16    conv_rgbaF_rgba16
#define conv_gF_g16          conv_F_16
#define conv_gAF_gA16        conv_gaF_ga16

static void
conv_rgba8_rgbaF (const Babl    *conversion,
                  unsigned char *src, 
                  unsigned char *dst, 
                  long           samples)
{
  conv_8_F (conversion, src, dst, samples * 4);
}

static void
conv_rgb8_rgbF (const Babl    *conversion,
                unsigned char *src, 
                unsigned char *dst, 
                long           samples)
{
  conv_8_F (conversion, src, dst, samples * 3);
}

static void
conv_ga8_gaF (const Babl    *conversion,
              unsigned char *src, 
              unsigned char *dst, 
              long           samples)
{
  conv_8_F (conversion, src, dst, samples * 2);
}

#define conv_rgbA8_rgbAF    conv_rgba8_rgbaF
#define conv_gA8_gAF        conv_ga8_gaF
#define conv_g8_gF          conv_8_F

static void
conv_rgba16_rgbaF (const Babl    *conversion,
                   unsigned char *src, 
                   unsigned char *dst, 
                   long           samples)
{
  conv_16_F (conversion, src, dst, samples * 4);
}

static void
conv_rgb16_rgbF (const Babl    *conversion,
                 unsigned char *src, 
                 unsigned char *dst, 
                 long           samples)
{
  conv_16_F (conversion, src, dst, samples * 3);
}

static void
conv_ga16_gaF (const Babl    *conversion,
               unsigned char *src, 
               unsigned char *dst, 
               long           samples)
{
  conv_16_F (conversion, src, dst, samples * 2);
}

#define conv_rgbA16_rgbAF    conv_rgba16_rgbaF
#define conv_gA16_gAF        conv_ga16_gaF
#define conv_g16_gF          conv_16_F

static void
conv_rgba16_rgba8 (const Babl    *conversion,
                   unsigned char *src, 
                   unsigned char *dst, 
                   long           samples)
{
  conv_16_8 (conversion, src, dst, samples * 4);
}

static void
conv_rgb16_rgb8 (const Babl    *conversion,
                 unsigned char *src, 
                 unsigned char *dst, 
                 long           samples)
{
  conv_16_8 (conversion, src, dst, samples * 3);
}

static void
conv_ga16_ga8 (const Babl    *conversion,
               unsigned char *src, 
               unsigned char *dst, 
               long           samples)
{
  conv_16_8 (conversion, src, dst, samples * 2);
}

#define conv_rgbA16_rgbA8    conv_rgba16_rgba8
#define conv_gA16_gA8        conv_ga16_ga8
#define conv_g16_g8          conv_16_8

static void
conv_rgba8_rgba16 (const Babl    *conversion,
                   unsigned char *src, 
                   unsigned char *dst, 
                   long           samples)
{
  conv_8_16 (conversion, src, dst, samples * 4);
}

static void
conv_rgb8_rgb16 (const Babl    *conversion,
                 unsigned char *src, 
                 unsigned char *dst, 
                 long           samples)
{
  conv_8_16 (conversion, src, dst, samples * 3);
}

static void
conv_ga8_ga16 (const Babl    *conversion,
               unsigned char *src, 
               unsigned char *dst, 
               long           samples)
{
  conv_8_16 (conversion, src, dst, samples * 2);
}

#define conv_rgbA8_rgbA16    conv_rgba8_rgba16
#define conv_gA8_gA16        conv_ga8_ga16
#define conv_g8_g16          conv_8_16

/* alpha conversions */

static void
conv_gaF_gAF (const Babl    *conversion,
              unsigned char *src, 
              unsigned char *dst, 
              long           samples)
{
  long n = samples;

  while (n--)
    {
      float alpha = (*(float *) (src + 4));

      *(float *) dst = ((*(float *) src) * alpha);
      dst           += 4;
      src           += 4;
      *(float *) dst = alpha;
      dst           += 4;
      src           += 4;
    }
}

static void
conv_gAF_gaF (const Babl    *conversion,
              unsigned char *src, 
              unsigned char *dst, 
              long           samples)
{
  long n = samples;

  while (n--)
    {
      float alpha = (*(float *) (src + 4));

      if (alpha == 0.0f)
        *(float *) dst = 0.0f;
      else
        *(float *) dst = ((*(float *) src) / alpha);
      dst           += 4;
      src           += 4;
      *(float *) dst = alpha;
      dst           += 4;
      src           += 4;
    }
}

/* alpha stripping and adding */

static void
conv_rgbaF_rgbF (const Babl    *conversion,
                 unsigned char *src, 
                 unsigned char *dst, 
                 long           samples)
{
  long n = samples;

  while (n--)
    {
      *(int *) dst = (*(int *) src);
      dst         += 4;
      src         += 4;
      *(int *) dst = (*(int *) src);
      dst         += 4;
      src         += 4;
      *(int *) dst = (*(int *) src);
      dst         += 4;
      src         += 4;
      src         += 4;
    }
}


static void
conv_gF_rgbaF (const Babl    *conversion,
               unsigned char *src, 
               unsigned char *dst, 
               long           samples)
{
  long n = samples;

  while (n--)
    {
      *(int *) dst   = (*(int *) src);
      dst           += 4;
      *(int *) dst   = (*(int *) src);
      dst           += 4;
      *(int *) dst   = (*(int *) src);
      dst           += 4;
      *(float *) dst = 1.0;
      dst           += 4;
      src           += 4;
    }
}

#define conv_gF_rgbAF conv_gF_rgbaF

static void
conv_rgbF_rgbaF (const Babl    *conversion,
                 unsigned char *src, 
                 unsigned char *dst, 
                 long           samples)
{
  long n = samples;

  while (n--)
    {
      *(int *) dst   = (*(int *) src);
      dst           += 4;
      src           += 4;
      *(int *) dst   = (*(int *) src);
      dst           += 4;
      src           += 4;
      *(int *) dst   = (*(int *) src);
      dst           += 4;
      src           += 4;
      *(float *) dst = 1.0;
      dst           += 4;
    }
}

static void
conv_gaF_gF (const Babl    *conversion,
             unsigned char *src, 
             unsigned char *dst, 
             long           samples)
{
  long n = samples;

  while (n--)
    {
      *(int *) dst = (*(int *) src);
      dst         += 4;
      src         += 4;
      src         += 4;
    }
}

static void
conv_gF_gaF (const Babl    *conversion,
             unsigned char *src, 
             unsigned char *dst, 
             long           samples)
{
  long n = samples;

  while (n--)
    {
      *(int *) dst   = (*(int *) src);
      dst           += 4;
      src           += 4;
      *(float *) dst = 1.0;
      dst           += 4;
    }
}

#define conv_gF_gAF        conv_gF_gaF

#define conv_rgbF_rgbAF    conv_rgbF_rgbaF

/* colorchannel dropping and adding */

static void
conv_gF_rgbF (const Babl    *conversion,
              unsigned char *src, 
              unsigned char *dst, 
              long           samples)
{
  long n = samples;

  while (n--)
    {
      int c;

      for (c = 0; c < 3; c++)
        {
          (*(int *) dst) = (*(int *) src);
          dst           += 4;
        }
      src += 4;
    }
}

static void
conv_gaF_rgbaF (const Babl    *conversion,
                unsigned char *src, 
                unsigned char *dst, 
                long           samples)
{
  long n = samples;

  while (n--)
    {
      int c;

      for (c = 0; c < 3; c++)
        {
          (*(int *) dst) = (*(int *) src);
          dst           += 4;
        }
      src           += 4;
      (*(int *) dst) = (*(int *) src);
      dst           += 4;
      src           += 4;
    }
}

#define conv_gAF_rgbAF    conv_gaF_rgbaF

/* other conversions coded for some optimisation reason or sumthin */

static void
conv_rgbaF_rgbA8 (const Babl    *conversion,
                  unsigned char *src, 
                  unsigned char *dst, 
                  long           samples)
{
  long n = samples;

  while (n--)
    {
      float alpha = (*(float *) (src + (4 * 3)));
      int   c;

      for (c = 0; c < 3; c++)
        {
          *(unsigned char *) dst = lrint (((*(float *) src) * alpha) * 255.0);
          dst                   += 1;
          src                   += 4;
        }
      *(unsigned char *) dst = lrint (alpha * 255.0);
      dst++;
      src += 4;
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
      int c;

      for (c = 0; c < 3; c++)
        {
          *(unsigned char *) dst = lrint ((*(float *) src) * 255.0);
          dst                   += 1;
          src                   += 4;
        }
      src += 4;
    }
}

static void
conv_rgbaF_rgb16 (const Babl    *conversion,
                  unsigned char *src, 
                  unsigned char *dst, 
                  long           samples)
{
  long n = samples;

  while (n--)
    {
      int c;

      for (c = 0; c < 3; c++)
        {
          *(unsigned short *) dst = lrint ((*(float *) src) * 65535.0);
          dst                    += 2;
          src                    += 4;
        }
      src += 4;
    }
}

static void
conv_rgba8_rgbA8 (const Babl    *conversion,
                  unsigned char *src, 
                  unsigned char *dst, 
                  long           samples)
{
  long n = samples;

  while (n--)
    {
      if (src[3] == 255)
        {
          *(unsigned int *) dst = *(unsigned int *) src;
        }
      else if (src[3] == 0)
        {
          *(unsigned int *) dst = 0;
        }
      else
        {
#define div_255(a) ((((a)+127)+(((a)+127)>>8))>>8)
          dst[0] = div_255 (src[0] * src[3]);
          dst[1] = div_255 (src[1] * src[3]);
          dst[2] = div_255 (src[2] * src[3]);
          dst[3] = src[3];
        }
      dst += 4;
      src += 4;
    }
}

static void
conv_rgbA8_rgba8 (const Babl    *conversion,
                  unsigned char *src, 
                  unsigned char *dst, 
                  long           samples)
{
  long n = samples;

  while (n--)
    {
      if (src[3] == 255)
        {
          *(unsigned int *) dst = *(unsigned int *) src;
          dst                  += 4;
        }
      else if (src[3] == 0)
        {
          *(unsigned int *) dst = 0;
          dst                  += 4;
        }
      else
        {
          unsigned int aa = ((255 << 16) - 255) / src[3];
          *dst++ = (src[0] * aa + 0x8000) >> 16;
          *dst++ = (src[1] * aa + 0x8000) >> 16;
          *dst++ = (src[2] * aa + 0x8000) >> 16;
          *dst++ = src[3];
        }
      src += 4;
    }
}

static void
conv_rgb8_rgba8 (const Babl    *conversion,
                 unsigned char *src, 
                 unsigned char *dst, 
                 long           samples)
{
  long n = samples;

  while (n--)
    {
      dst[0] = src[0];
      dst[1] = src[1];
      dst[2] = src[2];
      dst[3] = 255;
      src   += 3;
      dst   += 4;
    }

}

#define conv_rgb8_rgbA8    conv_rgb8_rgba8

static void
conv_rgba8_rgb8 (const Babl    *conversion,
                 unsigned char *src, 
                 unsigned char *dst, 
                 long           samples)
{
  long n = samples;

  while (n--)
    {
      dst[0] = src[0];
      dst[1] = src[1];
      dst[2] = src[2];
      src   += 4;
      dst   += 3;
    }
}

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
  const Babl *rgba16 = babl_format_new (
    babl_model ("RGBA"),
    babl_type ("u16"),
    babl_component ("R"),
    babl_component ("G"),
    babl_component ("B"),
    babl_component ("A"),
    NULL);
  const Babl *rgbaD = babl_format_new (
    babl_model ("RGBA"),
    babl_type ("double"),
    babl_component ("R"),
    babl_component ("G"),
    babl_component ("B"),
    babl_component ("A"),
    NULL);
  const Babl *rgba8 = babl_format_new (
    babl_model ("RGBA"),
    babl_type ("u8"),
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
  const Babl *rgbAD = babl_format_new (
    babl_model ("RaGaBaA"),
    babl_type ("double"),
    babl_component ("Ra"),
    babl_component ("Ga"),
    babl_component ("Ba"),
    babl_component ("A"),
    NULL);

  const Babl *rgbA16 = babl_format_new (
    babl_model ("RaGaBaA"),
    babl_type ("u16"),
    babl_component ("Ra"),
    babl_component ("Ga"),
    babl_component ("Ba"),
    babl_component ("A"),
    NULL);
  const Babl *rgbA8 = babl_format_new (
    babl_model ("RaGaBaA"),
    babl_type ("u8"),
    babl_component ("Ra"),
    babl_component ("Ga"),
    babl_component ("Ba"),
    babl_component ("A"),
    NULL);
  const Babl *rgbF = babl_format_new (
    babl_model ("RGB"),
    babl_type ("float"),
    babl_component ("R"),
    babl_component ("G"),
    babl_component ("B"),
    NULL);
  const Babl *rgb16 = babl_format_new (
    babl_model ("RGB"),
    babl_type ("u16"),
    babl_component ("R"),
    babl_component ("G"),
    babl_component ("B"),
    NULL);
  const Babl *rgb8 = babl_format_new (
    babl_model ("RGB"),
    babl_type ("u8"),
    babl_component ("R"),
    babl_component ("G"),
    babl_component ("B"),
    NULL);
  const Babl *gaF = babl_format_new (
    babl_model ("YA"),
    babl_type ("float"),
    babl_component ("Y"),
    babl_component ("A"),
    NULL);
  const Babl *gAF = babl_format_new (
    babl_model ("YaA"),
    babl_type ("float"),
    babl_component ("Ya"),
    babl_component ("A"),
    NULL);
  const Babl *gF = babl_format_new (
    babl_model ("Y"),
    babl_type ("float"),
    babl_component ("Y"),
    NULL);
  const Babl *ga16 = babl_format_new (
    babl_model ("YA"),
    babl_type ("u16"),
    babl_component ("Y"),
    babl_component ("A"),
    NULL);
  const Babl *gA16 = babl_format_new (
    babl_model ("YaA"),
    babl_type ("u16"),
    babl_component ("Ya"),
    babl_component ("A"),
    NULL);
  const Babl *g16 = babl_format_new (
    babl_model ("Y"),
    babl_type ("u16"),
    babl_component ("Y"),
    NULL);
  const Babl *ga8 = babl_format_new (
    babl_model ("YA"),
    babl_type ("u8"),
    babl_component ("Y"),
    babl_component ("A"),
    NULL);
  const Babl *gA8 = babl_format_new (
    babl_model ("YaA"),
    babl_type ("u8"),
    babl_component ("Ya"),
    babl_component ("A"),
    NULL);
  const Babl *g8 = babl_format_new (
    babl_model ("Y"),
    babl_type ("u8"),
    babl_component ("Y"),
    NULL);

#define o(src, dst) \
  babl_conversion_new (src, dst, "linear", conv_ ## src ## _ ## dst, NULL)

  o (rgbaF, rgbaD);
  o (rgbaD, rgbaF);
  o (rgbAF, rgbAD);
  o (rgbAD, rgbAF);
  o (rgbaF, rgba8);
  o (rgba8, rgbaF);
  o (rgbaF, rgba16);
  o (rgba16, rgbaF);
  o (rgbAF, rgbA8);
  o (rgbA8, rgbAF);
  o (rgbAF, rgbA16);
  o (rgbA16, rgbAF);
  o (rgbF, rgb8);
  o (rgb8, rgbF);
  o (rgbF, rgb16);
  o (rgb16, rgbF);
  o (rgba8, rgba16);
  o (rgba16, rgba8);
  o (rgbA8, rgbA16);
  o (rgbA16, rgbA8);
  o (rgb8, rgb16);
  o (rgb16, rgb8);
  o (gaF, ga8);
  o (gAF, gA8);
  o (gF, g8);
  o (gF, rgbAF);
  o (gF, rgbaF);
  o (ga8, gaF);
  o (gA8, gAF);
  o (g8, gF);
  o (gaF, ga16);
  o (gAF, gA16);
  o (gF, g16);
  o (ga16, gaF);
  o (gA16, gAF);
  o (g16, gF);
  o (ga16, ga8);
  o (g16, g8);
  o (ga8, ga16);
  o (gA8, gA16);
  o (g8, g16);
  o (gaF, gAF);
  o (gAF, gaF);
  o (rgbaF, rgbF);
  o (gaF, gF);
  o (rgbF, rgbaF);
  o (rgbF, rgbAF);
  o (gF, gaF);
  o (gF, gAF);
  o (gF, rgbF);
  o (gaF, rgbaF);
  o (gAF, rgbAF);
  o (rgbaF, rgb8);
  o (rgbA8, rgba8);
  o (rgba8, rgbA8);
  o (rgbaF, rgb16);
  o (rgb8, rgba8);
  o (rgb8, rgbA8);
  o (rgba8, rgb8);
  o (rgbaF, rgbA8);

  return 0;
}
