/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2005, 2018 Øyvind Kolås.
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

/* This file define the "cmy" and "cmyk" models and related formats these
 * are CMYK formats withthe components stored so that 0 means full coverage
 * and 1.0 means no coverage which makes additive compositing/blending work
 * like as if it was RGB

 * conversions should be made with reference to the icc profile in the space
 * using lcms2 for handling.
 */

#include "config.h"

#include <math.h>
#include <string.h>

#include "babl-internal.h"
#include "babl-base.h"
#include "base/util.h"


static void
cmyka_to_cmykA (const Babl *conversion,
                char       *src,
                char       *dst,
                long        n)
{
  while (n--)
    {
      double cyan    = ((double *) src)[0];
      double magenta = ((double *) src)[1];
      double yellow  = ((double *) src)[2];
      double key     = ((double *) src)[3];
      double alpha   = ((double *) src)[4];

      ((double *) dst)[0] = (cyan)    * alpha;
      ((double *) dst)[1] = (magenta) * alpha;
      ((double *) dst)[2] = (yellow)  * alpha;
      ((double *) dst)[3] = (key)     * alpha;
      ((double *) dst)[4] = alpha;

      src += 5 * sizeof (double);
      dst += 5 * sizeof (double);
    }
}

static void
cmykA_to_cmyka (const Babl *conversion,
                char       *src,
                char       *dst,
                long        n)
{
  while (n--)
    {
      double alpha   = ((double *) src)[4];
      double ralpha  = alpha>0.000001?1.0/alpha:0.0;
      double cyan   = ((double *) src)[0] * ralpha;
      double magenta= ((double *) src)[1] * ralpha;
      double yellow = ((double *) src)[2] * ralpha;
      double key    = ((double *) src)[3] * ralpha;

      ((double *) dst)[0] = cyan;
      ((double *) dst)[1] = magenta;
      ((double *) dst)[2] = yellow;
      ((double *) dst)[3] = key;
      ((double *) dst)[4] = alpha;

      src += 5 * sizeof (double);
      dst += 5 * sizeof (double);
    }
}

static void
cmyk_to_cmyka (const Babl *conversion,
               char       *src,
               char       *dst,
               long        n)
{
  while (n--)
    {
      double cyan    = ((double *) src)[0];
      double magenta = ((double *) src)[1];
      double yellow  = ((double *) src)[2];
      double key     = ((double *) src)[3];

      ((double *) dst)[0] = cyan;
      ((double *) dst)[1] = magenta;
      ((double *) dst)[2] = yellow;
      ((double *) dst)[3] = key;
      ((double *) dst)[4] = 1.0;

      src += 4 * sizeof (double);
      dst += 5 * sizeof (double);
    }
}

static void
cmyka_to_cmyk (const Babl *conversion,
               char       *src,
               char       *dst,
               long        n)
{
  while (n--)
    {
      double cyan    = ((double *) src)[0];
      double magenta = ((double *) src)[1];
      double yellow  = ((double *) src)[2];
      double key     = ((double *) src)[3];

      ((double *) dst)[0] = cyan;
      ((double *) dst)[1] = magenta;
      ((double *) dst)[2] = yellow;
      ((double *) dst)[3] = key;

      src += 5 * sizeof (double);
      dst += 4 * sizeof (double);
    }
}

/////////////////////
static void
cmyka_to_CMYKA (const Babl *conversion,
                char       *src,
                char       *dst,
                long        n)
{
  while (n--)
    {
      double cyan    = ((double *) src)[0];
      double magenta = ((double *) src)[1];
      double yellow  = ((double *) src)[2];
      double key     = ((double *) src)[3];
      double alpha   = ((double *) src)[4];

      ((double *) dst)[0] = (1.0-cyan)    * alpha;
      ((double *) dst)[1] = (1.0-magenta) * alpha;
      ((double *) dst)[2] = (1.0-yellow)  * alpha;
      ((double *) dst)[3] = (1.0-key)     * alpha;
      ((double *) dst)[4] = alpha;

      src += 5 * sizeof (double);
      dst += 5 * sizeof (double);
    }
}

