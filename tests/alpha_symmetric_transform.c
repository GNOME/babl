/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2005, 2017 Øyvind Kolås.
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
#include <math.h>
#include "babl-internal.h"

#define TOLERANCE    0.001
#define PIXELS       12

float source_buf [PIXELS * 4] =
{ 10.0, 1.0, 0.1,      1.0,
  10.0, 1.0, 0.1,      0.5,
  10.0, 1.0, 0.1,      0.1,
  10.0, 1.0, 0.1,      0.01,
  10.0, 1.0, 0.1,     -0.01,
  10.0, 1.0, 0.1,      1.5,
  10.0, 1.0, 0.0001,   0.000001,
  10.0, 1.0, 0.1 ,    -0.00001,
  10.0, 1.0, 0.1,      0.0,
  10.0, 1.0, 0.1,     -0.5,
  1000.0,10000.0, 100000.0, 0.001,
  5000.0,50000.0, 500000.0, 0.01,
};

float bounce_buf [PIXELS * 4];
float destination_buf [PIXELS * 4];

static int
test (void)
{
  int i;
  int OK = 1;

  babl_process (babl_fish ("RGBA float", "RaGaBaA float"),
                source_buf, bounce_buf,
                PIXELS);
  babl_process (babl_fish ("RaGaBaA float", "RGBA float"),
                bounce_buf, destination_buf,
                PIXELS);

  for (i = 0; i < PIXELS; i++)
    {
      for (int c = 0; c < 4; c++)
      {
      if (fabs (destination_buf[i*4+c] - source_buf[i*4+c]) > TOLERANCE)
        {
          babl_log ("separate alpha %i.%i: %.9f!=%.9f(ref)  ", i, c, destination_buf[i*4+c],
                                                  source_buf[i*4+c]);
          OK = 0;
        }
     //   fprintf (stdout, "%.19f ", destination_buf[i*4+c]);
      }
    //  fprintf (stdout, "\n");
    }

  fprintf (stdout, "\n");

  babl_process (babl_fish ("RaGaBaA float", "RGBA float"),
                source_buf, bounce_buf,
                PIXELS);
  babl_process (babl_fish ("RGBA float", "RaGaBaA float"),
                bounce_buf, destination_buf,
                PIXELS);

  for (i = 0; i < PIXELS; i++)
    {
      for (int c = 0; c < 4; c++)
      {
      if (fabs (destination_buf[i*4+c] - source_buf[i*4+c]) > TOLERANCE)
        {
          babl_log ("associatd-alpha %i.%i: %.9f!=%.9f(ref)  ", i, c, destination_buf[i*4+c],
                                                  source_buf[i*4+c]);
          OK = 0;
        }
      //  fprintf (stdout, "%.19f ", destination_buf[i*4+c]);
      }
    //  fprintf (stdout, "\n");
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
