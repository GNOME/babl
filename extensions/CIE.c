/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2005, 2014 Øyvind Kolås.
 * Copyright (C) 2009, Martin Nordholts
 * Copyright (C) 2014, Elle Stone
 * Copyright (C) 2017, 2018 Red Hat, Inc.
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
#include <stdint.h>
#include <string.h>

#if defined(USE_SSE2)
#include <emmintrin.h>
#endif /* defined(USE_SSE2) */

#include "babl-internal.h"
#include "extensions/util.h"

#define DEGREES_PER_RADIAN (180 / 3.14159265358979323846)
#define RADIANS_PER_DEGREE (1 / DEGREES_PER_RADIAN)

#define LAB_EPSILON       (216.0f / 24389.0f)
#define LAB_KAPPA         (24389.0f / 27.0f)

/* The constants below hard-code the D50-adapted sRGB ICC profile
 * reference white, aka the ICC profile D50 illuminant.
 *
 * In a properly ICC profile color-managed application, the profile
 * illuminant values should be retrieved from the image's
 * ICC profile's illuminant.
 *
 * At present, the ICC profile illuminant is always D50. This might
 * change when the next version of the ICC specs is released.
 *
 * As encoded in an actual V2 or V4 ICC profile,
 * the illuminant values are hexadecimal-rounded, as are the following
 * hard-coded D50 ICC profile illuminant values:
 */

#define D50_WHITE_REF_X   0.964202880f
#define D50_WHITE_REF_Y   1.000000000f
#define D50_WHITE_REF_Z   0.824905400f

#define NEAR_ZERO         0.0000000001f
#define D50_WHITE_REF_x   0.345702921222f
#define D50_WHITE_REF_y   0.358537532290f


static void types (void);
static void components (void);
static void models (void);
static void conversions (void);
static void formats (void);

int init (void);

int
init (void)
{
  types ();
  components ();
  models ();
  formats ();
  conversions ();
  return 0;
}

static void
components (void)
{
  babl_component_new ("CIE L", NULL);
  babl_component_new ("CIE a", "chroma", NULL);
  babl_component_new ("CIE b", "chroma", NULL);
  babl_component_new ("CIE C(ab)", "chroma", NULL);
  babl_component_new ("CIE H(ab)", "chroma", NULL);
  babl_component_new ("CIE X", NULL);
  babl_component_new ("CIE Y", NULL);
  babl_component_new ("CIE Z", NULL);
  babl_component_new ("CIE x", NULL);
  babl_component_new ("CIE y", NULL);
/*  babl_component_new ("CIE z", NULL);*/
}

static void
models (void)
{
  babl_model_new (
    "name", "CIE Lab",
    babl_component ("CIE L"),
    babl_component ("CIE a"),
    babl_component ("CIE b"),
    "CIE",
    NULL);

  babl_model_new (
    "name", "CIE Lab alpha",
    babl_component ("CIE L"),
    babl_component ("CIE a"),
    babl_component ("CIE b"),
    babl_component ("A"),
    "CIE",
    "alpha",
    NULL);

  babl_model_new (
    "name", "CIE LCH(ab)",
    babl_component ("CIE L"),
    babl_component ("CIE C(ab)"),
    babl_component ("CIE H(ab)"),
    "CIE",
    NULL);

  babl_model_new (
    "name", "CIE LCH(ab) alpha",
    babl_component ("CIE L"),
    babl_component ("CIE C(ab)"),
    babl_component ("CIE H(ab)"),
    babl_component ("A"),
    "CIE",
    "alpha",
    NULL);

  babl_model_new (
    "name", "CIE XYZ",
    babl_component ("CIE X"),
    babl_component ("CIE Y"),
    babl_component ("CIE Z"),
    "CIE",
    NULL);

  babl_model_new (
    "name", "CIE XYZ alpha",
    babl_component ("CIE X"),
    babl_component ("CIE Y"),
    babl_component ("CIE Z"),
    babl_component ("A"),
    "CIE",
    "alpha",
    NULL);

  babl_model_new (
    "name", "CIE xyY",
    babl_component ("CIE x"),
    babl_component ("CIE y"),
    babl_component ("CIE Y"),
    "CIE",
    NULL);

  babl_model_new (
    "name", "CIE xyY alpha",
    babl_component ("CIE x"),
    babl_component ("CIE y"),
    babl_component ("CIE Y"),
    babl_component ("A"),
    "CIE",
    "alpha",
    NULL);
}

static void  rgbcie_init (void);

/******** begin double RGB/CIE color space conversions ****************/

static inline void  ab_to_CHab    (double  a,
                                   double  b,
                                   double *to_C,
                                   double *to_H);

static inline void  CHab_to_ab    (double  C,
                                   double  H,
                                   double *to_a,
                                   double *to_b);

static inline void XYZ_to_LAB     (double  X,
                                   double  Y,
                                   double  Z,
                                   double *to_L,
                                   double *to_a,
                                   double *to_b
                                   );

static inline void LAB_to_XYZ     (double  L,
                                   double  a,
                                   double  b,
                                   double *to_X,
                                   double *to_Y,
                                   double *to_Z
                                   );

static inline void XYZ_to_xyY     (double  X,
                                   double  Y,
                                   double  Z,
                                   double *to_x,
                                   double *to_y,
                                   double *to_Y
                                   );

static inline void xyY_to_XYZ     (double  x,
                                   double  y,
                                   double  Y,
                                   double *to_X,
                                   double *to_Y,
                                   double *to_Z
                                   );

static inline void
XYZ_to_LAB (double  X,
            double  Y,
            double  Z,
            double *to_L,
            double *to_a,
            double *to_b)
{
  double xr = X / D50_WHITE_REF_X;
  double yr = Y / D50_WHITE_REF_Y;
  double zr = Z / D50_WHITE_REF_Z;

  double fx = xr > LAB_EPSILON ? cbrt (xr) : (LAB_KAPPA * xr + 16.0) / 116.0;
  double fy = yr > LAB_EPSILON ? cbrt (yr) : (LAB_KAPPA * yr + 16.0) / 116.0;
  double fz = zr > LAB_EPSILON ? cbrt (zr) : (LAB_KAPPA * zr + 16.0) / 116.0;

  *to_L = 116.0 * fy - 16.0;
  *to_a = 500.0 * (fx - fy);
  *to_b = 200.0 * (fy - fz);
}

static inline void
LAB_to_XYZ (double  L,
            double  a,
            double  b,
            double *to_X,
            double *to_Y,
            double *to_Z)
{
  double fy = (L + 16.0) / 116.0;
  double fy_cubed = fy * fy * fy;

  double fx = fy + a / 500.0;
  double fx_cubed = fx * fx * fx;

  double fz = fy - b / 200.0;
  double fz_cubed = fz * fz * fz;

  double yr = L > LAB_KAPPA * LAB_EPSILON ? fy_cubed : L / LAB_KAPPA;
  double xr = fx_cubed > LAB_EPSILON ? fx_cubed : (fx * 116.0 - 16.0) / LAB_KAPPA;
  double zr = fz_cubed > LAB_EPSILON ? fz_cubed : (fz * 116.0 - 16.0) / LAB_KAPPA;

  *to_X = xr * D50_WHITE_REF_X;
  *to_Y = yr * D50_WHITE_REF_Y;
  *to_Z = zr * D50_WHITE_REF_Z;
}


static inline void
XYZ_to_xyY (double  X,
            double  Y,
            double  Z,
            double *to_x,
            double *to_y,
            double *to_Y)
{
   double sum = X + Y + Z;
   if (sum < NEAR_ZERO)
	{ *to_Y = 0.0;
	  *to_x = D50_WHITE_REF_x;
	  *to_y = D50_WHITE_REF_y;
	}
	else 
	{
	*to_x = X / sum;
	*to_y = Y / sum;
	*to_Y = Y;
    }
}

static inline void
xyY_to_XYZ (double  x,
            double  y,
            double  Y,
            double *to_X,
            double *to_Y,
            double *to_Z)
{
   if ( Y < NEAR_ZERO ) 
	{ *to_X = 0.0;
	  *to_Y = 0.0;
	  *to_Z = 0.0;
	}
	else 
	{
	*to_X = (x * Y) / y;
	*to_Y = Y;
	*to_Z = ((1 - x - y) * Y) / y;
    }
}


/* rgb <-> XYZ */

static void
rgba_to_xyz (const Babl *conversion,
             char       *src,
             char       *dst,
             long        n)
{
  const Babl *space = babl_conversion_get_source_space (conversion);
  while (n--)
    {
      double RGB[3]  = {((double *) src)[0],
                        ((double *) src)[1],
                        ((double *) src)[2] };
      babl_space_to_xyz (space, RGB, (double*)dst);

      src += sizeof (double) * 4;
      dst += sizeof (double) * 3;
    }
}

static void
xyz_to_rgba (const Babl *conversion,
             char       *src,
             char       *dst,
             long        n)
{
  const Babl *space = babl_conversion_get_destination_space (conversion);
  while (n--)
    {
      babl_space_from_xyz (space, (double*)src, (double*) dst);
      ((double *) dst)[3] = 1.0;
      src += sizeof (double) * 3;
      dst += sizeof (double) * 4;
    }
}

static void
rgba_to_xyza (const Babl *conversion,char *src,
              char *dst,
              long  n)
{
  const Babl *space = babl_conversion_get_source_space (conversion);
  while (n--)
    {
      babl_space_to_xyz (space, (double*)src, (double*)dst);
      ((double *) dst)[3] = ((double *) src)[3];

      src += sizeof (double) * 4;
      dst += sizeof (double) * 4;
    }
}

static void
xyza_to_rgba (const Babl *conversion,char *src,
              char *dst,
              long  n)
{
  const Babl *space = babl_conversion_get_destination_space (conversion);
  while (n--)
    {
      babl_space_from_xyz (space, (double*)src, (double*) dst);
      ((double *) dst)[3] = ((double *) src)[3];

      src += sizeof (double) * 4;
      dst += sizeof (double) * 4;
    }
}


/* rgb -> xyY */

static void
rgba_to_xyY (const Babl *conversion,
             char       *src,
             char       *dst,
             long        n)
{
  const Babl *space = babl_conversion_get_source_space (conversion);
  while (n--)
    {
      double XYZ[3], x, y, Y;

      babl_space_to_xyz (space, (double*)src, XYZ);
      XYZ_to_xyY (XYZ[0], XYZ[1], XYZ[2], &x, &y, &Y);

      ((double *) dst)[0] = x;
      ((double *) dst)[1] = y;
      ((double *) dst)[2] = Y;

      src += sizeof (double) * 4;
      dst += sizeof (double) * 3;
    }
}

