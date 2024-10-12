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

#define MIN(a,b) ((a > b) ? b : a)
#define MAX(a,b) ((a < b) ? b : a)
#define EPSILON  1.0e-10

static void  
rgba_to_hsla     (const Babl *conversion,
                  char       *src,
                  char       *dst,
                  long        samples);
static void  
hsla_to_rgba     (const Babl *conversion,
                  char       *src,
                  char       *dst,
                  long        samples);
static void  
rgba_to_hsl      (const Babl *conversion,
                  char       *src,
                  char       *dst,
                  long        samples);
static void  
hsl_to_rgba      (const Babl *conversion,
                  char       *src,
                  char       *dst,
                  long        samples);
                  
static void  
rgb_to_hsl_step  (double *src,
                  double *dst);
                               
static void  
hsl_to_rgb_step  (double *src,
                  double *dst);

static inline double
hue2cpn  (double  p,
          double  q,
          double  hue);

/* Defined through macros below */

static inline void
rgb_nonlinear_to_hsl_step_double (double      *src,
                                  double      *dst);
static inline void
hsl_to_rgb_nonlinear_step_double (double      *src,
                                  double      *dst);

static inline void
rgb_nonlinear_to_hsl_step_float  (float      *src,
                                  float      *dst);
static inline void
hsl_to_rgb_nonlinear_step_float  (float      *src,
                                  float      *dst);

/* Non-Linear RGB conversion: double variants */

static void
rgba_nonlinear_to_hsla           (const Babl *conversion,
                                  char       *src,
                                  char       *dst,
                                  long        samples);
static void
hsla_to_rgba_nonlinear           (const Babl *conversion,
                                  char       *src,
                                  char       *dst,
                                  long        samples);

/* Non-Linear RGB conversion: float variants */

static void
rgba_nonlinear_to_hsla_float     (const Babl *conversion,
                                  char       *src,
                                  char       *dst,
                                  long        samples);
static void
hsla_to_rgba_nonlinear_float     (const Babl *conversion,
                                  char       *src,
                                  char       *dst,
                                  long        samples);


int init (void);

int
init (void)
{
  babl_component_new ("hue", NULL);
  babl_component_new ("saturation", NULL);
  babl_component_new ("lightness", NULL);
  babl_component_new ("alpha", "alpha", NULL);

  babl_model_new ("name", "HSL",
                  "doc", "HSL - Hue Saturation Lightness, an improvement over HSV; which uses lightness; defined as (MAX(R,G,B) + MIN(R,G,B))/2 for the grayscale axis; better than HSV, but look into the CIE based spaces for better perceptual uniformity. The HSL space is relative to the RGB space associated with the format.",
                  babl_component ("hue"),
                  babl_component ("saturation"),
                  babl_component ("lightness"),
                  NULL);
  babl_model_new ("name", "HSLA",
                  "doc", "HSL - with separate alpha component.",
                  babl_component ("hue"),
                  babl_component ("saturation"),
                  babl_component ("lightness"),
                  babl_component ("alpha"),
                  "alpha",
                  NULL);


  babl_conversion_new (babl_model ("RGBA"),
                       babl_model ("HSLA"),
                       "linear", rgba_to_hsla,
                       NULL);
  babl_conversion_new (babl_model ("RGBA"),
                       babl_model ("HSL"),
                       "linear", rgba_to_hsl,
                       NULL);
  babl_conversion_new (babl_model ("HSLA"),
                       babl_model ("RGBA"),
                       "linear", hsla_to_rgba,
                       NULL);
  babl_conversion_new (babl_model ("HSL"),
                       babl_model ("RGBA"),
                       "linear", hsl_to_rgba,
                       NULL);

  babl_conversion_new (babl_model ("R'G'B'A"),
                       babl_model ("HSLA"),
                       "linear", rgba_nonlinear_to_hsla,
                       NULL);

  babl_conversion_new (babl_model ("HSLA"),
                       babl_model ("R'G'B'A"),
                       "linear", hsla_to_rgba_nonlinear,
                       NULL);

  babl_format_new ("name", "HSLA float",
                   babl_model ("HSLA"),
                   babl_type ("float"),
                   babl_component ("hue"),
                   babl_component ("saturation"),
                   babl_component ("lightness"),
                   babl_component ("alpha"),
                   NULL);
  babl_format_new ("name", "HSL float",
                   babl_model ("HSL"),
                   babl_type ("float"),
                   babl_component ("hue"),
                   babl_component ("saturation"),
                   babl_component ("lightness"),
                   NULL);

  babl_conversion_new (babl_format ("R'G'B'A float"),
                       babl_format ("HSLA float"),
                       "linear", rgba_nonlinear_to_hsla_float,
                       NULL);
  babl_conversion_new (babl_format ("HSLA float"),
                       babl_format ("R'G'B'A float"),
                       "linear", hsla_to_rgba_nonlinear_float,
                       NULL);

  return 0;
}

