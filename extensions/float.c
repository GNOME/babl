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

#include "babl-internal.h"
#include "babl-cpuaccel.h"
#include "extensions/util.h"
#include "base/util.h"

static const Babl *trc_srgb = NULL;

static void
conv_rgbaF_linear_rgbAF_nonlinear (const Babl    *conversion,
                                   unsigned char *src,
                                   unsigned char *dst,
                                   long           samples)
{
   const Babl  *space = babl_conversion_get_destination_space (conversion);
   const Babl **trc   = (void*)space->space.trc;

   float *fsrc = (float *) src;
   float *fdst = (float *) dst;
   int n = samples;

   while (n--)
     {
       float alpha = fsrc[3];
       if (alpha < BABL_ALPHA_FLOOR)
       {
         if (alpha >= 0.0f)
           alpha = BABL_ALPHA_FLOOR;
         else if (alpha >= -BABL_ALPHA_FLOOR)
           alpha = -BABL_ALPHA_FLOOR;
       }
       *fdst++ = babl_trc_from_linear (trc[0], *fsrc++) * alpha;
       *fdst++ = babl_trc_from_linear (trc[1], *fsrc++) * alpha;
       *fdst++ = babl_trc_from_linear (trc[2], *fsrc++) * alpha;
       *fdst++ = alpha;
       fsrc++;
     }
}

static void
conv_rgbaF_linear_rgbAF_perceptual (const Babl    *conversion,
                                    unsigned char *src,
                                    unsigned char *dst,
                                    long           samples)
{
   float *fsrc = (float *) src;
   float *fdst = (float *) dst;
   int n = samples;

   while (n--)
     {
       float alpha = fsrc[3];
       if (alpha < BABL_ALPHA_FLOOR)
       {
         if (alpha >= 0.0f)
           alpha = BABL_ALPHA_FLOOR;
         else if (alpha >= -BABL_ALPHA_FLOOR)
           alpha = -BABL_ALPHA_FLOOR;
       }
       *fdst++ = babl_trc_from_linear (trc_srgb, *fsrc++) * alpha;
       *fdst++ = babl_trc_from_linear (trc_srgb, *fsrc++) * alpha;
       *fdst++ = babl_trc_from_linear (trc_srgb, *fsrc++) * alpha;
       *fdst++ = alpha;
       fsrc++;
     }
}


static void
conv_rgbAF_linear_rgbAF_nonlinear (const Babl    *conversion,
                                   unsigned char *src,
                                   unsigned char *dst,
                                   long           samples)
{
   const Babl  *space = babl_conversion_get_destination_space (conversion);
   const Babl **trc   = (void*)space->space.trc;

   float *fsrc = (float *) src;
   float *fdst = (float *) dst;
   int n = samples;

   while (n--)
     {
       float alpha = fsrc[3];
       if (alpha == 0)
         {
           *fdst++ = 0.0;
           *fdst++ = 0.0;
           *fdst++ = 0.0;
           *fdst++ = 0.0;
           fsrc+=4;
         }
       else
         {
           float alpha_recip = 1.0 / alpha;
           *fdst++ = babl_trc_from_linear (trc[0], *fsrc++ * alpha_recip) * alpha;
           *fdst++ = babl_trc_from_linear (trc[1], *fsrc++ * alpha_recip) * alpha;
           *fdst++ = babl_trc_from_linear (trc[2], *fsrc++ * alpha_recip) * alpha;
           *fdst++ = *fsrc++;
         }
     }
}

static void
conv_rgbAF_linear_rgbAF_perceptual (const Babl    *conversion,
                                    unsigned char *src,
                                    unsigned char *dst,
                                    long           samples)
{

   float *fsrc = (float *) src;
   float *fdst = (float *) dst;
   int n = samples;

   while (n--)
     {
       float alpha = fsrc[3];
       if (alpha == 0.0f)
         {
           *fdst++ = 0.0f;
           *fdst++ = 0.0f;
           *fdst++ = 0.0f;
           *fdst++ = 0.0f;
           fsrc+=4;
         }
       else
         {
           float alpha_recip = 1.0f / alpha;
           *fdst++ = babl_trc_from_linear (trc_srgb, *fsrc++ * alpha_recip) * alpha;
           *fdst++ = babl_trc_from_linear (trc_srgb, *fsrc++ * alpha_recip) * alpha;
           *fdst++ = babl_trc_from_linear (trc_srgb, *fsrc++ * alpha_recip) * alpha;
           *fdst++ = *fsrc++;
         }
     }
}