static void
rgba_to_xyYa (const Babl *conversion,char *src,
              char *dst,
              long  n)
{
  const Babl *space = babl_conversion_get_source_space (conversion);
  while (n--)
    {
      double alpha = ((double *) src)[3];
      double XYZ[3], x, y, Y;

      //convert RGB to XYZ
      babl_space_to_xyz (space, (double*)src, XYZ);

      //convert XYZ to xyY
      XYZ_to_xyY (XYZ[0], XYZ[1], XYZ[2], &x, &y, &Y);

      ((double *) dst)[0] = x;
      ((double *) dst)[1] = y;
      ((double *) dst)[2] = Y;
      ((double *) dst)[3] = alpha;

      src += sizeof (double) * 4;
      dst += sizeof (double) * 4;
    }
}

static void
rgbaf_to_xyYaf (const Babl *conversion,
                float *src,
                float *dst,
                long   samples)
{
  const Babl *space = babl_conversion_get_source_space (conversion);
  float m_0_0 = space->space.RGBtoXYZf[0] / D50_WHITE_REF_X;
  float m_0_1 = space->space.RGBtoXYZf[1] / D50_WHITE_REF_X;
  float m_0_2 = space->space.RGBtoXYZf[2] / D50_WHITE_REF_X;
  float m_1_0 = space->space.RGBtoXYZf[3] / D50_WHITE_REF_Y;
  float m_1_1 = space->space.RGBtoXYZf[4] / D50_WHITE_REF_Y;
  float m_1_2 = space->space.RGBtoXYZf[5] / D50_WHITE_REF_Y;
  float m_2_0 = space->space.RGBtoXYZf[6] / D50_WHITE_REF_Z;
  float m_2_1 = space->space.RGBtoXYZf[7] / D50_WHITE_REF_Z;
  float m_2_2 = space->space.RGBtoXYZf[8] / D50_WHITE_REF_Z;
  long n = samples;

  while (n--)
    {
      float x, y, X, Y, Z, r, g, b, a;
      r = src[0];
      g = src[1];
      b = src[2];
      a = src[3];

      if ( r < NEAR_ZERO && g < NEAR_ZERO && b < NEAR_ZERO )
        {
          Y = 0.0f;
          x = D50_WHITE_REF_x;
          y = D50_WHITE_REF_y;
        }
      else
        {
          X = m_0_0 * r + m_0_1 * g + m_0_2 * b;
          Y = m_1_0 * r + m_1_1 * g + m_1_2 * b;
          Z = m_2_0 * r + m_2_1 * g + m_2_2 * b;

          x = X / (X + Y + Z);
          y = Y / (X + Y + Z);
        }

      dst[0] = x;
      dst[1] = y;
      dst[2] = Y;
      dst[3] = a;

      src += 4;
      dst += 4;
    }
}

static void
rgbf_to_xyYf (const Babl *conversion,float *src,
              float *dst,
              long   samples)
{
  const Babl *space = babl_conversion_get_source_space (conversion);
  float m_0_0 = space->space.RGBtoXYZf[0] / D50_WHITE_REF_X;
  float m_0_1 = space->space.RGBtoXYZf[1] / D50_WHITE_REF_X;
  float m_0_2 = space->space.RGBtoXYZf[2] / D50_WHITE_REF_X;
  float m_1_0 = space->space.RGBtoXYZf[3] / D50_WHITE_REF_Y;
  float m_1_1 = space->space.RGBtoXYZf[4] / D50_WHITE_REF_Y;
  float m_1_2 = space->space.RGBtoXYZf[5] / D50_WHITE_REF_Y;
  float m_2_0 = space->space.RGBtoXYZf[6] / D50_WHITE_REF_Z;
  float m_2_1 = space->space.RGBtoXYZf[7] / D50_WHITE_REF_Z;
  float m_2_2 = space->space.RGBtoXYZf[8] / D50_WHITE_REF_Z;
  long n = samples;

  while (n--)
    {
  float x, y, X, Y, Z, r, g, b;
      r = src[0];
      g = src[1];
      b = src[2];

      if ( r < NEAR_ZERO && g < NEAR_ZERO && b < NEAR_ZERO )
        {
          Y = 0.0f;
          x = D50_WHITE_REF_x;
          y = D50_WHITE_REF_y;
        }
      else
        {
          X = m_0_0 * r + m_0_1 * g + m_0_2 * b;
          Y = m_1_0 * r + m_1_1 * g + m_1_2 * b;
          Z = m_2_0 * r + m_2_1 * g + m_2_2 * b;

          x = X / (X + Y + Z);
          y = Y / (X + Y + Z);
        }

      dst[0] = x;
      dst[1] = y;
      dst[2] = Y;

      src += 3;
      dst += 3;
    }
}


static void
rgbaf_to_xyYf (const Babl *conversion,
               float      *src,
               float      *dst,
               long        samples)
{
  const Babl *space = babl_conversion_get_source_space (conversion);
  float m_0_0 = space->space.RGBtoXYZf[0] / D50_WHITE_REF_X;
  float m_0_1 = space->space.RGBtoXYZf[1] / D50_WHITE_REF_X;
  float m_0_2 = space->space.RGBtoXYZf[2] / D50_WHITE_REF_X;
  float m_1_0 = space->space.RGBtoXYZf[3] / D50_WHITE_REF_Y;
  float m_1_1 = space->space.RGBtoXYZf[4] / D50_WHITE_REF_Y;
  float m_1_2 = space->space.RGBtoXYZf[5] / D50_WHITE_REF_Y;
  float m_2_0 = space->space.RGBtoXYZf[6] / D50_WHITE_REF_Z;
  float m_2_1 = space->space.RGBtoXYZf[7] / D50_WHITE_REF_Z;
  float m_2_2 = space->space.RGBtoXYZf[8] / D50_WHITE_REF_Z;
  long n = samples;

  while (n--)
    {
  float x, y, X, Y, Z, r, g, b;
      r = src[0];
      g = src[1];
      b = src[2];

      if ( r < NEAR_ZERO && g < NEAR_ZERO && b < NEAR_ZERO )
        {
          Y = 0.0f;
          x = D50_WHITE_REF_x;
          y = D50_WHITE_REF_y;
        }
      else
        {
          X = m_0_0 * r + m_0_1 * g + m_0_2 * b;
          Y = m_1_0 * r + m_1_1 * g + m_1_2 * b;
          Z = m_2_0 * r + m_2_1 * g + m_2_2 * b;

          x = X / (X + Y + Z);
          y = Y / (X + Y + Z);
        }

      dst[0] = x;
      dst[1] = y;
      dst[2] = Y;

      src += 4;
      dst += 3;
    }
}


/* xyY -> rgb */

static void
xyY_to_rgba (const Babl *conversion,
             char       *src,
             char       *dst,
             long        n)
{
  const Babl *space = babl_conversion_get_destination_space (conversion);
  while (n--)
    {
      double x = ((double *) src)[0];
      double y = ((double *) src)[1];
      double Y = ((double *) src)[2];

      double R, G, B, X, Z;

      //convert xyY to XYZ
      xyY_to_XYZ (x, y, Y, &X, &Y, &Z);

      //convert XYZ to RGB
      {
        double XYZ[3]  = {X,Y,Z};
        double RGB[3];
        babl_space_from_xyz (space, XYZ, RGB);
        R = RGB[0];
        G = RGB[1];
        B = RGB[2];
      }

      ((double *) dst)[0] = R;
      ((double *) dst)[1] = G;
      ((double *) dst)[2] = B;
      ((double *) dst)[3] = 1.0;

      src += sizeof (double) * 3;
      dst += sizeof (double) * 4;
    }
}


static void
xyYa_to_rgba (const Babl *conversion,char *src,
              char *dst,
              long  n)
{
  const Babl *space = babl_conversion_get_destination_space (conversion);
  while (n--)
    {
      double x     = ((double *) src)[0];
      double y     = ((double *) src)[1];
      double Y     = ((double *) src)[2];
      double alpha = ((double *) src)[3];

      double X, Z;

      //convert xyY to XYZ
      xyY_to_XYZ (x, y, Y, &X, &Y, &Z);

      {
        //convert XYZ to RGB
        double XYZ[3]  = {X,Y,Z};
        babl_space_from_xyz (space, XYZ, (double*)dst);
      }
      ((double *) dst)[3] = alpha;

      src += sizeof (double) * 4;
      dst += sizeof (double) * 4;
    }
}


static void
xyYf_to_rgbf (const Babl *conversion,float *src,
                float *dst,
                long   samples)
{
  const Babl *space = babl_conversion_get_source_space (conversion);
  float m_0_0 = space->space.XYZtoRGBf[0] * D50_WHITE_REF_X;
  float m_0_1 = space->space.XYZtoRGBf[1] * D50_WHITE_REF_Y;
  float m_0_2 = space->space.XYZtoRGBf[2] * D50_WHITE_REF_Z;
  float m_1_0 = space->space.XYZtoRGBf[3] * D50_WHITE_REF_X;
  float m_1_1 = space->space.XYZtoRGBf[4] * D50_WHITE_REF_Y;
  float m_1_2 = space->space.XYZtoRGBf[5] * D50_WHITE_REF_Z;
  float m_2_0 = space->space.XYZtoRGBf[6] * D50_WHITE_REF_X;
  float m_2_1 = space->space.XYZtoRGBf[7] * D50_WHITE_REF_Y;
  float m_2_2 = space->space.XYZtoRGBf[8] * D50_WHITE_REF_Z;
  long n = samples;

  while (n--)
    {
      float X, Z, r, g, b;
      float x = src[0];
      float y = src[1];
      float Y = src[2];

      if ( Y < NEAR_ZERO )
        {
          X = 0.0f;
          Y = 0.0f;
          Z = 0.0f;
        }
      else
        {
          X = (x * Y) / y;
          Y = Y;
          Z = ((1 - x - y) * Y) / y;
        }

      r = m_0_0 * X + m_0_1 * Y + m_0_2 * Z;
      g = m_1_0 * X + m_1_1 * Y + m_1_2 * Z;
      b = m_2_0 * X + m_2_1 * Y + m_2_2 * Z;

      dst[0] = r;
      dst[1] = g;
      dst[2] = b;

      src += 3;
      dst += 3;
    }
}



static void
xyYf_to_rgbaf (const Babl *conversion,
               float      *src,
               float      *dst,
               long        samples)
{
  const Babl *space = babl_conversion_get_source_space (conversion);
  float m_0_0 = space->space.XYZtoRGBf[0] * D50_WHITE_REF_X;
  float m_0_1 = space->space.XYZtoRGBf[1] * D50_WHITE_REF_Y;
  float m_0_2 = space->space.XYZtoRGBf[2] * D50_WHITE_REF_Z;
  float m_1_0 = space->space.XYZtoRGBf[3] * D50_WHITE_REF_X;
  float m_1_1 = space->space.XYZtoRGBf[4] * D50_WHITE_REF_Y;
  float m_1_2 = space->space.XYZtoRGBf[5] * D50_WHITE_REF_Z;
  float m_2_0 = space->space.XYZtoRGBf[6] * D50_WHITE_REF_X;
  float m_2_1 = space->space.XYZtoRGBf[7] * D50_WHITE_REF_Y;
  float m_2_2 = space->space.XYZtoRGBf[8] * D50_WHITE_REF_Z;
  long n = samples;

  while (n--)
    {
      float X, Z, r, g, b;
      float x = src[0];
      float y = src[1];
      float Y = src[2];


      if ( Y < NEAR_ZERO )
        {
          X = 0.0f;
          Y = 0.0f;
          Z = 0.0f;
        }
      else
        {
          X = (x * Y) / y;
          Y = Y;
          Z = ((1 - x - y) * Y) / y;
        }

      r = m_0_0 * X + m_0_1 * Y + m_0_2 * Z;
      g = m_1_0 * X + m_1_1 * Y + m_1_2 * Z;
      b = m_2_0 * X + m_2_1 * Y + m_2_2 * Z;

      dst[0] =    r;
      dst[1] =    g;
      dst[2] =    b;
      dst[3] = 1.0f;

      src += 3;
      dst += 4;
    }
}

