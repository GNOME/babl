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
#include "babl-matrix.h"

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
  double whitepoint[3]; /* CIE XYZ whitepoint */

  double RGBtoXYZ[9]; /* matrices for conversions */
  double XYZtoRGB[9];
  float  RGBtoXYZf[9]; /* matrices for conversions */
  float  XYZtoRGBf[9];

  /* the space should contain matrix to/from XYZ */
  /* and before converting a span, all that needs to be
     rigged is merging matrices */

} BablSpace;




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
