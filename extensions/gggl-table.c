/*
 * This file was part of gggl, it implements a variety of pixel conversion
 * functions that are usable with babl, the file needs more cleanup.
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
#include <stdint.h>

#include "babl.h"
#include "extensions/util.h"

/*
 * Implemented according to information read from:
 *
 * http://www.cinenet.net/~spitzak/conversion/sketches_0265.pdf
 *
 * due to ability to be able to relicence gggl under a different
 * licence than GPL, I avoided the temptation to look at the
 * source files in the same location, in case I was going to
 * need this piece of code for projects where GPL compatibility
 * was a must.
 *
 */

/* lookup tables used in conversion */

static float          table_8_F[1 << 8];
static float          table_16_F[1 << 16];
static unsigned char  table_F_8[1 << 16];
static unsigned short table_F_16[1 << 16];

static uint32_t      *table_8_F_int = NULL;

static int table_inited = 0;

static void
table_init (void)
{
  if (table_inited)
    return;

  table_8_F_int = (void*)(table_8_F);

  table_inited = 1;

  /* fill tables for conversion from integer to float */
  {
    int i;
    for (i = 0; i < 1 << 8; i++)
      {
        table_8_F[i] = (i * 1.0) / 255.0;
      }
    for (i = 0; i < 1 << 16; i++)
      table_16_F[i] = (i * 1.0) / 65535.0;
  }
  /* fill tables for conversion from float to integer */
  {
    union
    {
      float          f;
      unsigned short s[2];
    } u;
    u.f = 0.0;

    u.s[0] = 0x8000;

    for (u.s[1] = 0; u.s[1] < 65535; u.s[1] += 1)
      {
        unsigned char  c;
        unsigned short s;

        if (u.f <= 0.0)
          {
            c = 0;
            s = 0;
          }
        else if (u.f >= 1.0)
          {
            c = 255;
            s = 65535;
          }
        else
          {
            c = u.f * 255 + 0.5f;
            s = u.f * 65535 + 0.5f;
          }

        /*fprintf (stderr, "%2.3f=%03i %05i ", f, c, (*hi));
           / if (! ((*hi)%9))
           /         fprintf (stderr, "\n"); */

        table_F_8[u.s[1]]  = c;
        table_F_16[u.s[1]] = s;
      }
  }
  /* patch tables to ensure 1:1 conversions back and forth */
  if (0)
    {                           /*FIXME: probably not the right way to do it,.. must sit down and scribble on paper */
      int i;
      for (i = 0; i < 256; i++)
        {
          float           f  = table_8_F[i];
          unsigned short *hi = ((unsigned short *) (void *) &f);
          unsigned short *lo = ((unsigned short *) (void *) &f);
          *lo              = 0;
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
    float          f;
    unsigned short s[2];
  } u;
  u.f = f;
  return u.s[1];
}

static void
conv_F_8 (const Babl    *conversion,
          unsigned char *src, 
          unsigned char *dst, 
          long samples)
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


static void
conv_F_16 (const Babl    *conversion,
           unsigned char *src, 
           unsigned char *dst, 
           long samples)
{
  long n = samples;

  if (!table_inited)
    table_init ();
  while (n--)
    {
      register float f = (*(float *) src);
      *(unsigned short *) dst = table_F_16[gggl_float_to_index16 (f)];
      dst                    += 2;
      src                    += 4;
    }
}

static void
conv_8_F (const Babl    *conversion,
          unsigned char *src, 
          unsigned char *dst, 
          long samples)
{
  long n = samples;

  if (!table_inited)
    table_init ();
  while (n--)
    {
      (*(uint32_t *) dst) = table_8_F_int[*(unsigned char *) src];
      dst             += 4;
      src             += 1;
    }
}

static void
conv_rgb8_rgbaF (const Babl    *conversion,
                 unsigned char *src, 
                 unsigned char *dst, 
                 long samples)
{
  long n = samples;

  if (!table_inited)
    table_init ();
  while (n--)
    {
      (*(uint32_t *) dst) = table_8_F_int[*(unsigned char *) src];
      dst             += 4;
      src             += 1;
      (*(uint32_t *) dst) = table_8_F_int[*(unsigned char *) src];
      dst             += 4;
      src             += 1;
      (*(uint32_t *) dst) = table_8_F_int[*(unsigned char *) src];
      dst             += 4;
      src             += 1;
      (*(float    *) dst) = 1.0;
      dst             += 4;
    }
}

static void
conv_16_F (const Babl    *conversion,
           unsigned char *src, 
           unsigned char *dst, 
           long samples)
{
  long n = samples;

  if (!table_inited)
    table_init ();
  while (n--)
    {
      (*(float *) dst) = table_16_F[*(unsigned short *) src];
      dst             += 4;
      src             += 2;
    }
}

static void
conv_rgbaF_rgb8 (const Babl    *conversion,
                 unsigned char *src, 
                 unsigned char *dst, 
                 long samples)
{
  long n = samples;

  while (n--)
    {
      register float f = (*(float *) src);
      *(unsigned char *) dst = table_F_8[gggl_float_to_index16 (f)];
      src                   += 4;
      dst                   += 1;

      f                      = (*(float *) src);
      *(unsigned char *) dst = table_F_8[gggl_float_to_index16 (f)];
      src                   += 4;
      dst                   += 1;

      f                      = (*(float *) src);
      *(unsigned char *) dst = table_F_8[gggl_float_to_index16 (f)];
      src                   += 4;
      dst                   += 1;

      src += 4;
    }
}


/*********/
static void
conv_rgbaF_rgba8 (const Babl    *conversion,
                  unsigned char *src, 
                  unsigned char *dst, 
                  long samples)
{
  conv_F_8 (conversion, src, dst, samples * 4);
}

static void
conv_rgbF_rgb8 (const Babl    *conversion,
                unsigned char *src, 
                unsigned char *dst, 
                long samples)
{
  conv_F_8 (conversion, src, dst, samples * 3);
}

static void
conv_gaF_ga8 (const Babl    *conversion,
              unsigned char *src, 
              unsigned char *dst, 
              long samples)
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
                   long samples)
{
  conv_F_16 (conversion, src, dst, samples * 4);
}