static void
xyYaf_to_rgbaf (const Babl *conversion,
                float      *src,
                float      *dst,
                long        samples)
{
  const Babl *space = babl_conversion_get_source_space (conversion);
  float m_0_0 = space->space.XYZtoRGBf[0] * D50_WHITE_REF_X;
  float m_0_1 = space->space.XYZtoRGBf[1] * D50_WHITE_REF_Y;
  float m_0_2 = space->space.XYZtoRGBf[2] * D50_WHITE_REF_Z;
  float m_1_0 = space->space.XYZtoRGBf[3] * D50_WHITE_REF_X;
  float m_1_1 = space->space.XYZtoRGBf[4] * D50_WHITE_REF_Y;
  float m_1_2 = space->space.XYZtoRGBf[5] * D50_WHITE_REF_Z;
  float m_2_0 = space->space.XYZtoRGBf[6] * D50_WHITE_REF_X;
  float m_2_1 = space->space.XYZtoRGBf[7] * D50_WHITE_REF_Y;
  float m_2_2 = space->space.XYZtoRGBf[8] * D50_WHITE_REF_Z;
  long n = samples;

  while (n--)
    {
      float X, Z, r, g, b;
      float x = src[0];
      float y = src[1];
      float Y = src[2];
      float a = src[3];

      if ( Y < NEAR_ZERO )
        {
          X = 0.0f;
          Y = 0.0f;
          Z = 0.0f;
        }
      else
        {
          X = (x * Y) / y;
          Y = Y;
          Z = ((1 - x - y) * Y) / y;
        }

      r = m_0_0 * X + m_0_1 * Y + m_0_2 * Z;
      g = m_1_0 * X + m_1_1 * Y + m_1_2 * Z;
      b = m_2_0 * X + m_2_1 * Y + m_2_2 * Z;

      dst[0] = r;
      dst[1] = g;
      dst[2] = b;
      dst[3] = a;

      src += 4;
      dst += 4;
    }
}


/* rgb <-> LAB */

static void
rgba_to_lab (const Babl *conversion,
             char       *src,
             char       *dst,
             long        n)
{
  const Babl *space = babl_conversion_get_source_space (conversion);
  while (n--)
    {
      double XYZ[3], L, a, b;

      babl_space_to_xyz (space, (double*)src, XYZ);
      XYZ_to_LAB (XYZ[0], XYZ[1], XYZ[2], &L, &a, &b);

      ((double *) dst)[0] = L;
      ((double *) dst)[1] = a;
      ((double *) dst)[2] = b;

      src += sizeof (double) * 4;
      dst += sizeof (double) * 3;
    }
}


static void
lab_to_rgba (const Babl *conversion,
             char       *src,
             char       *dst,
             long        n)
{
  const Babl *space = babl_conversion_get_destination_space (conversion);
  while (n--)
    {
      double L = ((double *) src)[0];
      double a = ((double *) src)[1];
      double b = ((double *) src)[2];

      double X, Y, Z, R, G, B;

      //convert Lab to XYZ
      LAB_to_XYZ (L, a, b, &X, &Y, &Z);

      //convert XYZ to RGB
      {
        double XYZ[3]  = {X,Y,Z};
        double RGB[3];
        babl_space_from_xyz (space, XYZ, RGB);
        R = RGB[0];
        G = RGB[1];
        B = RGB[2];
      }

      ((double *) dst)[0] = R;
      ((double *) dst)[1] = G;
      ((double *) dst)[2] = B;
      ((double *) dst)[3] = 1.0;

      src += sizeof (double) * 3;
      dst += sizeof (double) * 4;
    }
}

static void
rgba_to_laba (const Babl *conversion,
              char       *src,
              char       *dst,
              long        n)
{
  const Babl *space = babl_conversion_get_source_space (conversion);
  while (n--)
    {
      double alpha = ((double *) src)[3];
      double XYZ[3], L, a, b;

      //convert RGB to XYZ
      babl_space_to_xyz (space, (double*)src, XYZ);

      //convert XYZ to Lab
      XYZ_to_LAB (XYZ[0], XYZ[1], XYZ[2], &L, &a, &b);

      ((double *) dst)[0] = L;
      ((double *) dst)[1] = a;
      ((double *) dst)[2] = b;
      ((double *) dst)[3] = alpha;

      src += sizeof (double) * 4;
      dst += sizeof (double) * 4;
    }
}

static void
laba_to_rgba (const Babl *conversion,
              char       *src,
              char       *dst,
              long        n)
{
  const Babl *space = babl_conversion_get_destination_space (conversion);
  while (n--)
    {
      double L     = ((double *) src)[0];
      double a     = ((double *) src)[1];
      double b     = ((double *) src)[2];
      double alpha = ((double *) src)[3];

      double X, Y, Z;

      //convert Lab to XYZ
      LAB_to_XYZ (L, a, b, &X, &Y, &Z);

      {
        //convert XYZ to RGB
        double XYZ[3]  = {X,Y,Z};
        babl_space_from_xyz (space, XYZ, (double*)dst);
      }
      ((double *) dst)[3] = alpha;

      src += sizeof (double) * 4;
      dst += sizeof (double) * 4;
    }
}


/* rgb <-> LCh */

static inline void
CHab_to_ab (double  C,
            double  H,
            double *to_a,
            double *to_b)
{
  *to_a = cos (H * RADIANS_PER_DEGREE) * C;
  *to_b = sin (H * RADIANS_PER_DEGREE) * C;
}

static inline void
ab_to_CHab (double  a,
            double  b,
            double *to_C,
            double *to_H)
{
  *to_C = sqrt ( (a * a) + (b * b) );
  *to_H = atan2 (b, a) * DEGREES_PER_RADIAN;

  // Keep H within the range 0-360
  if (*to_H < 0.0)
      *to_H += 360;
}

static void
rgba_to_lchab (const Babl *conversion,
               char       *src,
               char       *dst,
               long        n)
{
  const Babl *space = babl_conversion_get_source_space (conversion);

  while (n--)
    {
      double XYZ[3], L, a, b, C, H;

      //convert RGB to XYZ
      babl_space_to_xyz (space, (double *)src, XYZ);

      //convert XYZ to Lab
      XYZ_to_LAB (XYZ[0], XYZ[1], XYZ[2], &L, &a, &b);


      //convert Lab to LCH(ab)
      ab_to_CHab (a, b, &C, &H);

      ((double *) dst)[0] = L;
      ((double *) dst)[1] = C;
      ((double *) dst)[2] = H;

      src += sizeof (double) * 4;
      dst += sizeof (double) * 3;
    }
}

static void
lchab_to_rgba (const Babl *conversion,
               char       *src,
               char       *dst,
               long        n)
{
  const Babl *space = babl_conversion_get_source_space (conversion);

  while (n--)
    {
      double L = ((double *) src)[0];
      double C = ((double *) src)[1];
      double H = ((double *) src)[2];
      double a, b, X, Y, Z;

      //Convert LCH(ab) to Lab
      CHab_to_ab (C, H, &a, &b);

      //Convert LAB to XYZ
      LAB_to_XYZ (L, a, b, &X, &Y, &Z);

      //Convert XYZ to RGB
      {
        double XYZ[3]  = {X,Y,Z};
        babl_space_from_xyz (space, XYZ, (double*)dst);
      }

      ((double *) dst)[3] = 1.0;

      src += sizeof (double) * 3;
      dst += sizeof (double) * 4;
    }
}

static void
rgba_to_lchaba (const Babl *conversion,
                char       *src,
                char       *dst,
                long        n)
{
  const Babl *space = babl_conversion_get_source_space (conversion);

  while (n--)
    {
      double alpha = ((double *) src)[3];
      double XYZ[3], L, a, b, C, H;

      //convert RGB to XYZ
      babl_space_to_xyz (space, (double*)src, XYZ);

      //convert XYZ to Lab
      XYZ_to_LAB (XYZ[0], XYZ[1], XYZ[2], &L, &a, &b);

      //convert Lab to LCH(ab)
      ab_to_CHab (a, b, &C, &H);

      ((double *) dst)[0] = L;
      ((double *) dst)[1] = C;
      ((double *) dst)[2] = H;
      ((double *) dst)[3] = alpha;

      src += sizeof (double) * 4;
      dst += sizeof (double) * 4;
    }
}

static void
lchaba_to_rgba (const Babl *conversion,
                char       *src,
                char       *dst,
                long        n)
{
  const Babl *space = babl_conversion_get_destination_space (conversion);
  while (n--)
    {
      double L     = ((double *) src)[0];
      double C     = ((double *) src)[1];
      double H     = ((double *) src)[2];
      double alpha = ((double *) src)[3];
      double a, b, X, Y, Z;

      //Convert LCH(ab) to Lab
      CHab_to_ab (C, H, &a, &b);

      //Convert Lab to XYZ
      LAB_to_XYZ (L, a, b, &X, &Y, &Z);

      //Convert XYZ to RGB
      {
        double XYZ[3]  = {X,Y,Z};
        babl_space_from_xyz (space, XYZ, (double*)dst);
      }
      ((double *) dst)[3] = alpha;

      src += sizeof (double) * 4;
      dst += sizeof (double) * 4;
    }
}


/******** end double RGB/CIE color space conversions ******************/

/******** begin floating point RGB/CIE color space conversions ********/

/* origin: http://www.hackersdelight.org/hdcodetxt/acbrt.c.txt
 * permissions: http://www.hackersdelight.org/permissions.htm
 */
/* _cbrtf(x)
 * Return cube root of x
 */

static inline float
_cbrtf (float x)
{
  union { float f; uint32_t i; } u = { x };

  u.i = u.i / 4 + u.i / 16;
  u.i = u.i + u.i / 16;
  u.i = u.i + u.i / 256;
  u.i = 0x2a5137a0 + u.i;
  u.f = 0.33333333f * (2.0f * u.f + x / (u.f * u.f));
  u.f = 0.33333333f * (2.0f * u.f + x / (u.f * u.f));

  return u.f;
}

static inline float
cubef (float f)
{
  return f * f * f;
}

