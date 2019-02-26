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
 * <https://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "config.h"
#include "babl-internal.h"

#include "base/util.h"

int init (void);

static void
conv_rgba8_cairo24_le (const Babl    *conversion,
                       unsigned char *src, 
                       unsigned char *dst, 
                       long           samples)
{
  long n = samples;
  uint32_t *srci = (void *)src;
  uint32_t *dsti = (void *)dst;

  while (n--)
    {
      uint32_t orig = *srci++;
      uint32_t green_alpha = (orig & 0x0000ff00);
      uint32_t red_blue    = (orig & 0x00ff00ff);
      uint32_t red         = red_blue << 16;
      uint32_t blue        = red_blue >> 16;
      *dsti++              = green_alpha | red | blue | 0xff000000;
    }
}

static void
conv_rgb8_cairo24_le (const Babl    *conversion,
                      unsigned char *src, 
                      unsigned char *dst, 
                      long           samples)
{
  long n = samples;
  while (n--)
    {
      unsigned char red   = *src++;
      unsigned char green = *src++;
      unsigned char blue  = *src++;
      *dst++ = blue;
      *dst++ = green;
      *dst++ = red;
      *dst++ = 255;
    }


}

#if 0
static void
conv_rgbA8_premul_cairo32_le (const Babl    *conversion,
                              unsigned char *src, 
                              unsigned char *dst, 
                              long           samples)
{
  long n = samples;
  while (n--)
    {
      unsigned char red    = *src++;
      unsigned char green  = *src++;
      unsigned char blue   = *src++;
      unsigned char alpha  = *src++;

      *dst++ = blue;
      *dst++ = green;
      *dst++ = red;
      *dst++ = alpha;
    }
}
#else

static void
conv_rgbA8_premul_cairo32_le (const Babl    *conversion,
                              unsigned char *src, 
                              unsigned char *dst, 
                              long           samples)
{
  long n = samples;
  uint32_t *srci = (void *)src;
  uint32_t *dsti = (void *)dst;

  while (n--)
    {
      uint32_t orig = *srci++;
      uint32_t green_alpha = (orig & 0xff00ff00);
      uint32_t red_blue    = (orig & 0x00ff00ff);
      uint32_t red         = red_blue << 16;
      uint32_t blue        = red_blue >> 16;
      *dsti++              = green_alpha | red | blue;
    }
}
#endif

static void 
conv_cairo32_rgbA8_premul_le (const Babl    *conversion,
                              unsigned char *src, 
                              unsigned char *dst, 
                              long           samples)
{
  long n = samples;
  while (n--)
    {
      unsigned char blue   = *src++;
      unsigned char green  = *src++;
      unsigned char red    = *src++;
      unsigned char alpha  = *src++;

      *dst++ = red;
      *dst++ = green;
      *dst++ = blue;
      *dst++ = alpha;
    }
}

static void 
conv_cairo32_rgba8_le (const Babl    *conversion,
                       unsigned char *src, 
                       unsigned char *dst, 
                       long           samples)
{
  long n = samples;
  while (n--)
    {
      unsigned char blue   = *src++;
      unsigned char green  = *src++;
      unsigned char red    = *src++;
      unsigned char alpha  = *src++;

      if (alpha == 0)
      {
        *dst++ = 0;
        *dst++ = 0;
        *dst++ = 0;
        *dst++ = 0;
      }
      else if (alpha == 255)
      {
        *dst++ = red;
        *dst++ = green;
        *dst++ = blue;
        *dst++ = alpha;
      }
      else
      {
        float falpha = alpha / 255.0;
        float recip_alpha = 1.0 / falpha;
 //       unsigned int aa = ((255 << 16) + alpha) / falpha + 0.5;


        *dst++ = ((red/255.0) * recip_alpha) * 255 + 0.5f;
        *dst++ = ((green/255.0) * recip_alpha) * 255 + 0.5f;
        *dst++ = ((blue/255.0) * recip_alpha) * 255 + 0.5f;

//        *dst++ = (red   * aa + 0x8000) >> 16;
//        *dst++ = (green * aa + 0x8000) >> 16;
//        *dst++ = (blue  * aa + 0x8000) >> 16;
        *dst++ = alpha;
      }
    }
}


