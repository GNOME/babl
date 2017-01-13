/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2005, 2014 Øyvind Kolås.
 * Copyright (C) 2009, Martin Nordholts
 * Copyright (C) 2014, Elle Stone
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
 * <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include <math.h>
#include <string.h>

#include "babl.h"
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


int init (void);

static void types (void);
static void components (void);
static void models (void);
static void conversions (void);
static void formats (void);

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
  /* babl_component_new ("CIE X", NULL);
  babl_component_new ("CIE Y", NULL);
  babl_component_new ("CIE Z", NULL);*/
}

static void
models (void)
{
  babl_model_new (
    "name", "CIE Lab",
    babl_component ("CIE L"),
    babl_component ("CIE a"),
    babl_component ("CIE b"),
    NULL);

  babl_model_new (
    "name", "CIE Lab alpha",
    babl_component ("CIE L"),
    babl_component ("CIE a"),
    babl_component ("CIE b"),
    babl_component ("A"),
    NULL);

  babl_model_new (
    "name", "CIE LCH(ab)",
    babl_component ("CIE L"),
    babl_component ("CIE C(ab)"),
    babl_component ("CIE H(ab)"),
    NULL);

  babl_model_new (
    "name", "CIE LCH(ab) alpha",
    babl_component ("CIE L"),
    babl_component ("CIE C(ab)"),
    babl_component ("CIE H(ab)"),
    babl_component ("A"),
    NULL);
  /*babl_model_new (
    "name", "CIE XYZ",
    babl_component ("CIE X"),
    babl_component ("CIE Y"),
    babl_component ("CIE Z"),
    NULL);*/
}

static void  rgbcie_init (void);

static inline void  ab_to_CHab    (double  a,
                                   double  b,
                                   double *to_C,
                                   double *to_H);

static inline void  CHab_to_ab    (double  C,
                                   double  H,
                                   double *to_a,
                                   double *to_b);
                                   
static inline void RGB_to_XYZ     (double R,
                                   double G,
                                   double B,
                                   double *to_X,
                                   double *to_Y,
                                   double *to_Z);

static inline void XYZ_to_LAB     (double X,
                                   double Y,
                                   double Z,
                                   double *to_L,
                                   double *to_a,
                                   double *to_b
                                   );

static inline void LAB_to_XYZ     (double L,
                                   double a,
                                   double b,
                                   double *to_X,
                                   double *to_Y,
                                   double *to_Z
                                   );

static inline void XYZ_to_RGB     (double X,
                                   double Y,
                                   double Z,
                                   double *to_R,
                                   double *to_G,
                                   double *to_B);

static long
rgba_to_lab (char *src,
             char *dst,
             long  n)
{
  while (n--)
    {
      double R  = ((double *) src)[0];
      double G  = ((double *) src)[1];
      double B  = ((double *) src)[2];
      double X, Y, Z, L, a, b;
      
      //convert RGB to XYZ
      RGB_to_XYZ (R, G, B, &X, &Y, &Z);
      
      //convert XYZ to Lab
      XYZ_to_LAB (X, Y, Z, &L, &a, &b);

      ((double *) dst)[0] = L;
      ((double *) dst)[1] = a;
      ((double *) dst)[2] = b;

      src += sizeof (double) * 4;
      dst += sizeof (double) * 3;
    }
  return n;
}

static long
lab_to_rgba (char *src,
             char *dst,
             long  n)
{
  while (n--)
    {
      double L = ((double *) src)[0];
      double a = ((double *) src)[1];
      double b = ((double *) src)[2];

      double X, Y, Z, R, G, B;
      
      //convert Lab to XYZ
      LAB_to_XYZ (L, a, b, &X, &Y, &Z);
      
      //convert XYZ to RGB
      XYZ_to_RGB (X, Y, Z, &R, &G, &B);
      ((double *) dst)[0] = R;
      ((double *) dst)[1] = G;
      ((double *) dst)[2] = B;
      ((double *) dst)[3] = 1.0;

      src += sizeof (double) * 3;
      dst += sizeof (double) * 4;
    }
  return n;
}


static long
rgba_to_laba (char *src,
              char *dst,
              long  n)
{
  while (n--)
    {
      double R     = ((double *) src)[0];
      double G     = ((double *) src)[1];
      double B     = ((double *) src)[2];
      double alpha = ((double *) src)[3];
      double X, Y, Z, L, a, b;
      
      //convert RGB to XYZ
      RGB_to_XYZ (R, G, B, &X, &Y, &Z);
      
      //convert XYZ to Lab
      XYZ_to_LAB (X, Y, Z, &L, &a, &b);

      ((double *) dst)[0] = L;
      ((double *) dst)[1] = a;
      ((double *) dst)[2] = b;
      ((double *) dst)[3] = alpha;

      src += sizeof (double) * 4;
      dst += sizeof (double) * 4;
    }
  return n;
}

