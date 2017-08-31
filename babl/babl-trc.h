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
              BABL_TRC_GAMMA_1_8,
              BABL_TRC_GAMMA_2_2,
              BABL_TRC_SRGB,
              BABL_TRC_LUT}
BablTRCType;

typedef struct
{
  BablInstance     instance;
  BablTRCType      type;
  int              lut_size;
  double           gamma;
  float            rgamma;
  char             name[128];
  float           *lut;
  float           *inv_lut;
  float          (*fun_to_linear)(const Babl *trc_, float val);
  float          (*fun_from_linear)(const Babl *trc_, float val);
} BablTRC;

void
babl_trc_class_init (void);

#endif