static void
CMYKA_to_cmyka (const Babl *conversion,
                char       *src,
                char       *dst,
                long        n)
{
  while (n--)
    {
      double alpha   = ((double *) src)[4];
      double ralpha  = alpha>0.000001?1.0/alpha:0.0;
      double cyan   = ((double *) src)[0] * ralpha;
      double magenta= ((double *) src)[1] * ralpha;
      double yellow = ((double *) src)[2] * ralpha;
      double key    = ((double *) src)[3] * ralpha;

      ((double *) dst)[0] = 1.0-cyan;
      ((double *) dst)[1] = 1.0-magenta;
      ((double *) dst)[2] = 1.0-yellow;
      ((double *) dst)[3] = 1.0-key;
      ((double *) dst)[4] = alpha;

      src += 5 * sizeof (double);
      dst += 5 * sizeof (double);
    }
}



static void
CMYK_to_cmyka (const Babl *conversion,
               char       *src,
               char       *dst,
               long        n)
{
  while (n--)
    {
      double cyan    = ((double *) src)[0];
      double magenta = ((double *) src)[1];
      double yellow  = ((double *) src)[2];
      double key     = ((double *) src)[3];

      ((double *) dst)[0] = 1.0-cyan;
      ((double *) dst)[1] = 1.0-magenta;
      ((double *) dst)[2] = 1.0-yellow;
      ((double *) dst)[3] = 1.0-key;
      ((double *) dst)[4] = 1.0;

      src += 4 * sizeof (double);
      dst += 5 * sizeof (double);
    }
}

static void
cmyka_to_CMYK (const Babl *conversion,
               char       *src,
               char       *dst,
               long        n)
{
  while (n--)
    {
      double cyan    = ((double *) src)[0];
      double magenta = ((double *) src)[1];
      double yellow  = ((double *) src)[2];
      double key     = ((double *) src)[3];

      ((double *) dst)[0] = 1.0-cyan;
      ((double *) dst)[1] = 1.0-magenta;
      ((double *) dst)[2] = 1.0-yellow;
      ((double *) dst)[3] = 1.0-key;

      src += 5 * sizeof (double);
      dst += 4 * sizeof (double);
    }
}

static void
cmyka_to_CMYKa (const Babl *conversion,
                char       *src,
                char       *dst,
                long        n)
{
  while (n--)
    {
      double cyan    = ((double *) src)[0];
      double magenta = ((double *) src)[1];
      double yellow  = ((double *) src)[2];
      double key     = ((double *) src)[3];
      double alpha   = ((double *) src)[4];

      ((double *) dst)[0] = 1.0-cyan;
      ((double *) dst)[1] = 1.0-magenta;
      ((double *) dst)[2] = 1.0-yellow;
      ((double *) dst)[3] = 1.0-key;
      ((double *) dst)[4] = alpha;

      src += 5 * sizeof (double);
      dst += 5 * sizeof (double);
    }
}







#if 0
static void
rgba_to_cmykA (const Babl *conversion,
               char       *src,
               char       *dst,
               long        n)
{
  while (n--)
    {
      double red   = (((double *) src)[0]);
      double green = (((double *) src)[1]);
      double blue  = (((double *) src)[2]);
      double alpha = ((double *) src)[3];

      double cyan, magenta, yellow, key;

      double pullout = 1.0;

      cyan    = 1.0 - red;
      magenta = 1.0 - green;
      yellow  = 1.0 - blue;

      key = 1.0;
      if (cyan < key) key = cyan;
      if (magenta < key) key = magenta;
      if (yellow < key) key = yellow;

      key *= pullout;

      if (key < 1.0)
        {
          cyan    = (cyan - key)    / (1.0 - key);
          magenta = (magenta - key) / (1.0 - key);
          yellow  = (yellow - key)  / (1.0 - key);
        }
      else
        {
          cyan    = 0.0;
          magenta = 0.0;
          yellow  = 0.0;
        }

      ((double *) dst)[0] = (1.0-cyan) * alpha;
      ((double *) dst)[1] = (1.0-magenta) * alpha;
      ((double *) dst)[2] = (1.0-yellow) * alpha;
      ((double *) dst)[3] = (1.0-key) * alpha;
      ((double *) dst)[4] = alpha;

      src += 4 * sizeof (double);
      dst += 5 * sizeof (double);
    }
}

