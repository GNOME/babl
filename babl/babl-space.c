/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2017 Øyvind Kolås.
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

#define MAX_SPACES   100

#include "config.h"
#include "babl-internal.h"
#include "base/util.h"

static BablSpace space_db[MAX_SPACES];

static void babl_chromatic_adaptation_matrix (const double *whitepoint,
                                              const double *target_whitepoint,
                                              double       *chad_matrix)
{
  double bradford[9]={ 0.8951000, 0.2664000, -0.1614000,
                      -0.7502000, 1.7135000,  0.0367000,
                      0.0389000, -0.0685000, 1.0296000};
  double bradford_inv[9]={0.9869929,-0.1470543, 0.1599627,
                          0.4323053, 0.5183603, 0.0492912,
                          -0.0085287, 0.0400428, 0.9684867};

  double vec_a[3];
  double vec_b[3];

  babl_matrix_mul_vector (bradford, whitepoint, vec_a);
  babl_matrix_mul_vector (bradford, target_whitepoint, vec_b);

  memset (chad_matrix, 0, sizeof (double) * 9);

  chad_matrix[0] = vec_b[0] / vec_a[0];
  chad_matrix[4] = vec_b[1] / vec_a[1];
  chad_matrix[8] = vec_b[2] / vec_a[2];

  babl_matrix_mul_matrix (bradford_inv, chad_matrix, chad_matrix);
  babl_matrix_mul_matrix (chad_matrix, bradford, chad_matrix);
}

#define LAB_EPSILON       (216.0f / 24389.0f)
#define LAB_KAPPA         (24389.0f / 27.0f)

#if 1
#define D50_WHITE_REF_X   0.964202880f
#define D50_WHITE_REF_Y   1.000000000f
#define D50_WHITE_REF_Z   0.824905400f
#else
#define D50_WHITE_REF_X   0.964200000f
#define D50_WHITE_REF_Y   1.000000000f
#define D50_WHITE_REF_Z   0.824900000f
#endif

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

/* round all values to s15f16 precision and brute-force
 * jitter +/- 1 all entries for best uniform gray axis - this
 * also optimizes the accuracy of the matrix for floating point
 * computations.
 *
 * the inverse matrix should be equalized against the original
 * matrix looking for the bit-exact inverse of this integer-solution.
 *
 */
static void 
babl_matrix_equalize (double *in_mat)
{
  double mat[9];
  int j[9];
  int best_j[9];
  double in[12] = {1.0,  1.0,  1.0,   // white
                   0.0,  0.0,  0.0,   // black
                   0.5,  0.5,  0.5,   // gray
                   0.33, 0.33, 0.33}; // grey
  double out[12] = {};
  double lab[12] = {};
  double best_error = 1000000.0;
  int i;

  for (i = 0; i < 9; i++)
    best_j[i] = 0;

  for (j[0] = -1; j[0] <= 1; j[0]++)
  for (j[1] = -1; j[1] <= 1; j[1]++)
  for (j[2] = -1; j[2] <= 1; j[2]++)
  for (j[3] = -1; j[3] <= 1; j[3]++)
  for (j[4] = -1; j[4] <= 1; j[4]++)
  for (j[5] = -1; j[5] <= 1; j[5]++)
  for (j[6] = -1; j[6] <= 1; j[6]++)
  for (j[7] = -1; j[7] <= 1; j[7]++)
  for (j[8] = -1; j[8] <= 1; j[8]++)
  {
    double error = 0;

    for (i = 0; i < 9; i++)
    {
      int32_t val = in_mat[i] * 65536.0 + 0.5f;
      mat[i] = val / 65536.0 + j[i] / 65536.0;
    }
    for (i = 0; i < 4; i++)
    {
      babl_matrix_mul_vector (mat, &in[i*3], &out[i*3]);
    }
    for (i = 0; i < 4; i++)
    {
      XYZ_to_LAB (out[i*3+0], out[i*3+1], out[i*3+2],
                 &lab[i*3+0], &lab[i*3+1], &lab[i*3+2]);
    }
#define square(a) ((a)*(a))
    error += square (lab[0*3+0]-100.0f); // L white = 100.0
    error += square (lab[1*3+0]-0.0f);   // L black = 0.0

    for (i = 0; i < 4; i++)
    {
      error += square (lab[i*3+1]);      // a = 0.0
      error += square (lab[i*3+2]);      // b = 0.0
    }
#undef square
    if (error <= best_error)
    {
      best_error = error;
      memcpy (&best_j[0], &j[0], sizeof (best_j));
    }
  }
  for (i = 0; i < 9; i++)
  {
    int32_t val = in_mat[i] * 65536.0 + 0.5f;
    in_mat[i] = val / 65536.0 + best_j[i] / 65536.0;
  }
}

