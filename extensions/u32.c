/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2018, Øyvind Kolås.
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

static inline void
conv_u32_u16 (const Babl    *conversion,
              unsigned char *src,
              unsigned char *dst,
              long           samples)


{
  uint32_t *src32 = (uint32_t*) src;
  uint16_t *dst16 = (uint16_t*) dst;
  long n = samples;

  while (n--)
    {
      *dst16++ = (*src32++)>>16;
    }
}

static inline void
conv_u16_u32 (const Babl    *conversion,
              unsigned char *src,
              unsigned char *dst,
              long           samples)


{
  uint16_t *src16 = (uint16_t*) src;
  uint32_t *dst32 = (uint32_t*) dst;
  long n = samples;

  while (n--)
    {
      *dst32++ = (*src16++) * 65536.99f;
    }
}


static void
conv_yau32_yau16 (const Babl      *conversion,
                    unsigned char *src,
                    unsigned char *dst,
                    long           samples)


{
  conv_u32_u16 (conversion, src, dst, samples * 2);
}

static void
conv_rgbu32_rgbu16 (const Babl    *conversion,
                    unsigned char *src,
                    unsigned char *dst,
                    long           samples)


{
  conv_u32_u16 (conversion, src, dst, samples * 3);
}

static void
conv_rgbu16_rgbu32 (const Babl    *conversion,
                    unsigned char *src,
                    unsigned char *dst,
                    long           samples)


{
  conv_u16_u32 (conversion, src, dst, samples * 3);
}

static void
conv_yau16_yau32 (const Babl      *conversion,
                    unsigned char *src,
                    unsigned char *dst,
                    long           samples)


{
  conv_u16_u32 (conversion, src, dst, samples * 2);
}


static void
conv_rgbau32_rgbau16 (const Babl    *conversion,
                      unsigned char *src,
                      unsigned char *dst,
                      long           samples)


{
  conv_u32_u16 (conversion, src, dst, samples * 4);
}

static void
conv_rgbau16_rgbau32 (const Babl    *conversion,
                      unsigned char *src,
                      unsigned char *dst,
                      long           samples)


{
  conv_u16_u32 (conversion, src, dst, samples * 4);
}


static inline void
conv_rgba32_rgb32 (const Babl    *conversion,
                   unsigned char *src,
                   unsigned char *dst,
                   long           samples)
{
  uint32_t *src32 = (uint32_t*) src;
  uint32_t *dst32 = (uint32_t*) dst;
  long n = samples;

  while (n--)
    {
      *dst32++ = (*src32++);
      *dst32++ = (*src32++);
      *dst32++ = (*src32++);
      src32++;
    }
}

static inline void
conv_rgb32_rgba32 (const Babl    *conversion,
                   unsigned char *src,
                   unsigned char *dst,
                   long           samples)
{
  uint32_t *src32 = (uint32_t*) src;
  uint32_t *dst32 = (uint32_t*) dst;
  long n = samples;

  while (n--)
    {
      *dst32++ = (*src32++);
      *dst32++ = (*src32++);
      *dst32++ = (*src32++);
      *dst32++ = 4294967295;
    }
}


static inline void
conv_yu32_yau32 (const Babl    *conversion,
                 unsigned char *src,
                 unsigned char *dst,
                 long           samples)
{
  uint32_t *src32 = (uint32_t*) src;
  uint32_t *dst32 = (uint32_t*) dst;
  long n = samples;

  while (n--)
    {
      *dst32++ = (*src32++);
      *dst32++ = 4294967295;
    }
}



int init (void);