static void
conv_rgbaF_linear_rgbaF_nonlinear (const Babl    *conversion,
                                   unsigned char *src, 
                                   unsigned char *dst, 
                                   long           samples)
{
   const Babl  *space = babl_conversion_get_destination_space (conversion);
   const Babl **trc   = (void*)space->space.trc;

   float *fsrc = (float *) src;
   float *fdst = (float *) dst;
   int n = samples;

   while (n--)
     {
       *fdst++ = babl_trc_from_linear (trc[0], *fsrc++);
       *fdst++ = babl_trc_from_linear (trc[1], *fsrc++);
       *fdst++ = babl_trc_from_linear (trc[2], *fsrc++);
       *fdst++ = *fsrc++;
     }
}

static void
conv_rgbaF_linear_rgbaF_perceptual (const Babl    *conversion,
                                    unsigned char *src, 
                                    unsigned char *dst, 
                                    long           samples)
{
   float *fsrc = (float *) src;
   float *fdst = (float *) dst;
   int n = samples;

   while (n--)
     {
       *fdst++ = babl_trc_from_linear (trc_srgb, *fsrc++);
       *fdst++ = babl_trc_from_linear (trc_srgb, *fsrc++);
       *fdst++ = babl_trc_from_linear (trc_srgb, *fsrc++);
       *fdst++ = *fsrc++;
     }
}


static void
conv_rgbF_linear_rgbF_nonlinear (const Babl    *conversion,
                                 unsigned char *src,
                                 unsigned char *dst,
                                 long           samples)
{
   const Babl  *space = babl_conversion_get_destination_space (conversion);
   const Babl **trc   = (void*)space->space.trc;
   float *fsrc = (float *) src;
   float *fdst = (float *) dst;
   int n = samples;

   while (n--)
     {
       *fdst++ = babl_trc_from_linear (trc[0], *fsrc++);
       *fdst++ = babl_trc_from_linear (trc[1], *fsrc++);
       *fdst++ = babl_trc_from_linear (trc[2], *fsrc++);
     }
}

static void
conv_rgbF_linear_rgbF_perceptual (const Babl    *conversion,
                                  unsigned char *src,
                                  unsigned char *dst,
                                  long           samples)
{
   float *fsrc = (float *) src;
   float *fdst = (float *) dst;
   int n = samples;

   while (n--)
     {
       *fdst++ = babl_trc_from_linear (trc_srgb, *fsrc++);
       *fdst++ = babl_trc_from_linear (trc_srgb, *fsrc++);
       *fdst++ = babl_trc_from_linear (trc_srgb, *fsrc++);
     }
}


static void
conv_rgbaF_nonlinear_rgbaF_linear (const Babl    *conversion,
                                   unsigned char *src,
                                   unsigned char *dst,
                                   long           samples)
{
   const Babl  *space = babl_conversion_get_destination_space (conversion);
   const Babl **trc   = (void*)space->space.trc;
   float *fsrc = (float *) src;
   float *fdst = (float *) dst;
   int n = samples;

   while (n--)
     {
       *fdst++ = babl_trc_to_linear (trc[0], *fsrc++);
       *fdst++ = babl_trc_to_linear (trc[1], *fsrc++);
       *fdst++ = babl_trc_to_linear (trc[2], *fsrc++);
       *fdst++ = *fsrc++;
     }
}

static void
conv_rgbaF_perceptual_rgbaF_linear (const Babl    *conversion,
                                    unsigned char *src,
                                    unsigned char *dst,
                                    long           samples)
{
   float *fsrc = (float *) src;
   float *fdst = (float *) dst;
   int n = samples;

   while (n--)
     {
       *fdst++ = babl_trc_to_linear (trc_srgb, *fsrc++);
       *fdst++ = babl_trc_to_linear (trc_srgb, *fsrc++);
       *fdst++ = babl_trc_to_linear (trc_srgb, *fsrc++);
       *fdst++ = *fsrc++;
     }
}