#define DEFINE_RGB_NL_TO_HSL_STEP(ctype) \
static inline void \
rgb_nonlinear_to_hsl_step_##ctype (ctype* src, \
                                   ctype* dst) \
{ \
  ctype min, max; \
  ctype hue, saturation, lightness; \
  int cpn_max; \
 \
  ctype red   = src[0]; \
  ctype green = src[1]; \
  ctype blue  = src[2]; \
 \
  max = MAX (red, MAX (green, blue)); \
  min = MIN (red, MIN (green, blue)); \
 \
  if (max - red < EPSILON) \
    cpn_max = 0; \
  else if (max - green < EPSILON) \
    cpn_max = 1; \
  else \
    cpn_max = 2; \
 \
  lightness = (max + min) / 2.0; \
 \
  if (max - min < EPSILON) \
    { \
      hue = saturation = 0; \
    } \
  else \
    { \
      ctype diff = max - min; \
      ctype sum  = max + min; \
      saturation = lightness > 0.5 ? diff / (2.0 - sum) : diff / sum; \
      switch (cpn_max) \
        { \
        case 0: hue = (green - blue)  / diff + (green < blue ? 6.0 : 0.0); break; \
        case 1: hue = (blue  - red)   / diff + 2.0; break; \
        case 2: hue = (red   - green) / diff + 4.0; break; \
        default: hue = 0.0; \
                 break; \
        } \
      hue /= 6.0; \
    } \
 \
  dst[0] = hue; \
  dst[1] = saturation; \
  dst[2] = lightness; \
}

DEFINE_RGB_NL_TO_HSL_STEP(double)
DEFINE_RGB_NL_TO_HSL_STEP(float)

static inline void
rgb_to_hsl_step (double* src,
                 double* dst)
{
  double nonlinear_rgb[3];

  nonlinear_rgb[0] = linear_to_gamma_2_2 (src[0]);
  nonlinear_rgb[1] = linear_to_gamma_2_2 (src[1]);
  nonlinear_rgb[2] = linear_to_gamma_2_2 (src[2]);

  rgb_nonlinear_to_hsl_step_double (nonlinear_rgb, dst);
}


static void
rgba_to_hsla (const Babl *conversion,
              char       *src,
              char       *dst,
              long        samples)
{
  long n = samples;

  while (n--)
    {
      double alpha = ((double *) src)[3];

      rgb_to_hsl_step ((double *) src, (double *) dst);

      ((double *) dst)[3] = alpha;

      src += 4 * sizeof (double);
      dst += 4 * sizeof (double);
    }
}


static void
rgba_to_hsl (const Babl *conversion,
             char       *src,
             char       *dst,
             long        samples)
{
  long n = samples;

  while (n--)
    {
      rgb_to_hsl_step ((double *) src, (double *) dst);

      src += 4 * sizeof (double);
      dst += 3 * sizeof (double);
    }
}


static inline double
hue2cpn (double p,
         double q, 
         double hue)
{
  if (hue < 0.0) hue += 1.0;
  if (hue > 1.0) hue -= 1.0;
  if (hue < 1.0 / 6.0) return p + (q - p) * 6.0 * hue;
  if (hue < 1.0 / 2.0) return q;
  if (hue < 2.0 / 3.0) return p + (q - p) * (2.0 / 3.0 - hue) * 6.0;
  return p;
}


