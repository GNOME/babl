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
#include "babl.h"
#include "babl-ids.h"
#include "util.h"

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


static void
copy_strip_1 (int    src_bands,
              void **src,
              int   *src_pitch,
              int    dst_bands,
              void **dst,
              int   *dst_pitch,
              int    n)
{
  BABL_PLANAR_SANITY
  while (n--)
    {
      int i;

      for (i=0;i<dst_bands;i++)
        {
          double foo;
          if (i<src_bands)
            foo = *(double *) src[i];
          else
            foo = 1.0;
          *(double*)dst[i] = foo;
        }

      BABL_PLANAR_STEP
    }
}

void
babl_core_init (void)
{
  babl_type_new (
    "double",
    "id",          BABL_DOUBLE,
    "bits",        64,
    NULL);

  babl_conversion_new (
    babl_type_id (BABL_DOUBLE),
    babl_type_id (BABL_DOUBLE),
    "linear",      convert_double_double,
    NULL
  );

  babl_component_new (
    "R",
    "id", BABL_RED,
    "luma",
    "chroma",
    NULL);

  babl_component_new (
   "G",
    "id",   BABL_GREEN,
   "luma", 
   "chroma",
   NULL);
  
  babl_component_new (
   "B",
    "id",   BABL_BLUE,
   "luma",
   "chroma",
   NULL);

  babl_component_new (
   "A",
   "id",    BABL_ALPHA,
   "alpha",
   NULL);

  babl_model_new (
    "id", BABL_RGBA,
    babl_component_id (BABL_RED),
    babl_component_id (BABL_GREEN),
    babl_component_id (BABL_BLUE),
    babl_component_id (BABL_ALPHA),
    NULL);

  babl_conversion_new (
    babl_model_id (BABL_RGBA),
    babl_model_id (BABL_RGBA),
    "planar",      copy_strip_1,
    NULL
  );
}
