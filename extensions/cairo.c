/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2012 Øyvind Kolås.
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

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "babl.h"

#include "base/util.h"

int init (void);

static inline long
conv_rgba8_cairo24_le (unsigned char *src, unsigned char *dst, long samples)
{
  long n = samples;
  while (n--)
    {
      dst[0] = src[2];
      dst[1] = src[1];
      dst[2] = src[0];
      dst[3] = 255;
      src+=4;
      dst+=4;
    }
  return samples;
}

static inline long
conv_rgb8_cairo24_le (unsigned char *src, unsigned char *dst, long samples)
{
  long n = samples;
  while (n--)
    {
      dst[0] = src[2];
      dst[1] = src[1];
      dst[2] = src[0];
      dst[3] = 255;
      src+=3;
      dst+=4;
    }
  return samples;
}

static inline long
conv_rgbA8_premul_cairo32_le (unsigned char *src, unsigned char *dst, long samples)
{
  long n = samples;
  while (n--)
    {
      dst[0] = src[2];
      dst[1] = src[1];
      dst[2] = src[0];
      dst[3] = src[3];
      src+=4;
      dst+=4;
    }
  return samples;
}

static inline long
conv_rgbA8_cairo32_le (unsigned char *src, unsigned char *dst, long samples)
{
  long n = samples;
  while (n--)
    {
#define div_255(a) ((((a)+128)+(((a)+128)>>8))>>8)
      dst[0] = div_255 (src[2] * src[3]);
      dst[1] = div_255 (src[1] * src[3]);
      dst[2] = div_255 (src[0] * src[3]);
#undef div_255
      dst[3] = src[3];
      src+=4;
      dst+=4;
    }
  return samples;
}

static long
conv_rgbafloat_cairo32_le (unsigned char *src,
                           unsigned char *dst,
                           long           samples)
{
  float *fsrc = (float *) src;
  unsigned char *cdst = (unsigned char *) dst;
  int n = samples;

  while (n--)
    {
      float alpha = fsrc[3];
      if (alpha >= 1.0)
      {
        int val = babl_linear_to_gamma_2_2f (fsrc[2]) * 0xff + 0.5f;
        *cdst++ = val > 0xff ? 0xff : val < 0 ? 0 : val;
        val = babl_linear_to_gamma_2_2f (fsrc[1]) * 0xff + 0.5f;
        *cdst++ = val > 0xff ? 0xff : val < 0 ? 0 : val;
        val = babl_linear_to_gamma_2_2f (fsrc[0]) * 0xff + 0.5f;
        *cdst++ = val > 0xff ? 0xff : val < 0 ? 0 : val;
        *cdst++ = 0xff;
        fsrc+=4;
      }
      else if (alpha <= 0.0)
      {
        (*(uint32_t*)cdst)=0;
        cdst+=4;
        fsrc+=4;
      }
      else
      {
        float balpha = alpha * 0xff;
        int val = babl_linear_to_gamma_2_2f (fsrc[2]) * balpha + 0.5f;
        *cdst++ = val > 0xff ? 0xff : val < 0 ? 0 : val;
        val = babl_linear_to_gamma_2_2f (fsrc[1]) * balpha + 0.5f;
        *cdst++ = val > 0xff ? 0xff : val < 0 ? 0 : val;
        val = babl_linear_to_gamma_2_2f (fsrc[0]) * balpha + 0.5f;
        *cdst++ = val > 0xff ? 0xff : val < 0 ? 0 : val;
        *cdst++ = balpha + 0.5f;
        fsrc+=4;
      }
    }
  return samples;
}

int
init (void)
{
  int   testint  = 23;
  char *testchar = (char*) &testint;
  int   littleendian = (testchar[0] == 23);

  if (littleendian)
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

      const Babl *f24 = babl_format_new (
        "name", "cairo-RGB24",
        babl_model ("R'G'B'"),
        babl_type ("u8"),
        babl_component ("B'"),
        babl_component ("G'"),
        babl_component ("R'"),
        babl_component ("PAD"),
        NULL
      );

      babl_conversion_new (babl_format ("R'aG'aB'aA u8"), f32, "linear", 
                           conv_rgbA8_premul_cairo32_le, NULL);
      babl_conversion_new (babl_format ("R'G'B'A u8"), f32, "linear",
                           conv_rgbA8_cairo32_le, NULL);

      babl_conversion_new (babl_format ("RGBA float"), f32, "linear",
                           conv_rgbafloat_cairo32_le, NULL);

      babl_conversion_new (babl_format ("R'G'B'A u8"), f24, "linear", 
                           conv_rgba8_cairo24_le, NULL);
      babl_conversion_new (babl_format ("R'G'B' u8"), f24, "linear", 
                           conv_rgb8_cairo24_le, NULL);
    }
  else
    {
      babl_format_new (
        "name", "cairo-ARGB32",
        babl_model ("R'aG'aB'aA"),
        babl_type ("u8"),
        babl_component ("A"),
        babl_component ("R'a"),
        babl_component ("G'a"),
        babl_component ("B'a"),
        NULL
      );
      babl_format_new (
        "name", "cairo-RGB24",
        babl_model ("R'G'B'"),
        babl_type ("u8"),
        babl_component ("PAD"),
        babl_component ("R'"),
        babl_component ("G'"),
        babl_component ("B'"),
        NULL
      );
    }
  babl_format_new (
    "name", "cairo-A8",
    babl_model ("YA"),
    babl_type ("u8"),
    babl_component ("A"),
    NULL
    );
  
  return 0;
}