static void 
conv_cairo32_rgbAF_premul_le (const Babl    *conversion,
                              unsigned char *src, 
                              unsigned char *dst_char, 
                              long           samples)
{
  long n = samples;
  float *dst = (void*)dst_char;
  while (n--)
    {
      unsigned char blue   = *src++;
      unsigned char green  = *src++;
      unsigned char red    = *src++;
      unsigned char alpha  = *src++;

      *dst++ = red / 255.0;
      *dst++ = green / 255.0;
      *dst++ = blue / 255.0;
      *dst++ = alpha / 255.0;
    }
}

static void
conv_rgba8_cairo32_le (const Babl    *conversion,
                       unsigned char *src, 
                       unsigned char *dst, 
                       long           samples)
{
  long n = samples;
  uint32_t *dsti = (void*) dst;
  while (n--)
    {
      unsigned char alpha = src[3];
#if SIZE_MAX >= UINT64_MAX /* 64-bit */
      uint64_t rbag = ((uint64_t) src[0] << 48) |
                      ((uint64_t) src[2] << 32) |
                      ((uint64_t) 255    << 16) |
                      ((uint64_t) src[1] <<  0);
      rbag *= alpha;
      rbag += 0x0080008000800080;
      rbag += (rbag >> 8) & 0x00ff00ff00ff00ff;
      rbag &= 0xff00ff00ff00ff00;
      *dsti++ = (uint32_t) (rbag >>  0) |
                (uint32_t) (rbag >> 40);
#else /* 32-bit */
      uint32_t rb = ((uint32_t) src[0] << 16) |
                    ((uint32_t) src[2] <<  0);
      uint64_t ag = ((uint32_t) 255    << 16) |
                    ((uint32_t) src[1] <<  0);
      rb *= alpha;
      ag *= alpha;
      rb += 0x00800080;
      ag += 0x00800080;
      rb += (rb >> 8) & 0x00ff00ff;
      ag += (ag >> 8) & 0x00ff00ff;
      rb &= 0xff00ff00;
      ag &= 0xff00ff00;
      *dsti++ = (uint32_t) (ag >> 0) |
                (uint32_t) (rb >> 8);
#endif
      src+=4;
    }
}

static void
conv_rgb8_cairo32_le (const Babl    *conversion,
                      unsigned char *src, 
                      unsigned char *dst, 
                      long           samples)
{
  long n = samples;
  while (n--)
    {
      unsigned char red    = *src++;
      unsigned char green  = *src++;
      unsigned char blue   = *src++;

      *dst++ = blue;
      *dst++ = green;
      *dst++ = red;
      *dst++ = 0xff;
    }
}




static void
conv_yA8_cairo32_le (const Babl    *conversion,
                     unsigned char *src, 
                     unsigned char *dst, 
                     long           samples)
{
  long n = samples;
  while (n--)
    {
#define div_255(a) ((((a)+128)+(((a)+128)>>8))>>8)

      unsigned char gray   = *src++;
      unsigned char alpha  = *src++;
      unsigned char val = div_255 (gray * alpha);

#undef div_255

      *dst++ = val;
      *dst++ = val;
      *dst++ = val;
      *dst++ = alpha;
    }
}

static void
conv_yA16_cairo32_le (const Babl    *conversion,
                      unsigned char *src, 
                      unsigned char *dst, 
                      long           samples)
{
  long n = samples;
  uint16_t *ssrc = (void*) src;
  while (n--)
    {
      float alpha = (ssrc[1]) / 65535.0f;
      int val = (ssrc[0] * alpha) * (0xff / 65535.0f ) + 0.5f;
      *dst++ = val;
      *dst++ = val;
      *dst++ = val;
      *dst++ = (alpha * 0xff + 0.5f);
      ssrc+=2;
    }
}

static void
conv_y8_cairo32_le (const Babl    *conversion,
                    unsigned char *src, 
                    unsigned char *dst, 
                    long           samples)
{
  long n = samples;
  while (n--)
    {
      unsigned char val = *src++;
      *dst++ = val;
      *dst++ = val;
      *dst++ = val;
      *dst++ = 0xff;
    }
}

static void
conv_y16_cairo32_le (const Babl    *conversion,
                     unsigned char *src, 
                     unsigned char *dst, 
                     long           samples)
{
  long n = samples;
  uint16_t *s16 = (void*)src;
  while (n--)
    {
#define div_257(a) ((((a)+128)-(((a)+128)>>8))>>8)
      uint16_t v16 = *s16++;
      unsigned char val = div_257(v16);
#undef dib_257
      *dst++ = val;
      *dst++ = val;
      *dst++ = val;
      *dst++ = 0xff;
    }
}

