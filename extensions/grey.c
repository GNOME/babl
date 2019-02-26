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

#include "config.h"
#include <stdio.h>

#include "babl-internal.h"

#include "base/util.h"
#include "extensions/util.h"

static void
conv_rgbaF_linear_y8_linear (const Babl    *conversion,
                             unsigned char *src,
                             unsigned char *dst,
                             long           samples)
{
  const Babl *space = babl_conversion_get_source_space (conversion);
  const double *rgbtoxyz = babl_space_get_rgbtoxyz (space);
  const float RGB_LUMINANCE_RED_FLOAT   = rgbtoxyz[3];
  const float RGB_LUMINANCE_GREEN_FLOAT = rgbtoxyz[4];
  const float RGB_LUMINANCE_BLUE_FLOAT  = rgbtoxyz[5];

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
}

static void
conv_rgbaF_linear_yF_linear (const Babl    *conversion,
                             unsigned char *src,
                             unsigned char *dst,
                             long           samples)
{
  const Babl *space = babl_conversion_get_source_space (conversion);
  const double *rgbtoxyz = babl_space_get_rgbtoxyz (space);
  const float RGB_LUMINANCE_RED_FLOAT = rgbtoxyz[3];
  const float RGB_LUMINANCE_GREEN_FLOAT = rgbtoxyz[4];
  const float RGB_LUMINANCE_BLUE_FLOAT = rgbtoxyz[5];

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
}

static void
conv_rgbaF_linear_yaF_linear (const Babl    *conversion,
                              unsigned char *src,
                              unsigned char *dst,
                              long           samples)
{
  const Babl *space = babl_conversion_get_source_space (conversion);
  const double *rgbtoxyz = babl_space_get_rgbtoxyz (space);
  const float RGB_LUMINANCE_RED_FLOAT = rgbtoxyz[3];
  const float RGB_LUMINANCE_GREEN_FLOAT = rgbtoxyz[4];
  const float RGB_LUMINANCE_BLUE_FLOAT = rgbtoxyz[5];

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