static void
Yf_to_Lf (const Babl *conversion,
          float      *src,
          float      *dst,
          long        samples)
{
  long n = samples;

  while (n--)
    {
      float yr = src[0];
      float L  = yr > LAB_EPSILON ? 116.0f * _cbrtf (yr) - 16 : LAB_KAPPA * yr;

      dst[0] = L;

      src++;
      dst++;
    }
}

static void
Yaf_to_Lf (const Babl *conversion,
           float      *src,
           float      *dst,
           long        samples)
{
  long n = samples;

  while (n--)
    {
      float yr = src[0];
      float L  = yr > LAB_EPSILON ? 116.0f * _cbrtf (yr) - 16 : LAB_KAPPA * yr;

      dst[0] = L;

      src += 2;
      dst += 1;
    }
}

static void
Yaf_to_Laf (const Babl *conversion,
            float      *src,
            float      *dst,
            long        samples)
{
  long n = samples;

  while (n--)
    {
      float yr = src[0];
      float a  = src[1];
      float L  = yr > LAB_EPSILON ? 116.0f * _cbrtf (yr) - 16 : LAB_KAPPA * yr;

      dst[0] = L;
      dst[1] = a;

      src += 2;
      dst += 2;
    }
}

static void
rgbf_to_Labf (const Babl *conversion,
              float      *src,
              float      *dst,
              long        samples)
{
  const Babl *space = babl_conversion_get_source_space (conversion);
  float m_0_0 = space->space.RGBtoXYZf[0] / D50_WHITE_REF_X;
  float m_0_1 = space->space.RGBtoXYZf[1] / D50_WHITE_REF_X;
  float m_0_2 = space->space.RGBtoXYZf[2] / D50_WHITE_REF_X;
  float m_1_0 = space->space.RGBtoXYZf[3] / D50_WHITE_REF_Y;
  float m_1_1 = space->space.RGBtoXYZf[4] / D50_WHITE_REF_Y;
  float m_1_2 = space->space.RGBtoXYZf[5] / D50_WHITE_REF_Y;
  float m_2_0 = space->space.RGBtoXYZf[6] / D50_WHITE_REF_Z;
  float m_2_1 = space->space.RGBtoXYZf[7] / D50_WHITE_REF_Z;
  float m_2_2 = space->space.RGBtoXYZf[8] / D50_WHITE_REF_Z;
  long n = samples;

  while (n--)
    {
      float r = src[0];
      float g = src[1];
      float b = src[2];

      float xr = m_0_0 * r + m_0_1 * g + m_0_2 * b;
      float yr = m_1_0 * r + m_1_1 * g + m_1_2 * b;
      float zr = m_2_0 * r + m_2_1 * g + m_2_2 * b;

      float fx = xr > LAB_EPSILON ? _cbrtf (xr) : (LAB_KAPPA * xr + 16.0f) / 116.0f;
      float fy = yr > LAB_EPSILON ? _cbrtf (yr) : (LAB_KAPPA * yr + 16.0f) / 116.0f;
      float fz = zr > LAB_EPSILON ? _cbrtf (zr) : (LAB_KAPPA * zr + 16.0f) / 116.0f;

      float L = 116.0f * fy - 16.0f;
      float A = 500.0f * (fx - fy);
      float B = 200.0f * (fy - fz);

      dst[0] = L;
      dst[1] = A;
      dst[2] = B;

      src += 3;
      dst += 3;
    }
}

static void
rgbaf_to_Lf (const Babl *conversion,
             float      *src,
             float      *dst,
             long        samples)
{
  const Babl *space = babl_conversion_get_source_space (conversion);
  float m_1_0 = space->space.RGBtoXYZf[3] / D50_WHITE_REF_Y;
  float m_1_1 = space->space.RGBtoXYZf[4] / D50_WHITE_REF_Y;
  float m_1_2 = space->space.RGBtoXYZf[5] / D50_WHITE_REF_Y;
  long n = samples;

  while (n--)
    {
      float r = src[0];
      float g = src[1];
      float b = src[2];

      float yr = m_1_0 * r + m_1_1 * g + m_1_2 * b;
      float L = yr > LAB_EPSILON ? 116.0f * _cbrtf (yr) - 16 : LAB_KAPPA * yr;

      dst[0] = L;

      src += 4;
      dst += 1;
    }
}

static void
rgbaf_to_Labf (const Babl *conversion,
               float      *src,
               float      *dst,
               long        samples)
{
  const Babl *space = babl_conversion_get_source_space (conversion);
  float m_0_0 = space->space.RGBtoXYZf[0] / D50_WHITE_REF_X;
  float m_0_1 = space->space.RGBtoXYZf[1] / D50_WHITE_REF_X;
  float m_0_2 = space->space.RGBtoXYZf[2] / D50_WHITE_REF_X;
  float m_1_0 = space->space.RGBtoXYZf[3] / D50_WHITE_REF_Y;
  float m_1_1 = space->space.RGBtoXYZf[4] / D50_WHITE_REF_Y;
  float m_1_2 = space->space.RGBtoXYZf[5] / D50_WHITE_REF_Y;
  float m_2_0 = space->space.RGBtoXYZf[6] / D50_WHITE_REF_Z;
  float m_2_1 = space->space.RGBtoXYZf[7] / D50_WHITE_REF_Z;
  float m_2_2 = space->space.RGBtoXYZf[8] / D50_WHITE_REF_Z;
  long n = samples;

  while (n--)
    {
      float r = src[0];
      float g = src[1];
      float b = src[2];

      float xr = m_0_0 * r + m_0_1 * g + m_0_2 * b;
      float yr = m_1_0 * r + m_1_1 * g + m_1_2 * b;
      float zr = m_2_0 * r + m_2_1 * g + m_2_2 * b;

      float fx = xr > LAB_EPSILON ? _cbrtf (xr) : (LAB_KAPPA * xr + 16.0f) / 116.0f;
      float fy = yr > LAB_EPSILON ? _cbrtf (yr) : (LAB_KAPPA * yr + 16.0f) / 116.0f;
      float fz = zr > LAB_EPSILON ? _cbrtf (zr) : (LAB_KAPPA * zr + 16.0f) / 116.0f;

      float L = 116.0f * fy - 16.0f;
      float A = 500.0f * (fx - fy);
      float B = 200.0f * (fy - fz);

      dst[0] = L;
      dst[1] = A;
      dst[2] = B;

      src += 4;
      dst += 3;
    }
}

static void
rgbaf_to_Labaf (const Babl *conversion,
                float      *src,
                float      *dst,
                long        samples)
{
  const Babl *space = babl_conversion_get_source_space (conversion);
  float m_0_0 = space->space.RGBtoXYZf[0] / D50_WHITE_REF_X;
  float m_0_1 = space->space.RGBtoXYZf[1] / D50_WHITE_REF_X;
  float m_0_2 = space->space.RGBtoXYZf[2] / D50_WHITE_REF_X;
  float m_1_0 = space->space.RGBtoXYZf[3] / D50_WHITE_REF_Y;
  float m_1_1 = space->space.RGBtoXYZf[4] / D50_WHITE_REF_Y;
  float m_1_2 = space->space.RGBtoXYZf[5] / D50_WHITE_REF_Y;
  float m_2_0 = space->space.RGBtoXYZf[6] / D50_WHITE_REF_Z;
  float m_2_1 = space->space.RGBtoXYZf[7] / D50_WHITE_REF_Z;
  float m_2_2 = space->space.RGBtoXYZf[8] / D50_WHITE_REF_Z;
  long n = samples;

  while (n--)
    {
      float r = src[0];
      float g = src[1];
      float b = src[2];
      float a = src[3];

      float xr = m_0_0 * r + m_0_1 * g + m_0_2 * b;
      float yr = m_1_0 * r + m_1_1 * g + m_1_2 * b;
      float zr = m_2_0 * r + m_2_1 * g + m_2_2 * b;

      float fx = xr > LAB_EPSILON ? _cbrtf (xr) : (LAB_KAPPA * xr + 16.0f) / 116.0f;
      float fy = yr > LAB_EPSILON ? _cbrtf (yr) : (LAB_KAPPA * yr + 16.0f) / 116.0f;
      float fz = zr > LAB_EPSILON ? _cbrtf (zr) : (LAB_KAPPA * zr + 16.0f) / 116.0f;

      float L = 116.0f * fy - 16.0f;
      float A = 500.0f * (fx - fy);
      float B = 200.0f * (fy - fz);

      dst[0] = L;
      dst[1] = A;
      dst[2] = B;
      dst[3] = a;

      src += 4;
      dst += 4;
    }
}

static void
Labf_to_Lf (const Babl *conversion,
            float      *src,
            float      *dst,
            long        samples)
{
  long n = samples;

  while (n--)
    {
      dst[0] = src[0];

      src += 3;
      dst += 1;
    }
}

static void
Labaf_to_Lf (const Babl *conversion,
             float      *src,
             float      *dst,
             long        samples)
{
  long n = samples;

  while (n--)
    {
      dst[0] = src[0];

      src += 4;
      dst += 1;
    }
}

static void
Labf_to_rgbf (const Babl *conversion,
              float      *src,
              float      *dst,
              long        samples)
{
  const Babl *space = babl_conversion_get_source_space (conversion);
  float m_0_0 = space->space.XYZtoRGBf[0] * D50_WHITE_REF_X;
  float m_0_1 = space->space.XYZtoRGBf[1] * D50_WHITE_REF_Y;
  float m_0_2 = space->space.XYZtoRGBf[2] * D50_WHITE_REF_Z;
  float m_1_0 = space->space.XYZtoRGBf[3] * D50_WHITE_REF_X;
  float m_1_1 = space->space.XYZtoRGBf[4] * D50_WHITE_REF_Y;
  float m_1_2 = space->space.XYZtoRGBf[5] * D50_WHITE_REF_Z;
  float m_2_0 = space->space.XYZtoRGBf[6] * D50_WHITE_REF_X;
  float m_2_1 = space->space.XYZtoRGBf[7] * D50_WHITE_REF_Y;
  float m_2_2 = space->space.XYZtoRGBf[8] * D50_WHITE_REF_Z;
  long n = samples;

  while (n--)
    {
      float L = src[0];
      float A = src[1];
      float B = src[2];

      float fy = (L + 16.0f) / 116.0f;
      float fx = fy + A / 500.0f;
      float fz = fy - B / 200.0f;

      float yr = L > LAB_KAPPA * LAB_EPSILON ? cubef (fy) : L / LAB_KAPPA;
      float xr = cubef (fx) > LAB_EPSILON ? cubef (fx) : (fx * 116.0f - 16.0f) / LAB_KAPPA;
      float zr = cubef (fz) > LAB_EPSILON ? cubef (fz) : (fz * 116.0f - 16.0f) / LAB_KAPPA;

      float r = m_0_0 * xr + m_0_1 * yr + m_0_2 * zr;
      float g = m_1_0 * xr + m_1_1 * yr + m_1_2 * zr;
      float b = m_2_0 * xr + m_2_1 * yr + m_2_2 * zr;

      dst[0] = r;
      dst[1] = g;
      dst[2] = b;

      src += 3;
      dst += 3;
    }
}


