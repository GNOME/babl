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


static void
conv_rgbaD_linear_rgbAD_gamma (const Babl    *conversion,
                               unsigned char *src,
                               unsigned char *dst,
                               long           samples)
{
   const Babl  *space = babl_conversion_get_destination_space (conversion);
   const Babl **trc   = (void*)space->space.trc;

   double *fsrc = (double *) src;
   double *fdst = (double *) dst;
   int n = samples;

   while (n--)
     {
       double alpha = fsrc[3];
       if (alpha <= BABL_ALPHA_FLOOR)
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
conv_rgbAD_linear_rgbAD_gamma (const Babl    *conversion,
                               unsigned char *src,
                               unsigned char *dst,
                               long           samples)
{
   const Babl  *space = babl_conversion_get_destination_space (conversion);
   const Babl **trc   = (void*)space->space.trc;

   double *fsrc = (double *) src;
   double *fdst = (double *) dst;
   int n = samples;

   while (n--)
     {
       double alpha = fsrc[3];
       if (alpha == 0.0)
         {
           *fdst++ = 0.0;
           *fdst++ = 0.0;
           *fdst++ = 0.0;
           *fdst++ = 0.0;
           fsrc+=4;
         }
       else
         {
           double alpha_recip = 1.0 / alpha;
           *fdst++ = babl_trc_from_linear (trc[0], *fsrc++ * alpha_recip) * alpha;
           *fdst++ = babl_trc_from_linear (trc[1], *fsrc++ * alpha_recip) * alpha;
           *fdst++ = babl_trc_from_linear (trc[2], *fsrc++ * alpha_recip) * alpha;
           *fdst++ = *fsrc++;
         }
     }
}

static void
conv_rgbaD_linear_rgbaD_gamma (const Babl    *conversion,
                               unsigned char *src, 
                               unsigned char *dst, 
                               long           samples)
{
   const Babl  *space = babl_conversion_get_destination_space (conversion);
   const Babl **trc   = (void*)space->space.trc;

   double *fsrc = (double *) src;
   double *fdst = (double *) dst;
   int n = samples;

   while (n--)
     {
       *fdst++ = babl_trc_from_linear (trc[0], *fsrc++);
       *fdst++ = babl_trc_from_linear (trc[1], *fsrc++);
       *fdst++ = babl_trc_from_linear (trc[2], *fsrc++);
       *fdst++ = *fsrc++;
     }
}

#define conv_rgbaD_linear_rgbD_linear conv_rgbaD_gamma_rgbD_gamma

static void
conv_rgbaD_linear_rgbD_linear (const Babl    *conversion,
                               unsigned char *src,
                               unsigned char *dst,
                               long           samples)
{
   double *fsrc = (double *) src;
   double *fdst = (double *) dst;
   int n = samples;

   while (n--)
     {
       *fdst++ = *fsrc++;
       *fdst++ = *fsrc++;
       *fdst++ = *fsrc++;
       fsrc++;
     }
}


static void
conv_rgbD_linear_rgbD_gamma (const Babl    *conversion,
                             unsigned char *src, 
                             unsigned char *dst, 
                             long           samples)
{
   const Babl  *space = babl_conversion_get_destination_space (conversion);
   const Babl **trc   = (void*)space->space.trc;
   double *fsrc = (double *) src;
   double *fdst = (double *) dst;
   int n = samples;

   while (n--)
     {
       *fdst++ = babl_trc_from_linear (trc[0], *fsrc++);
       *fdst++ = babl_trc_from_linear (trc[1], *fsrc++);
       *fdst++ = babl_trc_from_linear (trc[2], *fsrc++);
     }
}


static void
conv_rgbaD_gamma_rgbaD_linear (const Babl    *conversion,
                               unsigned char *src, 
                               unsigned char *dst, 
                               long           samples)
{
   const Babl  *space = babl_conversion_get_destination_space (conversion);
   const Babl **trc   = (void*)space->space.trc;
   double *fsrc = (double *) src;
   double *fdst = (double *) dst;
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
conv_rgbD_gamma_rgbD_linear (const Babl    *conversion,
                             unsigned char *src, 
                             unsigned char *dst, 
                             long           samples)
{
   const Babl  *space = babl_conversion_get_destination_space (conversion);
   const Babl **trc   = (void*)space->space.trc;
   double *fsrc = (double *) src;
   double *fdst = (double *) dst;
   int n = samples;

   while (n--)
     {
       *fdst++ = babl_trc_to_linear (trc[0], *fsrc++);
       *fdst++ = babl_trc_to_linear (trc[1], *fsrc++);
       *fdst++ = babl_trc_to_linear (trc[2], *fsrc++);
     }
}


static void
conv_rgbD_linear_rgbaD_linear (const Babl    *conversion,
                               unsigned char *src, 
                               unsigned char *dst, 
                               long           samples)
{
   const Babl  *space = babl_conversion_get_destination_space (conversion);
   const Babl **trc   = (void*)space->space.trc;
   double *fsrc = (double *) src;
   double *fdst = (double *) dst;
   int n = samples;

   while (n--)
     {
       *fdst++ = babl_trc_to_linear (trc[0], *fsrc++);
       *fdst++ = babl_trc_to_linear (trc[1], *fsrc++);
       *fdst++ = babl_trc_to_linear (trc[2], *fsrc++);
       *fdst++ = 1.0;
     }
}


#define conv_rgbD_gamma_rgbaD_gamma conv_rgbD_linear_rgbaD_linear

#define o(src, dst) \
  babl_conversion_new (src, dst, "linear", conv_ ## src ## _ ## dst, NULL)

int init (void);

int
init (void)
{
  const Babl *rgbaD_linear = babl_format_new (
    babl_model ("RGBA"),
    babl_type ("double"),
    babl_component ("R"),
    babl_component ("G"),
    babl_component ("B"),
    babl_component ("A"),
    NULL);
  const Babl *rgbAD_linear = babl_format_new (
    babl_model ("RaGaBaA"),
    babl_type ("double"),
    babl_component ("Ra"),
    babl_component ("Ga"),
    babl_component ("Ba"),
    babl_component ("A"),
    NULL);
  const Babl *rgbaD_gamma = babl_format_new (
    babl_model ("R'G'B'A"),
    babl_type ("double"),
    babl_component ("R'"),
    babl_component ("G'"),
    babl_component ("B'"),
    babl_component ("A"),
    NULL);
  const Babl *rgbAD_gamma = babl_format_new (
    babl_model ("R'aG'aB'aA"),
    babl_type ("double"),
    babl_component ("R'a"),
    babl_component ("G'a"),
    babl_component ("B'a"),
    babl_component ("A"),
    NULL);
  const Babl *rgbD_linear = babl_format_new (
    babl_model ("RGB"),
    babl_type ("double"),
    babl_component ("R"),
    babl_component ("G"),
    babl_component ("B"),
    NULL);
  const Babl *rgbD_gamma = babl_format_new (
    babl_model ("R'G'B'"),
    babl_type ("double"),
    babl_component ("R'"),
    babl_component ("G'"),
    babl_component ("B'"),
    NULL);

  o (rgbAD_linear, rgbAD_gamma);
  o (rgbaD_linear, rgbAD_gamma);
  o (rgbaD_linear, rgbaD_gamma);
  o (rgbaD_gamma,  rgbaD_linear);
  o (rgbD_linear, rgbD_gamma);
  o (rgbD_gamma,  rgbD_linear);
  o (rgbaD_linear, rgbD_linear);
  o (rgbaD_gamma,  rgbD_gamma);


  o (rgbD_linear, rgbaD_linear);
  o (rgbD_gamma, rgbaD_gamma);
  o (rgbaD_linear, rgbD_linear);
  o (rgbaD_gamma, rgbD_gamma);

  return 0;
}