static void 
babl_space_compute_matrices (BablSpace *space, 
                             BablSpaceFlags equalize_matrix)
{
#define _ space->
  /* transform spaces xy(Y) specified data to XYZ */
  double red_XYZ[3]        = { _ xr / _ yr, 1.0, ( 1.0 - _ xr - _ yr) / _ yr};
  double green_XYZ[3]      = { _ xg / _ yg, 1.0, ( 1.0 - _ xg - _ yg) / _ yg};
  double blue_XYZ[3]       = { _ xb / _ yb, 1.0, ( 1.0 - _ xb - _ yb) / _ yb};
  double whitepoint_XYZ[3] = { _ xw / _ yw, 1.0, ( 1.0 - _ xw - _ yw) / _ yw};
  double D50_XYZ[3]        = {0.96420288, 1.0, 0.82490540};
#undef _

  double mat[9] = {red_XYZ[0], green_XYZ[0], blue_XYZ[0],
                   red_XYZ[1], green_XYZ[1], blue_XYZ[1],
                   red_XYZ[2], green_XYZ[2], blue_XYZ[2]};
  double inv_mat[9];
  double S[3];
  double chad[9];

  babl_matrix_invert (mat, inv_mat);
  babl_matrix_mul_vector (inv_mat, whitepoint_XYZ, S);

  mat[0] *= S[0]; mat[1] *= S[1]; mat[2] *= S[2];
  mat[3] *= S[0]; mat[4] *= S[1]; mat[5] *= S[2];
  mat[6] *= S[0]; mat[7] *= S[1]; mat[8] *= S[2];

  babl_chromatic_adaptation_matrix (whitepoint_XYZ, D50_XYZ, chad);

  babl_matrix_mul_matrix (chad, mat, mat);

  if (equalize_matrix)
    babl_matrix_equalize (mat);

  memcpy (space->RGBtoXYZ, mat, sizeof (mat));

  babl_matrix_invert (mat, inv_mat);

  memcpy (space->XYZtoRGB, inv_mat, sizeof (mat));

  babl_matrix_to_float (space->RGBtoXYZ, space->RGBtoXYZf);
  babl_matrix_to_float (space->XYZtoRGB, space->XYZtoRGBf);
}

const Babl *
babl_space (const char *name)
{
  int i;
  for (i = 0; space_db[i].instance.class_type; i++)
    if (!strcmp (space_db[i].instance.name, name))
      return (Babl*)&space_db[i];
  return NULL;
}

Babl *
_babl_space_for_lcms (const char *icc_data, 
                      int         icc_length)
{
  int i=0;
  BablSpace space;


  for (i = 0; space_db[i].instance.class_type; i++)
  {
    if (space_db[i].icc_length ==
        icc_length &&
        (memcmp (space_db[i].icc_profile, icc_data, icc_length) == 0))
    {
        return (void*)&space_db[i];
    }
  }

  memset (&space, 0, sizeof(space));
  space.instance.class_type = BABL_SPACE;
  space.instance.id         = 0;

  if (i >= MAX_SPACES-1)
  {
    babl_log ("too many BablSpaces");
    return NULL;
  }

  /* initialize it with copy of srgb content */
  {
    const BablSpace *srgb = &babl_space("sRGB")->space;
    memcpy (&space.xw,
            &srgb->xw,
((char*)&srgb->icc_profile -
(char*)&srgb->xw));
  }

  space_db[i]=space;
  space_db[i].instance.name = space_db[i].name;
  snprintf (space_db[i].name, sizeof (space_db[i].name), "space-lcms-%i", i);


  return (Babl*)&space_db[i];
}

const Babl *
babl_space_from_rgbxyz_matrix (const char *name,
                               double wx, double wy, double wz,
                               double rx, double gx, double bx,
                               double ry, double gy, double by,
                               double rz, double gz, double bz,
                               const Babl *trc_red,
                               const Babl *trc_green,
                               const Babl *trc_blue)
{
  int i=0;
  BablSpace space;
  space.instance.class_type = BABL_SPACE;
  space.instance.id         = 0;

  /* transplant matrixes */

  space.RGBtoXYZ[0] = rx;
  space.RGBtoXYZ[1] = gx;
  space.RGBtoXYZ[2] = bx;
  space.RGBtoXYZ[3] = ry;
  space.RGBtoXYZ[4] = gy;
  space.RGBtoXYZ[5] = by;
  space.RGBtoXYZ[6] = rz;
  space.RGBtoXYZ[7] = gz;
  space.RGBtoXYZ[8] = bz;

  babl_matrix_invert (space.RGBtoXYZ, space.XYZtoRGB);

  babl_matrix_to_float (space.RGBtoXYZ, space.RGBtoXYZf);
  babl_matrix_to_float (space.XYZtoRGB, space.XYZtoRGBf);

  {
    double red[3]={1.,.0,.0};
    double xyz[3]={1.,.0,.0};
    _babl_space_to_xyz ((Babl*)&space, &red[0], &xyz[0]);
    space.xr = xyz[0] / (xyz[0] + xyz[1] + xyz[2]);
    space.yr = xyz[1] / (xyz[0] + xyz[1] + xyz[2]);
  }
  {
    double green[3]={0.,1.0,.0};
    double xyz[3]={0.,1.0,.0};
    _babl_space_to_xyz ((Babl*)&space, &green[0], &xyz[0]);
    space.xg = xyz[0] / (xyz[0] + xyz[1] + xyz[2]);
    space.yg = xyz[1] / (xyz[0] + xyz[1] + xyz[2]);
  }
  {
    double blue[3]={0.,.0,1.0};
    double xyz[3]={0.,1.0,.0};
    _babl_space_to_xyz ((Babl*)&space, &blue[0], &xyz[0]);
    space.xb = xyz[0] / (xyz[0] + xyz[1] + xyz[2]);
    space.yb = xyz[1] / (xyz[0] + xyz[1] + xyz[2]);
  }
  space.xw = wx / (wx+wy+wz);
  space.yw = wy / (wx+wy+wz);

  space.whitepoint[0] = wx;
  space.whitepoint[1] = wy;
  space.whitepoint[2] = wz;

  space.trc[0] = trc_red;
  space.trc[1] = trc_green?trc_green:trc_red;
  space.trc[2] = trc_blue?trc_blue:trc_red;

  for (i = 0; space_db[i].instance.class_type; i++)
  {
    int offset = ((char*)&space_db[i].xr) - (char*)(&space_db[i]);
    int size   = ((char*)&space_db[i].trc) + sizeof(space_db[i].trc) - ((char*)&space_db[i].xr);

    if (memcmp ((char*)(&space_db[i]) + offset, ((char*)&space) + offset, size)==0)
      {
        return (void*)&space_db[i];
      }
  }
  if (i >= MAX_SPACES-1)
  {
    babl_log ("too many BablSpaces");
    return NULL;
  }

  space_db[i]=space;
  space_db[i].instance.name = space_db[i].name;
  if (name)
    snprintf (space_db[i].name, sizeof (space_db[i].name), "%s", name);
  else
          /* XXX: this can get longer than 256bytes ! */
    snprintf (space_db[i].name, sizeof (space_db[i].name),
             "space-%.4f,%.4f_%.4f,%.4f_%.4f,%.4f_%.4f,%.4f_%s,%s,%s",
             wx,wy,rx,ry,bx,by,gx,gy,babl_get_name (space.trc[0]),
             babl_get_name(space.trc[1]), babl_get_name(space.trc[2]));

  babl_space_get_icc ((Babl*)&space_db[i], NULL);
  return (Babl*)&space_db[i];
}

