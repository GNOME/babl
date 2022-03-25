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

#define PIXELS       4
#define TOLERANCE    0.05

float source_buf [PIXELS * 3] =
{ 0.950, 1.000, 1.089,
  1.000, 0.000, 0.000,
  0.000, 1.000, 0.000,
  0.000, 0.000, 1.000
};

float reference_buf [PIXELS * 3] =
{ 100.00, -2.467186, -19.400648,
  0.0, 437.147125, 0.0,
  100.0, -431.034485, 172.4137,
  0.0, 0.0, -185.6406,
};

float destination_buf [PIXELS * 3];

static int
test (void)
{
  int i;
  int OK = 1;

  babl_process (babl_fish ("CIE XYZ float", "CIE Lab float"),
                source_buf, destination_buf,
                PIXELS);

  for (i = 0; i < PIXELS * 3; i++)
    {
      if (fabs (1.0 * destination_buf[i] - reference_buf[i]) > TOLERANCE)
        {
          fprintf (stderr, "%2i (component: %2i%%3=%i, test no: %2i/3=%i) is %f should be %f\n",
                    i, i, i % 3, i, i / 3, destination_buf[i], reference_buf[i]);
          OK = 0;
        }
    }
  if (!OK)
    return -1;
  return 0;
}

int
main (void)
{
  babl_init ();
  if (test ())
    return -1;
  babl_exit ();
  return 0;
}