static long
laba_to_rgba (char *src,
              char *dst,
              long  n)
{
  while (n--)
    {
      double L     = ((double *) src)[0];
      double a     = ((double *) src)[1];
      double b     = ((double *) src)[2];
      double alpha = ((double *) src)[3];

      double X, Y, Z, R, G, B;
      
      //convert Lab to XYZ
      LAB_to_XYZ (L, a, b, &X, &Y, &Z);
      
      //convert XYZ to RGB
      XYZ_to_RGB (X, Y, Z, &R, &G, &B);
      ((double *) dst)[0] = R;
      ((double *) dst)[1] = G;
      ((double *) dst)[2] = B;
      ((double *) dst)[3] = alpha;

      src += sizeof (double) * 4;
      dst += sizeof (double) * 4;
    }
  return n;
}

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

static long
rgba_to_lchab (char *src,
               char *dst,
               long  n)
{
  while (n--)
    {
      double R = ((double *) src)[0];
      double G = ((double *) src)[1];
      double B = ((double *) src)[2];
      double X, Y, Z, L, a, b, C, H;

      //convert RGB to XYZ
      RGB_to_XYZ (R, G, B, &X, &Y, &Z);
      
      //convert XYZ to Lab
      XYZ_to_LAB (X, Y, Z, &L, &a, &b);
      
      //convert Lab to LCH(ab)
      ab_to_CHab (a, b, &C, &H);

      ((double *) dst)[0] = L;
      ((double *) dst)[1] = C;
      ((double *) dst)[2] = H;

      src += sizeof (double) * 4;
      dst += sizeof (double) * 3;
    }
  return n;
}

static long
lchab_to_rgba (char *src,
               char *dst,
               long  n)
{
  while (n--)
    {
      double L = ((double *) src)[0];
      double C = ((double *) src)[1];
      double H = ((double *) src)[2];
      double a, b, X, Y, Z, R, G, B;
      
      //Convert LCH(ab) to Lab
      CHab_to_ab (C, H, &a, &b);
      
      //Convert LAB to XYZ
      LAB_to_XYZ (L, a, b, &X, &Y, &Z);
      
      //Convert XYZ to RGB
      XYZ_to_RGB (X, Y, Z, &R, &G, &B);

      ((double *) dst)[0] = R;
      ((double *) dst)[1] = G;
      ((double *) dst)[2] = B;
      ((double *) dst)[3] = 1.0;

      src += sizeof (double) * 3;
      dst += sizeof (double) * 4;
    }
  return n;
}


static long
rgba_to_lchaba (char *src,
                char *dst,
                long  n)
{
  while (n--)
    {
      double R = ((double *) src)[0];
      double G = ((double *) src)[1];
      double B = ((double *) src)[2];
      double alpha = ((double *) src)[3];
      double X, Y, Z, L, a, b, C, H;

      //convert RGB to XYZ
      RGB_to_XYZ (R, G, B, &X, &Y, &Z);
      
      //convert XYZ to Lab
      XYZ_to_LAB (X, Y, Z, &L, &a, &b);
      
      //convert Lab to LCH(ab)
      ab_to_CHab (a, b, &C, &H);

      ((double *) dst)[0] = L;
      ((double *) dst)[1] = C;
      ((double *) dst)[2] = H;
      ((double *) dst)[3] = alpha;

      src += sizeof (double) * 4;
      dst += sizeof (double) * 4;
    }
  return n;
}

static long
lchaba_to_rgba (char *src,
                char *dst,
                long  n)
{
  while (n--)
    {
      double L     = ((double *) src)[0];
      double C     = ((double *) src)[1];
      double H     = ((double *) src)[2];
      double alpha = ((double *) src)[3];
      double a, b, X, Y, Z, R, G, B;
      
      //Convert LCH(ab) to Lab
      CHab_to_ab (C, H, &a, &b);
      
      //Convert Lab to XYZ
      LAB_to_XYZ (L, a, b, &X, &Y, &Z);
      
      //Convert XYZ to RGB
      XYZ_to_RGB (X, Y, Z, &R, &G, &B);
      
      ((double *) dst)[0] = R;
      ((double *) dst)[1] = G;
      ((double *) dst)[2] = B;
      ((double *) dst)[3] = alpha;

      src += sizeof (double) * 4;
      dst += sizeof (double) * 4;
    }
  return n;
}

