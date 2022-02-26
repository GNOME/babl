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
conv_yaF_linear_yAF_linear (const Babl    *conversion,
                            unsigned char *__restrict__ src,
                            unsigned char *__restrict__ dst,
                            long           samples)
{
   float *fsrc = (float *) src;
   float *fdst = (float *) dst;
   int n = samples;

   while (n--)
     {
       float alpha = fsrc[1];
       float used_alpha = babl_epsilon_for_zero_float (alpha);
       *fdst++ = (*fsrc++) * used_alpha;
       *fdst++ = alpha;
       fsrc++;
     }
}


static void
conv_yAF_linear_yaF_linear (const Babl    *conversion,
                            unsigned char *__restrict__ src,
                            unsigned char *__restrict__ dst,
                            long           samples)
{
   float *fsrc = (float *) src;
   float *fdst = (float *) dst;
   int n = samples;

   while (n--)
     {
       float alpha = fsrc[1];
       float alpha_reciprocal = 1.0f/babl_epsilon_for_zero_float (alpha);
       *fdst++ = (*fsrc++) * alpha_reciprocal;
       *fdst++ = alpha;
       fsrc++;
     }
}


static void
conv_yaF_linear_yAF_nonlinear (const Babl    *conversion,
                               unsigned char *__restrict__ src,
                               unsigned char *__restrict__ dst,
                               long           samples)
{
   const Babl  *space = babl_conversion_get_destination_space (conversion);
   const Babl **trc   = (void*)space->space.trc;

   float *fsrc = (float *) src;
   float *fdst = (float *) dst;
   int n = samples;

   while (n--)
     {
       float alpha = fsrc[1];
       float used_alpha = babl_epsilon_for_zero_float (alpha);
       *fdst++ = babl_trc_from_linear (trc[0], *fsrc++) * used_alpha;
       *fdst++ = alpha;
       fsrc++;
     }
}

static void
conv_rgbaF_linear_rgbAF_nonlinear (const Babl    *conversion,
                                   unsigned char *__restrict__ src,
                                   unsigned char *__restrict__ dst,
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
       float used_alpha = babl_epsilon_for_zero_float (alpha);
       *fdst++ = babl_trc_from_linear (trc[0], *fsrc++) * used_alpha;
       *fdst++ = babl_trc_from_linear (trc[1], *fsrc++) * used_alpha;
       *fdst++ = babl_trc_from_linear (trc[2], *fsrc++) * used_alpha;
       *fdst++ = alpha;
       fsrc++;
     }
}

static void
conv_rgbaF_linear_rgbAF_perceptual (const Babl    *conversion,
                                    unsigned char *__restrict__ src,
                                    unsigned char *__restrict__ dst,
                                    long           samples)
{
   float *fsrc = (float *) src;
   float *fdst = (float *) dst;
   int n = samples;

   while (n--)
     {
       float alpha = fsrc[3];
       float used_alpha = babl_epsilon_for_zero (alpha);
       *fdst++ = babl_trc_from_linear (trc_srgb, *fsrc++) * used_alpha;
       *fdst++ = babl_trc_from_linear (trc_srgb, *fsrc++) * used_alpha;
       *fdst++ = babl_trc_from_linear (trc_srgb, *fsrc++) * used_alpha;
       *fdst++ = alpha;
       fsrc++;
     }
}


static void
conv_rgbAF_linear_rgbAF_nonlinear (const Babl    *conversion,
                                   unsigned char *__restrict__ src,
                                   unsigned char *__restrict__ dst,
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
           float alpha_recip = 1.0f / alpha;
           *fdst++ = babl_trc_from_linear (trc[0], *fsrc++ * alpha_recip) * alpha;
           *fdst++ = babl_trc_from_linear (trc[1], *fsrc++ * alpha_recip) * alpha;
           *fdst++ = babl_trc_from_linear (trc[2], *fsrc++ * alpha_recip) * alpha;
           *fdst++ = *fsrc++;
         }
     }
}


static void
conv_yAF_linear_yAF_nonlinear (const Babl    *conversion,
                               unsigned char *__restrict__ src,
                               unsigned char *__restrict__ dst,
                               long           samples)
{
   const Babl  *space = babl_conversion_get_destination_space (conversion);
   const Babl **trc   = (void*)space->space.trc;

   float *fsrc = (float *) src;
   float *fdst = (float *) dst;
   int n = samples;

   while (n--)
     {
       float alpha = fsrc[1];
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
           float alpha_recip = 1.0f / alpha;
           *fdst++ = babl_trc_from_linear (trc[0], *fsrc++ * alpha_recip) * alpha;
           *fdst++ = *fsrc++;
         }
     }
}