static void
conv_rgbA_gamma_float_cairo32_le (const Babl    *conversion,
                                  unsigned char *src,
                                  unsigned char *dst,
                                  long           samples)
{
  float *fsrc = (float *) src;
  unsigned char *cdst = (unsigned char *) dst;
  int n = samples;

  while (n--)
    {
      int val = fsrc[2] * 255.0f  + 0.5f;
      *cdst++ = val >= 0xff ? 0xff : val <= 0 ? 0 : val;
      val = fsrc[1] * 255.0f + 0.5f;
      *cdst++ = val >= 0xff ? 0xff : val <= 0 ? 0 : val;
      val = fsrc[0] * 255.0f + 0.5f;
      *cdst++ = val >= 0xff ? 0xff : val <= 0 ? 0 : val;
      val = fsrc[3] * 255.0f + 0.5f;
      *cdst++ = val >= 0xff ? 0xff : val <= 0 ? 0 : val;
      fsrc+=4;
    }
}

static void
conv_rgbafloat_cairo32_le (const Babl    *conversion,
                           unsigned char *src,
                           unsigned char *dst,
                           long           samples)
{
  const Babl  *space = babl_conversion_get_destination_space (conversion);
  const Babl **trc   = (void*)space->space.trc;

  float *fsrc = (float *) src;
  unsigned char *cdst = (unsigned char *) dst;
  int n = samples;

  while (n--)
    {
      float red    = *fsrc++;
      float green  = *fsrc++;
      float blue   = *fsrc++;
      float alpha  = *fsrc++;
      if (alpha >= 1.0)
      {
        int val = babl_trc_from_linear (trc[2], blue) * 0xff + 0.5f;
        *cdst++ = val >= 0xff ? 0xff : val <= 0 ? 0 : val;
        val = babl_trc_from_linear (trc[1], green) * 0xff + 0.5f;
        *cdst++ = val >= 0xff ? 0xff : val <= 0 ? 0 : val;
        val = babl_trc_from_linear (trc[0], red) * 0xff + 0.5f;
        *cdst++ = val >= 0xff ? 0xff : val <= 0 ? 0 : val;
        *cdst++ = 0xff;
      }
      else if (alpha <= 0.0)
      {
        (*(uint32_t*)cdst)=0;
        cdst+=4;
      }
      else
      {
        float balpha = alpha * 0xff;
        int val = babl_trc_from_linear (trc[2], blue) * balpha + 0.5f;
        *cdst++ = val >= 0xff ? 0xff : val <= 0 ? 0 : val;
        val = babl_trc_from_linear (trc[1], green) * balpha + 0.5f;
        *cdst++ = val >= 0xff ? 0xff : val <= 0 ? 0 : val;
        val = babl_trc_from_linear (trc[0], red) * balpha + 0.5f;
        *cdst++ = val >= 0xff ? 0xff : val <= 0 ? 0 : val;
        *cdst++ = balpha + 0.5f;
      }
    }
}