static inline float
cubef (float f)
{
  return f * f * f;
}

/* origin: FreeBSD /usr/src/lib/msun/src/s_cbrtf.c */
/*
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 * Debugged and optimized by Bruce D. Evans.
 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/* _cbrtf(x)
 * Return cube root of x
 */

#include <math.h>
#include <stdint.h>

static const unsigned
B1 = 709958130, /* B1 = (127-127.0/3-0.03306235651)*2**23 */
B2 = 642849266; /* B2 = (127-127.0/3-24/3-0.03306235651)*2**23 */

static inline float _cbrtf(float x)
{
	float r,T;
	union {float f; uint32_t i;} u = {x};
	uint32_t hx = u.i & 0x7fffffff;

	if (hx >= 0x7f800000)  /* cbrt(NaN,INF) is itself */
		return x + x;

	/* rough cbrt to 5 bits */
	if (hx < 0x00800000) {  /* zero or subnormal? */
		if (hx == 0)
			return x;  /* cbrt(+-0) is itself */
		u.f = x*0x1p24f;
		hx = u.i & 0x7fffffff;
		hx = hx/3 + B2;
	} else
		hx = hx/3 + B1;
	u.i &= 0x80000000;
	u.i |= hx;

	T = u.f;
	r = T*T*T;
	T = T*((float)x+x+r)/(x+r+r);

	r = T*T*T;
	T = T*((float)x+x+r)/(x+r+r);

	return T;
}

static long
Yaf_to_Laf (float *src,
            float *dst,
            long   samples)
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

  return samples;
}


static long
rgbf_to_Labf (float *src,
              float *dst,
              long   samples)
{
  long n = samples;

  while (n--)
    {
      float r = src[0];
      float g = src[1];
      float b = src[2];

      float xr = 0.43603516f / D50_WHITE_REF_X * r + 0.38511658f / D50_WHITE_REF_X * g + 0.14305115f / D50_WHITE_REF_X * b;
      float yr = 0.22248840f / D50_WHITE_REF_Y * r + 0.71690369f / D50_WHITE_REF_Y * g + 0.06060791f / D50_WHITE_REF_Y * b;
      float zr = 0.01391602f / D50_WHITE_REF_Z * r + 0.09706116f / D50_WHITE_REF_Z * g + 0.71392822f / D50_WHITE_REF_Z * b;

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

  return samples;
}

static long
rgbaf_to_Labaf (float *src,
                float *dst,
                long   samples)
{
  long n = samples;

  while (n--)
    {
      float r = src[0];
      float g = src[1];
      float b = src[2];
      float a = src[3];

      float xr = 0.43603516f / D50_WHITE_REF_X * r + 0.38511658f / D50_WHITE_REF_X * g + 0.14305115f / D50_WHITE_REF_X * b;
      float yr = 0.22248840f / D50_WHITE_REF_Y * r + 0.71690369f / D50_WHITE_REF_Y * g + 0.06060791f / D50_WHITE_REF_Y * b;
      float zr = 0.01391602f / D50_WHITE_REF_Z * r + 0.09706116f / D50_WHITE_REF_Z * g + 0.71392822f / D50_WHITE_REF_Z * b;

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

  return samples;
}

static long
Labf_to_rgbf (float *src,
                float *dst,
                long   samples)
{
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

      float r =  3.134274799724f * D50_WHITE_REF_X * xr -1.617275708956f * D50_WHITE_REF_Y * yr -0.490724283042f * D50_WHITE_REF_Z * zr;
      float g = -0.978795575994f * D50_WHITE_REF_X * xr +1.916161689117f * D50_WHITE_REF_Y * yr +0.033453331711f * D50_WHITE_REF_Z * zr;
      float b =  0.071976988401f * D50_WHITE_REF_X * xr -0.228984974402f * D50_WHITE_REF_Y * yr +1.405718224383f * D50_WHITE_REF_Z * zr;

      dst[0] = r;
      dst[1] = g;
      dst[2] = b;

      src += 3;
      dst += 3;
    }

  return samples;
}

static long
Labaf_to_rgbaf (float *src,
                float *dst,
                long   samples)
{
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

      float r =  3.134274799724f * D50_WHITE_REF_X * xr -1.617275708956f * D50_WHITE_REF_Y * yr -0.490724283042f * D50_WHITE_REF_Z * zr;
      float g = -0.978795575994f * D50_WHITE_REF_X * xr +1.916161689117f * D50_WHITE_REF_Y * yr +0.033453331711f * D50_WHITE_REF_Z * zr;
      float b =  0.071976988401f * D50_WHITE_REF_X * xr -0.228984974402f * D50_WHITE_REF_Y * yr +1.405718224383f * D50_WHITE_REF_Z * zr;

      dst[0] = r;
      dst[1] = g;
      dst[2] = b;
      dst[3] = a;

      src += 4;
      dst += 4;
    }

  return samples;
}