#define DEFINE_HSL_TO_RBG_NONLINEAR_STEP(ctype) \
static inline void \
hsl_to_rgb_nonlinear_step_##ctype (ctype *src, \
                                   ctype *dst) \
{ \
  ctype hue        = src[0]; \
  ctype saturation = src[1]; \
  ctype lightness  = src[2]; \
 \
  if (saturation < 1e-7) \
    { \
      dst[0] = dst[1] = dst[2] = lightness; \
    } \
  else \
    { \
      ctype q = lightness < 0.5 ? \
        lightness * (1 + saturation) : \
        lightness + saturation - lightness * saturation; \
 \
      ctype p = 2 * lightness - q; \
 \
      hue  = fmod (hue, 1.0); \
      hue += hue < 0.0; \
 \
      dst[0] = hue2cpn (p, q, hue + 1.0/3.0); \
      dst[1] = hue2cpn (p, q, hue); \
      dst[2] = hue2cpn (p, q, hue - 1.0/3.0); \
    } \
}

DEFINE_HSL_TO_RBG_NONLINEAR_STEP(double)
DEFINE_HSL_TO_RBG_NONLINEAR_STEP(float)

static void
hsl_to_rgb_step (double *src,
                 double *dst)
{
  hsl_to_rgb_nonlinear_step_double (src, dst);

  dst[0] = gamma_2_2_to_linear (dst[0]);
  dst[1] = gamma_2_2_to_linear (dst[1]);
  dst[2] = gamma_2_2_to_linear (dst[2]);
}


static void
hsla_to_rgba (const Babl *conversion,
              char       *src,
              char       *dst,
              long        samples)
{
  long n = samples;

  while (n--)
    {
      double alpha = ((double *) src)[3];

      hsl_to_rgb_step ((double *) src, (double *) dst);

      ((double *) dst)[3] = alpha;

      src += 4 * sizeof (double);
      dst += 4 * sizeof (double);
    }
}


static void
hsl_to_rgba (const Babl *conversion,
             char       *src,
             char       *dst,
             long        samples)
{
  long n = samples;

  while (n--)
    {
      hsl_to_rgb_step ((double *) src, (double *) dst);

      ((double *) dst)[3] = 1.0;

      src += 3 * sizeof (double);
      dst += 4 * sizeof (double);
    }
}

static void
rgba_nonlinear_to_hsla (const Babl *conversion,
                        char       *src,
                        char       *dst,
                        long        samples)
{
  long n = samples;

  while (n--)
    {
      double alpha = ((double *) src)[3];

      rgb_nonlinear_to_hsl_step_double ((double *) src, (double *) dst);

      ((double *) dst)[3] = alpha;

      src += 4 * sizeof (double);
      dst += 4 * sizeof (double);
    }
}

static void
hsla_to_rgba_nonlinear (const Babl *conversion,
                        char       *src,
                        char       *dst,
                        long        samples)
{
  long n = samples;

  while (n--)
    {
      double alpha = ((double *) src)[3];

      hsl_to_rgb_nonlinear_step_double ((double *) src, (double *) dst);

      ((double *) dst)[3] = alpha;

      src += 4 * sizeof (double);
      dst += 4 * sizeof (double);
    }
}

/** Float variants **/

static void
rgba_nonlinear_to_hsla_float (const Babl *conversion,
                              char       *src,
                              char       *dst,
                              long        samples)
{
  long n = samples;

  while (n--)
    {
      float alpha = ((float *) src)[3];

      rgb_nonlinear_to_hsl_step_float ((float *) src, (float *) dst);

      ((float *) dst)[3] = alpha;

      src += 4 * sizeof (float);
      dst += 4 * sizeof (float);
    }
}

static void
hsla_to_rgba_nonlinear_float (const Babl *conversion,
                              char       *src,
                              char       *dst,
                              long        samples)
{
  long n = samples;

  while (n--)
    {
      float alpha = ((float *) src)[3];

      hsl_to_rgb_nonlinear_step_float ((float *) src, (float *) dst);

      ((float *) dst)[3] = alpha;

      src += 4 * sizeof (float);
      dst += 4 * sizeof (float);
    }
}