static void
conv_yafloat_cairo32_le (const Babl    *conversion,
                         unsigned char *src,
                         unsigned char *dst,
                         long           samples)
{
  const Babl  *space = babl_conversion_get_destination_space (conversion);
  const Babl **trc   = (void*)space->space.trc;
  float *fsrc = (float *) src;
  unsigned char *cdst = (unsigned char *) dst;
  int n = samples;

  while (n--)
    {
      float gray   = *fsrc++;
      float alpha  = *fsrc++;
      if (alpha >= 1.0)
      {
        int val = babl_trc_from_linear (trc[0], gray) * 0xff + 0.5f;
        val = val >= 0xff ? 0xff : val <= 0 ? 0 : val;
        *cdst++ = val;
        *cdst++ = val;
        *cdst++ = val;
        *cdst++ = 0xff;
      }
      else if (alpha <= 0.0)
      {
        (*(uint32_t*)cdst)=0;
        cdst+=4;
      }
      else
      {
        float balpha = alpha * 0xff;
        int val = babl_trc_from_linear (trc[0], gray) * balpha + 0.5f;
        val = val >= 0xff ? 0xff : val <= 0 ? 0 : val;
        *cdst++ = val;
        *cdst++ = val;
        *cdst++ = val;
        *cdst++ = balpha + 0.5f;
      }
    }
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

      babl_conversion_new (f32, babl_format ("R'aG'aB'aA float"), "linear",
                           conv_cairo32_rgbAF_premul_le, NULL);

      babl_conversion_new (f32, babl_format ("R'aG'aB'aA u8"), "linear",
                           conv_cairo32_rgbA8_premul_le, NULL);

      babl_conversion_new (f32, babl_format ("R'G'B'A u8"), "linear",
                           conv_cairo32_rgba8_le, NULL);

      babl_conversion_new (babl_format ("R'aG'aB'aA u8"), f32, "linear",
                           conv_rgbA8_premul_cairo32_le, NULL);

      babl_conversion_new (babl_format ("R'G'B'A u8"), f32, "linear",
                           conv_rgba8_cairo32_le, NULL);


      babl_conversion_new (babl_format ("R'G'B' u8"), f32, "linear",
                           conv_rgb8_cairo32_le, NULL);

      babl_conversion_new (babl_format ("Y'A u8"), f32, "linear",
                           conv_yA8_cairo32_le, NULL);
      babl_conversion_new (babl_format ("Y'A u16"), f32, "linear",
                           conv_yA16_cairo32_le, NULL);

      babl_conversion_new (babl_format ("Y' u8"), f32, "linear",
                           conv_y8_cairo32_le, NULL);
      babl_conversion_new (babl_format ("Y' u16"), f32, "linear",
                           conv_y16_cairo32_le, NULL);

      babl_conversion_new (babl_format ("RGBA float"), f32, "linear",
                           conv_rgbafloat_cairo32_le, NULL);
      babl_conversion_new (babl_format ("YA float"), f32, "linear",
                           conv_yafloat_cairo32_le, NULL);

      babl_conversion_new (babl_format ("R'aG'aB'aA float"), f32, "linear",
                           conv_rgbA_gamma_float_cairo32_le, NULL);

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

      /* formats are registered - but no fast paths, this will be slow */
    }
  babl_format_new (
    "name", "cairo-A8",
    babl_model ("YA"),
    babl_type ("u8"),
    babl_component ("A"),
    NULL
    );


  /* formats that distribute different subset of the additive mixing variants
   * of CMYK producing two syntetic RGB formats we run in parallel to derive
   * a 4 instead of 3 component result, the same method could be used to
   * extend processing/drawing with cairo to spectral data.
   */
  if (littleendian)
  {
    babl_format_new ("name", "cairo-ACMK32",
                     babl_model ("camayakaA"),
                     babl_type ("u8"),
                     babl_component ("ka"),
                     babl_component ("ma"),
                     babl_component ("ca"),
                     babl_component ("A"),
                     NULL);
    babl_format_new ("name", "cairo-ACYK32",
                     babl_model ("camayakaA"),
                     babl_type ("u8"),
                     babl_component ("ka"),
                     babl_component ("ya"),
                     babl_component ("ca"),
                     babl_component ("A"),
                     NULL);
  }
  else
  {
    babl_format_new ("name", "cairo-ACMK32",
                     babl_model ("camayakaA"),
                     babl_type ("u8"),
                     babl_component ("A"),
                     babl_component ("ca"),
                     babl_component ("ma"),
                     babl_component ("ka"),
                     NULL);
    babl_format_new ("name", "cairo-ACYK32",
                     babl_model ("camayakaA"),
                     babl_type ("u8"),
                     babl_component ("A"),
                     babl_component ("ca"),
                     babl_component ("ya"),
                     babl_component ("ka"),
                     NULL);
  }

  /* companion subset formats for setting pango u16 RGB color values from cmykA
   * */
  babl_format_new ("name", "cykA u16",
                   babl_model ("cmykA"),
                   babl_type ("u16"),
                   babl_component ("cyan"),
                   babl_component ("yellow"),
                   babl_component ("key"),
                   babl_component ("A"),
                   NULL);
  babl_format_new ("name", "cmkA u16",
                   babl_model ("cmykA"),
                   babl_type ("u16"),
                   babl_component ("cyan"),
                   babl_component ("magenta"),
                   babl_component ("key"),
                   babl_component ("A"),
                   NULL);



  return 0;
}
