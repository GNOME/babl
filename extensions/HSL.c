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
          
int init (void);


int
init (void)
{
  babl_component_new ("hue", NULL);
  babl_component_new ("saturation", NULL);
  babl_component_new ("lightness", NULL);
  babl_component_new ("alpha", NULL);

  babl_model_new ("name", "HSLA",
                  babl_component ("hue"),
                  babl_component ("saturation"),
                  babl_component ("lightness"),
                  babl_component ("alpha"),
                  "alpha",
                  NULL);
  babl_model_new ("name", "HSL",
                  babl_component ("hue"),
                  babl_component ("saturation"),
                  babl_component ("lightness"),
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
  return 0;
}


static inline void
rgb_to_hsl_step (double* src,
                 double* dst)
{

  double min, max;
  double hue, saturation, lightness;
  int cpn_max;

  double red   = linear_to_gamma_2_2 (src[0]);
  double green = linear_to_gamma_2_2 (src[1]);
  double blue  = linear_to_gamma_2_2 (src[2]);

  max = MAX (red, MAX (green, blue));
  min = MIN (red, MIN (green, blue));

  if (max - red < EPSILON)
    cpn_max = 0;
  else if (max - green < EPSILON)
    cpn_max = 1;
  else
    cpn_max = 2;

  lightness = (max + min) / 2.0;

  if (max - min < EPSILON)
    {
      hue = saturation = 0;
    }
  else
    {
      double diff = max - min;
      double sum  = max + min;
      saturation = lightness > 0.5 ? diff / (2.0 - sum) : diff / sum;
      switch (cpn_max)
        {
        case 0: hue = (green - blue)  / diff + (green < blue ? 6.0 : 0.0); break;
        case 1: hue = (blue  - red)   / diff + 2.0; break;
        case 2: hue = (red   - green) / diff + 4.0; break;
        default: hue = 0.0;
          break;
        }
      hue /= 6.0;
    }

  dst[0] = hue;
  dst[1] = saturation;
  dst[2] = lightness;
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


static void
hsl_to_rgb_step (double *src,
                 double *dst)
{
  double hue        = src[0];
  double saturation = src[1];
  double lightness  = src[2];

  double red = 0, green = 0, blue = 0;

  if (saturation < 1e-7)
    {
      red = green = blue = lightness;
    }
  else
    {
      double q = lightness < 0.5 ?
        lightness * (1 + saturation) :
        lightness + saturation - lightness * saturation;

      double p = 2 * lightness - q;

      red   = hue2cpn (p, q, hue + 1.0/3.0);
      green = hue2cpn (p, q, hue);
      blue  = hue2cpn (p, q, hue - 1.0/3.0);
    }

  dst[0] = gamma_2_2_to_linear (red);
  dst[1] = gamma_2_2_to_linear (green);
  dst[2] = gamma_2_2_to_linear (blue);
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