static void
conv_rgbF_rgb16 (const Babl    *conversion,
                 unsigned char *src, 
                 unsigned char *dst, 
                 long samples)
{
  conv_F_16 (conversion, src, dst, samples * 3);
}

static void
conv_gaF_ga16 (const Babl    *conversion,
               unsigned char *src, 
               unsigned char *dst, 
               long samples)
{
  conv_F_16 (conversion, src, dst, samples * 2);
}

#define conv_rgbAF_rgbA16    conv_rgbaF_rgba16
#define conv_gF_g16          conv_F_16
#define conv_gAF_gA16        conv_gaF_ga16

static void
conv_rgba8_rgbaF (const Babl    *conversion,
                  unsigned char *src, 
                  unsigned char *dst, 
                  long samples)
{
  conv_8_F (conversion, src, dst, samples * 4);
}


static void
conv_rgb8_rgbF (const Babl    *conversion,
                unsigned char *src, 
                unsigned char *dst, 
                long samples)
{
  conv_8_F (conversion, src, dst, samples * 3);
}

static void
conv_ga8_gaF (const Babl    *conversion,
              unsigned char *src, 
              unsigned char *dst, 
              long samples)
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
                   long samples)
{
  conv_16_F (conversion, src, dst, samples * 4);
}

static void
conv_rgb16_rgbF (const Babl    *conversion,
                 unsigned char *src, 
                 unsigned char *dst, 
                 long samples)
{
  conv_16_F (conversion, src, dst, samples * 3);
}

static void
conv_ga16_gaF (const Babl    *conversion,
               unsigned char *src, 
               unsigned char *dst, 
               long samples)
{
  conv_16_F (conversion, src, dst, samples * 2);
}

#define conv_rgbA16_rgbAF    conv_rgba16_rgbaF
#define conv_gA16_gAF        conv_ga16_gaF
#define conv_g16_gF          conv_16_F

int init (void);

