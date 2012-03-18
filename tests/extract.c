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
 * <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "babl.h"


#define CHECK_CONV(test_name, componenttype, src_fmt, dst_fmt, src_pix, expected_pix) \
  {       \
  const Babl *fish;       \
  int i;       \
  fish = babl_fish (src_fmt, dst_fmt);       \
  if (!fish)       \
    {       \
      printf ("  %s failed to make fish\n", test_name);       \
      OK = 0;       \
    }       \
  for (i = 0; i < sizeof(src_pix)/sizeof(src_pix[0]); i ++)       \
    {       \
      int c;\
      componenttype result[10];       \
      babl_process (fish, src_pix[i], result, 1);       \
      for (c = 0; c < sizeof(expected_pix[i])/sizeof(expected_pix[i][0]); c++) \
      if (result[c] != expected_pix[i][c])       \
        {       \
          printf (" %s failed #%i[%i]   %i\n", test_name, i, c, result[c]);       \
          OK = 0;          \
        }       \
    }       \
  }

int
main (int    argc,
      char **argv)
{
  int OK = 1;
  babl_init ();

  {
    char in[][4]  = {{0,1,2,3},{4,5,6,7},{8,9,10,11}};
    char out[][1] = {{      3},{      7},{       11}};

    babl_format_new ("name", "A u8",
         babl_model ("YA"), babl_type ("u8"),
          babl_component ("A"), NULL);

    CHECK_CONV("extract alpha", char,
        babl_format("RGBA u8"), babl_format("A u8"),
        in, out);
  }

  {
    char in[][4]  = {{0,1,2,3},{4,5,6,7},{8,9,10,11}};
    char out[][1] = {{  1    },{  5    },{  9      }};

    babl_format_new ("name", "G u8",
         babl_model ("RGBA"), babl_type ("u8"),
          babl_component ("G"), NULL);

    CHECK_CONV("extract green", char,
         babl_format("RGBA u8"), babl_format("G u8"),
         in, out);
  }


  {
    char in[][4]  = {{0,1,2,3},{4,5,6,7},{8,9,10,11}};
    char out[][2] = {{  2,1  },{  6,5  },{  10,9   }};

    babl_format_new ("name", "BG u8",
         babl_model ("RGBA"), babl_type ("u8"),
          babl_component ("B"),
          babl_component ("G"), NULL);

    CHECK_CONV("extract green", char,
         babl_format("RGBA u8"), babl_format("BG u8"),
         in, out);
  }



  {
    float in[][4]  = {{0,1,2,3/255.0},{4,5,6,277/255.0},{8,9,10,101/255.0}};
    char out[][1] = {{       3},      {      255},{             101}};

    CHECK_CONV("extract alpha from float", char,
         babl_format("RGBA float"), babl_format("A u8"),
         in, out);
  }

  {
    char in[][4]   = {{1,2,3,4},{4,5,6,7},{8,9,10,11}};
    char out[][4]  = {{4,3,2,1},{7,6,5,4},{11,10,9,8}};

    babl_format_new ("name", "abgr",
         babl_model ("RGBA"), babl_type ("u8"),
          babl_component ("A"),
          babl_component ("B"),
          babl_component ("G"),
          babl_component ("R"),
          NULL);

    CHECK_CONV("bgra", char,
         babl_format("RGBA u8"), babl_format("abgr"),
         in, out);
  }

  {
    char in[][4]   = {{1,2,3,4},{4,5,6,7},{8,9,10,11}};
    //char out[][4]= {{3,0,0,4},{6,0,0,7},{10,0,0,11}};
    char out[][4]  = {{3,3,3,4},{6,6,6,7},{10,10,10,11}};

    CHECK_CONV("bPADa", char,
         babl_format("RGBA u8"),
         babl_format_new (babl_model ("RGBA"), babl_type ("u8"),
               babl_component ("B"),
               babl_component ("PAD"),
               babl_component ("PAD"),
               babl_component ("A"),
               NULL),
         in, out);
  }

  babl_exit ();
  return !OK;
}
