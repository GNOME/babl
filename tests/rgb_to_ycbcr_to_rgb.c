/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2005, Øyvind Kolås.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */


#include <math.h>
#include "babl.h"
#include "babl-internal.h"

#define PIXELS 6
#define TOLERANCE 0.001 

float source_buf [PIXELS*3]=
  {0.0, 0.0, 0.0,
   0.5, 0.5, 0.5,
   1.0, 1.0, 1.0,
   1.0, 0.0, 0.0,
   0.0, 1.0, 0.0,
   0.0, 0.0, 1.0};

float temp_buf        [PIXELS*3];
float destination_buf [PIXELS*3];

int
test (void)
{
  BablFish *fish;
  int       i;
  int      OK=1;

  
  fish = babl_fish (
    (Babl*) babl_pixel_format_new (
      "foo",
      babl_model ("rgb"),
      babl_type ("float"),
      babl_component ("red"),
      babl_component ("green"),
      babl_component ("blue"),
      NULL
    ),
    (Babl*) babl_pixel_format_new (
      "bar",
      babl_model ("ycbcr"),
      babl_type ("float"), 
      babl_component ("luminance"),
      babl_component ("cb"),
      babl_component ("cr"),
      NULL
    )
  );

  babl_fish_process (fish, source_buf, temp_buf, PIXELS);

  fish = babl_fish ((Babl*) babl_pixel_format ("bar"),
                    (Babl*) babl_pixel_format ("foo"));
  
  babl_fish_process (fish, temp_buf, destination_buf, PIXELS);

  for (i=0; i<PIXELS * 3; i++)
    {
      if (fabs(destination_buf[i] - source_buf[i]) > TOLERANCE)
        {
          babl_log ("%2i (%2i%%3=%i, %2i/3=%i) is %f should be %f",
                      i, i,i%3,    i,i/3,  destination_buf[i], source_buf[i]);
          OK=0;
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
  if (test())
    return -1;
  babl_destroy ();
  return 0;
}