int
init (void)
{
  const Babl *rgbaF = babl_format_new (
    babl_model ("R'G'B'A"),
    babl_type ("float"),
    babl_component ("R'"),
    babl_component ("G'"),
    babl_component ("B'"),
    babl_component ("A"),
    NULL);
  const Babl *rgba16 = babl_format_new (
    babl_model ("R'G'B'A"),
    babl_type ("u16"),
    babl_component ("R'"),
    babl_component ("G'"),
    babl_component ("B'"),
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
  const Babl *rgbAF = babl_format_new (
    babl_model ("R'aG'aB'aA"),
    babl_type ("float"),
    babl_component ("R'a"),
    babl_component ("G'a"),
    babl_component ("B'a"),
    babl_component ("A"),
    NULL);
  const Babl *rgbA16 = babl_format_new (
    babl_model ("R'aG'aB'aA"),
    babl_type ("u16"),
    babl_component ("R'a"),
    babl_component ("G'a"),
    babl_component ("B'a"),
    babl_component ("A"),
    NULL);
  const Babl *rgbA8 = babl_format_new (
    babl_model ("R'aG'aB'aA"),
    babl_type ("u8"),
    babl_component ("R'a"),
    babl_component ("G'a"),
    babl_component ("B'a"),
    babl_component ("A"),
    NULL);
  const Babl *rgbF = babl_format_new (
    babl_model ("R'G'B'"),
    babl_type ("float"),
    babl_component ("R'"),
    babl_component ("G'"),
    babl_component ("B'"),
    NULL);
  const Babl *rgb16 = babl_format_new (
    babl_model ("R'G'B'"),
    babl_type ("u16"),
    babl_component ("R'"),
    babl_component ("G'"),
    babl_component ("B'"),
    NULL);
  const Babl *rgb8 = babl_format_new (
    babl_model ("R'G'B'"),
    babl_type ("u8"),
    babl_component ("R'"),
    babl_component ("G'"),
    babl_component ("B'"),
    NULL);
  const Babl *gaF = babl_format_new (
    babl_model ("Y'A"),
    babl_type ("float"),
    babl_component ("Y'"),
    babl_component ("A"),
    NULL);
  const Babl *gAF = babl_format_new (
    babl_model ("Y'aA"),
    babl_type ("float"),
    babl_component ("Y'a"),
    babl_component ("A"),
    NULL);
  const Babl *gF = babl_format_new (
    babl_model ("Y'"),
    babl_type ("float"),
    babl_component ("Y'"),
    NULL);
  const Babl *ga16 = babl_format_new (
    babl_model ("Y'A"),
    babl_type ("u16"),
    babl_component ("Y'"),
    babl_component ("A"),
    NULL);
  const Babl *gA16 = babl_format_new (
    babl_model ("Y'aA"),
    babl_type ("u16"),
    babl_component ("Y'a"),
    babl_component ("A"),
    NULL);
  const Babl *g16 = babl_format_new (
    babl_model ("Y'"),
    babl_type ("u16"),
    babl_component ("Y'"),
    NULL);
  const Babl *ga8 = babl_format_new (
    babl_model ("Y'A"),
    babl_type ("u8"),
    babl_component ("Y'"),
    babl_component ("A"),
    NULL);
  const Babl *gA8 = babl_format_new (
    babl_model ("Y'aA"),
    babl_type ("u8"),
    babl_component ("Y'a"),
    babl_component ("A"),
    NULL);
  const Babl *g8 = babl_format_new (
    babl_model ("Y'"),
    babl_type ("u8"),
    babl_component ("Y'"),
    NULL);

#define o(src, dst) \
  babl_conversion_new (src, dst, "linear", conv_ ## src ## _ ## dst, NULL)

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
  o (gaF, ga8);
  o (gAF, gA8);
  o (gF, g8);
  o (ga8, gaF);
  o (gA8, gAF);
  o (g8, gF);
  o (gaF, ga16);
  o (gAF, gA16);
  o (gF, g16);
  o (ga16, gaF);
  o (gA16, gAF);
  o (g16, gF);
  o (rgbaF, rgb8);
  o (rgb8, rgbaF);

  if (!table_inited)
    table_init ();

  return 0;
}