int
init (void)
{
  babl_conversion_new (
    babl_format ("R'G'B'A u32"),
    babl_format ("R'G'B'A u16"),
    "linear",
    conv_rgbau32_rgbau16,
    NULL);
  babl_conversion_new (
    babl_format ("R'G'B' u32"),
    babl_format ("R'G'B' u16"),
    "linear",
    conv_rgbu32_rgbu16,
    NULL);
  babl_conversion_new (
    babl_format ("R~G~B~A u32"),
    babl_format ("R~G~B~A u16"),
    "linear",
    conv_rgbau32_rgbau16,
    NULL);
  babl_conversion_new (
    babl_format ("R~G~B~ u32"),
    babl_format ("R~G~B~ u16"),
    "linear",
    conv_rgbu32_rgbu16,
    NULL);
  babl_conversion_new (
    babl_format ("RGB u32"),
    babl_format ("RGB u16"),
    "linear",
    conv_rgbu32_rgbu16,
    NULL);
  babl_conversion_new (
    babl_format ("R'G'B' u16"),
    babl_format ("R'G'B' u32"),
    "linear",
    conv_rgbu16_rgbu32,
    NULL);
  babl_conversion_new (
    babl_format ("R~G~B~ u16"),
    babl_format ("R~G~B~ u32"),
    "linear",
    conv_rgbu16_rgbu32,
    NULL);
  babl_conversion_new (
    babl_format ("RGB u16"),
    babl_format ("RGB u32"),
    "linear",
    conv_rgbu16_rgbu32,
    NULL);
  babl_conversion_new (
    babl_format ("RGBA u32"),
    babl_format ("RGBA u16"),
    "linear",
    conv_rgbau32_rgbau16,
    NULL);
  babl_conversion_new (
    babl_format ("RGBA u16"),
    babl_format ("RGBA u32"),
    "linear",
    conv_rgbau16_rgbau32,
    NULL);

  babl_conversion_new (
    babl_format ("RaGaBaA u32"),
    babl_format ("RaGaBaA u16"),
    "linear",
    conv_rgbau32_rgbau16,
    NULL);
  babl_conversion_new (
    babl_format ("RaGaBaA u16"),
    babl_format ("RaGaBaA u32"),
    "linear",
    conv_rgbau16_rgbau32,
    NULL);
  babl_conversion_new (
    babl_format ("RGBA u32"),
    babl_format ("RGB u32"),
    "linear",
    conv_rgba32_rgb32,
    NULL);
  babl_conversion_new (
    babl_format ("RGB u32"),
    babl_format ("RGBA u32"),
    "linear",
    conv_rgb32_rgba32,
    NULL);
  babl_conversion_new (
    babl_format ("R'G'B'A u32"),
    babl_format ("R'G'B' u32"),
    "linear",
    conv_rgba32_rgb32,
    NULL);
  babl_conversion_new (
    babl_format ("R'G'B' u32"),
    babl_format ("R'G'B'A u32"),
    "linear",
    conv_rgb32_rgba32,
    NULL);
  babl_conversion_new (
    babl_format ("R~G~B~A u32"),
    babl_format ("R~G~B~ u32"),
    "linear",
    conv_rgba32_rgb32,
    NULL);
  babl_conversion_new (
    babl_format ("R~G~B~ u32"),
    babl_format ("R~G~B~A u32"),
    "linear",
    conv_rgb32_rgba32,
    NULL);
  babl_conversion_new (
    babl_format ("Y u32"),
    babl_format ("Y u16"),
    "linear",
    conv_u32_u16,
    NULL);
  babl_conversion_new (
    babl_format ("Y' u32"),
    babl_format ("Y' u16"),
    "linear",
    conv_u32_u16,
    NULL);
  babl_conversion_new (
    babl_format ("Y~ u32"),
    babl_format ("Y~ u16"),
    "linear",
    conv_u32_u16,
    NULL);
  babl_conversion_new (
    babl_format ("Y u16"),
    babl_format ("Y u32"),
    "linear",
    conv_u16_u32,
    NULL);
  babl_conversion_new (
    babl_format ("Y' u16"),
    babl_format ("Y' u32"),
    "linear",
    conv_u16_u32,
    NULL);
  babl_conversion_new (
    babl_format ("Y~ u16"),
    babl_format ("Y~ u32"),
    "linear",
    conv_u16_u32,
    NULL);

  babl_conversion_new (
    babl_format ("YA u32"),
    babl_format ("YA u16"),
    "linear",
    conv_yau32_yau16,
    NULL);
  babl_conversion_new (
    babl_format ("YaA u32"),
    babl_format ("YaA u16"),
    "linear",
    conv_yau32_yau16,
    NULL);
  babl_conversion_new (
    babl_format ("Y'A u32"),
    babl_format ("Y'A u16"),
    "linear",
    conv_yau32_yau16,
    NULL);

  babl_conversion_new (
    babl_format ("Y~A u32"),
    babl_format ("Y~A u16"),
    "linear",
    conv_yau32_yau16,
    NULL);
  babl_conversion_new (
    babl_format ("Y'aA u32"),
    babl_format ("Y'aA u16"),
    "linear",
    conv_yau32_yau16,
    NULL);
  babl_conversion_new (
    babl_format ("YA u16"),
    babl_format ("YA u32"),
    "linear",
    conv_yau16_yau32,
    NULL);
  babl_conversion_new (
    babl_format ("YaA u16"),
    babl_format ("YaA u32"),
    "linear",
    conv_yau16_yau32,
    NULL);
  babl_conversion_new (
    babl_format ("Y'A u16"),
    babl_format ("Y'A u32"),
    "linear",
    conv_yau16_yau32,
    NULL);

  babl_conversion_new (
    babl_format ("Y~A u16"),
    babl_format ("Y~A u32"),
    "linear",
    conv_yau16_yau32,
    NULL);
  babl_conversion_new (
    babl_format ("Y'aA u16"),
    babl_format ("Y'aA u32"),
    "linear",
    conv_yau16_yau32,
    NULL);
  babl_conversion_new (
    babl_format ("Y u32"),
    babl_format ("YA u32"),
    "linear",
    conv_yu32_yau32,
    NULL);
  babl_conversion_new (
    babl_format ("Y u32"),
    babl_format ("YaA u32"),
    "linear",
    conv_yu32_yau32,
    NULL);
  babl_conversion_new (
    babl_format ("Y' u32"),
    babl_format ("Y'A u32"),
    "linear",
    conv_yu32_yau32,
    NULL);
  babl_conversion_new (
    babl_format ("Y~ u32"),
    babl_format ("Y~A u32"),
    "linear",
    conv_yu32_yau32,
    NULL);
  babl_conversion_new (
    babl_format ("Y' u32"),
    babl_format ("Y'aA u32"),
    "linear",
    conv_yu32_yau32,
    NULL);

  return 0;
}
