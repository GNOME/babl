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

typedef struct {
  int16_t  integer;
  uint16_t fraction;
} s15f16_t;

typedef struct {
  int16_t  integer;
  uint16_t fraction;
} u8f8_t;

typedef struct {
  char str[5];
} sign_t;

#define icc_write(type, offset, value)  write_##type(icc,length,offset,value)
#define icc_read(type, offset)          read_##type(icc,length,offset)

static void write_u8 (char *icc, int length, int offset, uint8_t value)
{
  if (offset < 0 || offset > length)
    return;
  *(uint8_t*) (&icc[offset]) = value;
}

static void write_s8 (char *icc, int length, int offset, int8_t value)
{
  if (offset < 0 || offset > length)
    return;
  *(int8_t*) (&icc[offset]) = value;
}

static int read_u8 (const char *icc, int length, int offset)
{
/* all reading functions take both the char *pointer and the length of the
 * buffer, and all reads thus gets protected by this condition.
 */
  if (offset < 0 || offset > length)
    return 0;

  return *(uint8_t*) (&icc[offset]);
}

static int read_s8 (const char *icc, int length, int offset)
{
  if (offset < 0 || offset > length)
    return 0;

  return *(int8_t*) (&icc[offset]);
}

static void write_s16 (char *icc, int length, int offset, int16_t value)
{
  write_s8 (icc, length, offset + 0, value >> 8);
  write_u8 (icc, length, offset + 1, value & 0xff);
}

static int16_t read_s16 (const char *icc, int length, int offset)
{
  return icc_read (u8, offset + 1) +
         (read_s8 (icc, length, offset + 0) << 8);
}

static uint16_t read_u16 (const char *icc, int length, int offset)
{
  return icc_read (u8, offset + 1) +
         (icc_read (u8, offset + 0) << 8);
}

static void write_u16 (char *icc, int length, int offset, uint16_t value)
{
  write_u8 (icc, length, offset + 0, value >> 8);
  write_u8 (icc, length, offset + 1, value & 0xff);
}

static u8f8_t read_u8f8_ (const char *icc, int length, int offset)
{
  u8f8_t ret ={icc_read (u8, offset),
               icc_read (u8, offset + 1)};
  return ret;
}

static s15f16_t read_s15f16_ (const char *icc, int length, int offset)
{
  s15f16_t ret ={icc_read (s16, offset),
                 icc_read (u16, offset + 2)};
  return ret;
}

static void write_s15f16_ (char *icc, int length, int offset, s15f16_t val)
{
  icc_write (s16, offset, val.integer),
  icc_write (u16, offset + 2, val.fraction);
}

static s15f16_t d_to_s15f16 (double value)
{
  s15f16_t ret;
  ret.integer = floor (value);
  ret.fraction = fmod(value, 1.0) * 65535.999;
  return ret;
}

static double s15f16_to_d (s15f16_t fix)
{
  return fix.integer + fix.fraction / 65535.0;
}

static double u8f8_to_d (u8f8_t fix)
{
  return fix.integer + fix.fraction / 255.0;
}

static void write_s15f16 (char *icc, int length, int offset, double value)
{
   write_s15f16_ (icc, length, offset, d_to_s15f16 (value));
}

static double read_s15f16 (const char *icc, int length, int offset)
{
  return s15f16_to_d (read_s15f16_ (icc, length, offset));
}

static double read_u8f8 (const char *icc, int length, int offset)
{
  return u8f8_to_d (read_u8f8_ (icc, length, offset));
}

static inline void print_u8f8 (u8f8_t fix)
{
  int i;
  uint32_t foo;
  foo = fix.fraction;
  fprintf (stdout, "%i.", fix.integer);
  for (i = 0; i < 18; i++)
  {
    foo *= 10;
    fprintf (stdout, "%i", (foo / 256) % 10);
    foo = foo & 0xff;
  }
}