static void
conversions (void)
{
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
    babl_format ("RGB float"),
    babl_format ("CIE Lab float"),
    "linear", rgbf_to_Labf,
    NULL
  );
  babl_conversion_new (
    babl_format ("CIE Lab float"),
    babl_format ("RGB float"),
    "linear", Labf_to_rgbf,
    NULL
  );
  babl_conversion_new (
    babl_format ("RGBA float"),
    babl_format ("CIE Lab alpha float"),
    "linear", rgbaf_to_Labaf,
    NULL
  );
  babl_conversion_new (
    babl_format ("CIE Lab alpha float"),
    babl_format ("RGBA float"),
    "linear", Labaf_to_rgbaf,
    NULL
  );
  babl_conversion_new (
    babl_format ("YA float"),
    babl_format ("CIE L alpha float"),
    "linear", Yaf_to_Laf,
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
  /*babl_conversion_new (
    babl_model ("RGBA"),
    babl_model ("CIE XYZ"),
    "linear", RGB_to_XYZ,
    NULL
  );
  babl_conversion_new (
    babl_model ("CIE XYZ"),
    babl_model ("RGBA"),
    "linear", XYZ_to_RGB,
    NULL
  );*/

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
    
  /*babl_format_new (
    "name", "CIE XYZ float",
    babl_model ("CIE XYZ"),

    babl_type ("float"),
    babl_component ("CIE X"),
    babl_component ("CIE Y"),
    babl_component ("CIE Z"),
    NULL);*/

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
}


static inline long
convert_double_u8_scaled (double        min_val,
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
  return n;
}

static inline long
convert_u8_double_scaled (double        min_val,
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
  return n;
}