const Babl * 
babl_space_from_chromaticities (const char *name,
                                double wx, double wy,
                                double rx, double ry,
                                double gx, double gy,
                                double bx, double by,
                                const Babl *trc_red,
                                const Babl *trc_green,
                                const Babl *trc_blue,
                                BablSpaceFlags flags)
{
  int i=0;
  static BablSpace space;
  space.instance.class_type = BABL_SPACE;
  space.instance.id         = 0;

  space.xr = rx;
  space.yr = ry;
  space.xg = gx;
  space.yg = gy;
  space.xb = bx;
  space.yb = by;
  space.xw = wx;
  space.yw = wy;
  space.trc[0] = trc_red;
  space.trc[1] = trc_green?trc_green:trc_red;
  space.trc[2] = trc_blue?trc_blue:trc_red;

  space.whitepoint[0] = wx / wy;
  space.whitepoint[1] = 1.0;
  space.whitepoint[2] = (1.0 - wx - wy) / wy;

  for (i = 0; space_db[i].instance.class_type; i++)
  {
    int offset = ((char*)&space_db[i].xr) - (char*)(&space_db[i]);
    int size   = ((char*)&space_db[i].trc) + sizeof(space_db[i].trc) - ((char*)&space_db[i].xr);

    if (memcmp ((char*)(&space_db[i]) + offset, ((char*)&space) + offset, size)==0)
      {
        return (void*)&space_db[i];
      }
  }
  if (i >= MAX_SPACES-1)
  {
    babl_log ("too many BablSpaces");
    return NULL;
  }
  space_db[i]=space;
  space_db[i].instance.name = space_db[i].name;
  if (name)
    snprintf (space_db[i].name, sizeof (space_db[i].name), "%s", name);
  else
          /* XXX: this can get longer than 256bytes ! */
    snprintf (space_db[i].name, sizeof (space_db[i].name),
             "space-%.4f,%.4f_%.4f,%.4f_%.4f,%.4f_%.4f,%.4f_%s,%s,%s",
             wx,wy,rx,ry,bx,by,gx,gy,babl_get_name (space.trc[0]),
             babl_get_name(space.trc[1]), babl_get_name(space.trc[2]));

  /* compute matrixes */
  babl_space_compute_matrices (&space_db[i], flags);

  babl_space_get_icc ((Babl*)&space_db[i], NULL);
  return (Babl*)&space_db[i];
}

void
babl_space_class_for_each (BablEachFunction each_fun,
                           void            *user_data)
{
  int i=0;
  for (i = 0; space_db[i].instance.class_type; i++)
    if (each_fun (BABL (&space_db[i]), user_data))
      return;
}

