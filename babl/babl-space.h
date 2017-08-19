/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2017, Øyvind Kolås and others.
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

#ifndef _BABL_SPACE_H
#define _BABL_SPACE_H

#include <math.h>
#include <string.h>
#include "base/util.h"

BABL_CLASS_DECLARE (space);

typedef struct
{
  BablInstance     instance;
  double           xw;  // white-point chromaticity
  double           yw;

  double           xr;  // red primary chromaticity
  double           yr;

  double           xg;  // green primary chromaticity
  double           yg;

  double           xb;  // blue primary chromaticity
  double           yb;

  double           pad; // for when the numbers represent a matrix

  const Babl      *trc[3];
  char             name[128];

  double RGBtoXYZ[9]; /* matrices for conversions */
  double XYZtoRGB[9];

  /* the space should contain matrix to/from XYZ */
  /* and before converting a span, all that needs to be
     rigged is merging matrices */

} BablSpace;
#include <stdio.h>

#define m(matr, j, i)  matr[j*3+i]

static inline void babl_matrix_mul_matrix (const double *matA_,
                                           const double *matB_,
                                           double *out)
{
  int i, j;
  double matA[9];
  double matB[9];
  double t1, t2, t3;
  memcpy (matA, matA_, sizeof (matA));
  memcpy (matB, matB_, sizeof (matB));

  for (i = 0; i < 3; i++)
  {
    t1 = m(matA, i, 0);
    t2 = m(matA, i, 1);
    t3 = m(matA, i, 2);

    for (j = 0; j < 3; j ++)
    {
      m(out,i,j) = t1 * m(matB, 0, j);
      m(out,i,j) += t2 * m(matB, 1, j);
      m(out,i,j) += t3 * m(matB, 2, j);
    }
  }
}

static inline void babl_matrix_invert (const double *in, double *out)
{
  double mat[9];
  double det, invdet;
  memcpy (mat, in, sizeof (mat));
  det = m(mat, 0, 0) * (m(mat, 1, 1) *m(mat, 2, 2) - m(mat, 2, 1)*m(mat, 1, 2)) -
        m(mat, 0, 1) * (m(mat, 1, 0) *m(mat, 2, 2) - m(mat, 1, 2)*m(mat, 2, 0)) +
        m(mat, 0, 2) * (m(mat, 1, 0) *m(mat, 2, 1) - m(mat, 1, 1)*m(mat, 2, 0));
  invdet = 1.0 / det;
  m(out, 0, 0) = (m(mat, 1, 1) * m(mat, 2, 2) - m(mat, 2, 1) * m(mat, 1, 2)) * invdet;
  m(out, 0, 1) = (m(mat, 0, 2) * m(mat, 2, 1) - m(mat, 0, 1) * m(mat, 2, 2)) * invdet;
  m(out, 0, 2) = (m(mat, 0, 1) * m(mat, 1, 2) - m(mat, 0, 2) * m(mat, 1, 1)) * invdet;
  m(out, 1, 0) = (m(mat, 1, 2) * m(mat, 2, 0) - m(mat, 1, 0) * m(mat, 2, 2)) * invdet;
  m(out, 1, 1) = (m(mat, 0, 0) * m(mat, 2, 2) - m(mat, 0, 2) * m(mat, 2, 0)) * invdet;
  m(out, 1, 2) = (m(mat, 1, 0) * m(mat, 0, 2) - m(mat, 0, 0) * m(mat, 1, 2)) * invdet;
  m(out, 2, 0) = (m(mat, 1, 0) * m(mat, 2, 1) - m(mat, 2, 0) * m(mat, 1, 1)) * invdet;
  m(out, 2, 1) = (m(mat, 2, 0) * m(mat, 0, 1) - m(mat, 0, 0) * m(mat, 2, 1)) * invdet;
  m(out, 2, 2) = (m(mat, 0, 0) * m(mat, 1, 1) - m(mat, 1, 0) * m(mat, 0, 1)) * invdet;
}


static inline void babl_matrix_mul_vector (const double *mat, const double *v_in, double *v_out)
{
  double val[3]={v_in[0], v_in[1], v_in[2]};

  v_out[0] = m(mat, 0, 0) * val[0] + m(mat, 0, 1) * val[1] + m(mat, 0, 2) * val[2];
  v_out[1] = m(mat, 1, 0) * val[0] + m(mat, 1, 1) * val[1] + m(mat, 1, 2) * val[2];
  v_out[2] = m(mat, 2, 0) * val[0] + m(mat, 2, 1) * val[1] + m(mat, 2, 2) * val[2];
#undef m
}

static inline void babl_space_to_xyzf (const Babl *space, const float *rgb, float *xyz)
{
  BablSpace *space_ = (void*)space;
  double rgbmat[3] = {rgb[0], rgb[1], rgb[2]};
  double xyzmat[3];
  babl_matrix_mul_vector (space_->RGBtoXYZ, rgbmat, xyzmat);
  xyz[0] = xyzmat[0];
  xyz[1] = xyzmat[1];
  xyz[2] = xyzmat[2];
}

static inline void babl_space_from_xyzf (const Babl *space, const float *xyz, float *rgb)
{
  BablSpace *space_ = (void*)space;
  double xyzmat[3] = {xyz[0], xyz[1], xyz[2]};
  double rgbmat[3];
  babl_matrix_mul_vector (space_->XYZtoRGB, xyzmat, rgbmat);
  rgb[0] = rgbmat[0];
  rgb[1] = rgbmat[1];
  rgb[2] = rgbmat[2];
}

static inline void _babl_space_to_xyz (const Babl *space, const double *rgb, double *xyz)
{
  BablSpace *space_ = (void*)space;
  babl_matrix_mul_vector (space_->RGBtoXYZ, rgb, xyz);
}

static inline void _babl_space_from_xyz (const Babl *space, const double *xyz, double *rgb)
{
  BablSpace *space_ = (void*)space;
  babl_matrix_mul_vector (space_->XYZtoRGB, xyz, rgb);
}

void
babl_space_class_init (void);

#endif