static void
Labf_to_rgbaf (const Babl *conversion,float *src,
               float *dst,
               long   samples)
{
  const Babl *space = babl_conversion_get_source_space (conversion);
  float m_0_0 = space->space.XYZtoRGBf[0] * D50_WHITE_REF_X;
  float m_0_1 = space->space.XYZtoRGBf[1] * D50_WHITE_REF_Y;
  float m_0_2 = space->space.XYZtoRGBf[2] * D50_WHITE_REF_Z;
  float m_1_0 = space->space.XYZtoRGBf[3] * D50_WHITE_REF_X;
  float m_1_1 = space->space.XYZtoRGBf[4] * D50_WHITE_REF_Y;
  float m_1_2 = space->space.XYZtoRGBf[5] * D50_WHITE_REF_Z;
  float m_2_0 = space->space.XYZtoRGBf[6] * D50_WHITE_REF_X;
  float m_2_1 = space->space.XYZtoRGBf[7] * D50_WHITE_REF_Y;
  float m_2_2 = space->space.XYZtoRGBf[8] * D50_WHITE_REF_Z;
  long n = samples;

  while (n--)
    {
      float L = src[0];
      float A = src[1];
      float B = src[2];

      float fy = (L + 16.0f) / 116.0f;
      float fx = fy + A / 500.0f;
      float fz = fy - B / 200.0f;

      float yr = L > LAB_KAPPA * LAB_EPSILON ? cubef (fy) : L / LAB_KAPPA;
      float xr = cubef (fx) > LAB_EPSILON ? cubef (fx) : (fx * 116.0f - 16.0f) / LAB_KAPPA;
      float zr = cubef (fz) > LAB_EPSILON ? cubef (fz) : (fz * 116.0f - 16.0f) / LAB_KAPPA;

      float r = m_0_0 * xr + m_0_1 * yr + m_0_2 * zr;
      float g = m_1_0 * xr + m_1_1 * yr + m_1_2 * zr;
      float b = m_2_0 * xr + m_2_1 * yr + m_2_2 * zr;

      dst[0] = r;
      dst[1] = g;
      dst[2] = b;
      dst[3] = 1.0f;

      src += 3;
      dst += 4;
    }
}

static void
Labaf_to_rgbaf (const Babl *conversion,
                float      *src,
                float      *dst,
                long        samples)
{
  const Babl *space = babl_conversion_get_source_space (conversion);
  float m_0_0 = space->space.XYZtoRGBf[0] * D50_WHITE_REF_X;
  float m_0_1 = space->space.XYZtoRGBf[1] * D50_WHITE_REF_Y;
  float m_0_2 = space->space.XYZtoRGBf[2] * D50_WHITE_REF_Z;
  float m_1_0 = space->space.XYZtoRGBf[3] * D50_WHITE_REF_X;
  float m_1_1 = space->space.XYZtoRGBf[4] * D50_WHITE_REF_Y;
  float m_1_2 = space->space.XYZtoRGBf[5] * D50_WHITE_REF_Z;
  float m_2_0 = space->space.XYZtoRGBf[6] * D50_WHITE_REF_X;
  float m_2_1 = space->space.XYZtoRGBf[7] * D50_WHITE_REF_Y;
  float m_2_2 = space->space.XYZtoRGBf[8] * D50_WHITE_REF_Z;
  long n = samples;

  while (n--)
    {
      float L = src[0];
      float A = src[1];
      float B = src[2];
      float a = src[3];

      float fy = (L + 16.0f) / 116.0f;
      float fx = fy + A / 500.0f;
      float fz = fy - B / 200.0f;

      float yr = L > LAB_KAPPA * LAB_EPSILON ? cubef (fy) : L / LAB_KAPPA;
      float xr = cubef (fx) > LAB_EPSILON ? cubef (fx) : (fx * 116.0f - 16.0f) / LAB_KAPPA;
      float zr = cubef (fz) > LAB_EPSILON ? cubef (fz) : (fz * 116.0f - 16.0f) / LAB_KAPPA;

      float r = m_0_0 * xr + m_0_1 * yr + m_0_2 * zr;
      float g = m_1_0 * xr + m_1_1 * yr + m_1_2 * zr;
      float b = m_2_0 * xr + m_2_1 * yr + m_2_2 * zr;

      dst[0] = r;
      dst[1] = g;
      dst[2] = b;
      dst[3] = a;

      src += 4;
      dst += 4;
    }
}

static void
Labf_to_Lchabf (const Babl *conversion,
                float      *src,
                float      *dst,
                long        samples)
{
  long n = samples;

  while (n--)
    {
      float L = src[0];
      float A = src[1];
      float B = src[2];

      float C = sqrtf (A * A + B * B);
      float H = atan2f (B, A) * DEGREES_PER_RADIAN;

      // Keep H within the range 0-360
      if (H < 0.0f)
        H += 360.0f;

      dst[0] = L;
      dst[1] = C;
      dst[2] = H;

      src += 3;
      dst += 3;
    }
}

static void
Lchabf_to_Labf (const Babl *conversion,
                float      *src,
                float      *dst,
                long        samples)
{
  long n = samples;

  while (n--)
    {
      float L = src[0];
      float C = src[1];
      float H = src[2];

      float A = C * cosf (H * RADIANS_PER_DEGREE);
      float B = C * sinf (H * RADIANS_PER_DEGREE);

      dst[0] = L;
      dst[1] = A;
      dst[2] = B;

      src += 3;
      dst += 3;
    }
}

static void
Labaf_to_Lchabaf (const Babl *conversion,
                  float      *src,
                  float      *dst,
                  long        samples)
{
  long n = samples;

  while (n--)
    {
      float L = src[0];
      float A = src[1];
      float B = src[2];
      float a = src[3];

      float C = sqrtf (A * A + B * B);
      float H = atan2f (B, A) * DEGREES_PER_RADIAN;

      // Keep H within the range 0-360
      if (H < 0.0f)
        H += 360.0f;

      dst[0] = L;
      dst[1] = C;
      dst[2] = H;
      dst[3] = a;

      src += 4;
      dst += 4;
    }
}

static void
Lchabaf_to_Labaf (const Babl *conversion,
                  float      *src,
                  float      *dst,
                  long        samples)
{
  long n = samples;

  while (n--)
    {
      float L = src[0];
      float C = src[1];
      float H = src[2];
      float a = src[3];

      float A = C * cosf (H * RADIANS_PER_DEGREE);
      float B = C * sinf (H * RADIANS_PER_DEGREE);

      dst[0] = L;
      dst[1] = A;
      dst[2] = B;
      dst[3] = a;

      src += 4;
      dst += 4;
    }
}

#if defined(USE_SSE2)

/* This is an SSE2 version of Halley's method for approximating the
 * cube root of an IEEE float implementation.
 *
 * The scalar version is as follows:
 *
 * static inline float
 * _cbrt_5f (float x)
 * {
 *   union { float f; uint32_t i; } u = { x };
 *
 *   u.i = u.i / 3 + 709921077;
 *   return u.f;
 * }
 *
 * static inline float
 * _cbrta_halleyf (float a, float R)
 * {
 *   float a3 = a * a * a;
 *   float b = a * (a3 + R + R) / (a3 + a3 + R);
 *   return b;
 * }
 *
 * static inline float
 * _cbrtf (float x)
 * {
 *   float a;
 *
 *   a = _cbrt_5f (x);
 *   a = _cbrta_halleyf (a, x);
 *   a = _cbrta_halleyf (a, x);
 *   return a;
 * }
 *
 * The above scalar version seems to have originated from
 * http://metamerist.com/cbrt/cbrt.htm but that's not accessible
 * anymore. At present there's a copy in CubeRoot.cpp in the Skia
 * sources that's licensed under a BSD-style license. There's some
 * discussion on the implementation at
 * http://www.voidcn.com/article/p-gpwztojr-wt.html.
 *
 * Note that Darktable also has an SSE2 version of the same algorithm,
 * but uses only a single iteration of Halley's method, which is too
 * coarse.
 */
/* Return cube roots of the four single-precision floating point
 * components of x.
 */
static inline __m128
_cbrtf_ps_sse2 (__m128 x)
{
  const __m128i magic = _mm_set1_epi32 (709921077);

  __m128i xi = _mm_castps_si128 (x);
  __m128 xi_3 = _mm_div_ps (_mm_cvtepi32_ps (xi), _mm_set1_ps (3.0f));
  __m128i ai = _mm_add_epi32 (_mm_cvtps_epi32 (xi_3), magic);
  __m128 a = _mm_castsi128_ps (ai);

  __m128 a3 = _mm_mul_ps (_mm_mul_ps (a, a), a);
  __m128 divisor = _mm_add_ps (_mm_add_ps (a3, a3), x);
  a = _mm_div_ps (_mm_mul_ps (a, _mm_add_ps (a3, _mm_add_ps (x, x))), divisor);

  a3 = _mm_mul_ps (_mm_mul_ps (a, a), a);
  divisor = _mm_add_ps (_mm_add_ps (a3, a3), x);
  a = _mm_div_ps (_mm_mul_ps (a, _mm_add_ps (a3, _mm_add_ps (x, x))), divisor);

  return a;
}

static inline __m128
lab_r_to_f_sse2 (__m128 r)
{
  const __m128 epsilon = _mm_set1_ps (LAB_EPSILON);
  const __m128 kappa = _mm_set1_ps (LAB_KAPPA);

  const __m128 f_big = _cbrtf_ps_sse2 (r);

  const __m128 f_small = _mm_div_ps (_mm_add_ps (_mm_mul_ps (kappa, r), _mm_set1_ps (16.0f)),
                                     _mm_set1_ps (116.0f));

  const __m128 mask = _mm_cmpgt_ps (r, epsilon);
  const __m128 f = _mm_or_ps (_mm_and_ps (mask, f_big), _mm_andnot_ps (mask, f_small));
  return f;
}

static void
Yf_to_Lf_sse2 (const Babl  *conversion, 
               const float *src, 
               float       *dst, 
               long         samples)
{
  long i = 0;
  long remainder;

  if (((uintptr_t) src % 16) + ((uintptr_t) dst % 16) == 0)
    {
      const long n = (samples / 4) * 4;

      for ( ; i < n; i += 4)
        {
          __m128 Y = _mm_load_ps (src);

          __m128 fy = lab_r_to_f_sse2 (Y);

          __m128 L = _mm_sub_ps (_mm_mul_ps (_mm_set1_ps (116.0f), fy), _mm_set1_ps (16.0f));

          _mm_store_ps (dst, L);

          src += 4;
          dst += 4;
        }
    }

  remainder = samples - i;
  while (remainder--)
    {
      float yr = src[0];
      float L  = yr > LAB_EPSILON ? 116.0f * _cbrtf (yr) - 16 : LAB_KAPPA * yr;

      dst[0] = L;

      src++;
      dst++;
    }
}

