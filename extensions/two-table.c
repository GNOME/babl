/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2013, Daniel Sabo
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

#include <stdlib.h>
#include "babl.h"

#include "base/util.h"
#include "extensions/two-table-tables.h"

static inline unsigned char
conv_float_u8_two_table_map (float value)
{
  unsigned short index;
  unsigned char result;
  if (value < 0.0f)
    return 0;
  else if (value > 1.0f)
    return 0xFF;
  index = (unsigned short)(value * 0xFFFF);
  result = linear_to_gamma[index];

  if (value < u8_gamma_minimums[result])
    result -= 1;
  else if (value >= u8_gamma_minimums[result+1])
    result += 1;

  return result;
}

static void
conv_rgbafloat_linear_cairo24_le (const Babl    *conversion,
                                  unsigned char *src_char,
                                  unsigned char *dst,
                                  long           samples)
{
  long   n    = samples;
  float *src  = (float*)src_char;

  while (n--)
    {
      dst[0] = conv_float_u8_two_table_map (src[2]);
      dst[1] = conv_float_u8_two_table_map (src[1]);
      dst[2] = conv_float_u8_two_table_map (src[0]);
      src += 4;
      dst += 4;
    }
}

static void
conv_rgbfloat_linear_cairo24_le (const Babl    *conversion,
                                 unsigned char *src_char,
                                 unsigned char *dst,
                                 long           samples)
{
  long   n   = samples;
  float *src = (float*)src_char;

  while (n--)
    {
      dst[0] = conv_float_u8_two_table_map (src[2]);
      dst[1] = conv_float_u8_two_table_map (src[1]);
      dst[2] = conv_float_u8_two_table_map (src[0]);

      src += 3;
      dst += 4;
    }
}

static void
conv_rgbafloat_linear_rgbu8_gamma (const Babl    *conversion,
                                   unsigned char *src_char,
                                   unsigned char *dst,
                                   long           samples)
{
  long   n    = samples;
  float *src  = (float*)src_char;

  while (n--)
    {
      if (src[3] <= BABL_ALPHA_FLOOR)
        {
          dst[0] = 0;
          dst[1] = 0;
          dst[2] = 0;
        }
      else
        {
          dst[0] = conv_float_u8_two_table_map (src[0]);
          dst[1] = conv_float_u8_two_table_map (src[1]);
          dst[2] = conv_float_u8_two_table_map (src[2]);
        }
      src += 4;
      dst += 3;
    }
}


static void
conv_rgbafloat_linear_rgbau8_gamma (const Babl    *conversion,
                                    unsigned char *src_char,
                                    unsigned char *dst,
                                    long           samples)
{
  long   n    = samples;
  float *src  = (float*)src_char;

  while (n--)
    {
      dst[0] = conv_float_u8_two_table_map (src[0]);
      dst[1] = conv_float_u8_two_table_map (src[1]);
      dst[2] = conv_float_u8_two_table_map (src[2]);
      dst[3] = src[3] * 0xff + 0.5;
      src += 4;
      dst += 4;
    }
}

static void
conv_rgbfloat_linear_rgbu8_gamma (const Babl    *conversion,
                                  unsigned char *src_char,
                                  unsigned char *dst,
                                  long           samples)
{
  long   n   = samples;
  float *src = (float*)src_char;

  while (n--)
    {
      dst[0] = conv_float_u8_two_table_map (src[0]);
      dst[1] = conv_float_u8_two_table_map (src[1]);
      dst[2] = conv_float_u8_two_table_map (src[2]);

      src += 3;
      dst += 3;
    }
}

static void
conv_yfloat_linear_yu8_gamma (const Babl    *conversion,
                              unsigned char *src_char,
                              unsigned char *dst,
                              long           samples)
{
  long   n   = samples;
  float *src = (float*)src_char;

  while (n--)
    {
      *dst++ = conv_float_u8_two_table_map (*src++);
    }
}

static void
conv_yafloat_linear_yau8_gamma (const Babl    *conversion,
                                unsigned char *src_char,
                                unsigned char *dst,
                                long           samples)
{
  long   n   = samples;
  float *src = (float*)src_char;

  while (n--)
    {
      long int alpha;
      *dst++ = conv_float_u8_two_table_map (*src++);

      alpha  = rint (*src++ * 255.0);
      *dst++ = (alpha < 0) ? 0 : ((alpha > 255) ? 255 : alpha);
    }
}

int init (void);

int
init (void)
{
  int   testint  = 23;
  char *testchar = (char*) &testint;
  int   littleendian = (testchar[0] == 23);

  return 0; // temporarily disable, it is interfering with space invasion

  if (littleendian)
    {
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

      babl_conversion_new (babl_format ("RGB float"),
                           f24,
                           "linear",
                           conv_rgbfloat_linear_cairo24_le,
                           NULL);

      babl_conversion_new (babl_format ("RGBA float"),
                           f24,
                           "linear",
                           conv_rgbafloat_linear_cairo24_le,
                           NULL);
    }

  babl_conversion_new (babl_format ("RGBA float"),
                       babl_format ("R'G'B' u8"),
                       "linear",
                       conv_rgbafloat_linear_rgbu8_gamma,
                       NULL);

  babl_conversion_new (babl_format ("RGBA float"),
                       babl_format ("R'G'B'A u8"),
                       "linear",
                       conv_rgbafloat_linear_rgbau8_gamma,
                       NULL);

  babl_conversion_new (babl_format ("RGB float"),
                       babl_format ("R'G'B' u8"),
                       "linear",
                       conv_rgbfloat_linear_rgbu8_gamma,
                       NULL);

  babl_conversion_new (babl_format ("Y float"),
                       babl_format ("Y' u8"),
                       "linear",
                       conv_yfloat_linear_yu8_gamma,
                       NULL);

  babl_conversion_new (babl_format ("YA float"),
                       babl_format ("Y'A u8"),
                       "linear",
                       conv_yafloat_linear_yau8_gamma,
                       NULL);

  return 0;
}
