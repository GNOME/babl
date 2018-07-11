/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2012, Maxime Nicco <maxime.nicco@gmail.fr>
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

 /*
 * Adding test fo hsva colorspace
 *
 * The test is at 0.001 precision
 */

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "babl.h"

#include "common.inc"


int
main (int    argc,
      char **argv)
{
  int OK = 1;

  float rgba[][4] = {{ 1.0,    1.0,    1.0,    1.0 },
                     { 0.2140, 0.2140, 0.2140, 1.0 },
                     { 0,      0,      0,      1.0 },
                     { 1,      0,      0,      1.0 },
                     { 0.5209, 0.5225, 0,      1.0 },
                     { 0,      0.2140, 0,      1.0 },
                     { 0.2140, 1,      1,      1.0 },
                     { 0.2140, 0.2140, 1,      1.0 },
                     { 0.5215, 0.0508, 0.5225, 1.0 },
                     { 0.3509, 0.3710, 0.0178, 1.0 },
                     { 0.0533, 0.0106, 0.8235, 1.0 },
                     { 0.0126, 0.4132, 0.0529, 1.0 },
                     { 0.8709, 0.5754, 0.0042, 1.0 },
                     { 0.4537, 0.0291, 0.7814, 1.0 },
                     { 0.8501, 0.1813, 0.0814, 1.0 },
                     { 0.9954, 0.9418, 0.2448, 1.0 },
                     { 0.0099, 0.5953, 0.3081, 1.0 },
                     { 0.0366, 0.0193, 0.3150, 1.0 }};

  float hsva[][4] = {{ 0.0,   0.0,   1.0,   1.0 },
                     { 0.0,   0.0,   0.5,   1.0 },
                     { 0.0,   0.0,   0.0,   1.0 },
                     { 0.0,   1.0,   1.0,   1.0 },
                     { 0.167, 1.0,   0.75,  1.0 },
                     { 0.333, 1.0,   0.5,   1.0 },
                     { 0.5,   0.5,   1.0,   1.0 },
                     { 0.667, 0.5,   1.0,   1.0 },
                     { 0.833, 0.666, 0.75,  1.0 },
                     { 0.172, 0.779, 0.643, 1.0 },
                     { 0.698, 0.887, 0.918, 1.0 },
                     { 0.375, 0.828, 0.675, 1.0 },
                     { 0.137, 0.944, 0.941, 1.0 },
                     { 0.788, 0.792, 0.897, 1.0 },
                     { 0.040, 0.661, 0.931, 1.0 },
                     { 0.158, 0.467, 0.998, 1.0 },
                     { 0.451, 0.875, 0.795, 1.0 },
                     { 0.690, 0.75,  0.597, 1.0 }};

  babl_init ();

  CHECK_CONV_FLOAT ("rgba to hsva ", float, 0.001,
                    babl_format ("RGBA float"),
                    babl_format ("HSVA float"),
                    rgba, hsva);

  CHECK_CONV_FLOAT ("hsva to rgba ", float, 0.001,
                    babl_format ("HSVA float"),
                    babl_format ("RGBA float"),
                    hsva, rgba);

  babl_exit ();

  return !OK;
}
