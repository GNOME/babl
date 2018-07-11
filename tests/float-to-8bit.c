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
  
  //for (i = 0; i < 400000; i++)
  {
  {
    float in[][4]   = {{ 0.21582, -0.55, -0.14, 1.0 }, {0.2, 0.3, 0.5, 0.6}, {0.0, 1.0, 2.0, 3.0}};
    unsigned char out[][4]  = {{ 55, 0, 0, 255 }, {51,77,128,153}, {0,255,255,255}};

    CHECK_CONV("float -> u8 1", unsigned char,
        babl_format("R'G'B'A float"),
        babl_format("R'G'B'A u8"),
        in, out);
  }

  {
    float in[][4]   = {{ 0.21582, -0.55, -0.14, 1.0 }, {0.2, 0.3, 0.5, 0.6}, {0.0, 1.0, 2.0, 3.0}};
    unsigned char out[][4]  = {{ 10, 0, 0, 255 }, {8,19,55,153}, {0,255,255,255}};

    CHECK_CONV("float -> u8 2", unsigned char,
        babl_format("R'G'B'A float"),
        babl_format("RGBA u8"),
        in, out);
  }

  {
    float in[][4]   = {{ 0.21582, -0.55, -0.14, 1.0 }, {0.2, 0.3, 0.5, 0.6}, {0.0, 1.0, 2.0, 3.0}};
    unsigned char out[][4]  = {{ 55, 0, 0, 255 }, {51,77,128,153}, {0,255,255,255}};

    CHECK_CONV("float -> u8 3", unsigned char,
        babl_format("RGBA float"),
        babl_format("RGBA u8"),
        in, out);
  }

  {
    float in[][4]   = {{ 0.21582, -0.55, -0.14, 1.0 }, {0.2, 0.3, 0.5, 0.6}, {0.0, 1.0, 2.0, 3.0}};
    unsigned char out[][3]  = {{128, 0, 0}, {124,149,188}, {0,255,255}};

    CHECK_CONV("float -> u8 4", unsigned char,
        babl_format("RGBA float"),
        babl_format("R'G'B' u8"),
        in, out);
  }

  {
    float in[][4]   = {{ 0.21582, -0.55, -0.14, 1.0 }, {0.2, 0.3, 0.5, 0.6}, {0.0, 1.0, 2.0, 3.0}};
    unsigned char out[][4]  = {{128, 0, 0, 255 }, {156,188,235,153}, {0,156,213,255}};

    CHECK_CONV("float -> u8 5", unsigned char,
        babl_format("RaGaBaA float"),
        babl_format("R'G'B'A u8"),
        in, out);
  }

  {
    /*                                                 (0.5 / 0.6) * 255 = 212.5, I'm not going to worry about rounding that close... */
    float in[][4]   = {{ 0.21582, -0.55, -0.14, 1.0 }, {0.2, 0.301, 0.49998, 0.6}, {0.0, 3.0, 6.0, 3.0}};
    unsigned char out[][4]  = {{55, 0, 0, 255 }, {85,128,212,153}, {0,255,255,255}};

    CHECK_CONV("float -> u8 6", unsigned char,
        babl_format("R'aG'aB'aA float"),
        babl_format("R'G'B'A u8"),
        in, out);
  }
  }

  babl_exit ();
  return !OK;
}