static void
cmykA_to_rgba (const Babl *conversion,
               char       *src,
               char       *dst,
               long        n)
{
  while (n--)
    {
      double alpha   = ((double *) src)[4];
      double ralpha  = alpha>0.000001?1.0/alpha:0.0;
      double cyanI   = ((double *) src)[0] * ralpha;
      double magentaI= ((double *) src)[1] * ralpha;
      double yellowI = ((double *) src)[2] * ralpha;
      double keyI    = ((double *) src)[3] * ralpha;

      double cyan    = 1.0-cyanI;
      double magenta = 1.0-magentaI;
      double yellow  = 1.0-yellowI;
      double key     = 1.0-keyI;

      double red, green, blue;

      if (key < 1.0)
        {
          cyan    = cyan * (1.0 - key) + key;
          magenta = magenta * (1.0 - key) + key;
          yellow  = yellow * (1.0 - key) + key;
        }
      else
        {
          cyan = magenta = yellow = 1.0;
        }

      red   = 1.0 - cyan;
      green = 1.0 - magenta;
      blue  = 1.0 - yellow;

      ((double *) dst)[0] = (red);
      ((double *) dst)[1] = (green);
      ((double *) dst)[2] = (blue);
      ((double *) dst)[3] = alpha;

      src += 5 * sizeof (double);
      dst += 4 * sizeof (double);
    }
}

static void
rgba_to_cmyka (const Babl *conversion,
               char       *src,
               char       *dst,
               long        n)
{
  while (n--)
    {
      double red   = (((double *) src)[0]);
      double green = (((double *) src)[1]);
      double blue  = (((double *) src)[2]);
      double alpha = ((double *) src)[3];

      double cyan, magenta, yellow, key;

      double pullout = 1.0;

      cyan    = 1.0 - red;
      magenta = 1.0 - green;
      yellow  = 1.0 - blue;

      key = 1.0;
      if (cyan < key) key = cyan;
      if (magenta < key) key = magenta;
      if (yellow < key) key = yellow;

      key *= pullout;

      if (key < 1.0)
        {
          cyan    = (cyan - key)    / (1.0 - key);
          magenta = (magenta - key) / (1.0 - key);
          yellow  = (yellow - key)  / (1.0 - key);
        }
      else
        {
          cyan    = 0.0;
          magenta = 0.0;
          yellow  = 0.0;
        }

      ((double *) dst)[0] = 1.0-cyan;
      ((double *) dst)[1] = 1.0-magenta;
      ((double *) dst)[2] = 1.0-yellow;
      ((double *) dst)[3] = 1.0-key;
      ((double *) dst)[4] = alpha;

      src += 4 * sizeof (double);
      dst += 5 * sizeof (double);
    }
}

static void
cmyka_to_rgba (const Babl *conversion,
               char       *src,
               char       *dst,
               long        n)
{
  while (n--)
    {
      double cyan    = 1.0-((double *) src)[0];
      double magenta = 1.0-((double *) src)[1];
      double yellow  = 1.0-((double *) src)[2];
      double key     = 1.0-((double *) src)[3];
      double alpha   = ((double *) src)[4];

      double red, green, blue;

      if (key < 1.0)
        {
          cyan    = cyan * (1.0 - key) + key;
          magenta = magenta * (1.0 - key) + key;
          yellow  = yellow * (1.0 - key) + key;
        }
      else
        {
          cyan = magenta = yellow = 1.0;
        }

      red   = 1.0 - cyan;
      green = 1.0 - magenta;
      blue  = 1.0 - yellow;

      ((double *) dst)[0] = (red);
      ((double *) dst)[1] = (green);
      ((double *) dst)[2] = (blue);
      ((double *) dst)[3] = alpha;

      src += 5 * sizeof (double);
      dst += 4 * sizeof (double);
    }
}

static void
rgba_to_cmyk (const Babl *conversion,
              char       *src,
              char       *dst,
              long        n)
{
  while (n--)
    {
      double red   = (((double *) src)[0]);
      double green = (((double *) src)[1]);
      double blue  = (((double *) src)[2]);

      double cyan, magenta, yellow, key;

      double pullout = 1.0;

      cyan    = 1.0 - red;
      magenta = 1.0 - green;
      yellow  = 1.0 - blue;

      key = 1.0;
      if (cyan < key) key = cyan;
      if (magenta < key) key = magenta;
      if (yellow < key) key = yellow;

      key *= pullout;

      if (key < 1.0)
        {
          cyan    = (cyan - key)    / (1.0 - key);
          magenta = (magenta - key) / (1.0 - key);
          yellow  = (yellow - key)  / (1.0 - key);
        }
      else
        {
          cyan    = 0.0;
          magenta = 0.0;
          yellow  = 0.0;
        }

      ((double *) dst)[0] = 1.0-cyan;
      ((double *) dst)[1] = 1.0-magenta;
      ((double *) dst)[2] = 1.0-yellow;
      ((double *) dst)[3] = 1.0-key;

      src += 4 * sizeof (double);
      dst += 4 * sizeof (double);
    }
}

