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
convert_double_double (void *src,
                       void *dst,
                       int   src_pitch,
                       int   dst_pitch,
                       int   n)
{
  if (src_pitch == 64 &&
      dst_pitch == 64)
    {
      memcpy (dst, src, n/8);
      return;
    }
  while (n--)
    {
      (*(double *) dst) = (*(double *) src);
      dst += dst_pitch;
      src += src_pitch;
    }
}

void
babl_base_type_double (void)
{
  babl_type_new (
    "double",
    "id",          BABL_DOUBLE,
    "bits",        64,
    NULL);

  babl_conversion_new (
    "babl-base: double to double",
    "source",      babl_type_id (BABL_DOUBLE),
    "destination", babl_type_id (BABL_DOUBLE),
    "linear", convert_double_double,
    NULL
  );
}
