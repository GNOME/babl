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

#include <math.h>
#include <babl/babl.h>
#include <stdio.h>

#define PIXELS       6
#define TOLERANCE    0

unsigned char source_buf [PIXELS * 5] =
{ 0,     0,   0, 22, 33,
  127, 127, 127, 12, 33,
  255, 225, 255, 33, 33,
  255, 0.0, 0.0, 4,  33,
  0.0, 255, 0.0, 122,33,
  0.0, 0.0, 255, 222,33};

unsigned char cmk_buf [PIXELS * 4];
unsigned char cyk_buf [PIXELS * 4];
unsigned char dest_buf [PIXELS * 5];

static int
test (void)
{
  int i;
  int OK = 1;

  babl_process (babl_fish ("camayakaA u8", "cairo-ACYK32"),
                source_buf, cyk_buf,
                PIXELS);
  babl_process (babl_fish ("camayakaA u8", "cairo-ACMK32"),
                source_buf, cmk_buf,
                PIXELS);

  babl_process (babl_fish ("cairo-ACMK32", "camayakaA u8"),
                cmk_buf, dest_buf,
                PIXELS);
  babl_process (babl_fish ("cairo-ACYK32", "camayakaA u8"),
                cyk_buf, dest_buf,
                PIXELS);

  for (i = 0; i < PIXELS * 5; i++)
    {
      if (fabs (dest_buf[i] - source_buf[i]) > TOLERANCE)
        {
          fprintf (stderr, "%i is wrong\n", i);
          OK = 0;
        }
    }
  if (!OK)
    return -1;
  return 0;
}

int
main (int    argc,
      char **argv)
{
  babl_init ();
  if (test ())
    return -1;
  babl_exit ();
  return 0;
}