static void
cmyk_to_rgba (const Babl *conversion,
              char       *src,
              char       *dst,
              long        n)
{
  while (n--)
    {
      double cyan    = 1.0-((double *) src)[0];
      double magenta = 1.0-((double *) src)[1];
      double yellow  = 1.0-((double *) src)[2];
      double key     = 1.0-((double *) src)[3];

      double red, green, blue;

      if (key < 1.0)
        {
          cyan    = cyan * (1.0 - key) + key;
          magenta = magenta * (1.0 - key) + key;
          yellow  = yellow * (1.0 - key) + key;
        }
      else
        {
          cyan = magenta = yellow = 1.0;
        }

      red   = 1.0 - cyan;
      green = 1.0 - magenta;
      blue  = 1.0 - yellow;

      ((double *) dst)[0] = (red);
      ((double *) dst)[1] = (green);
      ((double *) dst)[2] = (blue);

      ((double *) dst)[3] = 1.0;

      src += 4 * sizeof (double);
      dst += 4 * sizeof (double);
    }
}

static void
rgba_to_cmy (const Babl *conversion,
             char       *src,
             char       *dst,
             long        n)
{
  while (n--)
    {
      double red   = (((double *) src)[0]);
      double green = (((double *) src)[1]);
      double blue  = (((double *) src)[2]);

      double cyan, magenta, yellow;

      cyan    = 1.0 - red;
      magenta = 1.0 - green;
      yellow  = 1.0 - blue;

      ((double *) dst)[0] = 1.0-cyan;
      ((double *) dst)[1] = 1.0-magenta;
      ((double *) dst)[2] = 1.0-yellow;

      src += 4 * sizeof (double);
      dst += 3 * sizeof (double);
    }
}

static void
cmy_to_rgba (const Babl *conversion,
             char       *src,
             char       *dst,
             long        n)
{
  while (n--)
    {
      double cyan    = 1.0-((double *) src)[0];
      double magenta = 1.0-((double *) src)[1];
      double yellow  = 1.0-((double *) src)[2];

      double red, green, blue;

      red   = 1.0 - cyan;
      green = 1.0 - magenta;
      blue  = 1.0 - yellow;

      ((double *) dst)[0] = (red);
      ((double *) dst)[1] = (green);
      ((double *) dst)[2] = (blue);

      ((double *) dst)[3] = 1.0;

      src += 3 * sizeof (double);
      dst += 4 * sizeof (double);
    }
}
#endif