#define MAKE_CONVERSIONS(name, min_val, max_val, min, max) \
  static long \
  convert_ ## name ## _double (char *src, \
                               char *dst, \
                               int src_pitch, \
                               int dst_pitch, \
                               long n)        \
  { \
    return convert_u8_double_scaled (min_val, max_val, min, max, \
                                     src, dst, src_pitch, dst_pitch, n); \
  }                                                               \
  static long \
  convert_double_ ## name (char *src, \
                           char *dst, \
                           int src_pitch, \
                           int dst_pitch, \
                           long n)        \
  { \
    return convert_double_u8_scaled (min_val, max_val, min, max, \
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
}

static inline long
convert_double_u16_scaled (double         min_val,
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
  return n;
}

static inline long
convert_u16_double_scaled (double         min_val,
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
  return n;
}

#define MAKE_CONVERSIONS(name, min_val, max_val, min, max)      \
  static long \
  convert_ ## name ## _double (char *src, \
                               char *dst, \
                               int src_pitch, \
                               int dst_pitch, \
                               long n)        \
  { \
    return convert_u16_double_scaled (min_val, max_val, min, max, \
                                      src, dst, src_pitch, dst_pitch, n); \
  }                                                               \
  static long \
  convert_double_ ## name (char *src, \
                           char *dst, \
                           int src_pitch, \
                           int dst_pitch, \
                           long n)        \
  { \
    return convert_double_u16_scaled (min_val, max_val, min, max, \
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
}

static void
types (void)
{
  types_u8 ();
  types_u16 ();
}


static void
rgbxyzrgb_init (void)
{
}

static inline void
RGB_to_XYZ (double R,
            double G,
            double B,
            double *to_X,
            double *to_Y,
            double *to_Z)
{
  double RGBtoXYZ[3][3];

/*
 * The variables below hard-code the D50-adapted sRGB RGB to XYZ matrix.
 *
 * In a properly ICC profile color-managed application, this matrix
 * is retrieved from the image's ICC profile's RGB colorants.
 *
 * */
  RGBtoXYZ[0][0]= 0.43603516;
  RGBtoXYZ[0][1]= 0.38511658;
  RGBtoXYZ[0][2]= 0.14305115;
  RGBtoXYZ[1][0]= 0.22248840;
  RGBtoXYZ[1][1]= 0.71690369;
  RGBtoXYZ[1][2]= 0.06060791;
  RGBtoXYZ[2][0]= 0.01391602;
  RGBtoXYZ[2][1]= 0.09706116;
  RGBtoXYZ[2][2]= 0.71392822;

/* Convert RGB to XYZ */
  *to_X = RGBtoXYZ[0][0]*R + RGBtoXYZ[0][1]*G + RGBtoXYZ[0][2]*B;
  *to_Y = RGBtoXYZ[1][0]*R + RGBtoXYZ[1][1]*G + RGBtoXYZ[1][2]*B;
  *to_Z = RGBtoXYZ[2][0]*R + RGBtoXYZ[2][1]*G + RGBtoXYZ[2][2]*B;

}

static inline void
XYZ_to_RGB (double X,
            double Y,
            double Z,
            double *to_R,
            double *to_G,
            double *to_B)
{
  double XYZtoRGB[3][3];

/*
 * The variables below hard-code the inverse of
 * the D50-adapted sRGB RGB to XYZ matrix.
 *
 * In a properly ICC profile color-managed application,
 * this matrix is the inverse of the matrix
 * retrieved from the image's ICC profile's RGB colorants.
 *
 */
  XYZtoRGB[0][0]=  3.134274799724;
  XYZtoRGB[0][1]= -1.617275708956;
  XYZtoRGB[0][2]= -0.490724283042;
  XYZtoRGB[1][0]= -0.978795575994;
  XYZtoRGB[1][1]=  1.916161689117;
  XYZtoRGB[1][2]=  0.033453331711;
  XYZtoRGB[2][0]=  0.071976988401;
  XYZtoRGB[2][1]= -0.228984974402;
  XYZtoRGB[2][2]=  1.405718224383;

/* Convert XYZ to RGB */
  *to_R = XYZtoRGB[0][0] * X + XYZtoRGB[0][1] * Y + XYZtoRGB[0][2] * Z;
  *to_G = XYZtoRGB[1][0] * X + XYZtoRGB[1][1] * Y + XYZtoRGB[1][2] * Z;
  *to_B = XYZtoRGB[2][0] * X + XYZtoRGB[2][1] * Y + XYZtoRGB[2][2] * Z;
}

static inline void
XYZ_to_LAB (double X,
            double Y,
            double Z,
            double *to_L,
            double *to_a,
            double *to_b)
{
  double f_x, f_y, f_z;
  
  double x_r = X / D50_WHITE_REF_X;
  double y_r = Y / D50_WHITE_REF_Y;
  double z_r = Z / D50_WHITE_REF_Z;

  if (x_r > LAB_EPSILON) f_x = pow(x_r, 1.0 / 3.0);
  else ( f_x = ((LAB_KAPPA * x_r) + 16) / 116.0 );

  if (y_r > LAB_EPSILON) f_y = pow(y_r, 1.0 / 3.0);
  else ( f_y = ((LAB_KAPPA * y_r) + 16) / 116.0 );

  if (z_r > LAB_EPSILON) f_z = pow(z_r, 1.0 / 3.0);
  else ( f_z = ((LAB_KAPPA * z_r) + 16) / 116.0 );

  *to_L = (116.0 * f_y) - 16.0;
  *to_a = 500.0 * (f_x - f_y);
  *to_b = 200.0 * (f_y - f_z);
}

static inline void
LAB_to_XYZ (double L,
            double a,
            double b,
            double *to_X,
            double *to_Y,
            double *to_Z)
{
  double fy, fx, fz, fx_cubed, fy_cubed, fz_cubed;
  double xr, yr, zr;

  fy = (L + 16.0) / 116.0;
  fy_cubed = fy*fy*fy;

  fz = fy - (b / 200.0);
  fz_cubed = fz*fz*fz;

  fx = (a / 500.0) + fy;
  fx_cubed = fx*fx*fx;

  if (fx_cubed > LAB_EPSILON) xr = fx_cubed;
  else xr = ((116.0 * fx) - 16) / LAB_KAPPA;

  if ( L > (LAB_KAPPA * LAB_EPSILON) ) yr = fy_cubed;
  else yr = (L / LAB_KAPPA);

  if (fz_cubed > LAB_EPSILON) zr = fz_cubed;
  else zr = ( (116.0 * fz) - 16 ) / LAB_KAPPA;

  *to_X = xr * D50_WHITE_REF_X;
  *to_Y = yr * D50_WHITE_REF_Y;
  *to_Z = zr * D50_WHITE_REF_Z;
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
