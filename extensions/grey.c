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
 * <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include <stdio.h>

#include "babl.h"

#include "base/util.h"
#include "base/rgb-constants.h"
#include "extensions/util.h"

/* There was some debate on #gimp about whether these constants
 * are accurate, for now I've elected to just follow whatever
 * babl/base does.
 *   - Daniel
 */

/* Float versions of the double constants in rgb-constants.h */
static const float RGB_LUMINANCE_RED_FLOAT = RGB_LUMINANCE_RED;
static const float RGB_LUMINANCE_GREEN_FLOAT = RGB_LUMINANCE_GREEN;
static const float RGB_LUMINANCE_BLUE_FLOAT = RGB_LUMINANCE_BLUE;

static long
conv_rgbaF_linear_y8_linear (unsigned char *src,
                             unsigned char *dst,
                             long           samples)
{
  static const float RGB_LUMINANCE_RED_FLOAT = RGB_LUMINANCE_RED;
  static const float RGB_LUMINANCE_GREEN_FLOAT = RGB_LUMINANCE_GREEN;
  static const float RGB_LUMINANCE_BLUE_FLOAT = RGB_LUMINANCE_BLUE;

  float *s = (float *) src;
  long   n = samples;

  while (n--)
    {
      float value;
      long int v;
      value  = *s++ * RGB_LUMINANCE_RED_FLOAT;
      value += *s++ * RGB_LUMINANCE_GREEN_FLOAT;
      value += *s++ * RGB_LUMINANCE_BLUE_FLOAT;
      s++;

      v = rint (value * 255.0);
      *dst++ = (v < 0) ? 0 : ((v > 255) ? 255 : v);
    }

  return samples;
}

static long
conv_rgbaF_linear_yF_linear (unsigned char *src,
                             unsigned char *dst,
                             long           samples)
{

  float *s = (float *) src;
  float *d = (float *) dst;
  long   n = samples;

  while (n--)
    {
      float value;
      value  = *s++ * RGB_LUMINANCE_RED_FLOAT;
      value += *s++ * RGB_LUMINANCE_GREEN_FLOAT;
      value += *s++ * RGB_LUMINANCE_BLUE_FLOAT;
      s++;
      *d++ = value;
    }

  return samples;
}

static long
conv_rgbaF_linear_yaF_linear (unsigned char *src,
                              unsigned char *dst,
                              long           samples)
{

  float *s = (float *) src;
  float *d = (float *) dst;
  long   n = samples;

  while (n--)
    {
      float value;
      value  = *s++ * RGB_LUMINANCE_RED_FLOAT;
      value += *s++ * RGB_LUMINANCE_GREEN_FLOAT;
      value += *s++ * RGB_LUMINANCE_BLUE_FLOAT;
      *d++ = value;
      *d++ = *s++;  /* alpha */
    }

  return samples;
}

int init (void);

int
init (void)
{
  babl_conversion_new (babl_format ("RGBA float"),
                       babl_format ("Y u8"),
                       "linear",
                       conv_rgbaF_linear_y8_linear,
                       NULL);

  babl_conversion_new (babl_format ("RGBA float"),
                       babl_format ("Y float"),
                       "linear",
                       conv_rgbaF_linear_yF_linear,
                       NULL);

  babl_conversion_new (babl_format ("RGBA float"),
                       babl_format ("YA float"),
                       "linear",
                       conv_rgbaF_linear_yaF_linear,
                       NULL);

  return 0;
}
