/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2016,  Sirio Bolaños Puchet <vorstar@mac.com>
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

 /*
 * Adding support for HCY colorspace, based on the reference implementation by
 * Kuzma Shapran (https://code.google.com/archive/p/colour-space-viewer)
 */

#include "config.h"

#include <math.h>
#include <string.h>

#include "babl.h"
#include "base/util.h"

/* Rec. 601 as in original code //Rec. 709 */
#define RGB_LUMA_RED 0.299 //0.2126
#define RGB_LUMA_GREEN 0.587 //0.7152
#define RGB_LUMA_BLUE 0.114 //0.0722
#define EPSILON 1e-10

static void rgba_to_hcya     (const Babl *conversion,
                              char       *src,
                              char       *dst,
                              long        samples);

static void hcya_to_rgba     (const Babl *conversion,
                              char       *src,
                              char       *dst,
                              long        samples);

static void rgba_to_hcy      (const Babl *conversion,
                              char       *src,
                              char       *dst,
                              long        samples);

static void hcy_to_rgba      (const Babl *conversion,
                              char       *src,
                              char       *dst,
                              long        samples);

static void 
rgba_to_hcy_step (char *src,
                  char *dst);

static void 
hcy_to_rgba_step (char *src,
                  char *dst);

static void components       (void);
static void models           (void);
static void conversions      (void);
static void formats          (void);

int init (void);

int
init (void)
{
  components  ();
  models      ();
  conversions ();
  formats     ();

  return 0;
}


static void
components (void)
{
  babl_component_new ("hue", NULL);
  babl_component_new ("HCY chroma", "chroma", NULL);
  babl_component_new ("HCY luma", "luma", NULL);
  babl_component_new ("alpha", NULL);
}

static void
models (void)
{
  babl_model_new (
    "name", "HCYA",
    babl_component ("hue"),
    babl_component ("HCY chroma"),
    babl_component ("HCY luma"),
    babl_component ("alpha"),
    "alpha",
    NULL
  );

  babl_model_new (
    "name", "HCY",
    babl_component ("hue"),
    babl_component ("HCY chroma"),
    babl_component ("HCY luma"),
    NULL
  );
}

static void
conversions (void)
{
  babl_conversion_new (
    babl_model ("RGBA"),
    babl_model ("HCYA"),
    "linear", rgba_to_hcya,
    NULL
  );

  babl_conversion_new (
    babl_model ("RGBA"),
    babl_model ("HCY"),
    "linear", rgba_to_hcy,
    NULL
  );

  babl_conversion_new (
    babl_model ("HCYA"),
    babl_model ("RGBA"),
    "linear", hcya_to_rgba,
    NULL
  );

  babl_conversion_new (
    babl_model ("HCY"),
    babl_model ("RGBA"),
    "linear", hcy_to_rgba,
    NULL
  );
}

static void
formats (void)
{
  babl_format_new (
    "name", "HCYA float",
    babl_model ("HCYA"),
    babl_type ("float"),
    babl_component ("hue"),
    babl_component ("HCY chroma"),
    babl_component ("HCY luma"),
    babl_component ("alpha"),
    NULL
  );

  babl_format_new (
    "name", "HCY float",
    babl_model ("HCY"),
    babl_type ("float"),
    babl_component ("hue"),
    babl_component ("HCY chroma"),
    babl_component ("HCY luma"),
    NULL
  );
}

static void
rgba_to_hcy_step (char *src,
                  char *dst)
{
  static const double weights[3] = {RGB_LUMA_RED,RGB_LUMA_GREEN,RGB_LUMA_BLUE};
  double hue, chroma, luma;
  double X, Y_peak = 0.;
  int H_sec = 4, t = -1;

  double rgb[3] = {linear_to_gamma_2_2 (((double *) src)[0]),
    linear_to_gamma_2_2 (((double *) src)[1]),
    linear_to_gamma_2_2 (((double *) src)[2])};
  int ix[3] = {0,1,2};

  if (rgb[0] < rgb[1]) {
    if (rgb[1] > rgb[2]) {
      if (rgb[0] < rgb[2]) { ix[1] = 2; ix[2] = 1; H_sec = 2; t = 1; }
      else { ix[0] = 2; ix[1] = 0; ix[2] = 1; H_sec = 2; }
    }
  } else {
    if (rgb[1] < rgb[2]) {
      if (rgb[0] < rgb[2]) { ix[0] = 1; ix[1] = 0; H_sec = 4; t = 1; }
      else { ix[0] = 1; ix[1] = 2; ix[2] = 0; H_sec = 6; }
    } else { ix[0] = 2; ix[2] = 0; H_sec = 0; t = 1; }
  }

  luma = weights[0] * rgb[0] + weights[1] * rgb[1] + weights[2] * rgb[2];
  chroma = rgb[ix[2]] - rgb[ix[0]];

  if (chroma >= EPSILON)
  {
    X = (rgb[ix[1]] - rgb[ix[0]]) / chroma;

    Y_peak = weights[ix[2]] + X * weights[ix[1]];
    if (luma != 0. && luma != 1.)
      chroma /= luma < Y_peak ? luma/Y_peak : (1. - luma)/(1. - Y_peak);

    hue = (H_sec + t * X) / 6.;
  }
  else
    chroma = hue = 0.0;

  ((double *) dst)[0] = hue;
  ((double *) dst)[1] = chroma;
  ((double *) dst)[2] = luma;
}

