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

  float rgba[][4] = {{  1.0,   1.0,   1.0, 1.0 }, {  0.5,   0.5,   0.5, 1.0}, {  0.0,   0.0,   0.0, 1.0},
                    {   1.0,   0.0,   0.0, 1.0 }, { 0.75,  0.75,   0.0, 1.0}, {  0.0,   0.5,   0.0, 1.0},
                    {   0.5,   1.0,   1.0, 1.0 }, {  0.5,   0.5,   1.0, 1.0}, { 0.75,  0.25,  0.75, 1.0},
                    { 0.628, 0.643, 0.142, 1.0 }, {0.255, 0.104, 0.918, 1.0}, {0.116, 0.675, 0.255, 1.0},
                    { 0.941, 0.785, 0.053, 1.0 }, {0.704, 0.187, 0.897, 1.0}, {0.931, 0.463, 0.316, 1.0},
                    { 0.998, 0.974, 0.532, 1.0 }, {0.099, 0.795, 0.591, 1.0}, {0.211, 0.149, 0.597, 1.0}};

  float hsva[][4] = {{    0.0,   0.0,   1.0, 1.0 }, {    0.0,   0.0,   0.5, 1.0}, {    0.0,   0.0,   0.0, 1.0},
                    {     0.0,   1.0,   1.0, 1.0 }, {   60.0,   1.0,  0.75, 1.0}, {  120.0,   1.0,   0.5, 1.0},
                    {   180.0,   0.5,   1.0, 1.0 }, {  240.0,   0.5,   1.0, 1.0}, {  300.0, 0.666,  0.75, 1.0},
                    {  61.796, 0.779, 0.643, 1.0 }, {251.130, 0.887, 0.918, 1.0}, {134.919, 0.828, 0.675, 1.0},
                    {  49.459, 0.944, 0.941, 1.0 }, {283.690, 0.792, 0.897, 1.0}, { 14.341, 0.661, 0.931, 1.0},
                    {  56.909, 0.467, 0.998, 1.0 }, {162.413, 0.875, 0.795, 1.0}, {248.303,  0.75, 0.597, 1.0}};


  babl_init ();


  CHECK_CONV("rgba to hsva ", float,
      babl_format("RGBA float"),
      babl_format("HSVA float"),
      rgba, hsva);



  CHECK_CONV("hsva to rgba ", float,
      babl_format("HSVA float"),
      babl_format("RGBA float"),
      hsva, rgba);


  babl_exit ();
  return !OK;
}
