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

#ifndef _BABL_TRC_H
#define _BABL_TRC_H

#include <math.h>
#include <string.h>
#include "base/util.h"

BABL_CLASS_DECLARE (trc);

typedef enum {BABL_TRC_LINEAR,
              BABL_TRC_GAMMA,
              BABL_TRC_SRGB,
              BABL_TRC_LUT} BablTRCType;

typedef struct
{
  BablInstance     instance;
  BablTRCType      type;
  int              lut_size;
  double           gamma;
  char             name[128];
  float           *lut;
  float           *inv_lut;
} BablTRC;

static inline double babl_trc_lut_from_linear (const Babl *trc_, double value)
{
  BablTRC *trc = (void*)trc_;
  int entry = value * trc->lut_size + 0.5;
  double ret = trc->inv_lut[
    (entry >= 0 && entry < trc->lut_size) ?
                               entry :
                               trc->lut_size-1];
  /* XXX: fixme, do linear interpolation */
  return ret;
}

static inline double babl_trc_lut_to_linear (const Babl *trc_, double value)
{
  BablTRC *trc = (void*)trc_;
  int entry = value * trc->lut_size + 0.5;
  double ret = trc->lut[
    (entry >= 0 && entry < trc->lut_size) ?
                               entry :
                               trc->lut_size-1];
  /* XXX: fixme, do linear interpolation */
  return ret;
}

static inline double _babl_trc_from_linear (const Babl *trc_, double value)
{
  BablTRC *trc = (void*)trc_;
  switch (trc->type)
  {
    case BABL_TRC_LINEAR:
            return value;
    case BABL_TRC_GAMMA:
            return pow (value, 1.0/trc->gamma);
    case BABL_TRC_SRGB:
            return babl_linear_to_gamma_2_2 (value);
    case BABL_TRC_LUT:
            return babl_trc_lut_from_linear (trc_, value);
  }
  return value;
}

static inline double _babl_trc_to_linear (const Babl *trc_, double value)
{
  BablTRC *trc = (void*)trc_;
  switch (trc->type)
  {
    case BABL_TRC_LINEAR:
            return value;
    case BABL_TRC_GAMMA:
            return pow (value, trc->gamma);
    case BABL_TRC_SRGB:
            return babl_gamma_2_2_to_linear (value);
    case BABL_TRC_LUT:
            return babl_trc_lut_to_linear (trc_, value);
  }
  return value;
}

static inline float _babl_trc_from_linearf (const Babl *trc_, float value)
{
  BablTRC *trc = (void*)trc_;
  switch (trc->type)
  {
    case BABL_TRC_LINEAR: return value;
    case BABL_TRC_GAMMA:  return powf (value, 1.0f/trc->gamma);
    case BABL_TRC_SRGB:   return babl_linear_to_gamma_2_2f (value);
    case BABL_TRC_LUT:    return babl_trc_lut_from_linear (trc_, value);
  }
  return value;
}

static inline float _babl_trc_to_linearf (const Babl *trc_, float value)
{
  BablTRC *trc = (void*)trc_;
  switch (trc->type)
  {
    case BABL_TRC_LINEAR: return value;
    case BABL_TRC_GAMMA:  return powf (value, trc->gamma);
    case BABL_TRC_SRGB:   return babl_gamma_2_2_to_linearf (value);
    case BABL_TRC_LUT:    return babl_trc_lut_to_linear (trc_, value);
  }
  return value;
}

void
babl_trc_class_init (void);

#endif