static void
Yaf_to_Lf_sse2 (const Babl  *conversion, 
                const float *src, 
                float       *dst, 
                long         samples)
{
  long i = 0;
  long remainder;

  if (((uintptr_t) src % 16) + ((uintptr_t) dst % 16) == 0)
    {
      const long n = (samples / 4) * 4;

      for ( ; i < n; i += 4)
        {
          __m128 YaYa0 = _mm_load_ps (src);
          __m128 YaYa1 = _mm_load_ps (src + 4);

          __m128 Y = _mm_shuffle_ps (YaYa0, YaYa1, _MM_SHUFFLE (2, 0, 2, 0));

          __m128 fy = lab_r_to_f_sse2 (Y);

          __m128 L = _mm_sub_ps (_mm_mul_ps (_mm_set1_ps (116.0f), fy), _mm_set1_ps (16.0f));

          _mm_store_ps (dst, L);

          src += 8;
          dst += 4;
        }
    }

  remainder = samples - i;
  while (remainder--)
    {
      float yr = src[0];
      float L  = yr > LAB_EPSILON ? 116.0f * _cbrtf (yr) - 16 : LAB_KAPPA * yr;

      dst[0] = L;

      src += 2;
      dst += 1;
    }
}

static void
rgbaf_to_Lf_sse2 (const Babl  *conversion, 
                  const float *src, 
                  float       *dst, 
                  long         samples)
{
  const Babl *space = babl_conversion_get_source_space (conversion);
  const float m_1_0 = space->space.RGBtoXYZf[3] / D50_WHITE_REF_Y;
  const float m_1_1 = space->space.RGBtoXYZf[4] / D50_WHITE_REF_Y;
  const float m_1_2 = space->space.RGBtoXYZf[5] / D50_WHITE_REF_Y;
  long i = 0;
  long remainder;

  if (((uintptr_t) src % 16) + ((uintptr_t) dst % 16) == 0)
    {
      const long    n = (samples / 4) * 4;
      const __m128 m_1_0_v = _mm_set1_ps (m_1_0);
      const __m128 m_1_1_v = _mm_set1_ps (m_1_1);
      const __m128 m_1_2_v = _mm_set1_ps (m_1_2);

      for ( ; i < n; i += 4)
        {
          __m128 rgba0 = _mm_load_ps (src);
          __m128 rgba1 = _mm_load_ps (src + 4);
          __m128 rgba2 = _mm_load_ps (src + 8);
          __m128 rgba3 = _mm_load_ps (src + 12);

          __m128 r = rgba0;
          __m128 g = rgba1;
          __m128 b = rgba2;
          __m128 a = rgba3;
          _MM_TRANSPOSE4_PS (r, g, b, a);

          {
            __m128 yr = _mm_add_ps (_mm_add_ps (_mm_mul_ps (m_1_0_v, r), _mm_mul_ps (m_1_1_v, g)),
                                    _mm_mul_ps (m_1_2_v, b));

            __m128 fy = lab_r_to_f_sse2 (yr);

            __m128 L = _mm_sub_ps (_mm_mul_ps (_mm_set1_ps (116.0f), fy), _mm_set1_ps (16.0f));

            _mm_store_ps (dst, L);
          }

          src += 16;
          dst += 4;
        }
    }

  remainder = samples - i;
  while (remainder--)
    {
      float r = src[0];
      float g = src[1];
      float b = src[2];

      float yr = m_1_0 * r + m_1_1 * g + m_1_2 * b;
      float L = yr > LAB_EPSILON ? 116.0f * _cbrtf (yr) - 16 : LAB_KAPPA * yr;

      dst[0] = L;

      src += 4;
      dst += 1;
    }
}

static void
rgbaf_to_Labaf_sse2 (const Babl  *conversion, 
                     const float *src, 
                     float       *dst, 
                     long         samples)
{
  const Babl *space = babl_conversion_get_source_space (conversion);
  const float m_0_0 = space->space.RGBtoXYZf[0] / D50_WHITE_REF_X;
  const float m_0_1 = space->space.RGBtoXYZf[1] / D50_WHITE_REF_X;
  const float m_0_2 = space->space.RGBtoXYZf[2] / D50_WHITE_REF_X;
  const float m_1_0 = space->space.RGBtoXYZf[3] / D50_WHITE_REF_Y;
  const float m_1_1 = space->space.RGBtoXYZf[4] / D50_WHITE_REF_Y;
  const float m_1_2 = space->space.RGBtoXYZf[5] / D50_WHITE_REF_Y;
  const float m_2_0 = space->space.RGBtoXYZf[6] / D50_WHITE_REF_Z;
  const float m_2_1 = space->space.RGBtoXYZf[7] / D50_WHITE_REF_Z;
  const float m_2_2 = space->space.RGBtoXYZf[8] / D50_WHITE_REF_Z;
  long i = 0;
  long remainder;

  if (((uintptr_t) src % 16) + ((uintptr_t) dst % 16) == 0)
    {
      const long    n = (samples / 4) * 4;
      const __m128 m_0_0_v = _mm_set1_ps (m_0_0);
      const __m128 m_0_1_v = _mm_set1_ps (m_0_1);
      const __m128 m_0_2_v = _mm_set1_ps (m_0_2);
      const __m128 m_1_0_v = _mm_set1_ps (m_1_0);
      const __m128 m_1_1_v = _mm_set1_ps (m_1_1);
      const __m128 m_1_2_v = _mm_set1_ps (m_1_2);
      const __m128 m_2_0_v = _mm_set1_ps (m_2_0);
      const __m128 m_2_1_v = _mm_set1_ps (m_2_1);
      const __m128 m_2_2_v = _mm_set1_ps (m_2_2);

      for ( ; i < n; i += 4)
        {
          __m128 Laba0;
          __m128 Laba1;
          __m128 Laba2;
          __m128 Laba3;

          __m128 rgba0 = _mm_load_ps (src);
          __m128 rgba1 = _mm_load_ps (src + 4);
          __m128 rgba2 = _mm_load_ps (src + 8);
          __m128 rgba3 = _mm_load_ps (src + 12);

          __m128 r = rgba0;
          __m128 g = rgba1;
          __m128 b = rgba2;
          __m128 a = rgba3;
          _MM_TRANSPOSE4_PS (r, g, b, a);

          {
            __m128 xr = _mm_add_ps (_mm_add_ps (_mm_mul_ps (m_0_0_v, r), _mm_mul_ps (m_0_1_v, g)),
                                    _mm_mul_ps (m_0_2_v, b));
            __m128 yr = _mm_add_ps (_mm_add_ps (_mm_mul_ps (m_1_0_v, r), _mm_mul_ps (m_1_1_v, g)),
                                    _mm_mul_ps (m_1_2_v, b));
            __m128 zr = _mm_add_ps (_mm_add_ps (_mm_mul_ps (m_2_0_v, r), _mm_mul_ps (m_2_1_v, g)),
                                    _mm_mul_ps (m_2_2_v, b));

            __m128 fx = lab_r_to_f_sse2 (xr);
            __m128 fy = lab_r_to_f_sse2 (yr);
            __m128 fz = lab_r_to_f_sse2 (zr);

            __m128 L = _mm_sub_ps (_mm_mul_ps (_mm_set1_ps (116.0f), fy), _mm_set1_ps (16.0f));
            __m128 A = _mm_mul_ps (_mm_set1_ps (500.0f), _mm_sub_ps (fx, fy));
            __m128 B = _mm_mul_ps (_mm_set1_ps (200.0f), _mm_sub_ps (fy, fz));

            Laba0 = L;
            Laba1 = A;
            Laba2 = B;
            Laba3 = a;
            _MM_TRANSPOSE4_PS (Laba0, Laba1, Laba2, Laba3);
          }

          _mm_store_ps (dst, Laba0);
          _mm_store_ps (dst + 4, Laba1);
          _mm_store_ps (dst + 8, Laba2);
          _mm_store_ps (dst + 12, Laba3);

          src += 16;
          dst += 16;
        }
    }

  remainder = samples - i;
  while (remainder--)
    {
      float r = src[0];
      float g = src[1];
      float b = src[2];
      float a = src[3];

      float xr = m_0_0 * r + m_0_1 * g + m_0_2 * b;
      float yr = m_1_0 * r + m_1_1 * g + m_1_2 * b;
      float zr = m_2_0 * r + m_2_1 * g + m_2_2 * b;

      float fx = xr > LAB_EPSILON ? _cbrtf (xr) : (LAB_KAPPA * xr + 16.0f) / 116.0f;
      float fy = yr > LAB_EPSILON ? _cbrtf (yr) : (LAB_KAPPA * yr + 16.0f) / 116.0f;
      float fz = zr > LAB_EPSILON ? _cbrtf (zr) : (LAB_KAPPA * zr + 16.0f) / 116.0f;

      float L = 116.0f * fy - 16.0f;
      float A = 500.0f * (fx - fy);
      float B = 200.0f * (fy - fz);

      dst[0] = L;
      dst[1] = A;
      dst[2] = B;
      dst[3] = a;

      src += 4;
      dst += 4;
    }
}

#endif /* defined(USE_SSE2) */