static void
conv_rgbAF_linear_rgbAF_perceptual (const Babl    *conversion,
                                    unsigned char *__restrict__ src,
                                    unsigned char *__restrict__ dst,
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
conv_yaF_linear_yaF_nonlinear (const Babl    *conversion,
                                   unsigned char *__restrict__ src, 
                                   unsigned char *__restrict__ dst, 
                                   long           samples)
{
   const Babl  *space = babl_conversion_get_destination_space (conversion);
   const Babl **trc   = (void*)space->space.trc;

   float *fsrc = (float *) src;
   float *fdst = (float *) dst;
   babl_trc_from_linear_buf (trc[0], fsrc, fdst, 2, 2, 1, samples);
}

static void
conv_rgbaF_linear_rgbaF_nonlinear (const Babl    *conversion,
                                   unsigned char *__restrict__ src, 
                                   unsigned char *__restrict__ dst, 
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
                                    unsigned char *__restrict__ src, 
                                    unsigned char *__restrict__ dst, 
                                    long           samples)
{
   float *fsrc = (float *) src;
   float *fdst = (float *) dst;
   babl_trc_from_linear_buf (trc_srgb, fsrc, fdst, 4, 4, 3, samples);
}

static void
conv_yF_linear_yF_nonlinear (const Babl    *conversion,
                             unsigned char *__restrict__ src,
                             unsigned char *__restrict__ dst,
                             long           samples)
{
   const Babl  *space = babl_conversion_get_destination_space (conversion);
   const Babl **trc   = (void*)space->space.trc;
   float *fsrc = (float *) src;
   float *fdst = (float *) dst;
   babl_trc_from_linear_buf (trc[0], fsrc, fdst, 1, 1, 1, samples);
}


static void
conv_rgbF_linear_rgbF_nonlinear (const Babl    *conversion,
                                 unsigned char *__restrict__ src,
                                 unsigned char *__restrict__ dst,
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
                                  unsigned char *__restrict__ src,
                                  unsigned char *__restrict__ dst,
                                  long           samples)
{
   float *fsrc = (float *) src;
   float *fdst = (float *) dst;
   babl_trc_from_linear_buf (trc_srgb, fsrc, fdst, 3, 3, 3, samples);
}

static void
conv_rgbaF_nonlinear_rgbaF_linear (const Babl    *conversion,
                                   unsigned char *__restrict__ src,
                                   unsigned char *__restrict__ dst,
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
conv_yaF_nonlinear_yaF_linear (const Babl    *conversion,
                               unsigned char *__restrict__ src,
                               unsigned char *__restrict__ dst,
                               long           samples)
{
   const Babl  *space = babl_conversion_get_destination_space (conversion);
   const Babl **trc   = (void*)space->space.trc;
   float *fsrc = (float *) src;
   float *fdst = (float *) dst;
   babl_trc_to_linear_buf (trc[0], fsrc, fdst, 2, 2, 1, samples);
}


static void
conv_rgbaF_perceptual_rgbaF_linear (const Babl    *conversion,
                                    unsigned char *__restrict__ src,
                                    unsigned char *__restrict__ dst,
                                    long           samples)
{
   float *fsrc = (float *) src;
   float *fdst = (float *) dst;
   babl_trc_to_linear_buf (trc_srgb, fsrc, fdst, 4, 4, 3, samples);
}


static void
conv_rgbF_nonlinear_rgbF_linear (const Babl    *conversion,
                                 unsigned char *__restrict__ src,
                                 unsigned char *__restrict__ dst,
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
conv_yF_nonlinear_yF_linear (const Babl    *conversion,
                                 unsigned char *__restrict__ src,
                                 unsigned char *__restrict__ dst,
                                 long           samples)
{
   const Babl  *space = babl_conversion_get_destination_space (conversion);
   const Babl **trc   = (void*)space->space.trc;
   float *fsrc = (float *) src;
   float *fdst = (float *) dst;

   babl_trc_to_linear_buf (trc[0], fsrc, fdst, 1, 1, 1, samples);
}

static void
conv_rgbF_perceptual_rgbF_linear (const Babl    *conversion,
                                  unsigned char *__restrict__ src,
                                  unsigned char *__restrict__ dst,
                                  long           samples)
{
   float *fsrc = (float *) src;
   float *fdst = (float *) dst;
   babl_trc_to_linear_buf (trc_srgb, fsrc, fdst, 3, 3, 3, samples);
}


#define o(src, dst) \
  babl_conversion_new (src, dst, "linear", conv_ ## src ## _ ## dst, NULL)

int init (void);
#include "babl-verify-cpu.inc"

int
init (void)
{
  BABL_VERIFY_CPU();
  {
  const Babl *yaF_linear = babl_format_new (
    babl_model ("YA"),
    babl_type ("float"),
    babl_component ("Y"),
    babl_component ("A"),
    NULL);
  const Babl *yAF_linear = babl_format_new (
    babl_model ("YaA"),
    babl_type ("float"),
    babl_component ("Ya"),
    babl_component ("A"),
    NULL);
  const Babl *yaF_nonlinear = babl_format_new (
    babl_model ("Y'A"),
    babl_type ("float"),
    babl_component ("Y'"),
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
  const Babl *yAF_nonlinear = babl_format_new (
    babl_model ("Y'aA"),
    babl_type ("float"),
    babl_component ("Y'a"),
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
  const Babl *yF_linear = babl_format_new (
    babl_model ("Y"),
    babl_type ("float"),
    babl_component ("Y"),
    NULL);
  const Babl *yF_nonlinear = babl_format_new (
    babl_model ("Y'"),
    babl_type ("float"),
    babl_component ("Y'"),
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


  o (yaF_linear, yAF_linear);
  o (yAF_linear, yaF_linear);
  o (yAF_linear, yAF_nonlinear);
  o (yaF_linear, yAF_nonlinear);
  o (yaF_linear, yaF_nonlinear);
  o (yaF_nonlinear,  yaF_linear);
  o (yF_linear, yF_nonlinear);
  o (yF_nonlinear,  yF_linear);

  o (rgbAF_linear, rgbAF_perceptual);
  o (rgbaF_linear, rgbAF_perceptual);
  o (rgbaF_linear, rgbaF_perceptual);
  o (rgbaF_perceptual,  rgbaF_linear);
  o (rgbF_linear, rgbF_perceptual);
  o (rgbF_perceptual,  rgbF_linear);
  }
  return 0;
}

