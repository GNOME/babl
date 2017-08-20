/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2017, Øyvind Kolås.
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
#include "babl-internal.h"
#include <stdio.h>
#include <stdlib.h>

#define ICC_HEADER_LEN 128
#define TAG_COUNT_OFF  ICC_HEADER_LEN

static int load_byte (const char *icc, int length, int offset)
{
/* all reading functions take both the char *pointer and the length of the
 * buffer, and all reads thus gets protected by this condition.
 */
  if (offset < 0 || offset > length)
    return 0;

  return *(uint8_t*) (&icc[offset]);
}

static int load_sbyte (const char *icc, int length, int offset)
{
  if (offset < 0 || offset > length)
    return 0;

  return *(int8_t*) (&icc[offset]);
}

static int16_t load_u1f15 (const char *icc, int length, int offset)
{
  return load_byte (icc, length, offset + 1) +
         (load_sbyte (icc, length, offset + 0) << 8);
}

static uint16_t load_u16 (const char *icc, int length, int offset)
{
  return load_byte (icc, length, offset + 1) +
         (load_byte (icc, length, offset + 0) << 8);
}

static double load_s15f16 (const char *icc, int length, int offset)
{
  return load_u1f15 (icc, length, offset) +
         load_u16 (icc, length, offset + 2) / 65535.0f;
}

static uint32_t load_u32 (const char *icc, int length, int offset)
{
  return load_byte (icc, length, offset + 3) +
         (load_byte (icc, length, offset + 2) << 8) +
         (load_byte (icc, length, offset + 1) << 16) +
         (load_byte (icc, length, offset + 0) << 24);
}

static void load_sign (const char *icc, int length,
                       int offset, char *sign)
{
  sign[0]=load_byte(icc, length, offset);
  sign[1]=load_byte(icc, length, offset + 1);
  sign[2]=load_byte(icc, length, offset + 2);
  sign[3]=load_byte(icc, length, offset + 3);
  sign[4]=0;
}

/* looks up offset and length for a specific icc tag
 */
static int icc_tag (const char *icc, int length,
                    const char *tag, int *offset, int *el_length)
{
  int tag_count = load_u32 (icc, length, TAG_COUNT_OFF);
  int t;

  for (t =  0; t < tag_count; t++)
  {
     char tag_signature[5];
     load_sign (icc, length, TAG_COUNT_OFF + 4 + 12 * t, tag_signature);
     if (!strcmp (tag_signature, tag))
     {
        if (offset)
          *offset = load_u32 (icc, length, TAG_COUNT_OFF + 4 + 12* t + 4);
        if (el_length)
          *el_length = load_u32 (icc, length, TAG_COUNT_OFF + 4 + 12* t + 4*2);
        return 1;
     }
  }
  return 0;
}

static const Babl *babl_trc_from_icc (const char *icc,
                                      int         length,
                                      char      **error)
{
  int offset = 0;
  {
    int count = load_u32 (icc, length, offset + 8);
    int i;
    {
      if (count == 0)
      {
        return babl_trc_gamma (1.0);
      }
      else if (count == 1)
      {
        return babl_trc_gamma (load_byte (icc, length, offset + 12) +
                               load_byte (icc, length, offset + 12 + 1)/255.0);
      }
      else
      {
        return babl_trc_gamma (2.2);

        // XXX: todo implement a curve trc babl type
        //      as well as detect sRGB curve from LUTs

        for (i = 0; i < count && i < 10; i ++)
        {
          fprintf (stdout, "%i=%i ", i, load_u16 (icc, length,
                                                  offset + 12 + i * 2));
          if (i % 7 == 0)
            fprintf (stdout, "\n");
        }
      }
    }
  }
  return NULL;
}

