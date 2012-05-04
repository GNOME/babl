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
#include "babl.h"

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
      dst[3] = src[0];
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
      dst[3] = src[0];
      src+=3;
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
      dst[0] = src[2];
      dst[1] = src[1];
      dst[2] = src[0];
      dst[3] = src[3];
      src+=4;
      dst+=4;
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
                           conv_rgbA8_cairo32_le, NULL);
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
