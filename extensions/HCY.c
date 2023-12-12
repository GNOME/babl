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
                  char *dst,
                  const double weights[3]);

static void
hcy_to_rgba_step (char *src,
                  char *dst,
                  const double weights[3]);

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
  babl_component_new ("alpha", "alpha", NULL);
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
    "name", "HCY float",
    babl_model ("HCY"),
    babl_type ("float"),
    babl_component ("hue"),
    babl_component ("HCY chroma"),
    babl_component ("HCY luma"),
    NULL
  );

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
}

static void
rgba_to_hcy_step (char *src,
                  char *dst,
                  const double weights[3])
{
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
                  char *dst,
                  const double weights[3])
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
    hue  = fmod (hue, 1.0);
    hue += hue < 0.0;
    hue *= 6.0;

    H_sec = (int) hue;

    switch (H_sec)
    {
      case 0:
        H_insec = hue - H_sec;
        Y_peak = weights[0] + H_insec * weights[1];
        chroma *= luma < Y_peak ? luma/Y_peak : (1. - luma)/(1. - Y_peak);
        X = chroma * H_insec;
        m = luma - (weights[0] * chroma + weights[1] * X);
        red = m + chroma; green = m + X; blue = m;
        break;
      case 1:
        H_insec = 1. - (hue - H_sec);
        Y_peak = weights[1] + H_insec * weights[0];
        chroma *= luma < Y_peak ? luma/Y_peak : (1. - luma)/(1. - Y_peak);
        X = chroma * H_insec;
        m = luma - (weights[0] * X + weights[1] * chroma);
        red = m + X; green = m + chroma; blue = m;
        break;
      case 2:
        H_insec = hue - H_sec;
        Y_peak = weights[1] + H_insec * weights[2];
        chroma *= luma < Y_peak ? luma/Y_peak : (1. - luma)/(1. - Y_peak);
        X = chroma * H_insec;
        m = luma - (weights[1] * chroma + weights[2] * X);
        red = m; green = m + chroma; blue = m + X;
        break;
      case 3:
        H_insec = 1. - (hue - H_sec);
        Y_peak = weights[2] + H_insec * weights[1];
        chroma *= luma < Y_peak ? luma/Y_peak : (1. - luma)/(1. - Y_peak);
        X = chroma * H_insec;
        m = luma - (weights[1] * X + weights[2] * chroma);
        red = m; green = m + X; blue = m + chroma;
        break;
      case 4:
        H_insec = hue - H_sec;
        Y_peak = weights[2] + H_insec * weights[0];
        chroma *= luma < Y_peak ? luma/Y_peak : (1. - luma)/(1. - Y_peak);
        X = chroma * H_insec;
        m = luma - (weights[0] * X + weights[2] * chroma);
        red = m + X; green = m; blue = m + chroma;
        break;
      default:
        H_insec = 1. - (hue - H_sec);
        Y_peak = weights[0] + H_insec * weights[2];
        chroma *= luma < Y_peak ? luma/Y_peak : (1. - luma)/(1. - Y_peak);
        X = chroma * H_insec;
        m = luma - (weights[0] * chroma + weights[2] * X);
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
  const Babl *space = babl_conversion_get_source_space (conversion);
  long n = samples;
  double weights[3];


  babl_space_get_rgb_luminance (space, &weights[0], &weights[1], &weights[2]);

  while (n--)
  {
    double alpha = ((double *) src)[3];

    rgba_to_hcy_step (src, dst, weights);

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
  const Babl *space = babl_conversion_get_source_space (conversion);
  long n = samples;
  double weights[3];

  space = babl_conversion_get_source_space (conversion);
  babl_space_get_rgb_luminance (space, &weights[0], &weights[1], &weights[2]);

  while (n--)
  {
    double alpha = ((double *) src)[3];

    hcy_to_rgba_step (src, dst, weights);

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
  const Babl *space = babl_conversion_get_source_space (conversion);
  long n = samples;
  double weights[3];

  space = babl_conversion_get_source_space (conversion);
  babl_space_get_rgb_luminance (space, &weights[0], &weights[1], &weights[2]);

  while (n--)
  {
    rgba_to_hcy_step (src, dst, weights);

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
  const Babl *space = babl_conversion_get_source_space (conversion);
  long n = samples;
  double weights[3];

  space = babl_conversion_get_source_space (conversion);
  babl_space_get_rgb_luminance (space, &weights[0], &weights[1], &weights[2]);

  while (n--)
  {
    hcy_to_rgba_step (src, dst, weights);

    ((double *) dst)[3] = 1.0;

    src += 3 * sizeof (double);
    dst += 4 * sizeof (double);
  }
}