static void
hcy_to_rgba_step (char *src,
                  char *dst)
{
  double red, green, blue;
  double Y_peak = 0., H_insec, X, m;
  int H_sec;

  double hue    = ((double *) src)[0];
  double chroma = ((double *) src)[1];
  double luma   = ((double *) src)[2];

  if(chroma < EPSILON) {
    red = green = blue = luma;
  } else {
    hue *= 6.;
    H_sec = (int)hue;

    switch (H_sec)
    {
      case 0:
        H_insec = hue - H_sec;
        Y_peak = RGB_LUMA_RED + H_insec * RGB_LUMA_GREEN;
        chroma *= luma < Y_peak ? luma/Y_peak : (1. - luma)/(1. - Y_peak);
        X = chroma * H_insec;
        m = luma - (RGB_LUMA_RED * chroma + RGB_LUMA_GREEN * X);
        red = m + chroma; green = m + X; blue = m;
        break;
      case 1:
        H_insec = 1. - (hue - H_sec);
        Y_peak = RGB_LUMA_GREEN + H_insec * RGB_LUMA_RED;
        chroma *= luma < Y_peak ? luma/Y_peak : (1. - luma)/(1. - Y_peak);
        X = chroma * H_insec;
        m = luma - (RGB_LUMA_RED * X + RGB_LUMA_GREEN * chroma);
        red = m + X; green = m + chroma; blue = m;
        break;
      case 2:
        H_insec = hue - H_sec;
        Y_peak = RGB_LUMA_GREEN + H_insec * RGB_LUMA_BLUE;
        chroma *= luma < Y_peak ? luma/Y_peak : (1. - luma)/(1. - Y_peak);
        X = chroma * H_insec;
        m = luma - (RGB_LUMA_GREEN * chroma + RGB_LUMA_BLUE * X);
        red = m; green = m + chroma; blue = m + X;
        break;
      case 3:
        H_insec = 1. - (hue - H_sec);
        Y_peak = RGB_LUMA_BLUE + H_insec * RGB_LUMA_GREEN;
        chroma *= luma < Y_peak ? luma/Y_peak : (1. - luma)/(1. - Y_peak);
        X = chroma * H_insec;
        m = luma - (RGB_LUMA_GREEN * X + RGB_LUMA_BLUE * chroma);
        red = m; green = m + X; blue = m + chroma;
        break;
      case 4:
        H_insec = hue - H_sec;
        Y_peak = RGB_LUMA_BLUE + H_insec * RGB_LUMA_RED;
        chroma *= luma < Y_peak ? luma/Y_peak : (1. - luma)/(1. - Y_peak);
        X = chroma * H_insec;
        m = luma - (RGB_LUMA_RED * X + RGB_LUMA_BLUE * chroma);
        red = m + X; green = m; blue = m + chroma;
        break;
      default:
        H_insec = 1. - (hue - H_sec);
        Y_peak = RGB_LUMA_RED + H_insec * RGB_LUMA_BLUE;
        chroma *= luma < Y_peak ? luma/Y_peak : (1. - luma)/(1. - Y_peak);
        X = chroma * H_insec;
        m = luma - (RGB_LUMA_RED * chroma + RGB_LUMA_BLUE * X);
        red = m + chroma; green = m; blue = m + X;
        break;
    }
  }

  ((double *) dst)[0] = gamma_2_2_to_linear (red);
  ((double *) dst)[1] = gamma_2_2_to_linear (green);
  ((double *) dst)[2] = gamma_2_2_to_linear (blue);
}

static void
rgba_to_hcya (const Babl *conversion,
              char       *src,
              char       *dst,
              long        samples)
{
  long n = samples;

  while (n--)
  {
    double alpha = ((double *) src)[3];

    rgba_to_hcy_step (src, dst);

    ((double *) dst)[3] = alpha;

    src += 4 * sizeof (double);
    dst += 4 * sizeof (double);
  }
}

static void
hcya_to_rgba (const Babl *conversion,char *src,
              char *dst,
              long  samples)
{
  long n = samples;

  while (n--)
  {
    double alpha = ((double *) src)[3];

    hcy_to_rgba_step (src, dst);

    ((double *) dst)[3] = alpha;

    src += 4 * sizeof (double);
    dst += 4 * sizeof (double);
  }
}

static void
rgba_to_hcy (const Babl *conversion,
             char       *src,
             char       *dst,
             long        samples)
{
  long n = samples;

  while (n--)
  {
    rgba_to_hcy_step (src, dst);

    src += 4 * sizeof (double);
    dst += 3 * sizeof (double);
  }
}

static void
hcy_to_rgba (const Babl *conversion,
             char       *src,
             char       *dst,
             long        samples)
{
  long n = samples;

  while (n--)
  {
    hcy_to_rgba_step (src, dst);

    ((double *) dst)[3] = 1.0;

    src += 3 * sizeof (double);
    dst += 4 * sizeof (double);
  }
}
