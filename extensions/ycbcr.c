/* babl - dynamically extendable universal pixel conversion library.
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

#include <math.h>
#include <string.h>

#include "babl.h"
#include "base/util.h"


static void components  (void);
static void models      (void);
static void conversions (void);
static void formats     (void);

int init (void);


int
init (void)
{
  components ();
  models ();
  conversions ();
  formats ();

  return 0;
}


static void
components (void)
{
  babl_component_new ("alpha", NULL);
}


static void
models (void)
{
  babl_model_new (
    "name", "Y'CbCr709",
    babl_component ("Y'"),
    babl_component ("Cb"),
    babl_component ("Cr"),
    NULL);

  babl_model_new (
    "name", "Y'CbCrA709",
    babl_component ("Y'"),
    babl_component ("Cb"),
    babl_component ("Cr"),
    babl_component ("alpha"),
    "alpha",
    NULL);
}


static void
rgba_to_ycbcra709 (const Babl *conversion,
                   char       *src,
                   char       *dst,
                   long        n)
{
  while (n--)
    {
      double red   = ((double *) src)[0];
      double green = ((double *) src)[1];
      double blue  = ((double *) src)[2];
      double alpha = ((double *) src)[3];

      double luminance, cb, cr;

      red   = linear_to_gamma_2_2 (red);
      green = linear_to_gamma_2_2 (green);
      blue  = linear_to_gamma_2_2 (blue);

      luminance =  0.2126 * red + 0.7152 * green + 0.0722 * blue;
      cb        =  (blue - luminance) / 1.8556;
      cr        =  (red  - luminance) / 1.5748;

      ((double *) dst)[0] = luminance;
      ((double *) dst)[1] = cb;
      ((double *) dst)[2] = cr;
      ((double *) dst)[3] = alpha;

      src += sizeof (double) * 4;
      dst += sizeof (double) * 4;
    }
}


static void
rgba_to_ycbcr709 (const Babl *conversion,
                  char       *src,
                  char       *dst,
                  long        n)
{
  while (n--)
    {
      double red   = ((double *) src)[0];
      double green = ((double *) src)[1];
      double blue  = ((double *) src)[2];

      double luminance, cb, cr;

      red   = linear_to_gamma_2_2 (red);
      green = linear_to_gamma_2_2 (green);
      blue  = linear_to_gamma_2_2 (blue);

      luminance =  0.2126 * red + 0.7152 * green + 0.0722 * blue;
      cb        =  (blue - luminance) / 1.8556;
      cr        =  (red  - luminance) / 1.5748;

      ((double *) dst)[0] = luminance;
      ((double *) dst)[1] = cb;
      ((double *) dst)[2] = cr;

      src += sizeof (double) * 4;
      dst += sizeof (double) * 3;
    }
}


static void
ycbcra709_to_rgba (const Babl *conversion,
                   char       *src,
                   char       *dst,
                   long        n)
{
  while (n--)
    {
      double luminance = ((double *) src)[0];
      double cb        = ((double *) src)[1];
      double cr        = ((double *) src)[2];
      double alpha     = ((double *) src)[3];

      double red, green, blue;

      red   = 1.0 * luminance + 0.0    * cb + 1.5748 * cr;
      green = 1.0 * luminance - 0.1873 * cb - 0.4681 * cr;
      blue  = 1.0 * luminance + 1.8556 * cb + 0.0    * cr;

      red   = gamma_2_2_to_linear (red);
      green = gamma_2_2_to_linear (green);
      blue  = gamma_2_2_to_linear (blue);

      ((double *) dst)[0] = red;
      ((double *) dst)[1] = green;
      ((double *) dst)[2] = blue;
      ((double *) dst)[3] = alpha;

      src += sizeof (double) * 4;
      dst += sizeof (double) * 4;
    }
}


static void
ycbcr709_to_rgba (const Babl *conversion,
                  char       *src,
                  char       *dst,
                  long        n)
{
  while (n--)
    {
      double luminance = ((double *) src)[0];
      double cb        = ((double *) src)[1];
      double cr        = ((double *) src)[2];

      double red, green, blue;

      red   = 1.0 * luminance + 0.0    * cb + 1.5748 * cr;
      green = 1.0 * luminance - 0.1873 * cb - 0.4681 * cr;
      blue  = 1.0 * luminance + 1.8556 * cb + 0.0    * cr;

      red   = gamma_2_2_to_linear (red);
      green = gamma_2_2_to_linear (green);
      blue  = gamma_2_2_to_linear (blue);

      ((double *) dst)[0] = red;
      ((double *) dst)[1] = green;
      ((double *) dst)[2] = blue;
      ((double *) dst)[3] = 1.0;

      src += sizeof (double) * 3;
      dst += sizeof (double) * 4;
    }
}


static void
conversions (void)
{
  babl_conversion_new (
    babl_model ("RGBA"),
    babl_model ("Y'CbCr709"),
    "linear", rgba_to_ycbcr709,
    NULL
  );
  babl_conversion_new (
    babl_model ("RGBA"),
    babl_model ("Y'CbCrA709"),
    "linear", rgba_to_ycbcra709,
    NULL
  );
  babl_conversion_new (
    babl_model ("Y'CbCrA709"),
    babl_model ("RGBA"),
    "linear", ycbcra709_to_rgba,
    NULL
  );
  babl_conversion_new (
    babl_model ("Y'CbCr709"),
    babl_model ("RGBA"),
    "linear", ycbcr709_to_rgba,
    NULL
  );
}


static void
formats (void)
{
  babl_format_new (
    babl_model ("Y'CbCrA709"),
    babl_type ("float"),
    babl_component ("Y'"),
    babl_type ("float"),
    babl_component ("Cb"),
    babl_component ("Cr"),
    babl_component ("alpha"),
    NULL);

  babl_format_new (
    babl_model ("Y'CbCr709"),
    babl_type ("float"),
    babl_component ("Y'"),
    babl_type ("float"),
    babl_component ("Cb"),
    babl_component ("Cr"),
    NULL);
}