void
babl_space_class_init (void)
{
#if 0
  babl_space_from_chromaticities ("sRGB",
               0.3127,  0.3290, /* D65 */
               0.6400,  0.3300,
               0.3000,  0.6000,
               0.1500,  0.0600,
               babl_trc("sRGB"), NULL, NULL, 1);
#else
  babl_space_from_chromaticities ("sRGB",
                0.3127,  0.3290, /* D65 */
                0.639998686, 0.330010138,
                0.300003784, 0.600003357,
                0.150002046, 0.059997204,
                babl_trc("sRGB"), NULL, NULL,
                0);
  /* hard-coded pre-quantized values - to match exactly what is used in standards see issue #18 */
#endif

  /* sRGB with linear TRCs is scRGB.
   */
  babl_space_from_chromaticities ("scRGB",
                0.3127,  0.3290, /* D65 */
                0.639998686, 0.330010138,
                0.300003784, 0.600003357,
                0.150002046, 0.059997204,
                babl_trc("linear"), NULL, NULL,
                0);
  /* hard-coded pre-quantized values - to match exactly what is used in standards see issue #18 */

  babl_space_from_chromaticities ("Rec2020",
               0.3127,  0.3290, /* D65 */
               0.708,  0.292,
               0.170,  0.797,
               0.131,  0.046,
               // XXX: is using sRGB TRC right?
               babl_trc("sRGB"), NULL, NULL, 1);

  babl_space_from_chromaticities (
      "Adobish",  /* a space that can be used as a place-holder for a sRGB like
space with displaced green coordinates from a big graphics software vendor that
would rather not it's name be directly used when referring to this color space,
this color space isn't exactly like theirs but close enough with babls own
computations of uniform gray axis */
      0.3127,  0.3290, /* D65 */
      0.6400,  0.3300,
      0.2100,  0.7100,
      0.1500,  0.0600,
      babl_trc("2.2"), NULL, NULL, 1);

  babl_space_from_chromaticities (
      "ProPhoto",
      0.34567, 0.3585,  /* D50 */
      0.7347,  0.2653,
      0.1596,  0.8404,
      0.0366,  0.0001,
      babl_trc("1.8"), NULL, NULL, 1);

  babl_space_from_chromaticities (
      "Apple",
      0.3127,  0.3290, /* D65 */
      0.6250,  0.3400,
      0.2800,  0.5950,
      0.1550,  0.0700,
      babl_trc("1.8"), NULL, NULL, 1);

#if 0
  babl_space_from_chromaticities (
     "WideGamut",
     0.34567, 0.3585,  /* D50 */
     0.7350,  0.2650,
     0.1150,  0.8260,
     0.1570,  0.0180,
     babl_trc("2.2"), NULL, NULL, 1);

  babl_space_from_chromaticities (
      "Best",
      0.34567, 0.3585,  /* D50 */
      0.7347,  0.2653,
      0.2150,  0.7750,
      0.1300,  0.0350,
      babl_trc("2.2"), NULL, NULL, 1);

  babl_space_from_chromaticities (
      "Beta",
      0.34567, 0.3585,  /* D50 */
      0.6888,  0.3112,
      0.1986,  0.7551,
      0.1265,  0.0352,
      babl_trc("2.2"), NULL, NULL, 1);

  babl_space_from_chromaticities (
      "Bruce",
      0.3127,  0.3290, /* D65 */
      0.6400,  0.3300,
      0.2800,  0.6500,
      0.1500,  0.0600,
      babl_trc("1.8"), NULL, NULL);

  babl_space_from_chromaticities (
      "PAL",
      0.3127,  0.3290, /* D65 */
      0.6400,  0.3300,
      0.2900,  0.6000,
      0.1500,  0.0600,
      babl_trc("2.2"), NULL, NULL, 1);

  babl_space_from_chromaticities (
      "SMPTE-C",
      0.3127,  0.3290, /* D65 */
      0.6300,  0.3300,
      0.3100,  0.5950,
      0.1550,  0.0700,
      babl_trc("2.2"), NULL, NULL, 1);

  babl_space_from_chromaticities (
      "ColorMatch",
      0.34567, 0.3585,  /* D50 */
      0.6300,  0.3400,
      0.2950,  0.6050,
      0.1500,  0.0750,
      babl_trc("1.8"), NULL, NULL, 1);

  babl_space_from_chromaticities (
     "Don RGB 4",
     0.34567, 0.3585,  /* D50 */
     0.6960,  0.3000,
     0.2150,  0.7650,
     0.1300,  0.0350,
     babl_trc("1.8"), NULL, NULL, 1);
#endif

  babl_space_from_chromaticities (
     "ACEScg",
      0.32168, 0.33767,
      0.713, 0.293,
      0.165, 0.830,
      0.128, 0.044,
      babl_trc("linear"), NULL, NULL, 1);

  babl_space_from_chromaticities (
     "ACES2065-1",
      0.32168, 0.33767,
      0.7347, 0.2653,
      0.0000, 1.0000,
      0.0001, -0.0770,
      babl_trc("linear"), NULL, NULL, 1);

}

void 
babl_space_to_xyz (const Babl   *space, 
                   const double *rgb, 
                   double       *xyz)
{
  _babl_space_to_xyz (space, rgb, xyz);
}

void 
babl_space_from_xyz (const Babl   *space, 
                     const double *xyz, 
                     double       *rgb)
{
  _babl_space_from_xyz (space, xyz, rgb);
}

const double *
babl_space_get_rgbtoxyz (const Babl *space)
{
  return space->space.RGBtoXYZ;
}

///////////////////


static void 
prep_conversion (const Babl *babl)
{
  Babl *conversion = (void*) babl;
  const Babl *source_space = babl_conversion_get_source_space (conversion);
  float *matrixf;
  int i;
  float *lut_red;
  float *lut_green;
  float *lut_blue;

  double matrix[9];
  babl_matrix_mul_matrix (
     (conversion->conversion.destination)->format.space->space.XYZtoRGB,
     (conversion->conversion.source)->format.space->space.RGBtoXYZ,
     matrix);

  matrixf = babl_calloc (sizeof (float), 9 + 256 * 3); // we leak this matrix , which is a singleton
  babl_matrix_to_float (matrix, matrixf);
  conversion->conversion.data = matrixf;

  lut_red = matrixf + 9;
  lut_green = lut_red + 256;
  lut_blue = lut_green + 256;
  for (i = 0; i < 256; i++)
  {
    lut_red[i] = babl_trc_to_linear (source_space->space.trc[0], i/255.0);
    lut_green[i] = babl_trc_to_linear (source_space->space.trc[1], i/255.0);
    lut_blue[i] = babl_trc_to_linear (source_space->space.trc[2], i/255.0);
  }
}

