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
    unsigned char out[][1]  = {    {2    },    {6}      ,    {10}     };

    CHECK_CONV("extract B'", unsigned char,
        babl_format("R'G'B'A u8"),
        babl_format_new ("name", "B' u8",
                          babl_model ("R'G'B'A"),
                          babl_type ("u8"),
                          babl_component ("B'"),
                          NULL),
        in, out);
  }

  babl_exit ();
  return !OK;
}
