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

#include <string.h>
#include <assert.h>

#include "babl.h"

static void
convert_u8_double (void *src,
                   void *dst,
                   int   n)
{
  while (n--)
    {
      (*(double *) dst) = (*(unsigned char *) src/255.0);
      dst += 8;
      src += 1;
    }
}

static void
convert_double_u8 (void *src,
                   void *dst,
                   int   n)
{
  while (n--)
    {
      double         dval = *(double *) src;
      unsigned char u8val;

      if (dval < 0)
        u8val = 0;
      else if (dval > 1)
        u8val = 255;
      else
        u8val = dval*255.0;

      *(unsigned char *) dst = u8val;
      dst += 1;
      src += 8;
    }
}

void
babl_base_type_u8 (void)
{
  babl_type_new (
    "u8",
    "id",   BABL_U8,
    "bits", 8,
    NULL);

  babl_conversion_new (
    "babl-base: u8 to double",
    "source",      babl_type_id (BABL_U8),
    "destination", babl_type_id (BABL_DOUBLE),
    "linear", convert_u8_double,
    NULL
  );

  babl_conversion_new (
    "babl-base: double to u8",
    "source",      babl_type_id (BABL_DOUBLE),
    "destination", babl_type_id (BABL_U8),
    "linear", convert_double_u8,
    NULL
  );
}
