/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2005, Øyvind Kolås.
 * Copyright (C) 2009, Martin Nordholts
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

/***********    RGB/CIE color space conversions *********   */

static void  rgbcie_init (void);

static void  ab_to_CHab           (double  a,
                                   double  b,
                                   double *to_C,
                                   double *to_H);

static void  CHab_to_ab           (double  C,
                                   double  H,
                                   double *to_a,
                                   double *to_b);
                                   
static void RGB_to_XYZ            (double R,
                                   double G,
                                   double B,
                                   double *to_X,
                                   double *to_Y,
                                   double *to_Z);

static void XYZ_to_LAB            (double X,
                                   double Y,
                                   double Z,
                                   double *to_L,
                                   double *to_a,
                                   double *to_b
                                   );

static void LAB_to_XYZ            (double L,
                                   double a,
                                   double b,
                                   double *to_X,
                                   double *to_Y,
                                   double *to_Z
                                   );

static void XYZ_to_RGB            (double X,
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

static void
CHab_to_ab (double  C,
            double  H,
            double *to_a,
            double *to_b)
{
  *to_a = cos (H * RADIANS_PER_DEGREE) * C;
  *to_b = sin (H * RADIANS_PER_DEGREE) * C;
}

static void
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

#define MAKE_CONVERSIONS(name, min_val, max_val, min, max)      \
  static long \
  convert_ ## name ## _double (char *src, \
                               char *dst, \
                               int src_pitch, \
                               int dst_pitch, \
                               long n)                               \
  { \
    return convert_u8_double_scaled (min_val, max_val, min, max, \
                                     src, dst, src_pitch, dst_pitch, n); \
  }                                                               \
  static long \
  convert_double_ ## name (char *src, \
                           char *dst, \
                           int src_pitch, \
                           int dst_pitch, \
                           long n)                                 \
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
                               long n)                               \
  { \
    return convert_u16_double_scaled (min_val, max_val, min, max, \
                                      src, dst, src_pitch, dst_pitch, n); \
  }                                                               \
  static long \
  convert_double_ ## name (char *src, \
                           char *dst, \
                           int src_pitch, \
                           int dst_pitch, \
                           long n)                                 \
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
    "id", "CIE u8 ab",
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

/* defines added to make it compile outside gimp */

#ifndef gboolean
#define gboolean    int
#endif
#ifndef FALSE
#define FALSE       0
#endif
#ifndef TRUE
#define TRUE        1
#endif


/* #include "config.h" */
#include <math.h>

static void
rgbxyzrgb_init (void)
{
}

static void
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

static void XYZ_to_RGB (double X,
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
 * */
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
  *to_R = XYZtoRGB[0][0]*X + XYZtoRGB[0][1]*Y + XYZtoRGB[0][2]*Z;
  *to_G = XYZtoRGB[1][0]*X + XYZtoRGB[1][1]*Y + XYZtoRGB[1][2]*Z;
  *to_B = XYZtoRGB[2][0]*X + XYZtoRGB[2][1]*Y + XYZtoRGB[2][2]*Z;

}

static void
XYZ_to_LAB (double X,
            double Y,
            double Z,
            double *to_L,
            double *to_a,
            double *to_b
            )
            
{

  const double kappa   = 903.3;//24389.0/27.0;
  const double epsilon = 0.008856;//216/24389.0;
  
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
 * 
 * */  
  const double X_reference_white = 0.964202880;
  const double Y_reference_white = 1.000000000;
  const double Z_reference_white = 0.824905400;
 
  double x_r = X/X_reference_white;
  double y_r = Y/Y_reference_white;
  double z_r = Z/Z_reference_white;
  
  double f_x, f_y, f_z;
  
  if (x_r > epsilon) f_x = pow(x_r, 1.0 / 3.0);
  else ( f_x = ((kappa * x_r) + 16) / 116.0 );
  
  if (y_r > epsilon) f_y = pow(y_r, 1.0 / 3.0);
  else ( f_y = ((kappa * y_r) + 16) / 116.0 );
  
  if (z_r > epsilon) f_z = pow(z_r, 1.0 / 3.0);
  else ( f_z = ((kappa * z_r) + 16) / 116.0 );


  *to_L = (116.0 * f_y) - 16.0;
  *to_a = 500.0 * (f_x - f_y);
  *to_b = 200.0 * (f_y - f_z);

}

static void LAB_to_XYZ (double L,
                        double a,
                        double b,
                        double *to_X,
                        double *to_Y,
                        double *to_Z)
{

  const double kappa   = 903.3;//24389.0/27.0;
  const double epsilon = 0.008856;//216/24389.0;
  
  double fy, fx, fz, fx_cubed, fy_cubed, fz_cubed;
  double xr, yr, zr;
  
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
 * 
 * */
  const double X_reference_white = 0.964202880;
  const double Y_reference_white = 1.000000000;
  const double Z_reference_white = 0.824905400;
  
  fy = (L + 16.0) / 116.0;
  fy_cubed = fy*fy*fy;
  
  fz = fy - (b / 200.0);
  fz_cubed = fz*fz*fz;
  
  fx = (a / 500.0) + fy;
  fx_cubed = fx*fx*fx;
  
  if (fx_cubed > epsilon) xr = fx_cubed;
  else xr = ((116.0 * fx) - 16) / kappa;

  if ( L > (kappa * epsilon) ) yr = fy_cubed;
  else yr = (L / kappa);

  if (fz_cubed > epsilon) zr = fz_cubed;
  else zr = ( (116.0 * fz) - 16 ) / kappa;
  
  *to_X = xr * X_reference_white;
  *to_Y = yr * Y_reference_white;
  *to_Z = zr * Z_reference_white;

}


/* Call this before using the RGB/CIE color space conversions */
static void
rgbcie_init (void)
{
  static gboolean initialized = FALSE;

  if (!initialized)
    {
      rgbxyzrgb_init ();
      initialized = TRUE;
    }
}

/***********  / RGB/CIE color space conversions *********   */