static inline void print_s15f16 (s15f16_t fix)
{
  int i;
  uint32_t foo;
  foo = fix.fraction;
  if (fix.integer < 0)
  {
    if (fix.integer == -1)
      fprintf (stdout, "-");
    fprintf (stdout, "%i.", fix.integer + 1);
    foo = 65535-fix.fraction;
    for (i = 0; i < 18; i++)
    {
      foo *= 10;
      fprintf (stdout, "%i", (foo / 65536) % 10);
      foo = foo & 0xffff;
    }
  }
  else
  {
  fprintf (stdout, "%i.", fix.integer);
  for (i = 0; i < 18; i++)
  {
    foo *= 10;
    fprintf (stdout, "%i", (foo / 65536) % 10);
    foo = foo & 0xffff;
  }
  }
}

static void write_u32 (char *icc,
                      int length,
                      int offset,
                      uint32_t value)
{
  int i;
  for (i = 0; i < 4; i ++)
  {
    write_u8 (icc, length, offset + i,
                  (value & 0xff000000) >> 24
                  );
    value <<= 8;
  }
}

static uint32_t read_u32 (const char *icc, int length, int offset)
{
  return icc_read (u8, offset + 3) +
         (icc_read (u8, offset + 2) << 8) +
         (icc_read (u8, offset + 1) << 16) +
         (icc_read (u8, offset + 0) << 24);
}

static sign_t read_sign (const char *icc, int length,
                         int offset)
{
  sign_t ret;
  ret.str[0]=icc_read (u8, offset);
  ret.str[1]=icc_read (u8, offset + 1);
  ret.str[2]=icc_read (u8, offset + 2);
  ret.str[3]=icc_read (u8, offset + 3);
  ret.str[4]=0;
  return ret;
}

static void write_sign (char *icc, int length,
                       int offset, char *sign)
{
  int i;
  for (i = 0; i < 4; i ++)
    icc_write (u8, offset + i, sign[i]);
}

/* looks up offset and length for a specific icc tag
 */