#define TRC_IN(rgba_in, rgba_out)  do{ int i;\
  for (i = 0; i < samples; i++) \
  { \
    rgba_out[i*4+3] = rgba_in[i*4+3]; \
  } \
  if ((source_space->space.trc[0] == source_space->space.trc[1]) && \
      (source_space->space.trc[1] == source_space->space.trc[2])) \
  { \
    const Babl *trc = (void*)source_space->space.trc[0]; \
    babl_trc_to_linear_buf(trc, rgba_in, rgba_out, 4, 4, 3, samples); \
  } \
  else \
  { \
    int c; \
    for (c = 0; c < 3; c ++) \
    { \
      const Babl *trc = (void*)source_space->space.trc[c]; \
      babl_trc_to_linear_buf(trc, rgba_in + c, rgba_out + c, 4, 4, 1, samples); \
    } \
  } \
}while(0)

#define TRC_OUT(rgba_in, rgba_out)  do{\
  { \
    int c; \
    if ((destination_space->space.trc[0] == destination_space->space.trc[1]) && \
        (destination_space->space.trc[1] == destination_space->space.trc[2])) \
    { \
      const Babl *trc = (void*)destination_space->space.trc[0]; \
      babl_trc_from_linear_buf(trc, rgba_in, rgba_out, 4, 4, 3, samples); \
    } \
    else \
    { \
      for (c = 0; c < 3; c ++) \
      { \
        const Babl *trc = (void*)destination_space->space.trc[c]; \
        babl_trc_from_linear_buf(trc, rgba_in + c, rgba_out + c, 4, 4, 1, samples); \
      } \
    } \
  }\
} while(0)




static inline void
universal_nonlinear_rgba_converter (const Babl    *conversion,
                                    unsigned char *src_char, 
                                    unsigned char *dst_char, 
                                    long           samples, 
                                    void          *data)
{
  const Babl *source_space = babl_conversion_get_source_space (conversion);
  const Babl *destination_space = babl_conversion_get_destination_space (conversion);

  float * matrixf = data;
  float *rgba_in = (void*)src_char;
  float *rgba_out = (void*)dst_char;

  TRC_IN(rgba_in, rgba_out);

  babl_matrix_mul_vectorff_buf4 (matrixf, rgba_out, rgba_out, samples);

  TRC_OUT(rgba_out, rgba_out);
}

static inline void
universal_nonlinear_rgb_linear_converter (const Babl    *conversion,
                                          unsigned char *src_char, 
                                          unsigned char *dst_char, 
                                          long           samples, 
                                          void          *data)
{
  const Babl *source_space = babl_conversion_get_source_space (conversion);
  float * matrixf = data;
  float *rgba_in = (void*)src_char;
  float *rgba_out = (void*)dst_char;

  TRC_IN(rgba_in, rgba_out);

  babl_matrix_mul_vectorff_buf4 (matrixf, rgba_out, rgba_out, samples);
}

static inline void
universal_nonlinear_rgba_u8_converter (const Babl    *conversion,
                                       unsigned char *src_char, 
                                       unsigned char *dst_char, 
                                       long           samples, 
                                       void          *data)
{
  const Babl *destination_space = conversion->conversion.destination->format.space;

  float * matrixf = data;
  float * in_trc_lut_red = matrixf + 9;
  float * in_trc_lut_green  = in_trc_lut_red + 256;
  float * in_trc_lut_blue  = in_trc_lut_green + 256;
  int i;
  uint8_t *rgba_in_u8 = (void*)src_char;
  uint8_t *rgba_out_u8 = (void*)dst_char;

  float *rgb = babl_malloc (sizeof(float) * 4 * samples);

  for (i = 0; i < samples; i++)
  {
    rgb[i*4+0]=in_trc_lut_red[rgba_in_u8[i*4+0]];
    rgb[i*4+1]=in_trc_lut_green[rgba_in_u8[i*4+1]];
    rgb[i*4+2]=in_trc_lut_blue[rgba_in_u8[i*4+2]];
    rgba_out_u8[i*4+3] = rgba_in_u8[i*4+3];
  }

  babl_matrix_mul_vectorff_buf4 (matrixf, rgb, rgb, samples);

  {
    const Babl *from_trc_red   = (void*)destination_space->space.trc[0];
    const Babl *from_trc_green = (void*)destination_space->space.trc[1];
    const Babl *from_trc_blue  = (void*)destination_space->space.trc[2];
    for (i = 0; i < samples * 4; i+=4)
    {
      rgba_out_u8[i+0] = babl_trc_from_linear (from_trc_red,   rgb[i+0]) * 255.5f;
      rgba_out_u8[i+1] = babl_trc_from_linear (from_trc_green, rgb[i+1]) * 255.5f;
      rgba_out_u8[i+2] = babl_trc_from_linear (from_trc_blue,  rgb[i+2]) * 255.5f;
    }
  }
  babl_free (rgb);
}


static inline void
universal_rgba_converter (const Babl    *conversion,
                          unsigned char *src_char, 
                          unsigned char *dst_char, 
                          long           samples, 
                          void          *data)
{
  float *matrixf = data;
  float *rgba_in = (void*)src_char;
  float *rgba_out = (void*)dst_char;

  babl_matrix_mul_vectorff_buf4 (matrixf, rgba_in, rgba_out, samples);
}

static inline void
universal_rgb_converter (const Babl    *conversion,
                         unsigned char *src_char, 
                         unsigned char *dst_char, 
                         long           samples, 
                         void          *data)
{
  float *matrixf = data;
  float *rgb_in = (void*)src_char;
  float *rgb_out = (void*)dst_char;

  babl_matrix_mul_vectorff_buf3 (matrixf, rgb_in, rgb_out, samples);
}


