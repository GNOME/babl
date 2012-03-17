/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2012 Øyvind Kolås.
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

#include <stdlib.h>
#include "babl.h"

int init (void);

int
init (void)
{
  int   testint  = 23;
  char *testchar = (char*) &testint;
  int   littleendian = (testchar[0] == 23);

  if (littleendian)
    {
      babl_format_new (
        "name", "cairo-ARGB32",
        babl_model ("R'aG'aB'aA"),
        babl_type ("u8"),
        babl_component ("B'a"),
        babl_component ("G'a"),
        babl_component ("R'a"),
        babl_component ("A"),
        NULL
      );

      babl_format_new (
        "name", "cairo-RGB24",
        babl_model ("R'G'B'"),
        babl_type ("u8"),
        babl_component ("B'"),
        babl_component ("G'"),
        babl_component ("R'"),
        babl_component ("PAD"),
        NULL
      );
    }
  else
    {
      babl_format_new (
        "name", "cairo-ARGB32",
        babl_model ("R'G'B'A"),
        babl_type ("u8"),
        babl_component ("A"),
        babl_component ("R'a"),
        babl_component ("G'a"),
        babl_component ("B'a"),
        NULL
      );
      babl_format_new (
        "name", "cairo-RGB24",
        babl_model ("R'G'B'"),
        babl_type ("u8"),
        babl_component ("PAD"),
        babl_component ("R'"),
        babl_component ("G'"),
        babl_component ("B'"),
        NULL
      );
    }
  babl_format_new (
    "name", "cairo-A8",
    babl_model ("YA"),
    babl_type ("u8"),
    babl_component ("A"),
    NULL
  );
  return 0;
}