static void
conversions (void)
{
  /* babl_model */

  babl_conversion_new (
    babl_model ("RGBA"),
    babl_model ("CIE Lab"),
    "linear", rgba_to_lab,
    NULL
  );
  babl_conversion_new (
    babl_model ("CIE Lab"),
    babl_model ("RGBA"),
    "linear", lab_to_rgba,
    NULL
  );
  babl_conversion_new (
    babl_model ("RGBA"),
    babl_model ("CIE Lab alpha"),
    "linear", rgba_to_laba,
    NULL
  );
    babl_conversion_new (
    babl_model ("CIE Lab alpha"),
    babl_model ("RGBA"),
    "linear", laba_to_rgba,
    NULL
  );
  babl_conversion_new (
    babl_model ("RGBA"),
    babl_model ("CIE LCH(ab)"),
    "linear", rgba_to_lchab,
    NULL
  );
  babl_conversion_new (
    babl_model ("CIE LCH(ab)"),
    babl_model ("RGBA"),
    "linear", lchab_to_rgba,
    NULL
  );
  babl_conversion_new (
    babl_model ("RGBA"),
    babl_model ("CIE LCH(ab) alpha"),
    "linear", rgba_to_lchaba,
    NULL
  );
  babl_conversion_new (
    babl_model ("CIE LCH(ab) alpha"),
    babl_model ("RGBA"),
    "linear", lchaba_to_rgba,
    NULL
  );
  babl_conversion_new (
    babl_model ("RGBA"),
    babl_model ("CIE XYZ"),
    "linear", rgba_to_xyz,
    NULL
  );
  babl_conversion_new (
    babl_model ("CIE XYZ"),
    babl_model ("RGBA"),
    "linear", xyz_to_rgba,
    NULL
  );
  babl_conversion_new (
    babl_model ("RGBA"),
    babl_model ("CIE XYZ alpha"),
    "linear", rgba_to_xyza,
    NULL
  );
  babl_conversion_new (
    babl_model ("CIE XYZ alpha"),
    babl_model ("RGBA"),
    "linear", xyza_to_rgba,
    NULL
  );
  babl_conversion_new (
    babl_model ("RGBA"),
    babl_model ("CIE xyY"),
    "linear", rgba_to_xyY,
    NULL
  );
  babl_conversion_new (
    babl_model ("CIE xyY"),
    babl_model ("RGBA"),
    "linear", xyY_to_rgba,
    NULL
  );
  babl_conversion_new (
    babl_model ("RGBA"),
    babl_model ("CIE xyY alpha"),
    "linear", rgba_to_xyYa,
    NULL
  );
  babl_conversion_new (
    babl_model ("CIE xyY alpha"),
    babl_model ("RGBA"),
    "linear", xyYa_to_rgba,
    NULL
  );

  /* babl_format */

  babl_conversion_new (
    babl_format ("RGB float"),
    babl_format ("CIE Lab float"),
    "linear", rgbf_to_Labf,
    NULL
  );
  babl_conversion_new (
    babl_format ("RGBA float"),
    babl_format ("CIE Lab float"),
    "linear", rgbaf_to_Labf,
    NULL
  );
  babl_conversion_new (
    babl_format ("RGBA float"),
    babl_format ("CIE Lab alpha float"),
    "linear", rgbaf_to_Labaf,
    NULL
  );
  babl_conversion_new (
    babl_format ("CIE Lab float"),
    babl_format ("RGB float"),
    "linear", Labf_to_rgbf,
    NULL
  );
  babl_conversion_new (
    babl_format ("CIE Lab float"),
    babl_format ("RGBA float"),
    "linear", Labf_to_rgbaf,
    NULL
  );
  babl_conversion_new (
    babl_format ("CIE Lab alpha float"),
    babl_format ("RGBA float"),
    "linear", Labaf_to_rgbaf,
    NULL
  );
  babl_conversion_new (
    babl_format ("Y float"),
    babl_format ("CIE L float"),
    "linear", Yf_to_Lf,
    NULL
  );
  babl_conversion_new (
    babl_format ("YA float"),
    babl_format ("CIE L float"),
    "linear", Yaf_to_Lf,
    NULL
  );
  babl_conversion_new (
    babl_format ("YA float"),
    babl_format ("CIE L alpha float"),
    "linear", Yaf_to_Laf,
    NULL
  );
  babl_conversion_new (
    babl_format ("RGBA float"),
    babl_format ("CIE L float"),
    "linear", rgbaf_to_Lf,
    NULL
  );
  babl_conversion_new (
    babl_format ("CIE Lab float"),
    babl_format ("CIE L float"),
    "linear", Labf_to_Lf,
    NULL
  );
  babl_conversion_new (
    babl_format ("CIE Lab alpha float"),
    babl_format ("CIE L float"),
    "linear", Labaf_to_Lf,
    NULL
  );
  babl_conversion_new (
    babl_format ("CIE Lab float"),
    babl_format ("CIE LCH(ab) float"),
    "linear", Labf_to_Lchabf,
    NULL
  );
  babl_conversion_new (
    babl_format ("CIE LCH(ab) float"),
    babl_format ("CIE Lab float"),
    "linear", Lchabf_to_Labf,
    NULL
  );
  babl_conversion_new (
    babl_format ("CIE Lab alpha float"),
    babl_format ("CIE LCH(ab) alpha float"),
    "linear", Labaf_to_Lchabaf,
    NULL
  );
  babl_conversion_new (
    babl_format ("CIE LCH(ab) alpha float"),
    babl_format ("CIE Lab alpha float"),
    "linear", Lchabaf_to_Labaf,
    NULL
  );
  babl_conversion_new (
    babl_format ("RGB float"),
    babl_format ("CIE xyY float"),
    "linear", rgbf_to_xyYf,
    NULL
  );
  babl_conversion_new (
    babl_format ("CIE xyY float"),
    babl_format ("RGB float"),
    "linear", xyYf_to_rgbf,
    NULL
  );
  babl_conversion_new (
    babl_format ("RGBA float"),
    babl_format ("CIE xyY float"),
    "linear", rgbaf_to_xyYf,
    NULL
  );
  babl_conversion_new (
    babl_format ("CIE xyY float"),
    babl_format ("RGBA float"),
    "linear", xyYf_to_rgbaf,
    NULL
  );
  babl_conversion_new (
    babl_format ("RGBA float"),
    babl_format ("CIE xyY alpha float"),
    "linear", rgbaf_to_xyYaf,
    NULL
  );
  babl_conversion_new (
    babl_format ("CIE xyY alpha float"),
    babl_format ("RGBA float"),
    "linear", xyYaf_to_rgbaf,
    NULL
  );

#if defined(USE_SSE2)

  if (babl_cpu_accel_get_support () & BABL_CPU_ACCEL_X86_SSE2)
    {
      babl_conversion_new (
        babl_format ("RGBA float"),
        babl_format ("CIE Lab alpha float"),
        "linear", rgbaf_to_Labaf_sse2,
        NULL
      );
      babl_conversion_new (
        babl_format ("Y float"),
        babl_format ("CIE L float"),
        "linear", Yf_to_Lf_sse2,
        NULL
      );
      babl_conversion_new (
        babl_format ("YA float"),
        babl_format ("CIE L float"),
        "linear", Yaf_to_Lf_sse2,
        NULL
      );
      babl_conversion_new (
        babl_format ("RGBA float"),
        babl_format ("CIE L float"),
        "linear", rgbaf_to_Lf_sse2,
        NULL
      );
    }

#endif /* defined(USE_SSE2) */

  rgbcie_init ();
}

static void
formats (void)
{
  babl_format_new (
    "name", "CIE Lab float",
    babl_model ("CIE Lab"),

    babl_type ("float"),
    babl_component ("CIE L"),
    babl_component ("CIE a"),
    babl_component ("CIE b"),
    NULL);

  babl_format_new (
    "name", "CIE XYZ float",
    babl_model ("CIE XYZ"),

    babl_type ("float"),
    babl_component ("CIE X"),
    babl_component ("CIE Y"),
    babl_component ("CIE Z"),
    NULL);

  babl_format_new (
    "name", "CIE XYZ alpha float",
    babl_model ("CIE XYZ"),

    babl_type ("float"),
    babl_component ("CIE X"),
    babl_component ("CIE Y"),
    babl_component ("CIE Z"),
    babl_component ("A"),
    NULL);

  babl_format_new (
    "name", "CIE Lab alpha float",
    babl_model ("CIE Lab alpha"),

    babl_type ("float"),
    babl_component ("CIE L"),
    babl_component ("CIE a"),
    babl_component ("CIE b"),
    babl_component ("A"),
    NULL);

  babl_format_new (
    "name", "CIE LCH(ab) float",
    babl_model ("CIE LCH(ab)"),

    babl_type ("float"),
    babl_component ("CIE L"),
    babl_component ("CIE C(ab)"),
    babl_component ("CIE H(ab)"),
    NULL);

  babl_format_new (
    "name", "CIE LCH(ab) alpha float",
    babl_model ("CIE LCH(ab) alpha"),

    babl_type ("float"),
    babl_component ("CIE L"),
    babl_component ("CIE C(ab)"),
    babl_component ("CIE H(ab)"),
    babl_component ("A"),
    NULL);

  babl_format_new (
    "name", "CIE L float",
    babl_model ("CIE Lab"),
    babl_type ("float"),
    babl_component ("CIE L"),
    NULL);

  babl_format_new (
    "name", "CIE L alpha float",
    babl_model ("CIE Lab alpha"),
    babl_type ("float"),
    babl_component ("CIE L"),
    babl_component ("A"),
    NULL);

  babl_format_new (
    "name", "CIE Lab u8",
    babl_model ("CIE Lab"),

    babl_type ("CIE u8 L"),
    babl_component ("CIE L"),
    babl_type ("CIE u8 ab"),
    babl_component ("CIE a"),
    babl_type ("CIE u8 ab"),
    babl_component ("CIE b"),
    NULL);

  babl_format_new (
    "name", "CIE Lab u16",
    babl_model ("CIE Lab"),

    babl_type ("CIE u16 L"),
    babl_component ("CIE L"),
    babl_type ("CIE u16 ab"),
    babl_component ("CIE a"),
    babl_type ("CIE u16 ab"),
    babl_component ("CIE b"),
    NULL);

  babl_format_new (
    "name", "CIE xyY float",
    babl_model ("CIE xyY"),

    babl_type ("float"),
    babl_component ("CIE x"),
    babl_component ("CIE y"),
    babl_component ("CIE Y"),
    NULL);

  babl_format_new (
    "name", "CIE xyY alpha float",
    babl_model ("CIE xyY alpha"),

    babl_type ("float"),
    babl_component ("CIE x"),
    babl_component ("CIE y"),
    babl_component ("CIE Y"),
    babl_component ("A"),
    NULL);
}


/******** end floating point RGB/CIE color space conversions **********/

/******** begin  integer RGB/CIE color space conversions **************/

static inline void
convert_double_u8_scaled (const Babl   *conversion,
                          double        min_val,
                          double        max_val,
                          unsigned char min,
                          unsigned char max,
                          char         *src,
                          char         *dst,
                          int           src_pitch,
                          int           dst_pitch,
                          long          n)
{
  while (n--)
    {
      double        dval = *(double *) src;
      unsigned char u8val;

      if (dval < min_val)
        u8val = min;
      else if (dval > max_val)
        u8val = max;
      else
        u8val = rint ((dval - min_val) / (max_val - min_val) * (max - min) + min);

      *(unsigned char *) dst = u8val;
      src                   += src_pitch;
      dst                   += dst_pitch;
    }
}

static inline void
convert_u8_double_scaled (const Babl   *conversion,
                          double        min_val,
                          double        max_val,
                          unsigned char min,
                          unsigned char max,
                          char         *src,
                          char         *dst,
                          int           src_pitch,
                          int           dst_pitch,
                          long          n)
{
  while (n--)
    {
      int    u8val = *(unsigned char *) src;
      double dval;

      if (u8val < min)
        dval = min_val;
      else if (u8val > max)
        dval = max_val;
      else
        dval = (u8val - min) / (double) (max - min) * (max_val - min_val) + min_val;

      (*(double *) dst) = dval;

      dst += dst_pitch;
      src += src_pitch;
    }
}

#define MAKE_CONVERSIONS(name, min_val, max_val, min, max) \
  static void \
  convert_ ## name ## _double (const Babl *c, char *src, \
                               char *dst, \
                               int src_pitch, \
                               int dst_pitch, \
                               long n)        \
  { \
    convert_u8_double_scaled (c, min_val, max_val, min, max, \
                              src, dst, src_pitch, dst_pitch, n); \
  }                                                               \
  static void \
  convert_double_ ## name (const Babl *c, char *src, \
                           char *dst, \
                           int src_pitch, \
                           int dst_pitch, \
                           long n)        \
  { \
    convert_double_u8_scaled (c, min_val, max_val, min, max, \
                              src, dst, src_pitch, dst_pitch, n); \
  }

/* source ICC.1:2004-10 */

