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
          printf (" %s failed #%i[%i]  got %i expected %i\n", test_name, i, c, result[c], expected_pix[i][c]);       \
          OK = 0;          \
        }       \
    }       \
  }

#include <assert.h>

int
main (int    argc,
      char **argv)
{
  int OK = 1;
  babl_init ();
  if(1){
    unsigned char in[][1]   = {{        0},{          1},{          2},{15}};
    unsigned char out[][4]  = {{0,0,0,255},{127,0,0,255},{0,127,0,255},{255,255,255,255}};
    const Babl *palA;// = babl_new_palette (NULL, 0);
    //Babl *palB = babl_new_palette (NULL, 0);
    //
    babl_new_palette (NULL, &palA, NULL);
    assert (palA);

    CHECK_CONV("pal to rgba", unsigned char,
        palA, babl_format("RGBA u8"),
        in, out);
  }
  if(0){
    unsigned char in[][2]   = {{    0,255},{      1,255},{      2,255},{15,200}};
    unsigned char out[][4]  = {{0,0,0,255},{127,0,0,255},{0,127,0,255},{255,255,255,255}};
    const Babl *palA;// = babl_new_palette (NULL, 0);
    //Babl *palB = babl_new_palette (NULL, 0);
    //
    babl_new_palette (NULL, NULL, &palA);
    assert (palA);

    CHECK_CONV("pal to rgba", unsigned char,
        palA, babl_format("RGBA u8"),
        in, out);

#if 0
    CHECK_CONV("pal to rgba", unsigned char,
        palB, babl_format("RGBA u8"),
        in, out);

    CHECK_CONV("pal to rgba", unsigned char,
        palA, babl_format("RGBA u8"),
        in, out);
#endif
  }
#if 0
  {
    unsigned char in[][4]  = {{0,0,0,255},{140,0,0,255},{0,127,0,255}};
    unsigned char out[][1] = {{        0},{          1},{          2}};

    CHECK_CONV("rgba to pal", unsigned char,
         babl_format("RGBA u8"), babl_new_palette ("palC", 0),
         in, out);
  }

  {
    unsigned char in[][4]  = {{0,0,0,255},{140,0,0,255},{0,127,0,127}};
    unsigned char out[][2] = {{    0,255},{      1,255},{      2,127}};

    CHECK_CONV("rgba to pal+alpha", unsigned char,
         babl_format("RGBA u8"), babl_new_palette ("palD", 1),
         in, out);
  }

  /* check with a custom floating point palette, _and_ alpha component  */
  {
    float palette[] = {
      0.5,  1.0,
      0.23, 0.42,
      1.0,  0.2
    };

    unsigned char in[][2]   = {{          0,255},{0,127},{       1,255},{         2,255}};
    unsigned char out[][4]  = {{128,128,128,255},{128,128,128,127},{59,59,59,107},{255,255,255,51}};

    Babl *pal = babl_new_palette (NULL, 1);

    babl_palette_set_palette (pal, babl_format ("YA float"), palette, 3);

    CHECK_CONV("rgba to YA float pal+alpha", unsigned char,
         pal, babl_format("RGBA u8"),
         in, out);
  }


  /* check with a custom floating point palette, _and_ alpha component  */
  {
    float palette[] = {
      0.5,  1.0,
      0.23, 0.42,
      1.0,  0.2
    };

    unsigned char in[][2]   = {{          0,255},{0,127},{       1,255},{         2,255}};
    unsigned char out[][4]  = {{128,128,128,255},{128,128,128,127},{59,59,59,107},{255,255,255,51}};

    Babl *pal = babl_new_palette (NULL, 1);

    babl_palette_set_palette (pal, babl_format ("YA float"), palette, 3);

    CHECK_CONV("rgba to YA float pal+alpha", unsigned char,
         pal, babl_format("RGBA u8"),
         in, out);
  }

  /* check with a custom floating point palette */
  {
    float palette[] = {
      0.5,  1.0,
      0.23, 0.42,
      1.0,  0.2
    };

    unsigned char in[][1]   = {{              0},{          1},{          2}};
    unsigned char out[][4]  = {{128,128,128,255},{59,59,59,107},{255,255,255,51}};

    Babl *pal = babl_new_palette (NULL, 0);

    babl_palette_set_palette (pal, babl_format ("YA float"), palette, 3);

    CHECK_CONV("rgba to float pal", unsigned char,
         pal, babl_format("RGBA u8"),
         in, out);
  }
#endif

  babl_exit ();
  return !OK;
}
