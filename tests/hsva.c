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
 * <http://www.gnu.org/licenses/>.
 */

 /*
 * Adding test fo hsva colorspace
 *
 * The test is at 0.001 precision
 */

#include "config.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <assert.h>
#include "babl.h"

#define CHECK_CONV(test_name, componenttype, src_fmt, dst_fmt, src_pix, expected_pix) \
  {       \
  const Babl *fish;       \
  int i;       \
  fish = babl_fish (src_fmt, dst_fmt);       \
  if (!fish)       \
    {       \
      printf ("  %s failed to make fish\n", test_name);       \
      OK = 0;       \
    }       \
  for (i = 0; i < sizeof(src_pix)/sizeof(src_pix[0]); i ++)       \
    {       \
      int c;\
      componenttype result[10];       \
      babl_process (fish, src_pix[i], result, 1);       \
      for (c = 0; c < sizeof(expected_pix[i])/sizeof(expected_pix[i][0]); c++) \
      if (fabs(result[c] - expected_pix[i][c]) > 0.001)       \
        {       \
          printf (" %s failed #%i[%i]  got %lf expected %lf\n", test_name, i, c, result[c], expected_pix[i][c]);       \
          OK = 0;          \
        }       \
    }       \
  }


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

  CHECK_CONV ("rgba to hsva ", float,
              babl_format ("RGBA float"),
              babl_format ("HSVA float"),
              rgba, hsva);

  CHECK_CONV ("hsva to rgba ", float,
              babl_format ("HSVA float"),
              babl_format ("RGBA float"),
              hsva, rgba);

  babl_exit ();

  return !OK;
}