static inline void
universal_nonlinear_rgb_u8_converter (const Babl    *conversion,
                                      unsigned char *src_char, 
                                      unsigned char *dst_char, 
                                      long           samples, 
                                      void          *data)
{
  const Babl *destination_space = conversion->conversion.destination->format.space;

  float * matrixf = data;
  float * in_trc_lut_red = matrixf + 9;
  float * in_trc_lut_green = in_trc_lut_red + 256;
  float * in_trc_lut_blue = in_trc_lut_green + 256;
  int i;
  uint8_t *rgb_in_u8 = (void*)src_char;
  uint8_t *rgb_out_u8 = (void*)dst_char;

  float *rgba_out = babl_malloc (sizeof(float) * 4 * samples);

  for (i = 0; i < samples; i++)
  {
    rgba_out[i*4+0]=in_trc_lut_red[rgb_in_u8[i*3+0]];
    rgba_out[i*4+1]=in_trc_lut_green[rgb_in_u8[i*3+1]];
    rgba_out[i*4+2]=in_trc_lut_blue[rgb_in_u8[i*3+2]];
    rgba_out[i*4+3]=rgb_in_u8[i*3+2] * 255.5f;
  }

  babl_matrix_mul_vectorff_buf4 (matrixf, rgba_out, rgba_out, samples);

  {
    int c;
    TRC_OUT(rgba_out, rgba_out);

    for (i = 0; i < samples; i++)
      for (c = 0; c < 3; c ++)
        rgb_out_u8[i*3+c] = rgba_out[i*4+c] * 255.5f;
  }

  babl_free (rgba_out);
}


#if defined(USE_SSE2)

#define m(matr, j, i)  matr[j*3+i]

#include <emmintrin.h>

static inline void babl_matrix_mul_vectorff_buf4_sse2 (const float *mat,
                                                       const float *v_in,
                                                       float       *v_out,
                                                       int          samples)
{
  const __v4sf m___0 = {m(mat, 0, 0), m(mat, 1, 0), m(mat, 2, 0), 0};
  const __v4sf m___1 = {m(mat, 0, 1), m(mat, 1, 1), m(mat, 2, 1), 0};
  const __v4sf m___2 = {m(mat, 0, 2), m(mat, 1, 2), m(mat, 2, 2), 1};
  int i;
  for (i = 0; i < samples; i ++)
  {
    __v4sf a, b, c = _mm_load_ps(&v_in[0]);
    a = (__v4sf) _mm_shuffle_epi32((__m128i)c, _MM_SHUFFLE(0,0,0,0));
    b = (__v4sf) _mm_shuffle_epi32((__m128i)c, _MM_SHUFFLE(1,1,1,1));
    c = (__v4sf) _mm_shuffle_epi32((__m128i)c, _MM_SHUFFLE(3,2,2,2));
    _mm_store_ps (v_out, m___0 * a + m___1 * b + m___2 * c);
    v_out += 4;
    v_in  += 4;
  }
  _mm_empty ();
}

#undef m


static inline void
universal_nonlinear_rgba_converter_sse2 (const Babl    *conversion,
                                         unsigned char *src_char, 
                                         unsigned char *dst_char, 
                                         long           samples, 
                                         void          *data)
{
  const Babl *source_space = babl_conversion_get_source_space (conversion);
  const Babl *destination_space = babl_conversion_get_destination_space (conversion);
  float * matrixf = data;
  float *rgba_in = (void*)src_char;
  float *rgba_out = (void*)dst_char;

  TRC_IN(rgba_in, rgba_out);

  babl_matrix_mul_vectorff_buf4_sse2 (matrixf, rgba_out, rgba_out, samples);

  TRC_OUT(rgba_out, rgba_out);
}


static inline void
universal_rgba_converter_sse2 (const Babl *conversion,
                               unsigned char *src_char, 
                               unsigned char *dst_char, 
                               long samples, 
                               void *data)
{
  float *matrixf = data;
  float *rgba_in = (void*)src_char;
  float *rgba_out = (void*)dst_char;

  babl_matrix_mul_vectorff_buf4_sse2 (matrixf, rgba_in, rgba_out, samples);
}

static inline void
universal_nonlinear_rgba_u8_converter_sse2 (const Babl    *conversion,
                                            unsigned char *src_char, 
                                            unsigned char *dst_char, 
                                            long           samples, 
                                            void          *data)
{
  const Babl *destination_space = conversion->conversion.destination->format.space;

  float * matrixf = data;
  float * in_trc_lut_red = matrixf + 9;
  float * in_trc_lut_green = in_trc_lut_red + 256;
  float * in_trc_lut_blue = in_trc_lut_green + 256;
  int i;
  uint8_t *rgba_in_u8 = (void*)src_char;
  uint8_t *rgba_out_u8 = (void*)dst_char;

  float *rgba_out = babl_malloc (sizeof(float) * 4 * samples);

  for (i = 0; i < samples * 4; i+= 4)
  {
    rgba_out[i+0]=in_trc_lut_red[rgba_in_u8[i+0]];
    rgba_out[i+1]=in_trc_lut_green[rgba_in_u8[i+1]];
    rgba_out[i+2]=in_trc_lut_blue[rgba_in_u8[i+2]];
    rgba_out_u8[i+3] = rgba_in_u8[i+3];
  }

  babl_matrix_mul_vectorff_buf4_sse2 (matrixf, rgba_out, rgba_out, samples);

  {
    int c;
    TRC_OUT(rgba_out, rgba_out);

    for (i = 0; i < samples * 4; i+= 4)
      for (c = 0; c < 3; c ++)
        rgba_out_u8[i+c] = rgba_out[i+c] * 255.5f;
  }

  babl_free (rgba_out);
}

