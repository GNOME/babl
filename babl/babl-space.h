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
 * <https://www.gnu.org/licenses/>.
 */

#ifndef _BABL_SPACE_H
#define _BABL_SPACE_H

#include "config.h"
#include <math.h>
#include <string.h>
#include "base/util.h"
#include "babl-matrix.h"

#ifdef HAVE_LCMS
#include <lcms2.h>
#endif

BABL_CLASS_DECLARE (space);

typedef struct
{
  int           is_cmyk;
#ifdef HAVE_LCMS
  cmsHPROFILE   lcms_profile;
  cmsHTRANSFORM lcms_to_rgba;
  cmsHTRANSFORM lcms_from_rgba;
#endif
} BablCMYK;

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

  const Babl      *trc[3];
  char             name[512]; // XXX: allocate this dynamically instead -
                              //      or use iccv4 style hashes for name.
  double whitepoint[3]; /* CIE XYZ whitepoint */

  double RGBtoXYZ[9]; /* matrices for conversions */
  double XYZtoRGB[9];
  float  RGBtoXYZf[9]; /* matrices for conversions */
  float  XYZtoRGBf[9];

  /* the space should contain matrix to/from XYZ */
  /* and before converting a span, all that needs to be
     rigged is merging matrices */

  /* we should here also add more things read from ICC profile,
   * making it possible to round-trip data. Unless it is sRGB, when
   * standard should win.
   */
  char *icc_profile;
  int   icc_length;

  BablCMYK cmyk;
} BablSpace;


static inline void babl_space_to_xyzf (const Babl *space, const float *rgb, float *xyz)
{
  BablSpace *space_ = (void*)space;
  babl_matrix_mul_vectorff (space_->RGBtoXYZf, rgb, xyz);
}

static inline void babl_space_from_xyzf (const Babl *space, const float *xyz, float *rgb)
{
  BablSpace *space_ = (void*)space;
  babl_matrix_mul_vectorff (space_->XYZtoRGBf, xyz, rgb);
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
