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
#include "babl-trc.h"

static BablSpace space_db[MAX_SPACES];

void babl_chromatic_adaptation_matrix (const double *whitepoint,
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

// cached equalized matrices generated for spaces used internally by babl
//
static double equalized_matrices[][9]=
{
 {0.673492431640625000, 0.165679931640625000, 0.125030517578125000,
  0.279052734375000000, 0.675354003906250000, 0.045593261718750000,
 -0.001907348632812500, 0.029968261718750000, 0.796844482421875000},

 {0.609756469726562500, 0.205276489257812500, 0.149169921875000000,
  0.311126708984375000, 0.625671386718750000, 0.063201904296875000,
  0.019485473632812500, 0.060867309570312500, 0.744552612304687500},

 {0.797714233398437500, 0.135208129882812500, 0.031280517578125000,
  0.288070678710937500, 0.711868286132812500, 0.000061035156250000,
  0.000015258789062500, 0.000015258789062500, 0.824874877929687500},

 {0.475555419921875000, 0.339706420898437500, 0.148941040039062500,
  0.255172729492187500, 0.672592163085937500, 0.072235107421875000,
  0.018463134765625000, 0.113342285156250000, 0.693099975585937500},

 {0.689895629882812500, 0.149765014648437500, 0.124542236328125000,
  0.284530639648437500, 0.671691894531250000, 0.043777465820312500,
 -0.006011962890625000, 0.009994506835937500, 0.820922851562500000},

 {0.990905761718750000, 0.012222290039062500,-0.038925170898437500,
  0.361907958984375000, 0.722503662109375000,-0.084411621093750000,
 -0.002685546875000000, 0.008239746093750000, 0.819351196289062500},
};


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

  for (int i = 0; i < sizeof (equalized_matrices)/
                      sizeof (equalized_matrices[0]); i++)
  {
    double diff_sum = 0.0f;
    for (int j = 0; j < 9; j++){ 
    double diff = equalized_matrices[i][j] - in_mat[j];
    diff *= diff;
    diff_sum += diff; }

    // the threshold is based on being ~double the biggest
    // difference seen in the default space set.

    if (diff_sum < 0.000000005) { 
      for (int j = 0; j < 9; j++){ 
        in_mat[j] = equalized_matrices[i][j];
      }
      return;
    }
  }

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

#if 0 // uncomment to generate code for pasting in cache
  fprintf (stderr, "{");
  for (i = 0; i < 9; i++)
  {
    if (i)
      fprintf (stderr, ", ");
    fprintf (stderr, "%.18f", in_mat[i]);
  }
  fprintf (stderr, "},\n");
#endif
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
  BablSpace space = {0,};


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
  BablSpace space = {0,};
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
  space.icc_type = BablICCTypeRGB;

  babl_matrix_invert (space.RGBtoXYZ, space.XYZtoRGB);

  babl_matrix_to_float (space.RGBtoXYZ, space.RGBtoXYZf);
  babl_matrix_to_float (space.XYZtoRGB, space.XYZtoRGBf);

  /* recover chromaticities from matrix */
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
  {
    snprintf (space_db[i].name, sizeof (space_db[i].name)-1,
             "space-%.4f,%.4f_%.4f,%.4f_%.4f,%.4f_%.4f,%.4f_%s,%s,%s",
             wx,wy,rx,ry,bx,by,gx,gy,babl_get_name (space.trc[0]),
             babl_get_name(space.trc[1]), babl_get_name(space.trc[2]));
    space_db[i].name[sizeof (space_db[i].name)-1]=0;
  }

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
  BablSpace space = {0,};
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
  space.icc_type = BablICCTypeRGB;

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

const Babl *
babl_space_from_gray_trc (const char *name,
                          const Babl *trc_gray,
                          BablSpaceFlags flags)
{
  int i=0;
  BablSpace space = {0,};
  space.instance.class_type = BABL_SPACE;
  space.instance.id         = 0;

  space.xw = 0.3127;
  space.yw = 0.3290;

  space.xr = 0.639998686;
  space.yr = 0.330010138;
  space.xg = 0.300003784;
  space.yg = 0.600003357;
  space.xb = 0.150002046;
  space.yb = 0.059997204;
  space.trc[0] = trc_gray;
  space.trc[1] = trc_gray;
  space.trc[2] = trc_gray;

  space.whitepoint[0] = space.xw / space.yw;
  space.whitepoint[1] = 1.0;
  space.whitepoint[2] = (1.0 - space.xw - space.yw) / space.yw;
  space.icc_type = BablICCTypeGray;

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
             "space-gray-%s", babl_get_name(space.trc[0]));

  /* compute matrixes */
  babl_space_compute_matrices (&space_db[i], 1);

  //babl_space_get_icc ((Babl*)&space_db[i], NULL);
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
    if (space->icc_type == BablICCTypeRGB &&
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
babl_space_is_rgb (const Babl *space)
{
  return space ? space->space.icc_type == BablICCTypeRGB : 0;
}

int
babl_space_is_cmyk (const Babl *space)
{
  return space?space->space.icc_type == BablICCTypeCMYK:0;
}

int
babl_space_is_gray (const Babl *space)
{
  return space?space->space.icc_type == BablICCTypeGray:0;
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

void
babl_space_get_rgb_luminance (const Babl *space,
                              double     *red_luminance,
                              double     *green_luminance,
                              double     *blue_luminance)
{
  if (!space)
    space = babl_space ("sRGB");
  if (red_luminance)
    *red_luminance = space->space.RGBtoXYZ[3];
  if (green_luminance)
    *green_luminance = space->space.RGBtoXYZ[4];
  if (blue_luminance)
    *blue_luminance = space->space.RGBtoXYZ[5];
}

double
babl_space_get_gamma (const Babl *space)
{
  if (space->space.trc[0] != space->space.trc[1] ||
      space->space.trc[1] != space->space.trc[2] ||
      space->space.trc[0]->trc.type != BABL_TRC_FORMULA_GAMMA)
    return 0.0;
  return space->space.trc[0]->trc.gamma;
}
