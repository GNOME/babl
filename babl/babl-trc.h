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

#ifndef _BABL_TRC_H
#define _BABL_TRC_H

#include <math.h>
#include <string.h>
#include "base/util.h"
#include "babl-polynomial.h"

BABL_CLASS_DECLARE (trc);

typedef enum {BABL_TRC_LINEAR,
              BABL_TRC_FORMULA_GAMMA,
              BABL_TRC_SRGB,
              BABL_TRC_FORMULA_SRGB,
              BABL_TRC_LUT}
BablTRCType;

typedef struct
{
  BablInstance     instance;
  BablTRCType      type;
  int              lut_size;
  double           gamma;
  float            rgamma;
  float          (*fun_to_linear)(const Babl *trc, float val);
  float          (*fun_from_linear)(const Babl *trc, float val);

  void           (*fun_to_linear_buf)(const Babl *trc,
                                      const float *in,
                                      float *out,
                                      int in_gap,
                                      int out_gap,
                                      int components,
                                      int count);
  void           (*fun_from_linear_buf)(const Babl *trc,
                                      const float *in,
                                      float *out,
                                      int in_gap,
                                      int out_gap,
                                      int components,
                                      int count);
  BablPolynomial   poly_gamma_to_linear;
  float            poly_gamma_to_linear_x0;
  float            poly_gamma_to_linear_x1;
  BablPolynomial   poly_gamma_from_linear;
  float            poly_gamma_from_linear_x0;
  float            poly_gamma_from_linear_x1;
  float           *lut;
  float           *inv_lut;
  char             name[128];
} BablTRC;

static inline void babl_trc_from_linear_buf (const Babl *trc_,
                                             const float *in, float *out,
                                             int in_gap, int out_gap,
                                             int components,
                                             int count)
{
  BablTRC *trc = (void*)trc_;
  trc->fun_from_linear_buf (trc_, in, out, in_gap, out_gap, components, count);
}

static inline void babl_trc_to_linear_buf (const Babl *trc_,
                                           const float *in, float *out,
                                           int in_gap, int out_gap,
                                           int components,
                                           int count)
{
  BablTRC *trc = (void*)trc_;
  trc->fun_to_linear_buf (trc_, in, out, in_gap, out_gap, components, count);
}

static inline float babl_trc_from_linear (const Babl *trc_, float value)
{
  BablTRC *trc = (void*)trc_;
  return trc->fun_from_linear (trc_, value);
}

static inline float babl_trc_to_linear (const Babl *trc_, float value)
{
  BablTRC *trc = (void*)trc_;
  return trc->fun_to_linear (trc_, value);
}

void
babl_trc_class_init (void);

#endif