static int icc_tag (const char *icc, int length,
                    const char *tag, int *offset, int *el_length)
{
  int tag_count = icc_read (u32, TAG_COUNT_OFF);
  int t;

  for (t =  0; t < tag_count; t++)
  {
     sign_t sign = icc_read (sign, TAG_COUNT_OFF + 4 + 12 * t);
     if (!strcmp (sign.str, tag))
     {
        if (offset)
          *offset = icc_read (u32, TAG_COUNT_OFF + 4 + 12* t + 4);
        if (el_length)
          *el_length = icc_read (u32, TAG_COUNT_OFF + 4 + 12* t + 4*2);
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
    int count = icc_read (u32, offset + 8);
    int i;
    {
      if (count == 0)
      {
        return babl_trc_gamma (1.0);
      }
      else if (count == 1)
      {
        return babl_trc_gamma (icc_read (u8f8, offset + 12));
      }
      else
      {
        return babl_trc_gamma (2.2);

        // XXX: todo implement a curve trc babl type
        //      as well as detect sRGB curve from LUTs

        for (i = 0; i < count && i < 10; i ++)
        {
          fprintf (stdout, "%i=%i ", i, icc_read (u16, offset + 12 + i * 2));
          if (i % 7 == 0)
            fprintf (stdout, "\n");
        }
      }
    }
  }
  return NULL;
}


const char *babl_space_rgb_to_icc (const Babl *babl, int *ret_length)
{
  const BablSpace *space = &babl->space;
  static char icc[8192];
  int length=4095;
  icc[length]=0;

#if 1
  icc_write (s8, 8,-2);
  assert (icc_read (s8, 8) == -2);
  icc_write (s8, 8, 3);     // ICC verison
  assert (icc_read (s8, 8) == 3);

  icc_write (u8, 8, 2);     // ICC verison
  assert (icc_read (u8, 8) == 2);

  icc_write (u16, 8, 3);     // ICC verison
  assert (icc_read (u16, 8) == 3);

  icc_write (s16, 8, -3);     // ICC verison
  assert (icc_read (s16, 8) == -3);

  icc_write (s16, 8, 9);     // ICC verison
  assert (icc_read (s16, 8) == 9);

  icc_write (u32, 8, 4);     // ICC verison
  assert (icc_read (u32, 8) == 4);
#endif

  icc_write (sign, 4, "babl");     // ICC verison
  icc_write (u8, 8, 2);     // ICC verison
  icc_write (u8, 9, 0x20);  // 2.2 for now..
  icc_write (u32,64, 0);    // rendering intent

  icc_write (s15f16,68, 0.96421); // Illuminant
  icc_write (s15f16,72, 1.0);
  icc_write (s15f16,76, 0.82491);

  icc_write (sign, 80, "babl"); // creator

  icc_write (sign, 12, "mntr");
  icc_write (sign, 16, "RGB ");
  icc_write (sign, 20, "XYZ ");

  icc_write (u16, 24, 2017); // babl profiles
  icc_write (u16, 26, 8);    // should
  icc_write (u16, 28, 21);   // use a fixed
  icc_write (u16, 30, 2);    // date
  icc_write (u16, 32, 25);   // that gets updated
  icc_write (u16, 34, 23);   // when the generator
  icc_write (u16, 34, 23);   // when the generator

  icc_write (sign, 36, "acsp"); // changes


  {
    int headpos = 0;
    int tags;
    int o, no;
    int p = 0;
    int psize = 0;

    tags = 10;
    no = o = 128 + 4 + 12 * tags;

    icc_write (u32,  128, tags);
#define ALLOC(tag, size) \
    no+=((4-o)%4);o = no;psize = size;\
    icc_write (sign, 128 + 4 + 4 * headpos++, tag);\
    icc_write (u32,  128 + 4 + 4 * headpos++, o);\
    icc_write (u32,  128 + 4 + 4 * headpos++, size);\
    p = no;\
    no+=size;
#define REALLOC(tag) \
    icc_write (sign, 128 + 4 + 4 * headpos++, tag);\
    icc_write (u32,  128 + 4 + 4 * headpos++, p); \
    icc_write (u32,  128 + 4 + 4 * headpos++, psize);

    ALLOC("wtpt", 20);
    icc_write (sign,o, "XYZ ");
    icc_write (u32, o + 4, 0);
    icc_write (s15f16, o + 8, space->whitepoint[0]);
    icc_write (s15f16, o + 12, space->whitepoint[1]);
    icc_write (s15f16, o + 16, space->whitepoint[2]);

    ALLOC("rXYZ", 20);
    icc_write (sign,o, "XYZ ");
    icc_write (u32, o + 4, 0);
    icc_write (s15f16, o + 8,  space->RGBtoXYZ[0]);
    icc_write (s15f16, o + 12, space->RGBtoXYZ[3]);
    icc_write (s15f16, o + 16, space->RGBtoXYZ[6]);

    ALLOC("gXYZ", 20);
    icc_write (sign,o, "XYZ ");
    icc_write (u32, o + 4, 0);
    icc_write (s15f16, o + 8,  space->RGBtoXYZ[1]);
    icc_write (s15f16, o + 12, space->RGBtoXYZ[4]);
    icc_write (s15f16, o + 16, space->RGBtoXYZ[7]);

    ALLOC("bXYZ", 20);
    icc_write (sign,o, "XYZ ");
    icc_write (u32, o + 4, 0);
    icc_write (s15f16, o + 8,  space->RGBtoXYZ[2]);
    icc_write (s15f16, o + 12, space->RGBtoXYZ[5]);
    icc_write (s15f16, o + 16, space->RGBtoXYZ[8]);

    ALLOC("rTRC", 14);
    icc_write (sign,o, "curv");
    icc_write (u32, o + 4, 0);
    icc_write (u32, o + 8, 1);
    icc_write (u16, o + 12, 334);

    if (space->trc[0] == space->trc[1] &&
        space->trc[0] == space->trc[2])
    {
      REALLOC("gTRC");
      REALLOC("bTRC");
    }
    else
    {
      ALLOC("gTRC", 14);
      icc_write (sign,o, "curv");
      icc_write (u32, o + 4, 0);
      icc_write (u32, o + 8, 1); /* forcing a linear curve */
      ALLOC("bTRC", 14);
      icc_write (sign,o, "curv");
      icc_write (u32, o + 4, 0);
      icc_write (u32, o + 8, 1); /* forcing a linear curve */
    }

    {
      char str[128];
      int i;
      sprintf (str, "babl");
      ALLOC("desc", 30 + strlen (str) + 1);
      icc_write (sign,o,"desc");
      icc_write (u32, o + 4, 0);
      icc_write (u32, o + 8, strlen(str));
      for (i = 0; str[i]; i++)
        icc_write (u8, o + 12 + i, str[i]);
      icc_write (u8, o + 12 + i, 0);

      REALLOC("dmnd");
    }

    {
      char str[128];
      int i;
      sprintf (str, "CC0/public domain");
      ALLOC("cprt", 8 + strlen (str) + 1);
      icc_write (sign,o, "text");
      icc_write (u32, o + 4, 0);
      for (i = 0; str[i]; i++)
        icc_write (u8, o + 8 + i, str[i]);
    }

    icc_write (u32, 0, no + 3);
    length = no + 3;
  }

  if (ret_length)
    *ret_length = length;
  return icc;
}

const Babl *
babl_space_rgb_icc (const char *icc,
                    int         length,
                    char      **error)
{
  int  profile_size     = icc_read (u32, 0);
  int  icc_ver_major    = icc_read (u8, 8);
  const Babl *trc_red   = NULL;
  const Babl *trc_green = NULL;
  const Babl *trc_blue  = NULL;
  sign_t profile_class, color_space;

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
  profile_class = icc_read (sign, 12);
  if (strcmp (profile_class.str, "mntr"))
  {
    *error = "not a monitor-class profile";
    return NULL;
  }
  color_space = icc_read (sign, 16);
  if (strcmp (color_space.str, "RGB "))
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
     channels   = icc_read (u16, offset + 8);
     phosporant = icc_read (u16, offset + 10);

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

     red_x   = icc_read (s15f16, offset + 12);
     red_y   = icc_read (s15f16, offset + 12 + 4);
     green_x = icc_read (s15f16, offset + 20);
     green_y = icc_read (s15f16, offset + 20 + 4);
     blue_x  = icc_read (s15f16, offset + 28);
     blue_y  = icc_read (s15f16, offset + 28 + 4);

     icc_tag (icc, length, "wtpt", &offset, &element_size);
     {
       double wX = icc_read (s15f16, offset + 8);
       double wY = icc_read (s15f16, offset + 8 + 4);
       double wZ = icc_read (s15f16, offset + 8 + 4 * 2);

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
           icc_tag (icc, length, "bXYZ", NULL, NULL) &&
           icc_tag (icc, length, "wtpt", NULL, NULL))
  {
     int offset, element_size;
     double rx, gx, bx;
     double ry, gy, by;
     double rz, gz, bz;

     double wX, wY, wZ;

     icc_tag (icc, length, "rXYZ", &offset, &element_size);
     rx = icc_read (s15f16, offset + 8 + 4 * 0);
     ry = icc_read (s15f16, offset + 8 + 4 * 1);
     rz = icc_read (s15f16, offset + 8 + 4 * 2);
     icc_tag (icc, length, "gXYZ", &offset, &element_size);
     gx = icc_read (s15f16, offset + 8 + 4 * 0);
     gy = icc_read (s15f16, offset + 8 + 4 * 1);
     gz = icc_read (s15f16, offset + 8 + 4 * 2);
     icc_tag (icc, length, "bXYZ", &offset, &element_size);
     bx = icc_read (s15f16, offset + 8 + 4 * 0);
     by = icc_read (s15f16, offset + 8 + 4 * 1);
     bz = icc_read (s15f16, offset + 8 + 4 * 2);
     icc_tag (icc, length, "wtpt", &offset, &element_size);
     wX = icc_read (s15f16, offset + 8);
     wY = icc_read (s15f16, offset + 8 + 4);
     wZ = icc_read (s15f16, offset + 8 + 4 * 2);

     return babl_space_rgb_matrix (NULL,
                wX, wY, wZ,
                rx, gx, bx,
                ry, gy, by,
                rz, gz, bz,
                trc_red, trc_green, trc_blue);
  }

  *error = "didnt find RGB primaries";
  return NULL;
}