void
babl_base_model_cmyk (void)
{
  babl_component_new ("cyan", NULL);
  babl_component_new ("yellow", NULL);
  babl_component_new ("magenta", NULL);
  babl_component_new ("key", NULL);


  babl_component_new ("ca", NULL);
  babl_component_new ("ma", NULL);
  babl_component_new ("ya", NULL);
  babl_component_new ("ka", NULL);


  babl_component_new ("Cyan", NULL);
  babl_component_new ("Yellow", NULL);
  babl_component_new ("Magenta", NULL);
  babl_component_new ("Key", NULL);


  babl_component_new ("Ca", NULL);
  babl_component_new ("Ma", NULL);
  babl_component_new ("Yea", NULL);
  babl_component_new ("Ka", NULL);

  babl_model_new (
    "name", "camayakaA",
    babl_component ("ca"),
    babl_component ("ma"),
    babl_component ("ya"),
    babl_component ("ka"),
    babl_component ("A"),
    "cmyk",
    "inverted",
    "alpha",
    "premultiplied",
    NULL
  );

  babl_model_new (
    "name", "cmykA",
    babl_component ("cyan"),
    babl_component ("magenta"),
    babl_component ("yellow"),
    babl_component ("key"),
    babl_component ("A"),
    "cmyk",
    "inverted",
    "alpha",
    NULL
  );
  babl_model_new (
    "name", "cmyk",
    babl_component ("cyan"),
    babl_component ("magenta"),
    babl_component ("yellow"),
    babl_component ("key"),
    "cmyk",
    "inverted",
    NULL
  );

  babl_model_new (
    "name", "CaMaYaKaA",
    babl_component ("Ca"),
    babl_component ("Ma"),
    babl_component ("Yea"),
    babl_component ("Ka"),
    babl_component ("A"),
    "cmyk",
    "alpha",
    "premultiplied",
    NULL
  );

  babl_model_new (
    "name", "CMYKA",
    babl_component ("Cyan"),
    babl_component ("Magenta"),
    babl_component ("Yellow"),
    babl_component ("Key"),
    babl_component ("A"),
    "cmyk",
    "alpha",
    NULL
  );
  babl_model_new (
    "name", "CMYK",
    babl_component ("Cyan"),
    babl_component ("Magenta"),
    babl_component ("Yellow"),
    babl_component ("Key"),
    "cmyk",
    NULL
  );

  babl_conversion_new (
    babl_model ("cmykA"),
    babl_model ("cmyk"),
    "linear", cmyka_to_cmyk,
    NULL
  );
  babl_conversion_new (
    babl_model ("cmyk"),
    babl_model ("cmykA"),
    "linear", cmyk_to_cmyka,
    NULL
  );
  babl_conversion_new (
    babl_model ("cmykA"),
    babl_model ("camayakaA"),
    "linear", cmyka_to_cmykA,
    NULL
  );
  babl_conversion_new (
    babl_model ("camayakaA"),
    babl_model ("cmykA"),
    "linear", cmykA_to_cmyka,
    NULL
  );

  babl_conversion_new (
    babl_model ("cmykA"),
    babl_model ("CMYKA"),
    "linear", cmyka_to_CMYKa,
    NULL
  );
  babl_conversion_new (
    babl_model ("CMYKA"),
    babl_model ("cmykA"),
    "linear", cmyka_to_CMYKa, // does the same job
    NULL
  );


  babl_conversion_new (
    babl_model ("cmykA"),
    babl_model ("CMYK"),
    "linear", cmyka_to_CMYK,
    NULL
  );
  babl_conversion_new (
    babl_model ("CMYK"),
    babl_model ("cmykA"),
    "linear", CMYK_to_cmyka,
    NULL
  );
  babl_conversion_new (
    babl_model ("cmykA"),
    babl_model ("CaMaYaKaA"),
    "linear", cmyka_to_CMYKA,
    NULL
  );
  babl_conversion_new (
    babl_model ("CaMaYaKaA"),
    babl_model ("cmykA"),
    "linear", CMYKA_to_cmyka,
    NULL
  );



#if 0

  babl_conversion_new (
    babl_model ("RGBA"),
    babl_model ("camayakaA"),
    "linear", rgba_to_cmykA,
    NULL
  );
  babl_conversion_new (
    babl_model ("camayakaA"),
    babl_model ("RGBA"),
    "linear", cmykA_to_rgba,
    NULL
  );

  babl_conversion_new (
    babl_model ("RGBA"),
    babl_model ("cmykA"),
    "linear", rgba_to_cmyka,
    NULL
  );
  babl_conversion_new (
    babl_model ("cmykA"),
    babl_model ("RGBA"),
    "linear", cmyka_to_rgba,
    NULL
  );
  babl_conversion_new (
    babl_model ("RGBA"),
    babl_model ("cmyk"),
    "linear", rgba_to_cmyk,
    NULL
  );
  babl_conversion_new (
    babl_model ("cmyk"),
    babl_model ("RGBA"),
    "linear", cmyk_to_rgba,
    NULL
  );
  babl_conversion_new (
    babl_model ("RGBA"),
    babl_model ("cmy"),
    "linear", rgba_to_cmy,
    NULL
  );
  babl_conversion_new (
    babl_model ("cmy"),
    babl_model ("RGBA"),
    "linear", cmy_to_rgba,
    NULL
  );
#endif

  babl_format_new (
    "name", "camayakaA float",
    babl_model ("camayakaA"),
    babl_type ("float"),
    babl_component ("ca"),
    babl_component ("ma"),
    babl_component ("ya"),
    babl_component ("ka"),
    babl_component ("A"),
    NULL
  );
  babl_format_new (
    "name", "cmykA float",
    babl_model ("cmykA"),
    babl_type ("float"),
    babl_component ("cyan"),
    babl_component ("magenta"),
    babl_component ("yellow"),
    babl_component ("key"),
    babl_component ("A"),
    NULL
  );
  babl_format_new (
    "name", "cmyk float",
    babl_model ("cmyk"),
    babl_type ("float"),
    babl_component ("cyan"),
    babl_component ("magenta"),
    babl_component ("yellow"),
    babl_component ("key"),
    NULL
  );

  babl_format_new (
    "name", "cmyk u8",
    babl_model ("cmyk"),
    babl_type ("u8"),
    babl_component ("cyan"),
    babl_component ("magenta"),
    babl_component ("yellow"),
    babl_component ("key"),
    NULL
  );
  babl_format_new (
    "name", "cmykA u8",
    babl_model ("cmykA"),
    babl_type ("u8"),
    babl_component ("cyan"),
    babl_component ("magenta"),
    babl_component ("yellow"),
    babl_component ("key"),
    babl_component ("A"),
    NULL
  );

  babl_format_new (
    "name", "cmyk u16",
    babl_model ("cmyk"),
    babl_type ("u16"),
    babl_component ("cyan"),
    babl_component ("magenta"),
    babl_component ("yellow"),
    babl_component ("key"),
    NULL
  );
  babl_format_new (
    "name", "cmykA u16",
    babl_model ("cmykA"),
    babl_type ("u16"),
    babl_component ("cyan"),
    babl_component ("magenta"),
    babl_component ("yellow"),
    babl_component ("key"),
    babl_component ("A"),
    NULL
  );
  babl_format_new (
    "name", "cmyk u32",
    babl_model ("cmyk"),
    babl_type ("u32"),
    babl_component ("cyan"),
    babl_component ("magenta"),
    babl_component ("yellow"),
    babl_component ("key"),
    NULL
  );
  babl_format_new (
    "name", "cmykA u32",
    babl_model ("cmykA"),
    babl_type ("u32"),
    babl_component ("cyan"),
    babl_component ("magenta"),
    babl_component ("yellow"),
    babl_component ("key"),
    babl_component ("A"),
    NULL
  );

  babl_format_new (
    "name", "cmyk float",
    babl_model ("cmyk"),
    babl_type ("float"),
    babl_component ("cyan"),
    babl_component ("magenta"),
    babl_component ("yellow"),
    babl_component ("key"),
    NULL
  );
  babl_format_new (
    "name", "cmykA float",
    babl_model ("cmykA"),
    babl_type ("float"),
    babl_component ("cyan"),
    babl_component ("magenta"),
    babl_component ("yellow"),
    babl_component ("key"),
    babl_component ("A"),
    NULL
  );

  babl_format_new (
    "name", "cmyk half",
    babl_model ("cmyk"),
    babl_type ("half"),
    babl_component ("cyan"),
    babl_component ("magenta"),
    babl_component ("yellow"),
    babl_component ("key"),
    NULL
  );
  babl_format_new (
    "name", "cmykA half",
    babl_model ("cmykA"),
    babl_type ("half"),
    babl_component ("cyan"),
    babl_component ("magenta"),
    babl_component ("yellow"),
    babl_component ("key"),
    babl_component ("A"),
    NULL
  );
  babl_format_new (
    "name", "camayakaA u16",
    babl_model ("camayakaA"),
    babl_type ("u16"),
    babl_component ("ca"),
    babl_component ("ma"),
    babl_component ("ya"),
    babl_component ("ka"),
    babl_component ("A"),
    NULL
  );
  babl_format_new (
    "name", "camayakaA u8",
    babl_model ("camayakaA"),
    babl_type ("u8"),
    babl_component ("ca"),
    babl_component ("ma"),
    babl_component ("ya"),
    babl_component ("ka"),
    babl_component ("A"),
    NULL
  );

  babl_format_new (
    "name", "camayakaA half",
    babl_model ("camayakaA"),
    babl_type ("half"),
    babl_component ("ca"),
    babl_component ("ma"),
    babl_component ("ya"),
    babl_component ("ka"),
    babl_component ("A"),
    NULL
  );

  /********************************/
  babl_format_new (
    "name", "CaMaYaKaA float",
    babl_model ("CaMaYaKaA"),
    babl_type ("float"),
    babl_component ("Ca"),
    babl_component ("Ma"),
    babl_component ("Ya"),
    babl_component ("Ka"),
    babl_component ("A"),
    NULL
  );
  babl_format_new (
    "name", "CMYKA float",
    babl_model ("CMYKA"),
    babl_type ("float"),
    babl_component ("Cyan"),
    babl_component ("Magenta"),
    babl_component ("Yellow"),
    babl_component ("Key"),
    babl_component ("A"),
    NULL
  );
  babl_format_new (
    "name", "CMYK float",
    babl_model ("CMYK"),
    babl_type ("float"),
    babl_component ("Cyan"),
    babl_component ("Magenta"),
    babl_component ("Yellow"),
    babl_component ("Key"),
    NULL
  );
  babl_format_new (
    "name", "CMYKA half",
    babl_model ("CMYKA"),
    babl_type ("half"),
    babl_component ("Cyan"),
    babl_component ("Magenta"),
    babl_component ("Yellow"),
    babl_component ("Key"),
    babl_component ("A"),
    NULL
  );
  babl_format_new (
    "name", "CMYK half",
    babl_model ("CMYK"),
    babl_type ("half"),
    babl_component ("Cyan"),
    babl_component ("Magenta"),
    babl_component ("Yellow"),
    babl_component ("Key"),
    NULL
  );

  babl_format_new (
    "name", "CMYK u8",
    babl_model ("CMYK"),
    babl_type ("u8"),
    babl_component ("Cyan"),
    babl_component ("Magenta"),
    babl_component ("Yellow"),
    babl_component ("Key"),
    NULL
  );
  babl_format_new (
    "name", "CMYKA u8",
    babl_model ("CMYKA"),
    babl_type ("u8"),
    babl_component ("Cyan"),
    babl_component ("Magenta"),
    babl_component ("Yellow"),
    babl_component ("Key"),
    babl_component ("A"),
    NULL
  );

  babl_format_new (
    "name", "CMYK u16",
    babl_model ("CMYK"),
    babl_type ("u16"),
    babl_component ("Cyan"),
    babl_component ("Magenta"),
    babl_component ("Yellow"),
    babl_component ("Key"),
    NULL
  );
  babl_format_new (
    "name", "CMYKA u16",
    babl_model ("CMYKA"),
    babl_type ("u16"),
    babl_component ("Cyan"),
    babl_component ("Magenta"),
    babl_component ("Yellow"),
    babl_component ("Key"),
    babl_component ("A"),
    NULL
  );
  babl_format_new (
    "name", "CMYK u32",
    babl_model ("CMYK"),
    babl_type ("u32"),
    babl_component ("Cyan"),
    babl_component ("Magenta"),
    babl_component ("Yellow"),
    babl_component ("Key"),
    NULL
  );
  babl_format_new (
    "name", "CMYKA u32",
    babl_model ("CMYKA"),
    babl_type ("u32"),
    babl_component ("Cyan"),
    babl_component ("Magenta"),
    babl_component ("Yellow"),
    babl_component ("Key"),
    babl_component ("A"),
    NULL
  );

  babl_format_new (
    "name", "CMYK float",
    babl_model ("CMYK"),
    babl_type ("float"),
    babl_component ("Cyan"),
    babl_component ("Magenta"),
    babl_component ("Yellow"),
    babl_component ("Key"),
    NULL
  );
  babl_format_new (
    "name", "CMYKA float",
    babl_model ("CMYKA"),
    babl_type ("float"),
    babl_component ("Cyan"),
    babl_component ("Magenta"),
    babl_component ("Yellow"),
    babl_component ("Key"),
    babl_component ("A"),
    NULL
  );
  babl_format_new (
    "name", "CaMaYaKaA u8",
    babl_model ("CaMaYaKaA"),
    babl_type ("u8"),
    babl_component ("Ca"),
    babl_component ("Ma"),
    babl_component ("Yea"),
    babl_component ("Ka"),
    babl_component ("A"),
    NULL
  );

  babl_format_new (
    "name", "CaMaYaKaA u16",
    babl_model ("CaMaYaKaA"),
    babl_type ("u16"),
    babl_component ("Ca"),
    babl_component ("Ma"),
    babl_component ("Yea"),
    babl_component ("Ka"),
    babl_component ("A"),
    NULL
  );
  babl_format_new (
    "name", "CaMaYaKaA half",
    babl_model ("CaMaYaKaA"),
    babl_type ("half"),
    babl_component ("Ca"),
    babl_component ("Ma"),
    babl_component ("Yea"),
    babl_component ("Ka"),
    babl_component ("A"),
    NULL
  );
}

