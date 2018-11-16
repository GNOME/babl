/* babl - dynamically extendable universal pixel conversion library.
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

#include <stdio.h>

#include <babl/babl.h>

#include "common.inc"


int
main (int    argc,
      char **argv)
{
  int OK = 1;

  float rgba[][4] = {{1.0     , 1.0     , 1.0     , 1.0},
                     {0.214041, 0.214041, 0.214041, 1.0},
                     {0.0     , 0.0     , 0.0     , 1.0},
                     {1.0     , 0.0     , 0.0     , 1.0},
                     {0.522519, 0.522519, 0.0     , 1.0},
                     {0.0     , 0.214041, 0.0     , 1.0},
                     {0.214041, 1.0     , 1.0     , 1.0},
                     {0.214041, 0.214041, 1.0     , 1.0},
                     {0.522520, 0.050876, 0.522521, 1.0},
                     {0.353069, 0.372000, 0.017878, 1.0},
                     {0.052772, 0.010679, 0.823194, 1.0},
                     {0.012693, 0.414530, 0.052934, 1.0},
                     {0.870621, 0.579515, 0.004228, 1.0},
                     {0.453672, 0.029212, 0.781390, 1.0},
                     {0.850554, 0.181933, 0.081839, 1.0},
                     {0.995195, 0.941644, 0.244979, 1.0},
                     {0.009836, 0.595745, 0.308242, 1.0},
                     {0.036595, 0.019338, 0.315257, 1.0},
                     {0.209470, 0.207646, 0.478434, 1.0}};

  float hsla[][4] = {{0.0,       0.0  ,   1.0  , 1.0},
                     {0.0,       0.0  ,   0.500, 1.0},
                     {0.0,       0.0  ,   0.0  , 1.0},
                     {0.0,       1.0  ,   0.500, 1.0},
                     {0.166667,  1.0  ,   0.375, 1.0},
                     {0.333333,  1.0  ,   0.250, 1.0},
                     {0.5,       1.0  ,   0.750, 1.0},
                     {0.666666,  1.0  ,   0.750, 1.0},
                     {0.833333,  0.500,   0.500, 1.0},
                     {0.171666,  0.638,   0.393, 1.0},
                     {0.6975,    0.832,   0.511, 1.0},
                     {0.374722,  0.707,   0.396, 1.0},
                     {0.1375,    0.893,   0.497, 1.0},
                     {0.788028,  0.775,   0.542, 1.0},
                     {0.039837,  0.817,   0.624, 1.0},
                     {0.158083,  0.991,   0.765, 1.0},
                     {0.451149,  0.779,   0.447, 1.0},
                     {0.689732,  0.601,   0.373, 1.0},
                     {0.668129,  0.290,   0.607, 1.0}};

  babl_init ();

  CHECK_CONV_FLOAT ("rgba to hsla ", float, 0.001,
                    babl_format ("RGBA float"),
                    babl_format ("HSLA float"),
                    rgba, hsla);

  CHECK_CONV_FLOAT ("hsla to rgba ", float, 0.001,
                    babl_format ("HSLA float"),
                    babl_format ("RGBA float"),
                    hsla, rgba);

  babl_exit ();

  return !OK;
}
