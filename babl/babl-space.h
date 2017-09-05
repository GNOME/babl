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
  float  *clut;
  int     clut_size[3];
  int     in_table_size;
  int     out_table_size;
  float   matrix[9];
  float  *in_table[3];
  float  *out_table[3];
} ICCv2CLUT;


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

   ICCv2CLUT *a2b0;
   ICCv2CLUT *b2a0;

} BablSpace;

static inline int lut_init (int lut_size, float value, float *rdiff)
{
  float fentry = value * (lut_size-1);
  int entry    = fentry;
  float diff   = fentry - entry;

  if (entry >= lut_size - 1) {
    entry = lut_size - 1;
    diff = 0.0;
  }
  else
  {
    if (entry < 0)
    {
      entry = 0;
      diff = 0.0;
    }
  }
  *rdiff = diff;
  return entry;
}

static inline float do_lut (float *lut, int lut_size, float value)
{
  int entry;
  float diff;
  entry = lut_init (lut_size, value, &diff);
  if (diff <= 0.0)
  {
    return lut[entry];
  }
  else
  {
    return lut[entry] * (1.0 - diff) + lut[entry+1] * diff;
  }
}

static inline void clut_interpol (ICCv2CLUT *clut, const double *ind, double *outd)
{
  float val[3] = {ind[0], ind[1], ind[2]};
  int   entry[3];
  float diff[3];
  int *dim = clut->clut_size;
  int c;

  babl_matrix_mul_vectorff (clut->matrix, val, val);

  for (c = 0; c < 3; c ++)
  {
     val[c] = do_lut (clut->in_table[c], clut->in_table_size, val[c]);
  }

  for (c = 0; c < 3; c++)
  {
     entry[c] = lut_init (dim[c], val[c], &diff[c]);
  }

  // needs rework for non-same sized acceses
#define IDX(a,b,c,comp) (((a) * dim[0] * dim[1] + (b) * dim[0] + (c)) * 3 + comp)

  for (c = 0; c < 3; c ++)
  {
#if 1
     val[c] =
        ((clut->clut[IDX(entry[0],  entry[1],   entry[2],c  )] * (1.0 - diff[0]) +
         clut->clut[IDX(entry[0]+1, entry[1],   entry[2],c  )] * (diff[0])) * (1.0 - diff[1]) +

        (clut->clut[IDX(entry[0],   entry[1]+1, entry[2],c  )] * (1.0 - diff[0]) +
         clut->clut[IDX(entry[0]+1, entry[1]+1, entry[2],c  )] * (diff[0])) * diff[1]) *
                        (1.0-diff[2]) +

        ((clut->clut[IDX(entry[0],  entry[1],   entry[2]+1,c)] * (1.0 - diff[0]) +
         clut->clut[IDX(entry[0]+1, entry[1],   entry[2]+1,c)] * (diff[0])) * (1.0 - diff[1]) +

        (clut->clut[IDX(entry[0],   entry[1]+1, entry[2]+1,c)] * (1.0 - diff[0]) +
         clut->clut[IDX(entry[0]+1, entry[1]+1, entry[2]+1,c)] * (diff[0])) * diff[1]) * diff[2];
#else
     val[c] = clut->clut[IDX(entry[0], entry[1], entry[2],c)];
#endif
    outd[c] = do_lut (clut->out_table[c], clut->out_table_size, val[c]);
  }
}

static inline void babl_space_to_xyzf (const Babl *space, const float *rgb, float *xyz)
{
  BablSpace *space_ = (void*)space;
  double rgbmat[3] = {rgb[0], rgb[1], rgb[2]};
  double xyzmat[3];
  if (space_->a2b0)
  {
    int c;
    for (c = 0; c < 3; c++)
      rgbmat[c] = babl_trc_from_linear (space_->trc[c], rgbmat[c]);
    clut_interpol (space_->a2b0, rgbmat, xyzmat);
  }
  else
  {
    babl_matrix_mul_vector (space_->RGBtoXYZ, rgbmat, xyzmat);
  }
  xyz[0] = xyzmat[0];
  xyz[1] = xyzmat[1];
  xyz[2] = xyzmat[2];
}


static inline void babl_space_from_xyzf (const Babl *space, const float *xyz, float *rgb)
{
  BablSpace *space_ = (void*)space;
  double xyzmat[3] = {xyz[0], xyz[1], xyz[2]};
  double rgbmat[3];
  if (space_->b2a0)
  {
    int c;
    clut_interpol (space_->b2a0, xyzmat, rgbmat);
    for (c = 0; c < 3; c++)
      rgbmat[c] = babl_trc_to_linear (space_->trc[c], rgbmat[c]);
  }
  else
  {
    babl_matrix_mul_vector (space_->XYZtoRGB, xyzmat, rgbmat);
  }
  rgb[0] = rgbmat[0];
  rgb[1] = rgbmat[1];
  rgb[2] = rgbmat[2];
}


static inline void _babl_space_to_xyz (const Babl *space_, const double *rgb, double *xyz)
{
  BablSpace *space = (void*)space_;
  if (space->a2b0)
  {
    int c;
    for (c = 0; c < 3; c++)
      xyz[c] = babl_trc_from_linear (space->trc[c], rgb[c]);
    clut_interpol (space->a2b0, xyz, xyz);
  }
  else
    babl_matrix_mul_vector (space->RGBtoXYZ, rgb, xyz);
}

static inline void _babl_space_from_xyz (const Babl *space_, const double *xyz, double *rgb)
{
  BablSpace *space = (void*)space_;
  if (space->b2a0)
  {
    clut_interpol (space->b2a0, xyz, rgb);
    {
      int c;
      for (c = 0; c < 3; c++)
        rgb[c] = babl_trc_to_linear (space->trc[c], rgb[c]);
    }
  }
  else
    babl_matrix_mul_vector (space->XYZtoRGB, xyz, rgb);
}

void
babl_space_class_init (void);



#endif
