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
  OK = ! babl_format_is_palette (babl_format_n (babl_type ("double"), 3));
  if(1){
    unsigned char in[][1]   = {{        0},{          1},{          2},{15}};
    unsigned char out[][4]  = {{0,0,0,255},{127,0,0,255},{0,127,0,255},{255,255,255,255}};
    const Babl *palA;// = babl_new_palette (NULL, 0);
    //Babl *palB = babl_new_palette (NULL, 0);
    //
    babl_new_palette (NULL, &palA, NULL);
    assert (palA);

    CHECK_CONV("pal to rgba", unsigned char,
        palA, babl_format("R'G'B'A u8"),
        in, out);
  }
  {
    unsigned char in[][2]   = {{    0,255},{      1,255},{      2,255},{15,200}};
    unsigned char out[][4]  = {{0,0,0,255},{127,0,0,255},{0,127,0,255},{255,255,255,200}};
    const Babl *palA;
    babl_new_palette (NULL, NULL, &palA);
    assert (palA);

    CHECK_CONV("palA to rgba", unsigned char,
        palA, babl_format("R'G'B'A u8"),
        in, out);
  }
#if 1
  {
    unsigned char in[][4]  = {{0,0,0,255},{140,0,0,255},{0,127,0,255}};
    unsigned char out[][1] = {{        0},{          1},{          2}};
    const Babl *palA;
    babl_new_palette ("palC", &palA, NULL);

    CHECK_CONV("rgba to pal", unsigned char,
         babl_format("R'G'B'A u8"), palA,
         in, out);
  }

  {
    unsigned char in[][4]  = {{0,0,0,255},{140,0,0,255},{0,127,0,127}};
    unsigned char out[][2] = {{    0,255},{      1,255},{      2,127}};
    const Babl *pal;
    babl_new_palette ("palC", NULL, &pal);

    CHECK_CONV("rgba to pal+alpha", unsigned char,
         babl_format("R'G'B'A u8"), pal,
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

    const Babl *pal;
    babl_new_palette (NULL, NULL, &pal);

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

    const Babl *pal;
    babl_new_palette (NULL, NULL, &pal);

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

    const Babl *pal;
    babl_new_palette (NULL, &pal, NULL);

    babl_palette_set_palette (pal, babl_format ("YA float"), palette, 3);

    CHECK_CONV("rgba to float pal", unsigned char,
         pal, babl_format("RGBA u8"),
         in, out);

    {
      const Babl *p2;
      p2 = babl_format_with_space ((void*)pal, babl_space ("ACEScg"));


    CHECK_CONV("rgba to float pal", unsigned char,
         p2, babl_format("RGBA u8"),
         in, out);


    }

  }
#endif



  babl_exit ();
  return !OK;
}