static inline void
universal_nonlinear_rgb_u8_converter_sse2 (const Babl    *conversion,
                                           unsigned char *src_char, 
                                           unsigned char *dst_char, 
                                           long           samples, 
                                           void          *data)
{
  const Babl *destination_space = conversion->conversion.destination->format.space;

  float * matrixf = data;
  float * in_trc_lut_red = matrixf + 9;
  float * in_trc_lut_green = in_trc_lut_red + 256;
  float * in_trc_lut_blue = in_trc_lut_green + 256;
  int i;
  uint8_t *rgb_in_u8 = (void*)src_char;
  uint8_t *rgb_out_u8 = (void*)dst_char;

  float *rgba_out = babl_malloc (sizeof(float) * 4 * samples);

  for (i = 0; i < samples; i++)
  {
    rgba_out[i*4+0]=in_trc_lut_red[rgb_in_u8[i*3+0]];
    rgba_out[i*4+1]=in_trc_lut_green[rgb_in_u8[i*3+1]];
    rgba_out[i*4+2]=in_trc_lut_blue[rgb_in_u8[i*3+2]];
  }

  babl_matrix_mul_vectorff_buf4_sse2 (matrixf, rgba_out, rgba_out, samples);

  {
    int c;
    TRC_OUT(rgba_out, rgba_out);

    for (i = 0; i < samples; i++)
      for (c = 0; c < 3; c ++)
        rgb_out_u8[i*3+c] = rgba_out[i*4+c] * 255.5f;
  }

  babl_free (rgba_out);
}


static inline void
universal_nonlinear_rgb_linear_converter_sse2 (const Babl    *conversion,
                                               unsigned char *src_char, 
                                               unsigned char *dst_char, 
                                               long           samples, 
                                               void          *data)
{
  const Babl *source_space = babl_conversion_get_source_space (conversion);
  float * matrixf = data;
  float *rgba_in = (void*)src_char;
  float *rgba_out = (void*)dst_char;

  TRC_IN(rgba_in, rgba_out);

  babl_matrix_mul_vectorff_buf4_sse2 (matrixf, rgba_out, rgba_out, samples);
}
#endif


static int
add_rgb_adapter (Babl *babl,
                 void *space)
{
  if (babl != space)
  {

#if defined(USE_SSE2)
    if ((babl_cpu_accel_get_support () & BABL_CPU_ACCEL_X86_SSE) &&
        (babl_cpu_accel_get_support () & BABL_CPU_ACCEL_X86_SSE2))
    {
       prep_conversion(babl_conversion_new(babl_format_with_space("RGBA float", space),
                       babl_format_with_space("RGBA float", babl),
                       "linear", universal_rgba_converter_sse2,
                       NULL));
       prep_conversion(babl_conversion_new(babl_format_with_space("RGBA float", babl),
                       babl_format_with_space("RGBA float", space),
                       "linear", universal_rgba_converter_sse2,
                       NULL));
       prep_conversion(babl_conversion_new(babl_format_with_space("R'G'B'A float", space),
                       babl_format_with_space("R'G'B'A float", babl),
                       "linear", universal_nonlinear_rgba_converter_sse2,
                       NULL));
       prep_conversion(babl_conversion_new(babl_format_with_space("R'G'B'A float", babl),
                       babl_format_with_space("R'G'B'A float", space),
                       "linear", universal_nonlinear_rgba_converter_sse2,
                       NULL));
       prep_conversion(babl_conversion_new(babl_format_with_space("R'G'B'A u8", space),
                       babl_format_with_space("R'G'B'A u8", babl),
                       "linear", universal_nonlinear_rgba_u8_converter_sse2,
                       NULL));
       prep_conversion(babl_conversion_new(babl_format_with_space("R'G'B'A u8", babl),
                       babl_format_with_space("R'G'B'A u8", space),
                       "linear", universal_nonlinear_rgba_u8_converter_sse2,
                       NULL));

       prep_conversion(babl_conversion_new(babl_format_with_space("R'G'B' u8", space),
                       babl_format_with_space("R'G'B' u8", babl),
                       "linear", universal_nonlinear_rgb_u8_converter_sse2,
                       NULL));
       prep_conversion(babl_conversion_new(babl_format_with_space("R'G'B' u8", babl),
                       babl_format_with_space("R'G'B' u8", space),
                       "linear", universal_nonlinear_rgb_u8_converter_sse2,
                       NULL));
    }
    //else
#endif
    {
       prep_conversion(babl_conversion_new(babl_format_with_space("RGBA float", space),
                       babl_format_with_space("RGBA float", babl),
                       "linear", universal_rgba_converter,
                       NULL));
       prep_conversion(babl_conversion_new(babl_format_with_space("RGBA float", babl),
                       babl_format_with_space("RGBA float", space),
                       "linear", universal_rgba_converter,
                       NULL));
       prep_conversion(babl_conversion_new(babl_format_with_space("R'G'B'A float", space),
                       babl_format_with_space("R'G'B'A float", babl),
                       "linear", universal_nonlinear_rgba_converter,
                       NULL));
       prep_conversion(babl_conversion_new(babl_format_with_space("R'G'B'A float", babl),
                       babl_format_with_space("R'G'B'A float", space),
                       "linear", universal_nonlinear_rgba_converter,
                       NULL));
       prep_conversion(babl_conversion_new(babl_format_with_space("R'G'B'A u8", space),
                       babl_format_with_space("R'G'B'A u8", babl),
                       "linear", universal_nonlinear_rgba_u8_converter,
                       NULL));
       prep_conversion(babl_conversion_new(babl_format_with_space("R'G'B'A u8", babl),
                       babl_format_with_space("R'G'B'A u8", space),
                       "linear", universal_nonlinear_rgba_u8_converter,
                       NULL));

       prep_conversion(babl_conversion_new(babl_format_with_space("R'G'B' u8", space),
                       babl_format_with_space("R'G'B' u8", babl),
                       "linear", universal_nonlinear_rgb_u8_converter,
                       NULL));
       prep_conversion(babl_conversion_new(babl_format_with_space("R'G'B' u8", babl),
                       babl_format_with_space("R'G'B' u8", space),
                       "linear", universal_nonlinear_rgb_u8_converter,
                       NULL));
    }

    prep_conversion(babl_conversion_new(babl_format_with_space("RGB float", space),
                    babl_format_with_space("RGB float", babl),
                    "linear", universal_rgb_converter,
                    NULL));
    prep_conversion(babl_conversion_new(babl_format_with_space("RGB float", babl),
                    babl_format_with_space("RGB float", space),
                    "linear", universal_rgb_converter,
                    NULL));
  }
  return 0;
}

