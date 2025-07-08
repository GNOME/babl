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
#include "babl-internal.h"

#ifdef _WIN32
#define putenv _putenv
#endif

 static const struct 
  { 
    const char *from_format; 
    const char *to_format; 
  } 
  fishes[] = 
  { 
    { "Y' u8",          "RaGaBaA float" }, 
    { "Y u8",           "RaGaBaA float" }, 
    { "R'G'B'A u8",     "RaGaBaA float" }, 
    { "R'G'B'A float",  "R'G'B'A u8"    }, 
    { "R'G'B'A float",  "R'G'B' u8"     }, 
    { "R'G'B'A u8",     "RGBA float"    }, 
    { "RGBA float",     "R'G'B'A u8"    }, 
    { "RGBA float",     "R'G'B'A u8"    }, 
    { "RGBA float",     "R'G'B'A float" }, 
    { "Y' u8",          "R'G'B' u8"     }, 
    { "Y u8",           "Y float"       }, 
    { "R'G'B' u8",      "cairo-RGB24"   }, 
    { "R'G'B' u8",      "R'G'B'A float" }, 
    { "R'G'B' u8",      "R'G'B'A u8"    }, 
    { "R'G'B'A u8",     "R'G'B'A float" }, 
    { "R'G'B'A u8",     "cairo-ARGB32"  }, 
    { "R'G'B'A double", "RGBA float"    }, 
    { "R'G'B'A float",  "RGBA double"   }, 
    { "R'G'B' u8",      "RGB float"     }, 
    { "RGB float",      "R'G'B'A float" }, 
    { "R'G'B' u8",      "RGBA float"    }, 
    { "RaGaBaA float",  "R'G'B'A float" }, 
    { "RaGaBaA float",  "RGBA float"    }, 
    { "RGBA float",     "RaGaBaA float" }, 
    { "R'G'B' u8",      "RaGaBaA float" }, 
    { "cairo-ARGB32",   "R'G'B'A u8"    } 
  };

int
main (void)
{
  putenv ("BABL_DEBUG_CONVERSIONS" "=" "1");
  putenv ("BABL_DEBUG_MISSING" "=" "1");
  babl_init ();
  
  for (size_t i = 0; i < sizeof (fishes)/sizeof(fishes[0]);i ++)
  {
    babl_fish (babl_format (fishes[i].from_format),
               babl_format (fishes[i].to_format));
  }

  babl_exit ();
  return 0;
}