static void
conv_rgbF_nonlinear_rgbF_linear (const Babl    *conversion,
                                 unsigned char *src,
                                 unsigned char *dst,
                                 long           samples)
{
   const Babl  *space = babl_conversion_get_destination_space (conversion);
   const Babl **trc   = (void*)space->space.trc;
   float *fsrc = (float *) src;
   float *fdst = (float *) dst;
   int n = samples;

   while (n--)
     {
       *fdst++ = babl_trc_to_linear (trc[0], *fsrc++);
       *fdst++ = babl_trc_to_linear (trc[1], *fsrc++);
       *fdst++ = babl_trc_to_linear (trc[2], *fsrc++);
     }
}

static void
conv_rgbF_perceptual_rgbF_linear (const Babl    *conversion,
                                  unsigned char *src,
                                  unsigned char *dst,
                                  long           samples)
{
   float *fsrc = (float *) src;
   float *fdst = (float *) dst;
   int n = samples;

   while (n--)
     {
       *fdst++ = babl_trc_to_linear (trc_srgb, *fsrc++);
       *fdst++ = babl_trc_to_linear (trc_srgb, *fsrc++);
       *fdst++ = babl_trc_to_linear (trc_srgb, *fsrc++);
     }
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
  const Babl *rgbaF_nonlinear = babl_format_new (
    babl_model ("R'G'B'A"),
    babl_type ("float"),
    babl_component ("R'"),
    babl_component ("G'"),
    babl_component ("B'"),
    babl_component ("A"),
    NULL);
  const Babl *rgbaF_perceptual = babl_format_new (
    babl_model ("R~G~B~A"),
    babl_type ("float"),
    babl_component ("R~"),
    babl_component ("G~"),
    babl_component ("B~"),
    babl_component ("A"),
    NULL);
  const Babl *rgbAF_nonlinear = babl_format_new (
    babl_model ("R'aG'aB'aA"),
    babl_type ("float"),
    babl_component ("R'a"),
    babl_component ("G'a"),
    babl_component ("B'a"),
    babl_component ("A"),
    NULL);
  const Babl *rgbAF_perceptual = babl_format_new (
    babl_model ("R~aG~aB~aA"),
    babl_type ("float"),
    babl_component ("R~a"),
    babl_component ("G~a"),
    babl_component ("B~a"),
    babl_component ("A"),
    NULL);
  const Babl *rgbF_linear = babl_format_new (
    babl_model ("RGB"),
    babl_type ("float"),
    babl_component ("R"),
    babl_component ("G"),
    babl_component ("B"),
    NULL);
  const Babl *rgbF_nonlinear = babl_format_new (
    babl_model ("R'G'B'"),
    babl_type ("float"),
    babl_component ("R'"),
    babl_component ("G'"),
    babl_component ("B'"),
    NULL);
  const Babl *rgbF_perceptual = babl_format_new (
    babl_model ("R~G~B~"),
    babl_type ("float"),
    babl_component ("R~"),
    babl_component ("G~"),
    babl_component ("B~"),
    NULL);
  trc_srgb = babl_trc("sRGB");

  o (rgbAF_linear, rgbAF_nonlinear);
  o (rgbaF_linear, rgbAF_nonlinear);
  o (rgbaF_linear, rgbaF_nonlinear);
  o (rgbaF_nonlinear,  rgbaF_linear);
  o (rgbF_linear, rgbF_nonlinear);
  o (rgbF_nonlinear,  rgbF_linear);

  o (rgbAF_linear, rgbAF_perceptual);
  o (rgbaF_linear, rgbAF_perceptual);
  o (rgbaF_linear, rgbaF_perceptual);
  o (rgbaF_perceptual,  rgbaF_linear);
  o (rgbF_linear, rgbF_perceptual);
  o (rgbF_perceptual,  rgbF_linear);

  return 0;
}