/* The first time a new Babl space is used - for creation of a fish, is when
 * this function is called, it adds conversions hooks that provides its formats
 * with conversions internally as well as for conversions to and from other RGB
 * spaces.
 */
void 
_babl_space_add_universal_rgb (const Babl *space)
{
  babl_space_class_for_each (add_rgb_adapter, (void*)space);
}


const Babl *
babl_space_match_trc_matrix (const Babl *trc_red,
                             const Babl *trc_green,
                             const Babl *trc_blue,
                             float rx, float ry, float rz,
                             float gx, float gy, float gz,
                             float bx, float by, float bz)
{
  int i;
  double delta = 0.001;
  for (i = 0; space_db[i].instance.class_type; i++)
  {
    BablSpace *space = &space_db[i];
    if (space->cmyk.is_cmyk == 0 &&
        trc_red == space->trc[0] &&
        trc_green == space->trc[1] &&
        trc_blue == space->trc[2] &&
        fabs(rx - space->RGBtoXYZ[0]) < delta &&
        fabs(ry - space->RGBtoXYZ[3]) < delta &&
        fabs(rz - space->RGBtoXYZ[6]) < delta &&
        fabs(gx - space->RGBtoXYZ[1]) < delta &&
        fabs(gy - space->RGBtoXYZ[4]) < delta &&
        fabs(gz - space->RGBtoXYZ[7]) < delta &&
        fabs(bx - space->RGBtoXYZ[2]) < delta &&
        fabs(by - space->RGBtoXYZ[5]) < delta &&
        fabs(bz - space->RGBtoXYZ[8]) < delta)
     {
       return (void*)&space_db[i];
     }
  }
  return NULL;
}

const Babl *
babl_space_with_trc (const Babl *babl, 
                     const Babl *trc)
{
  double xw, yw, xr, yr, xg, yg, xb, yb;
  const Babl *red_trc = NULL;
  const Babl *green_trc = NULL;
  const Babl *blue_trc = NULL;

  babl_space_get (babl,
                  &xw, &yw,
                  &xr, &yr,
                  &xg, &yg,
                  &xb, &yb,
                  &red_trc, &green_trc, &blue_trc);
  if (red_trc == trc && green_trc == trc && blue_trc == trc)
    return babl;
  return babl_space_from_chromaticities (NULL,
                                         xw, yw, xr, yr, xg, yg, xb, yb, trc, trc, trc,
                                         BABL_SPACE_FLAG_EQUALIZE);

}

void 
babl_space_get (const Babl *babl,
                double *xw, double *yw,
                double *xr, double *yr,
                double *xg, double *yg,
                double *xb, double *yb,
                const Babl **red_trc,
                const Babl **green_trc,
                const Babl **blue_trc)
{
  const BablSpace *space = &babl->space;
  /* XXX: note: for spaces set by matrix should be possible to derive
                the chromaticities of r,g,b and thus even then keep this
                is canonical data
   */
  if(xw)*xw = space->xw;
  if(yw)*yw = space->yw;
  if(xr)*xr = space->xr;
  if(yr)*yr = space->yr;
  if(xg)*xg = space->xg;
  if(yg)*yg = space->yg;
  if(xb)*xb = space->xb;
  if(yb)*yb = space->yb;
  if(red_trc)*red_trc = space->trc[0];
  if(green_trc)*green_trc = space->trc[1];
  if(blue_trc)*blue_trc = space->trc[2];
}

int 
babl_space_is_cmyk (const Babl *space)
{
  return space?space->space.cmyk.is_cmyk:0;
}

/* Trademarks:
 *
 * International Color Consortium is a registered trademarks of the.
 * International Color Consortium.
 * Apple is a trademark or registered trademark of Apple Inc in many countries.
 * Adobish is meant to concisely convey resemblence/compatibility with Adobe
 * RGB- without actualy being it, Adobe is a trademark or registered trademark
 * of Adobe Systems Incorporated in many countires.
 */

