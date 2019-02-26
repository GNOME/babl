/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2016, Øyvind Kolås.
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
#include <stdint.h>

#include "babl.h"

#include "base/util.h"
#include "extensions/util.h"

static void
conv_rgbu16_rgbau16 (const Babl    *conversion,
                     unsigned char *src, 
                     unsigned char *dst, 
                     long           samples)


{
  uint16_t *src16 = (uint16_t*) src;
  uint16_t *dst16 = (uint16_t*) dst;
  long n = samples;

  while (n--)
    {
      *dst16++ = *src16++;
      *dst16++ = *src16++;
      *dst16++ = *src16++;
      *dst16++ = 0xffff;
    }
}

static void
conv_yu16_yau16 (const Babl    *conversion,
                 unsigned char *src, 
                 unsigned char *dst, 
                 long           samples)


{
  uint16_t *src16 = (uint16_t*) src;
  uint16_t *dst16 = (uint16_t*) dst;
  long n = samples;

  while (n--)
    {
      *dst16++ = *src16++;
      *dst16++ = 0xffff;
    }
}

int init (void);

int
init (void)
{
  babl_conversion_new (
    babl_format ("R'G'B' u16"),
    babl_format ("R'G'B'A u16"),
    "linear",
    conv_rgbu16_rgbau16,
    NULL);

  babl_conversion_new (
    babl_format ("R~G~B~ u16"),
    babl_format ("R~G~B~A u16"),
    "linear",
    conv_rgbu16_rgbau16,
    NULL);

  babl_conversion_new (
    babl_format ("Y' u16"),
    babl_format ("Y'A u16"),
    "linear",
    conv_yu16_yau16,
    NULL);

  babl_conversion_new (
    babl_format ("Y~ u16"),
    babl_format ("Y~A u16"),
    "linear",
    conv_yu16_yau16,
    NULL);

  babl_conversion_new (
    babl_format ("RGB u16"),
    babl_format ("RGBA u16"),
    "linear",
    conv_rgbu16_rgbau16,
    NULL);

  babl_conversion_new (
    babl_format ("Y u16"),
    babl_format ("YA u16"),
    "linear",
    conv_yu16_yau16,
    NULL);
  return 0;
}
