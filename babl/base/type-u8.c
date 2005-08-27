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

static inline void
convert_double_u8_scaled (double        min_val,
                          double        max_val,
                          unsigned char min,
                          unsigned char max,
                          void         *src,
                          void         *dst,
                          int           src_pitch,
                          int           dst_pitch,
                          int           n)
{
  while (n--)
    {
      double         dval = *(double *) src;
      unsigned char u8val;

      if (dval < min_val)
        u8val = min;
      else if (dval > max_val)
        u8val = max;
      else
        u8val = (dval-min_val) / (max_val-min_val) * (max-min) + min;

      *(unsigned char *) dst = u8val;
      src += src_pitch;
      dst += dst_pitch;
    }
}

static inline void
convert_u8_double_scaled (double        min_val,
                          double        max_val,
                          unsigned char min,
                          unsigned char max,
                          void         *src,
                          void         *dst,
                          int           src_pitch,
                          int           dst_pitch,
                          int           n)
{
  while (n--)
    {
      int    u8val = *(unsigned char *) src;
      double dval;

      if (u8val < min)
        dval = min_val;
      else if (u8val > max)
        dval = max_val;
      else
        dval  = (u8val-min) / (double)(max-min) * (max_val-min_val) + min_val;

      (*(double *) dst) = dval;

      dst += dst_pitch;
      src += src_pitch;
    }
}

#define MAKE_CONVERSIONS(name, min_val, max_val, min, max)      \
static void                                                     \
convert_##name##_double (void *src,                             \
                         void *dst,                             \
                         int   src_pitch,                       \
                         int   dst_pitch,                       \
                         int   n)                               \
{                                                               \
  convert_u8_double_scaled (min_val, max_val, min, max,         \
                            src, dst, src_pitch, dst_pitch, n); \
}                                                               \
static void                                                     \
convert_double_##name (void *src,                               \
                       void *dst,                               \
                       int   src_pitch,                         \
                       int   dst_pitch,                         \
                       int   n)                                 \
{                                                               \
  convert_double_u8_scaled (min_val, max_val, min, max,         \
                            src, dst, src_pitch, dst_pitch, n); \
}

MAKE_CONVERSIONS (u8,        0.0, (255.0F/256.0F)*1.0, 0x00, 0xff);
MAKE_CONVERSIONS (u8_luma,   0.0, 1.0, 16, 235);
MAKE_CONVERSIONS (u8_chroma, 0.0, 1.0, 16, 240);

/* source ICC.1:2004-10 */
MAKE_CONVERSIONS (u8_l,  0.0, 100.0,    0x00, 0xff);
MAKE_CONVERSIONS (u8_ab, -128.0, 127.0, 0x00, 0xff);

void
babl_base_type_u8 (void)
{
  babl_type_new (
    "u8",
    "id",   BABL_U8,
    "bits", 8,
    NULL);

  babl_type_new (
    "u8-luma",
    "id",             BABL_U8_LUMA,
    "bits",           8,
    NULL
  );

  babl_type_new (
    "u8-chroma",
    "id",             BABL_U8_CHROMA,
    "integer",
    "unsigned",
    "bits",           8,
    "min",    (long) 16,
    "max",    (long)240,
    "min_val",     -0.5,
    "max_val",      0.5,
    NULL
  );

  babl_type_new (
    "u8-CIE-L",
    "id",       BABL_U8_CIE_L,
    "integer",
    "unsigned",
    "bits",         8,
    "min_val",    0.0,
    "max_val",  100.0,
    NULL
  );

  babl_type_new (
    "u8-CIE-ab",
    "id",       BABL_U8_CIE_AB,
    "integer",
    "unsigned",
    "bits",         8,
    "min_val",  -50.0,
    "max_val",   50.0,
    NULL
  );

  babl_conversion_new (
    "babl-base: u8 to double",
    "source",      babl_type_id (BABL_U8),
    "destination", babl_type_id (BABL_DOUBLE),
    "linear",      convert_u8_double,
    NULL
  );
  babl_conversion_new (
    "babl-base: double to u8",
    "source",      babl_type_id (BABL_DOUBLE),
    "destination", babl_type_id (BABL_U8),
    "linear",      convert_double_u8,
    NULL
  );


  babl_conversion_new (
    "babl-base: u8-luma to double",
    "source",      babl_type_id (BABL_U8_LUMA),
    "destination", babl_type_id (BABL_DOUBLE),
    "linear",      convert_u8_luma_double,
    NULL
  );
  babl_conversion_new (
    "babl-base: double to u8-luma",
    "source",      babl_type_id (BABL_DOUBLE),
    "destination", babl_type_id (BABL_U8_LUMA),
    "linear",      convert_double_u8_luma,
    NULL
  );

  babl_conversion_new (
    "babl-base: u8-chroma to double",
    "source",      babl_type_id (BABL_U8_CHROMA),
    "destination", babl_type_id (BABL_DOUBLE),
    "linear",      convert_u8_chroma_double,
    NULL
  );
  babl_conversion_new (
    "babl-base: double to u8-chroma",
    "source",      babl_type_id (BABL_DOUBLE),
    "destination", babl_type_id (BABL_U8_CHROMA),
    "linear",      convert_double_u8_chroma,
    NULL
  );

  babl_conversion_new (
    "babl-base: u8-CIE-L to double",
    "source",      babl_type_id (BABL_U8_CIE_L),
    "destination", babl_type_id (BABL_DOUBLE),
    "linear",      convert_u8_l_double,
    NULL
  );
  babl_conversion_new (
    "babl-base: double to u8-CIE-L",
    "source",      babl_type_id (BABL_DOUBLE),
    "destination", babl_type_id (BABL_U8_CIE_L),
    "linear",      convert_double_u8_l,
    NULL
  );

  babl_conversion_new (
    "babl-base: u8-CIE-ab to double",
    "source",      babl_type_id (BABL_U8_CIE_AB),
    "destination", babl_type_id (BABL_DOUBLE),
    "linear",      convert_u8_ab_double,
    NULL
  );
  babl_conversion_new (
    "babl-base: double to u8-CIE-ab",
    "source",      babl_type_id (BABL_DOUBLE),
    "destination", babl_type_id (BABL_U8_CIE_AB),
    "linear",      convert_double_u8_ab,
    NULL
  );
}