MAKE_CONVERSIONS (u8_l, 0.0, 100.0, 0x00, 0xff)
MAKE_CONVERSIONS (u8_ab, -128.0, 127.0, 0x00, 0xff)

#undef MAKE_CONVERSIONS

static inline void
convert_float_u8_scaled (const Babl     *conversion,
                          float          min_val,
                          float          max_val,
                          unsigned char  min,
                          unsigned char  max,
                          char          *src,
                          char          *dst,
                          int            src_pitch,
                          int            dst_pitch,
                          long           n)
{
  while (n--)
    {
      float        dval = *(float *) src;
      unsigned char u8val;

      if (dval < min_val)
        u8val = min;
      else if (dval > max_val)
        u8val = max;
      else
        u8val = rint ((dval - min_val) / (max_val - min_val) * (max - min) + min);

      *(unsigned char *) dst = u8val;
      src                   += src_pitch;
      dst                   += dst_pitch;
    }
}

static inline void
convert_u8_float_scaled (const Babl    *conversion,
                          float         min_val,
                          float         max_val,
                          unsigned char min,
                          unsigned char max,
                          char         *src,
                          char         *dst,
                          int           src_pitch,
                          int           dst_pitch,
                          long          n)
{
  while (n--)
    {
      int    u8val = *(unsigned char *) src;
      float dval;

      if (u8val < min)
        dval = min_val;
      else if (u8val > max)
        dval = max_val;
      else
        dval = (u8val - min) / (float) (max - min) * (max_val - min_val) + min_val;

      (*(float *) dst) = dval;

      dst += dst_pitch;
      src += src_pitch;
    }
}

#define MAKE_CONVERSIONS(name, min_val, max_val, min, max) \
  static void \
  convert_ ## name ## _float (const Babl *c, char *src, \
                               char *dst, \
                               int src_pitch, \
                               int dst_pitch, \
                               long n)        \
  { \
    convert_u8_float_scaled (c, min_val, max_val, min, max, \
                              src, dst, src_pitch, dst_pitch, n); \
  }                                                               \
  static void \
  convert_float_ ## name (const Babl *c, char *src, \
                           char *dst, \
                           int src_pitch, \
                           int dst_pitch, \
                           long n)        \
  { \
    convert_float_u8_scaled (c, min_val, max_val, min, max, \
                              src, dst, src_pitch, dst_pitch, n); \
  }

/* source ICC.1:2004-10 */

MAKE_CONVERSIONS (u8_l, 0.0, 100.0, 0x00, 0xff)
MAKE_CONVERSIONS (u8_ab, -128.0, 127.0, 0x00, 0xff)

#undef MAKE_CONVERSIONS

static void
types_u8 (void)
{
  babl_type_new (
    "CIE u8 L",
    "integer",
    "unsigned",
    "bits", 8,
    "min_val", 0.0,
    "max_val", 100.0,
    NULL
  );

  babl_type_new (
    "CIE u8 ab",
    "integer",
    "unsigned",
    "bits", 8,
    "min_val", -128.0,
    "max_val", 127.0,
    NULL
  );

  babl_conversion_new (
    babl_type ("CIE u8 L"),
    babl_type ("double"),
    "plane", convert_u8_l_double,
    NULL
  );
  babl_conversion_new (
    babl_type ("double"),
    babl_type ("CIE u8 L"),
    "plane", convert_double_u8_l,
    NULL
  );

  babl_conversion_new (
    babl_type ("CIE u8 ab"),
    babl_type ("double"),
    "plane", convert_u8_ab_double,
    NULL
  );
  babl_conversion_new (
    babl_type ("double"),
    babl_type ("CIE u8 ab"),
    "plane", convert_double_u8_ab,
    NULL
  );

  babl_conversion_new (
    babl_type ("CIE u8 L"),
    babl_type ("float"),
    "plane", convert_u8_l_float,
    NULL
  );
  babl_conversion_new (
    babl_type ("float"),
    babl_type ("CIE u8 L"),
    "plane", convert_float_u8_l,
    NULL
  );

  babl_conversion_new (
    babl_type ("CIE u8 ab"),
    babl_type ("float"),
    "plane", convert_u8_ab_float,
    NULL
  );
  babl_conversion_new (
    babl_type ("float"),
    babl_type ("CIE u8 ab"),
    "plane", convert_float_u8_ab,
    NULL
  );
}

static inline void
convert_double_u16_scaled (const Babl    *conversion,
                           double         min_val,
                           double         max_val,
                           unsigned short min,
                           unsigned short max,
                           char          *src,
                           char          *dst,
                           int            src_pitch,
                           int            dst_pitch,
                           long           n)
{
  while (n--)
    {
      double         dval = *(double *) src;
      unsigned short u16val;

      if (dval < min_val)
        u16val = min;
      else if (dval > max_val)
        u16val = max;
      else
        u16val = rint ((dval - min_val) / (max_val - min_val) * (max - min) + min);

      *(unsigned short *) dst = u16val;
      dst                    += dst_pitch;
      src                    += src_pitch;
    }
}

static inline void
convert_u16_double_scaled (const Babl    *conversion,
                           double         min_val,
                           double         max_val,
                           unsigned short min,
                           unsigned short max,
                           char          *src,
                           char          *dst,
                           int            src_pitch,
                           int            dst_pitch,
                           long           n)
{
  while (n--)
    {
      int    u16val = *(unsigned short *) src;
      double dval;

      if (u16val < min)
        dval = min_val;
      else if (u16val > max)
        dval = max_val;
      else
        dval = (u16val - min) / (double) (max - min) * (max_val - min_val) + min_val;

      (*(double *) dst) = dval;
      dst              += dst_pitch;
      src              += src_pitch;
    }
}

#define MAKE_CONVERSIONS(name, min_val, max_val, min, max)      \
  static void \
  convert_ ## name ## _double (const Babl *c, char *src, \
                               char *dst, \
                               int src_pitch, \
                               int dst_pitch, \
                               long n)        \
  { \
    convert_u16_double_scaled (c, min_val, max_val, min, max, \
                               src, dst, src_pitch, dst_pitch, n); \
  }                                                               \
  static void \
  convert_double_ ## name (const Babl *c, char *src, \
                           char *dst, \
                           int src_pitch, \
                           int dst_pitch, \
                           long n)        \
  { \
    convert_double_u16_scaled (c, min_val, max_val, min, max, \
                               src, dst, src_pitch, dst_pitch, n); \
  }

MAKE_CONVERSIONS (u16_l, 0.0, 100.0, 0x00, 0xffff)
MAKE_CONVERSIONS (u16_ab, -128.0, 127.0, 0x00, 0xffff)

#undef MAKE_CONVERSIONS


static inline void
convert_float_u16_scaled (const Babl     *conversion,
                           float          min_val,
                           float          max_val,
                           unsigned short min,
                           unsigned short max,
                           char          *src,
                           char          *dst,
                           int            src_pitch,
                           int            dst_pitch,
                           long           n)
{
  while (n--)
    {
      float         dval = *(float *) src;
      unsigned short u16val;

      if (dval < min_val)
        u16val = min;
      else if (dval > max_val)
        u16val = max;
      else
        u16val = rint ((dval - min_val) / (max_val - min_val) * (max - min) + min);

      *(unsigned short *) dst = u16val;
      dst                    += dst_pitch;
      src                    += src_pitch;
    }
}

static inline void
convert_u16_float_scaled (const Babl     *conversion,
                           float          min_val,
                           float          max_val,
                           unsigned short min,
                           unsigned short max,
                           char          *src,
                           char          *dst,
                           int            src_pitch,
                           int            dst_pitch,
                           long           n)
{
  while (n--)
    {
      int    u16val = *(unsigned short *) src;
      float dval;

      if (u16val < min)
        dval = min_val;
      else if (u16val > max)
        dval = max_val;
      else
        dval = (u16val - min) / (float) (max - min) * (max_val - min_val) + min_val;

      (*(float *) dst) = dval;
      dst              += dst_pitch;
      src              += src_pitch;
    }
}

#define MAKE_CONVERSIONS(name, min_val, max_val, min, max)      \
  static void \
  convert_ ## name ## _float (const Babl *c, char *src, \
                               char *dst, \
                               int src_pitch, \
                               int dst_pitch, \
                               long n)        \
  { \
    convert_u16_float_scaled (c, min_val, max_val, min, max, \
                               src, dst, src_pitch, dst_pitch, n); \
  }                                                               \
  static void \
  convert_float_ ## name (const Babl *c, char *src, \
                           char *dst, \
                           int src_pitch, \
                           int dst_pitch, \
                           long n)        \
  { \
    convert_float_u16_scaled (c, min_val, max_val, min, max, \
                               src, dst, src_pitch, dst_pitch, n); \
  }

MAKE_CONVERSIONS (u16_l, 0.0, 100.0, 0x00, 0xffff)
MAKE_CONVERSIONS (u16_ab, -128.0, 127.0, 0x00, 0xffff)

#undef MAKE_CONVERSIONS

static void
types_u16 (void)
{
  babl_type_new (
    "CIE u16 L",
    "integer",
    "unsigned",
    "bits", 16,
    "min_val", 0.0,
    "max_val", 100.0,
    NULL
  );

  babl_type_new (
    "CIE u16 ab",
    "integer",
    "unsigned",
    "bits", 16,
    "min_val", -128.0,
    "max_val", 127.0,
    NULL
  );


  babl_conversion_new (
    babl_type ("CIE u16 L"),
    babl_type ("double"),
    "plane", convert_u16_l_double,
    NULL
  );
  babl_conversion_new (
    babl_type ("double"),
    babl_type ("CIE u16 L"),
    "plane", convert_double_u16_l,
    NULL
  );

  babl_conversion_new (
    babl_type ("CIE u16 ab"),
    babl_type ("double"),
    "plane", convert_u16_ab_double,
    NULL
  );
  babl_conversion_new (
    babl_type ("double"),
    babl_type ("CIE u16 ab"),
    "plane", convert_double_u16_ab,
    NULL
  );

  babl_conversion_new (
    babl_type ("CIE u16 L"),
    babl_type ("float"),
    "plane", convert_u16_l_float,
    NULL
  );
  babl_conversion_new (
    babl_type ("float"),
    babl_type ("CIE u16 L"),
    "plane", convert_float_u16_l,
    NULL
  );

  babl_conversion_new (
    babl_type ("CIE u16 ab"),
    babl_type ("float"),
    "plane", convert_u16_ab_float,
    NULL
  );
  babl_conversion_new (
    babl_type ("float"),
    babl_type ("CIE u16 ab"),
    "plane", convert_float_u16_ab,
    NULL
  );
}

static void
types (void)
{
  types_u8 ();
  types_u16 ();
}

/******** end  integer RGB/CIE color space conversions ****************/

static void
rgbxyzrgb_init (void)
{
}

static void
rgbcie_init (void)
{
  static int initialized = 0;

  if (!initialized)
    {
      rgbxyzrgb_init ();
      initialized = 1;
    }
}
