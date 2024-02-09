/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2018, 2024 Øyvind Kolås, Michael Natterer
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
 *
 * this is an interim file, to keep babl from complaining about double
 * registration of cmyk formats.
 */

#include "config.h"
#include <string.h>
#include <stdio.h>

#include "babl.h"
#include "base/util.h"

int init (void);

int
init (void)
{
  static const char *babl_types[] =
  {   
    "u8",              
    "u16",             
    "u32",             
    "half",            
    "float",
    "double"
  };

  int i;

  for (i = 0; i < sizeof(babl_types)/sizeof(babl_types[0]); i++)
    {
      char name[16];
  
      snprintf (name, sizeof (name), "R %s", babl_types[i]);
      babl_format_new ("name", name,
                       babl_model ("RGBA"),
                       babl_type (babl_types[i]),
                       babl_component ("R"),
                       NULL);
      snprintf (name, sizeof (name), "R' %s", babl_types[i]);
      babl_format_new ("name", name,
                       babl_model ("R'G'B'A"),
                       babl_type (babl_types[i]),
                       babl_component ("R'"),
                       NULL);
      snprintf (name, sizeof (name), "R~ %s", babl_types[i]);
      babl_format_new ("name", name,
                       babl_model ("R~G~B~A"),
                       babl_type (babl_types[i]),
                       babl_component ("R~"),
                       NULL);

      snprintf (name, sizeof (name), "G %s", babl_types[i]);
      babl_format_new ("name", name,
                       babl_model ("RGBA"),
                       babl_type (babl_types[i]),
                       babl_component ("G"),
                       NULL);
      snprintf (name, sizeof (name), "G' %s", babl_types[i]);
      babl_format_new ("name", name,
                       babl_model ("R'G'B'A"),
                       babl_type (babl_types[i]),
                       babl_component ("G'"),
                       NULL);
      snprintf (name, sizeof (name), "G~ %s", babl_types[i]);
      babl_format_new ("name", name,
                       babl_model ("R~G~B~A"),
                       babl_type (babl_types[i]),
                       babl_component ("G~"),
                       NULL);

      snprintf (name, sizeof (name), "B %s", babl_types[i]);
      babl_format_new ("name", name,
                       babl_model ("RGBA"),
                       babl_type (babl_types[i]),
                       babl_component ("B"),
                       NULL);
      snprintf (name, sizeof (name), "B' %s", babl_types[i]);
      babl_format_new ("name", name,
                       babl_model ("R'G'B'A"),
                       babl_type (babl_types[i]),
                       babl_component ("B'"),
                       NULL);
      snprintf (name, sizeof (name), "B~ %s", babl_types[i]);
      babl_format_new ("name", name,
                       babl_model ("R~G~B~A"),
                       babl_type (babl_types[i]),
                       babl_component ("B~"),
                       NULL);

      snprintf (name, sizeof (name), "A %s", babl_types[i]);
      babl_format_new ("name", name,
                       babl_model ("RGBA"),
                       babl_type (babl_types[i]),
                       babl_component ("A"),
                       NULL);
    }

  return 0;
}
