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
main (void)
{
  int OK = 1;

  float rgba[][4] = {{1.0     , 1.0     , 1.0     , 1.0},
                     {0.0     , 1.0     , 0.0     , 1.0},
                     {0.5     , 0.5     , 0.5     , 1.0},
                     {0.0     , 1.0     , 1.0     , 1.0}};

  float cmyk[][4] = {{0.0,       0.0  ,   0.0  , 0.0},
                     {1.0,       0.0  ,   1.0  , 0.0},
                     {0.0,       0.0  ,   0.0  , 0.5},
                     {1.0,       0.0  ,   0.0  , 0.0}};

  babl_init ();

  CHECK_CONV_FLOAT ("rgba to cmyk ", float, 0.001,
                    babl_format ("RGBA float"),
                    babl_format ("CMYK float"),
                    rgba, cmyk);

  CHECK_CONV_FLOAT ("cmyk to rgba ", float, 0.001,
                    babl_format ("CMYK float"),
                    babl_format ("RGBA float"),
                    cmyk, rgba);

  babl_exit ();

  return !OK;
}
