/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2005, Øyvind Kolås.
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
  babl_init ();
  {
    unsigned char in[][4]   = {{0,1,2,3  },{4,5,6,7    },{8,9,10,11  }};
    unsigned char out1[][1]  = {{0},        {4},  {8}  };
    unsigned char out2[][2]  = {{0,1},      {4,5},         {8,9}  };
    unsigned char out4[][4]  = {{0,1,2,3},  {4,5,6,7},    {8,9,10,11}  };
    unsigned char out5[][5]  = {{0,1,2,3,0},  {4,5,6,7,0},{8,9,10,11,0}  };

    CHECK_CONV("RGBAu8 to n1'", unsigned char,
        babl_format("R'G'B'A u8"),
        babl_format_n (babl_type ("u8"), 1),
        in, out1);

    CHECK_CONV("RGBAu8 to n2'", unsigned char,
        babl_format("R'G'B'A u8"),
        babl_format_n (babl_type ("u8"), 2),
        in, out2);

    CHECK_CONV("RGBAu8 to n4'", unsigned char,
        babl_format("R'G'B'A u8"),
        babl_format_n (babl_type ("u8"), 4),
        in, out4);

    CHECK_CONV("RGBAu8 to n5'", unsigned char,
        babl_format("R'G'B'A u8"),
        babl_format_n (babl_type ("u8"), 5),
        in, out5);
  }
  {
    unsigned char in[][3]   = {{0,1,2  },{4,5,6    },{8,9,10  }};
    unsigned char out1[][1]  = {{0},        {4},  {8}  };
    unsigned char out2[][2]  = {{0,1},      {4,5},         {8,9}  };
    unsigned char out4[][4]  = {{0,1,2,0},  {4,5,6,0},    {8,9,10,0}  };
    unsigned char out5[][5]  = {{0,1,2,0,0},  {4,5,6,0,0},{8,9,10,0,0}  };
    unsigned char out6[][6]  = {{0,1,2,0,0,0},  {4,5,6,0,0,0},{8,9,10,0,0,0}  };

    CHECK_CONV("RGBu8 to n1'", unsigned char,
        babl_format("R'G'B' u8"),
        babl_format_n (babl_type ("u8"), 1),
        in, out1);


    CHECK_CONV("RGBu8 to n2'", unsigned char,
        babl_format("R'G'B' u8"),
        babl_format_n (babl_type ("u8"), 2),
        in, out2);


    CHECK_CONV("RGBu8 to n4'", unsigned char,
        babl_format("R'G'B' u8"),
        babl_format_n (babl_type ("u8"), 4),
        in, out4);

    CHECK_CONV("RGBu8 to n5'", unsigned char,
        babl_format("R'G'B' u8"),
        babl_format_n (babl_type ("u8"), 5),
        in, out5);

    CHECK_CONV("RGBu8 to n6'", unsigned char,
        babl_format("R'G'B' u8"),
        babl_format_n (babl_type ("u8"), 6),
        in, out6);
  }

  babl_exit ();
  return !OK;
}