const Babl *
babl_space_rgb_icc (const char *icc,
                    int         length,
                    char      **error)
{
  int  profile_size     = load_u32 (icc, length, 0);
  int  icc_ver_major    = load_byte (icc, length, 8);
  const Babl *trc_red   = NULL;
  const Babl *trc_green = NULL;
  const Babl *trc_blue  = NULL;
  char profile_class[5];
  char color_space[5];

  if (profile_size != length)
  {
    *error = "icc profile length inconsistency";
    return NULL;
  }
  if (icc_ver_major > 2)
  {
    *error = "only ICC v2 profiles supported";
    return NULL;
  }
  load_sign (icc, length, 12, profile_class);
  if (strcmp (profile_class, "mntr"))
  {
    *error = "not a monitor-class profile";
    return NULL;
  }
  load_sign (icc, length, 16, color_space);
  if (strcmp (color_space, "RGB "))
  {
    *error = "not defining an RGB space";
    return NULL;
  }
  {
     int offset, element_size;
     if (icc_tag (icc, length, "rTRC", &offset, &element_size))
     {
       trc_red = babl_trc_from_icc (icc + offset, element_size, error);
       if (*error) return NULL;
     }
     if (icc_tag (icc, length, "gTRC", &offset, &element_size))
     {
       trc_green = babl_trc_from_icc (icc + offset, element_size, error);
       if (*error) return NULL;
     }
     if (icc_tag (icc, length, "bTRC", &offset, &element_size))
     {
       trc_blue = babl_trc_from_icc (icc + offset, element_size, error);
       if (*error) return NULL;
     }
  }

  if (!trc_red || !trc_green || !trc_blue)
  {
     *error = "missing TRC";
     return NULL;
  }

  if (icc_tag (icc, length, "chrm", NULL, NULL) &&
      icc_tag (icc, length, "wtpt", NULL, NULL))
  {
     int offset, element_size;
     double red_x, red_y, green_x, green_y, blue_x, blue_y;
     int channels, phosporant;

     icc_tag (icc, length, "chrm", &offset, &element_size);
     channels   = load_u16 (icc, length, offset + 8);
     phosporant = load_u16 (icc, length, offset + 10);

     if (phosporant != 0)
     {
       *error = "unhandled phosporants, please report bug";
       return NULL;
     }
     if (channels != 3)
     {
       *error = "unexpected non 3 count of channels";
       return NULL;
     }

     red_x   = load_s15f16 (icc, length, offset + 12);
     red_y   = load_s15f16 (icc, length, offset + 12 + 4);
     green_x = load_s15f16 (icc, length, offset + 20);
     green_y = load_s15f16 (icc, length, offset + 20 + 4);
     blue_x  = load_s15f16 (icc, length, offset + 28);
     blue_y  = load_s15f16 (icc, length, offset + 28 + 4);

     icc_tag (icc, length, "wtpt", &offset, &element_size);
     {
       double wX = load_s15f16 (icc, length, offset + 8);
       double wY = load_s15f16 (icc, length, offset + 8 + 4);
       double wZ = load_s15f16 (icc, length, offset + 8 + 4 * 2);

       return babl_space_rgb_chromaticities (NULL,
                       wX / (wX + wY + wZ),
                       wY / (wX + wY + wZ),
                       red_x, red_y,
                       green_x, green_y,
                       blue_x, blue_y,
                       trc_red, trc_green, trc_blue);

     }
  }
  else if (icc_tag (icc, length, "rXYZ", NULL, NULL) &&
           icc_tag (icc, length, "gXYZ", NULL, NULL) &&
           icc_tag (icc, length, "bXYZ", NULL, NULL))
  {
     int offset, element_size;
     double rx, gx, bx;
     double ry, gy, by;
     double rz, gz, bz;

     icc_tag (icc, length, "rXYZ", &offset, &element_size);
     rx = load_s15f16 (icc, length, offset + 8 + 4 * 0);
     ry = load_s15f16 (icc, length, offset + 8 + 4 * 1);
     rz = load_s15f16 (icc, length, offset + 8 + 4 * 2);
     icc_tag (icc, length, "gXYZ", &offset, &element_size);
     gx = load_s15f16 (icc, length, offset + 8 + 4 * 0);
     gy = load_s15f16 (icc, length, offset + 8 + 4 * 1);
     gz = load_s15f16 (icc, length, offset + 8 + 4 * 2);
     icc_tag (icc, length, "bXYZ", &offset, &element_size);
     bx = load_s15f16 (icc, length, offset + 8 + 4 * 0);
     by = load_s15f16 (icc, length, offset + 8 + 4 * 1);
     bz = load_s15f16 (icc, length, offset + 8 + 4 * 2);

     return babl_space_rgb_matrix (NULL,
                rx, gx, bx,
                ry, gy, by,
                rz, gz, bz,
                trc_red, trc_green, trc_blue);
  }

  *error = "didnt find RGB primaries";
  return NULL;
}
