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
#include <stdio.h>
#include <stdlib.h>
#include "babl.h"

static int
cairo24_rgb_cairo24 (void)
{
  int OK = 1, i;

  for (i = 0; i < 256; ++i)
    {
      /* Valgrind complains if 'tmp' is not initialized
       * and the brittle RTTI (BABL_IS_BABL) produces a
       * crash if it contains { 0x00, 0xb1, 0xba, 0x00 }
       */
      unsigned char tmp[4] = { 0, /*0xb1, 0xba, 0x00*/ };
      /* Valgrind complains if 'output' is not initialized
       * and the brittle RTTI (BABL_IS_BABL) produces a
       * crash if it contains 0xbab100
       */
      unsigned int output = ~ 0xbab100;
      unsigned int input = (i * 256 + i) * 256 + i;

      babl_process (babl_fish ("cairo-RGB24", "R'G'B' u8"), &input, &tmp[0], 1);
      babl_process (babl_fish ("R'G'B' u8", "cairo-RGB24"), &tmp[0], &output, 1);

      if ((input & 0x00ffffff) != (output & 0x00ffffff))
        {
          fprintf (stderr , "%08x -> %d %d %d -> %08x\n",
                   input, tmp[0], tmp[1], tmp[2], output);
          OK = 0;
        }
    }

  return OK;
}

static int
rgb_cairo24_rgb (void)
{
  int OK = 1, i;

  for (i = 0; i < 256; ++i)
    {
      /* As above */
      unsigned int tmp = ~ 0xbab100;
      unsigned char output[4] = { 0x00, /*0xb1, 0xba, 0x00*/ };
      unsigned char input[4] = { i, i, i, 17 };

      babl_process (babl_fish ("R'G'B' u8", "cairo-RGB24"), input, &tmp, 1);
      babl_process (babl_fish ("cairo-RGB24", "R'G'B' u8"), &tmp, output, 1);

      if (input[0] != output[0] || input[1] != output[1] || input[2] != output[2])
        {
          fprintf (stderr , "%d %d %d -> %08x -> %d %d %d\n",
                   input[0], input[1], input[2], tmp, output[0], output[1], output[2]);
          OK = 0;
        }
    }

  return OK;
}

int
main (void)
{
  int OK;

  babl_init ();

  OK = cairo24_rgb_cairo24 () && rgb_cairo24_rgb ();

  babl_exit ();

  return !OK;
}
