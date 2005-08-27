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

#include "babl.h"
#include <math.h>
#include "babl-internal.h"

#define BUFFER_LENGTH  4
#define TOLERANCE 0.003

unsigned char u8_buf [BUFFER_LENGTH]= {   0,  255, 127,   63, };
float float_ref_buf[BUFFER_LENGTH]  = { 0.0,  1.0, 0.5, 0.25, };
float float_buf [BUFFER_LENGTH];

int
test (void)
{
  Babl *fish;
  int   i;
  int   OK=1;

  
  fish = babl_fish (
    babl_format_new (
      "foo",
      babl_model ("Y"),
      babl_type ("u8"),
      babl_component ("Y"),
      NULL
    ),
    babl_format_new (
      "bar",
      babl_model ("Y"),
      babl_type ("float"),
      babl_component ("Y"),
      NULL
    ));

  babl_process (fish, 
     u8_buf, float_buf, 
     BUFFER_LENGTH);
  
  for (i=0; i<BUFFER_LENGTH; i++)
    {
      if (fabs (float_buf[i] - float_ref_buf[i]) > TOLERANCE)
        {
          babl_log ("%i .. %f-%f=%f",
           u8_buf[i], float_buf[i], float_ref_buf[i],
           fabs (float_buf[i] - float_ref_buf[i])
           );
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



