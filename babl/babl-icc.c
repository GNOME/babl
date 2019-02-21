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
 * <https://www.gnu.org/licenses/>.
 */

#include "../config.h"
#include "babl-internal.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct ICC {
  char *data;
  int   length;

  int   tags;
  int   headpos;
  int   o, no;
  int   p;
  int   psize;
} ICC;

ICC *
icc_state_new (char *data, 
               int   length, 
               int   tags);

ICC *
icc_state_new (char *data, 
               int   length, 
               int   tags)
{
  ICC *ret = babl_calloc (sizeof (ICC), 1);
  ret->data = data;
  ret->length = length;
  ret->tags = tags;

  return ret;
}

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

#define icc_write(type, offset, value)  write_##type(state,offset,value)
#define icc_read(type, offset)          read_##type(state,offset)

static void 
write_u8 (ICC    *state, 
          int     offset, 
          uint8_t value)
{
  if (offset < 0 || offset >= state->length)
    return;
  *(uint8_t*) (&state->data[offset]) = value;
}

static void 
write_s8 (ICC   *state, 
          int    offset, 
          int8_t value)
{
  if (offset < 0 || offset >= state->length)
    return;
  *(int8_t*) (&state->data[offset]) = value;
}

static int 
read_u8 (ICC *state, 
         int  offset)
{
/* all reading functions take both the char *pointer and the length of the
 * buffer, and all reads thus gets protected by this condition.
 */
  if (offset < 0 || offset > state->length)
    return 0;

  return *(uint8_t*) (&state->data[offset]);
}

static int 
read_s8 (ICC *state, 
         int  offset)
{
  if (offset < 0 || offset > state->length)
    return 0;

  return *(int8_t*) (&state->data[offset]);
}

static void 
write_s16 (ICC    *state, 
           int     offset, 
           int16_t value)
{
  write_s8 (state, offset + 0, value >> 8);
  write_u8 (state, offset + 1, value & 0xff);
}

static int16_t 
read_s16 (ICC *state, 
          int  offset)
{
  return icc_read (u8, offset + 1) +
         (read_s8 (state, offset + 0) << 8); //XXX: transform to icc_read macro
}

static 
uint16_t read_u16 (ICC *state, 
                   int  offset)
{
  return icc_read (u8, offset + 1) +
         (icc_read (u8, offset + 0) << 8);
}

static void 
write_u16 (ICC     *state, 
           int      offset, 
           uint16_t value)
{
  write_u8 (state, offset + 0, value >> 8);
  write_u8 (state, offset + 1, value & 0xff);
}

static u8f8_t 
read_u8f8_ (ICC *state, 
            int  offset)
{
  u8f8_t ret ={icc_read (u8, offset),
               icc_read (u8, offset + 1)};
  return ret;
}

static s15f16_t 
read_s15f16_ (ICC *state, 
              int  offset)
{
  s15f16_t ret ={icc_read (s16, offset),
                 icc_read (u16, offset + 2)};
  return ret;
}

static void 
write_u8f8_ (ICC   *state, 
             int    offset, 
             u8f8_t val)
{
  icc_write (u8, offset,     val.integer),
  icc_write (u8, offset + 1, val.fraction);
}

static void 
write_s15f16_ (ICC     *state, 
               int      offset, 
               s15f16_t val)
{
  icc_write (s16, offset, val.integer),
  icc_write (u16, offset + 2, val.fraction);
}

static s15f16_t 
d_to_s15f16 (double value)
{
  s15f16_t ret;
  ret.integer = floor (value);
  ret.fraction = fmod(value, 1.0) * 65536.0;
  return ret;
}

static u8f8_t 
d_to_u8f8 (double value)
{
  u8f8_t ret;
  ret.integer = floor (value);
  ret.fraction = fmod(value, 1.0) * 256.0;
  return ret;
}

static double 
s15f16_to_d (s15f16_t fix)
{
  return fix.integer + fix.fraction / 65536.0;
}

static double 
u8f8_to_d (u8f8_t fix)
{
  return fix.integer + fix.fraction / 256.0;
}

static void 
write_s15f16 (ICC   *state,
              int    offset,
              double value)
{
   write_s15f16_ (state, offset, d_to_s15f16 (value));
}

static void 
write_u8f8 (ICC   *state,
            int    offset,
            double value)
{
  write_u8f8_ (state, offset, d_to_u8f8 (value));
}


static double 
read_s15f16 (ICC *state, 
             int  offset)
{
  return s15f16_to_d (read_s15f16_ (state, offset));
}

static double 
read_u8f8 (ICC *state, 
           int  offset)
{
  return u8f8_to_d (read_u8f8_ (state, offset));
}

#if 0
static inline void 
print_u8f8 (u8f8_t fix)
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

static inline void 
print_s15f16 (s15f16_t fix)
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
#endif

static void 
write_u32 (ICC     *state, 
           int      offset, 
           uint32_t value)
{
  int i;
  for (i = 0; i < 4; i ++)
  {
    write_u8 (state, offset + i,
                  (value & 0xff000000) >> 24
                  );
    value <<= 8;
  }
}

static uint32_t 
read_u32 (ICC *state, 
          int  offset)
{
  return icc_read (u8, offset + 3) +
         (icc_read (u8, offset + 2) << 8) +
         (icc_read (u8, offset + 1) << 16) +
         (icc_read (u8, offset + 0) << 24);
}

static sign_t 
read_sign (ICC *state, 
           int  offset)
{
  sign_t ret;
  ret.str[0]=icc_read (u8, offset);
  ret.str[1]=icc_read (u8, offset + 1);
  ret.str[2]=icc_read (u8, offset + 2);
  ret.str[3]=icc_read (u8, offset + 3);
  ret.str[4]=0;
  return ret;
}

static void 
write_sign (ICC        *state, 
            int         offset, 
            const char *sign)
{
  int i;
  for (i = 0; i < 4; i ++)
    icc_write (u8, offset + i, sign[i]);
}

/* looks up offset and length for a specific icc tag
 */
static int 
icc_tag (ICC        *state,
         const char *tag, 
         int        *offset, 
         int        *el_length)
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

static const Babl *
babl_trc_from_icc (ICC         *state, 
                   int          offset,                                      
                   const char **error)
{
  {
    int count = icc_read (u32, offset + 8);
    int i;
    if (!strcmp (state->data + offset, "para"))
    {
         int function_type = icc_read (u16, offset + 8);
         float g;
         switch (function_type)
         {
            case 0:
              g = icc_read (s15f16, offset + 12 + 4 * 0);
              return babl_trc_gamma (g);
              break;
            case 3:
              {
                float a,b,c,d;
                g = icc_read (s15f16, offset + 12 + 4 * 0);
                a = icc_read (s15f16, offset + 12 + 4 * 1);
                b = icc_read (s15f16, offset + 12 + 4 * 2);
                c = icc_read (s15f16, offset + 12 + 4 * 3);
                d = icc_read (s15f16, offset + 12 + 4 * 4);
                //fprintf (stderr, "%f %f %f %f %f\n", g, a, b, c, d);
                return babl_trc_formula_srgb (g, a, b, c, d);
              }
              break;
            case 4:
              {
                float a,b,c,d,e,f;
                g = icc_read (s15f16, offset + 12 + 4 * 0);
                a = icc_read (s15f16, offset + 12 + 4 * 1);
                b = icc_read (s15f16, offset + 12 + 4 * 2);
                c = icc_read (s15f16, offset + 12 + 4 * 3);
                d = icc_read (s15f16, offset + 12 + 4 * 4);
                e = icc_read (s15f16, offset + 12 + 4 * 5);
                f = icc_read (s15f16, offset + 12 + 4 * 6);
                fprintf (stderr, "%f %f %f %f %f %f %f\n",
                              g, a, b, c, d, e, f);
            {
              fprintf (stderr, "unhandled parametric sRGB formula TRC type %i\n", function_type);
              *error = "unhandled sRGB formula like TRC";
              return babl_trc_gamma (2.2);
            }
                              }
              break;
            default:
              *error = "unhandled parametric TRC";
              fprintf (stderr, "unhandled parametric TRC type %i\n", function_type);
              return babl_trc_gamma (2.2);
            break;
         }
    }
    else
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
        const Babl *ret;
        float *lut;

        lut = babl_malloc (sizeof (float) * count);

        for (i = 0; i < count; i ++)
        {
          lut[i] = icc_read (u16, offset + 12 + i * 2) / 65535.0;
        }

        ret = babl_trc_lut_find (lut, count);
        if (ret)
          return ret;

        ret = babl_trc_lut (NULL, count, lut);
        babl_free (lut);
        return ret;
      }
    }
  }
  return NULL;
}

static void 
icc_allocate_tag (ICC        *state, 
                  const char *tag, 
                  int         size)
{
    while (state->no % 4 != 0)
      state->no++;

    state->o = state->no;state->psize = size;
    icc_write (sign, 128 + 4 + 4 * state->headpos++, tag);
    icc_write (u32,  128 + 4 + 4 * state->headpos++, state->o);
    icc_write (u32,  128 + 4 + 4 * state->headpos++, size);
    state->p = state->no;\
    state->no+=size;
}

static void 
icc_duplicate_tag(ICC        *state, 
                  const char *tag)
{
    icc_write (sign, 128 + 4 + 4 * state->headpos++, tag);
    icc_write (u32,  128 + 4 + 4 * state->headpos++, state->p);
    icc_write (u32,  128 + 4 + 4 * state->headpos++, state->psize);
}

/* brute force optimized 26 entry sRGB LUT */
static const uint16_t lut_srgb_26[]={0,202,455,864,1423,2154,3060,4156,5454,6960,8689,10637,12821,15247,17920,20855,24042,27501,31233,35247,39549,44132,49018,54208,59695,65535};


void write_trc (ICC           *state,
                const char    *name,
                const BablTRC *trc,
                BablICCFlags   flags);
                
void write_trc (ICC           *state,
                const char    *name,
                const BablTRC *trc,
                BablICCFlags   flags)
{
switch (trc->type)
{
  case BABL_TRC_LINEAR:
    icc_allocate_tag (state, name, 13);
    icc_write (sign, state->o, "curv");
    icc_write (u32, state->o + 4, 0);
    icc_write (u32, state->o + 8, 0);
    break;
  case BABL_TRC_FORMULA_GAMMA:
    icc_allocate_tag (state, name, 14);
    icc_write (sign, state->o, "curv");
    icc_write (u32, state->o + 4, 0);
    icc_write (u32, state->o + 8, 1);
    icc_write (u8f8, state->o + 12, trc->gamma);
    break;
  case BABL_TRC_LUT:
    icc_allocate_tag (state, name, 12 + trc->lut_size * 2);
    icc_write (sign, state->o, "curv");
    icc_write (u32, state->o + 4, 0);
    icc_write (u32, state->o + 8, trc->lut_size);
    {
      int j;
      for (j = 0; j < trc->lut_size; j ++)
        icc_write (u16, state->o + 12 + j * 2, (int)(trc->lut[j]*65535.5f));
    }
    break;
  // this is the case catching things not directly representable in v2
  case BABL_TRC_SRGB:
    if (flags == BABL_ICC_COMPACT_TRC_LUT)
    {
      int lut_size = 26;
      icc_allocate_tag (state, name, 12 + lut_size * 2);
      icc_write (sign, state->o, "curv");
      icc_write (u32, state->o + 4, 0);
      icc_write (u32, state->o + 8, lut_size);
      {
        int j;
        for (j = 0; j < lut_size; j ++)
        icc_write (u16, state->o + 12 + j * 2, lut_srgb_26[j]);
      }
      break;
    }
  case BABL_TRC_FORMULA_SRGB:
    {
      int lut_size = 512;
      if (flags == BABL_ICC_COMPACT_TRC_LUT)
        lut_size = 128;

      icc_allocate_tag (state, name, 12 + lut_size * 2);
      icc_write (sign, state->o, "curv");
      icc_write (u32, state->o + 4, 0);
      icc_write (u32, state->o + 8, lut_size);
      {
        int j;
        for (j = 0; j < lut_size; j ++)
        icc_write (u16, state->o + 12 + j * 2,
            babl_trc_to_linear ((void*)trc, j / (lut_size-1.0)) * 65535.5);
      }
    }
}
}

static void 
symmetry_test (ICC *state);

char *
babl_space_to_icc (const Babl  *babl,
                         const char  *description,
                         const char  *copyright,
                         BablICCFlags flags,
                         int         *ret_length)
{
  const BablSpace *space = &babl->space;
  char icc[65536];
  int length=65535;
  ICC *state = icc_state_new (icc, length, 10);

  icc[length]=0;

  symmetry_test (state);

  icc_write (sign, 4, "babl");  // ICC verison
  icc_write (u8, 8, 2);         // ICC verison
  icc_write (u8, 9, 0x20);      // 2.2 for now..
  icc_write (u32,64, 0);        // rendering intent

  icc_write (s15f16,68, 0.96421); // Illuminant
  icc_write (s15f16,72, 1.0);
  icc_write (s15f16,76, 0.82491);

  icc_write (sign, 80, "babl"); // creator

  icc_write (sign, 12, "mntr");
  icc_write (sign, 16, "RGB ");
  icc_write (sign, 20, "XYZ ");

  icc_write (u16, 24, 2222);  // babl profiles
  icc_write (u16, 26, 11);    // should
  icc_write (u16, 28, 11);    // use a fixed
  icc_write (u16, 30,  3);    // date
  icc_write (u16, 32, 44);    // that gets updated
  icc_write (u16, 34, 55);    // when the generator changes

  icc_write (sign, 36, "acsp"); // changes

  {
    state->tags = 9; /* note: we could reserve a couple of spots and
                        still use a very simple allocator and
                        still be valid - albeit with tiny waste of
                        space.
                */
    state->no = state->o = 128 + 4 + 12 * state->tags;

    icc_write (u32,  128, state->tags);

    icc_allocate_tag (state, "wtpt", 20);
    icc_write (sign, state->o, "XYZ ");
    icc_write (u32,  state->o + 4, 0);
    icc_write (s15f16, state->o + 8,  space->whitepoint[0]);
    icc_write (s15f16, state->o + 12, space->whitepoint[1]);
    icc_write (s15f16, state->o + 16, space->whitepoint[2]);

    icc_allocate_tag (state, "rXYZ", 20);
    icc_write (sign, state->o, "XYZ ");
    icc_write (u32,  state->o + 4, 0);
    icc_write (s15f16, state->o + 8,  space->RGBtoXYZ[0]);
    icc_write (s15f16, state->o + 12, space->RGBtoXYZ[3]);
    icc_write (s15f16, state->o + 16, space->RGBtoXYZ[6]);

    icc_allocate_tag (state, "gXYZ", 20);
    icc_write (sign, state->o, "XYZ ");
    icc_write (u32, state->o + 4, 0);
    icc_write (s15f16, state->o + 8,  space->RGBtoXYZ[1]);
    icc_write (s15f16, state->o + 12, space->RGBtoXYZ[4]);
    icc_write (s15f16, state->o + 16, space->RGBtoXYZ[7]);

    icc_allocate_tag (state, "bXYZ", 20);
    icc_write (sign, state->o, "XYZ ");
    icc_write (u32, state->o + 4, 0);
    icc_write (s15f16, state->o + 8,  space->RGBtoXYZ[2]);
    icc_write (s15f16, state->o + 12, space->RGBtoXYZ[5]);
    icc_write (s15f16, state->o + 16, space->RGBtoXYZ[8]);

    write_trc (state, "rTRC", &space->trc[0]->trc, flags);

    if (space->trc[0] == space->trc[1] &&
        space->trc[0] == space->trc[2])
    {
      icc_duplicate_tag (state, "gTRC");
      icc_duplicate_tag (state, "bTRC");
    }
    else
    {
      write_trc (state, "gTRC", &space->trc[1]->trc, flags);
      write_trc (state, "bTRC", &space->trc[2]->trc, flags);
    }

    {
      char str[128]="CC0/public domain";
      int i;
      if (!copyright) copyright = str;
      icc_allocate_tag(state, "cprt", 8 + strlen (copyright) + 1);
      icc_write (sign, state->o, "text");
      icc_write (u32, state->o + 4, 0);
      for (i = 0; copyright[i]; i++)
        icc_write (u8, state->o + 8 + i, copyright[i]);
    }
    {
      char str[128]="babl";
      int i;
      if (!description) description = str;
      icc_allocate_tag(state, "desc", 90 + strlen (description) + 0);
      icc_write (sign, state->o,"desc");
      icc_write (u32, state->o + 4, 0);
      icc_write (u32, state->o + 8, strlen(description) + 1);
      for (i = 0; description[i]; i++)
        icc_write (u8, state->o + 12 + i, description[i]);
    }


    icc_write (u32, 0, state->no + 0);
    length = state->no + 0;
  }

  if (ret_length)
    *ret_length = length;

  babl_free (state);
  {
    char *ret = malloc (length);
    memcpy (ret, icc, length);
    return ret;
  }
}

const char *
babl_space_get_icc (const Babl *babl, 
                    int        *length)
{
  if (!babl->space.icc_profile)
  {
    /* overriding constness of babl */
    Babl *babl_noconst = (void*) babl;
    babl_noconst->space.icc_profile = babl_space_to_icc (babl,
                              "babl profile", NULL, 0,
                              &babl_noconst->space.icc_length);
  }
  if (length) *length = babl->space.icc_length;
  return babl->space.icc_profile;
}


typedef uint32_t UTF32;
typedef uint16_t UTF16;
typedef uint8_t  UTF8;

typedef enum {
  strictConversion = 0,
  lenientConversion
} ConversionFlags;

static int 
ConvertUTF16toUTF8 (const UTF16   **sourceStart,
                    const UTF16    *sourceEnd,
                    UTF8          **targetStart,
                    UTF8           *targetEnd,
                    ConversionFlags flags);

static char *
icc_decode_mluc (ICC        *state,
                 int         offset,
                 int         element_length,
                 const char *lang,
                 const char *country)
{
  int n_records   = icc_read (u32, offset + 8);
  int record_size = icc_read (u32, offset + 12);
  int i;
  int o = 16;
  for (i = 0; i < n_records; i++)
  {
    char icountry[3]="  ";
    char ilang[3]="  ";

    ilang[0]    = icc_read(u8, offset + o + 0);
    ilang[1]    = icc_read(u8, offset + o + 1);
    icountry[0] = icc_read(u8, offset + o + 2);
    icountry[1] = icc_read(u8, offset + o + 3);

    if (((!lang || !strcmp (lang, ilang)) &&
         (!country || !strcmp (country, icountry))) ||
         (i == n_records - 1))
    {
      int slength = (icc_read(u32, offset + o + 4))/2;
      int soffset = icc_read(u32, offset + o + 8);
      UTF16 *tmp_ret = babl_calloc (sizeof (uint16_t), slength + 1);
      UTF16 *tmp_ret2 = tmp_ret;
      unsigned char *ret = babl_calloc (1, slength * 4 + 1); // worst case scenario
      unsigned char *ret2 = ret;
      int j;

      for (j = 0; j < slength; j++)
      {
        tmp_ret[j] = icc_read(u16, offset + soffset + j * 2);
      }
      tmp_ret[j] = 0;
      memset (ret, 0, slength * 4 + 1);
      ConvertUTF16toUTF8 ((void*)&tmp_ret2, tmp_ret + slength, &ret2, ret + slength, lenientConversion);
      babl_free(tmp_ret);
      { // trim down to actually used utf8
        unsigned char *tmp = (void*)strdup ((void*)ret);
        babl_free (ret);
        ret = tmp;
      }
      return (void*)ret;
    }
    o+=record_size;
  }
  return NULL;
}

static char *
decode_string (ICC        *state, 
               const char *tag, 
               const char *lang, 
               const char *country)
{
  int offset, element_size;

  if (!icc_tag (state, tag, &offset, &element_size))
    return NULL;

  if (!strcmp (state->data + offset, "mluc"))
  {
    return icc_decode_mluc (state, offset, element_size, lang, country);
  }
  else if (!strcmp (state->data + offset, "text"))
  {
    return strdup (state->data + offset + 8);
  }
  else if (!strcmp (state->data + offset, "desc"))
  {
    return strdup (state->data + offset + 12);
  }
  return NULL;
}

#ifdef HAVE_LCMS
static cmsHPROFILE sRGBProfile = 0;
#endif

const Babl *
babl_space_from_icc (const char   *icc_data,
                     int           icc_length,
                     BablIccIntent intent,
                     const char  **error)
{
  ICC  *state = icc_state_new ((char*)icc_data, icc_length, 0);
  int   profile_size    = icc_read (u32, 0);
  //int   icc_ver_major   = icc_read (u8, 8);
  const Babl *trc_red   = NULL;
  const Babl *trc_green = NULL;
  const Babl *trc_blue  = NULL;
  const char *int_err;
  Babl *ret = NULL;
  int speed_over_accuracy = intent & BABL_ICC_INTENT_PERFORMANCE;

  sign_t profile_class, color_space, pcs;

  if (!error) error = &int_err;
  *error = NULL;

  if (profile_size != icc_length)
  {
    *error = "icc profile length inconsistency";
  }
  else
  {
    profile_class = icc_read (sign, 12);
    color_space = icc_read (sign, 16);

    if (!strcmp (color_space.str, "CMYK"))
    {
       ret = _babl_space_for_lcms (icc_data, icc_length);
       if (ret->space.cmyk.is_cmyk)
         return ret;
       ret->space.cmyk.is_cmyk = 1;
       ret->space.icc_length = icc_length;
       ret->space.icc_profile = malloc (icc_length);
       memcpy (ret->space.icc_profile, icc_data, icc_length);

#ifdef HAVE_LCMS
       if (sRGBProfile == 0)
       {
         const Babl *rgb = babl_space("scRGB"); /* should use a forced linear profile */
         sRGBProfile = cmsOpenProfileFromMem(rgb->space.icc_profile, rgb->space.icc_length);
       }

       ret->space.cmyk.lcms_profile = cmsOpenProfileFromMem(ret->space.icc_profile, ret->space.icc_length);

/* these are not defined by lcms2.h we hope that following the existing pattern of pixel-format definitions work */
#ifndef TYPE_CMYKA_DBL
#define TYPE_CMYKA_DBL      (FLOAT_SH(1)|COLORSPACE_SH(PT_CMYK)|EXTRA_SH(1)|CHANNELS_SH(4)|BYTES_SH(0))
#endif
#ifndef TYPE_RGBA_DBL
#define TYPE_RGBA_DBL      (FLOAT_SH(1)|COLORSPACE_SH(PT_RGB)|EXTRA_SH(1)|CHANNELS_SH(3)|BYTES_SH(0))
#endif

       ret->space.cmyk.lcms_to_rgba = cmsCreateTransform(ret->space.cmyk.lcms_profile, TYPE_CMYKA_DBL,
                                                    sRGBProfile, TYPE_RGBA_DBL,
                                                    INTENT_RELATIVE_COLORIMETRIC, cmsFLAGS_BLACKPOINTCOMPENSATION);
// INTENT_PERCEPTUAL,0);//intent & 7, 0);
       ret->space.cmyk.lcms_from_rgba = cmsCreateTransform(sRGBProfile, TYPE_RGBA_DBL,
                                                      ret->space.cmyk.lcms_profile, TYPE_CMYKA_DBL,
                                                    INTENT_RELATIVE_COLORIMETRIC, cmsFLAGS_BLACKPOINTCOMPENSATION);
                                                    //  INTENT_PERCEPTUAL,0);//intent & 7, 0);
       cmsCloseProfile (ret->space.cmyk.lcms_profile); // XXX keep it open in case of CMYK to CMYK transforms needed?
#endif
       return ret;
    }

    if (strcmp (color_space.str, "RGB "))
      *error = "not defining an RGB space";
    else
     {
       if (strcmp (profile_class.str, "mntr"))
         *error = "not a monitor-class profile";
     }
  }

  if (!*error)
  {
    pcs = icc_read (sign, 20);
    if (strcmp (pcs.str, "XYZ "))
      *error = "PCS is not XYZ";
  }

  if (!*error)
  switch (intent & 7) /* enum of intent is in lowest bits */
  {
    case BABL_ICC_INTENT_RELATIVE_COLORIMETRIC:
      /* that is what we do well */

      if (!speed_over_accuracy)
      {
        if (icc_tag (state, "A2B0", NULL, NULL) &&
            icc_tag (state, "B2A0", NULL, NULL))
        {
          *error = "use lcms, accuracy desired and cluts are present";
        }
      }

      break;
    case BABL_ICC_INTENT_PERCEPTUAL:
      /* if there is an A2B0 and B2A0 tags, we do not do what that
       * profile is capable of - since the CLUT code is work in progress
       * not in git master yet.
       */
      if (icc_tag (state, "A2B0", NULL, NULL) &&
          icc_tag (state, "B2A0", NULL, NULL))
      {
        *error = "profile contains perceptual luts and perceptual was explicitly asked for, babl does not yet support CLUTs";
      }
      else
      {
        intent = BABL_ICC_INTENT_RELATIVE_COLORIMETRIC;
      }
      break;
    case BABL_ICC_INTENT_ABSOLUTE_COLORIMETRIC:
      *error = "absolute colormetric not implemented";
      break;
    case BABL_ICC_INTENT_SATURATION:
      *error = "absolute stauration not supported";
      break;
  }

  {
     int offset, element_size;
     if (!*error && icc_tag (state, "rTRC", &offset, &element_size))
     {
       trc_red = babl_trc_from_icc (state, offset, error);
     }
     if (!*error && icc_tag (state, "gTRC", &offset, &element_size))
     {
       trc_green = babl_trc_from_icc (state, offset, error);
     }
     if (!*error && icc_tag (state, "bTRC", &offset, &element_size))
     {
       trc_blue = babl_trc_from_icc (state, offset, error);
     }
  }

  if (!*error && (!trc_red || !trc_green || !trc_blue))
  {
     *error = "missing TRCs";
  }

  if (*error)
  {

    babl_free (state);
    return NULL;
  }

  if (icc_tag (state, "rXYZ", NULL, NULL) &&
      icc_tag (state, "gXYZ", NULL, NULL) &&
      icc_tag (state, "bXYZ", NULL, NULL) &&
      icc_tag (state, "wtpt", NULL, NULL))
  {
     int offset, element_size;
     double rx, gx, bx;
     double ry, gy, by;
     double rz, gz, bz;

     double wX, wY, wZ;

     icc_tag (state, "rXYZ", &offset, &element_size);
     rx = icc_read (s15f16, offset + 8 + 4 * 0);
     ry = icc_read (s15f16, offset + 8 + 4 * 1);
     rz = icc_read (s15f16, offset + 8 + 4 * 2);
     icc_tag (state, "gXYZ", &offset, &element_size);
     gx = icc_read (s15f16, offset + 8 + 4 * 0);
     gy = icc_read (s15f16, offset + 8 + 4 * 1);
     gz = icc_read (s15f16, offset + 8 + 4 * 2);
     icc_tag (state, "bXYZ", &offset, &element_size);
     bx = icc_read (s15f16, offset + 8 + 4 * 0);
     by = icc_read (s15f16, offset + 8 + 4 * 1);
     bz = icc_read (s15f16, offset + 8 + 4 * 2);
     icc_tag (state, "wtpt", &offset, &element_size);
     wX = icc_read (s15f16, offset + 8);
     wY = icc_read (s15f16, offset + 8 + 4);
     wZ = icc_read (s15f16, offset + 8 + 4 * 2);

      /* detect inconsistent Argyll cLUT + matrix profiles */
      if (icc_tag (state, "A2B0", NULL, NULL) ||
          icc_tag (state, "B2A0", NULL, NULL))
      {
        if (rz > rx)
        {
           *error = "Inconsistent ICC profile detected, profile contains both cLUTs and a matrix with swapped primaries, this likely means it is an intentionally inconsistent Argyll profile is in use; this profile is only capable of high accuracy rendering and does not permit acceleration for interactive previews.";
           fprintf (stderr, "babl ICC warning: %s\n", *error);
           babl_free (state);
           return NULL;
        }
      }

     ret = (void*)babl_space_match_trc_matrix (trc_red, trc_green, trc_blue,
                                        rx, ry, rz, gx, gy, gz, bx, by, bz);
     if (ret)
     {
        babl_free (state);
        return ret;
     }

     {
       ret  = (void*)babl_space_from_rgbxyz_matrix (NULL,
                wX, wY, wZ,
                rx, gx, bx,
                ry, gy, by,
                rz, gz, bz,
                trc_red, trc_green, trc_blue);

       babl_free (state);
       ret->space.icc_length = icc_length;
       ret->space.icc_profile = malloc (icc_length);
       memcpy (ret->space.icc_profile, icc_data, icc_length);
       return ret;
     }
  }
  else if (icc_tag (state, "chrm", NULL, NULL) &&
           icc_tag (state, "wtpt", NULL, NULL))
  {
     int offset, element_size;
     double red_x, red_y, green_x, green_y, blue_x, blue_y;
     int channels, phosporant;

     icc_tag (state, "chrm", &offset, &element_size);
     channels   = icc_read (u16, offset + 8);
     phosporant = icc_read (u16, offset + 10);

     if (phosporant != 0)
     {
       *error = "unhandled phosporants, please report bug against babl with profile";
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

     icc_tag (state, "wtpt", &offset, &element_size);
     {
       double wX = icc_read (s15f16, offset + 8);
       double wY = icc_read (s15f16, offset + 8 + 4);
       double wZ = icc_read (s15f16, offset + 8 + 4 * 2);
       babl_free (state);

       ret = (void*) babl_space_from_chromaticities (NULL,
                     wX / (wX + wY + wZ),
                     wY / (wX + wY + wZ),
                     red_x, red_y,
                     green_x, green_y,
                     blue_x, blue_y,
                     trc_red, trc_green, trc_blue, 1);

       ret->space.icc_length = icc_length;
       ret->space.icc_profile = malloc (icc_length);
       memcpy (ret->space.icc_profile, icc_data, icc_length);

       return ret;
     }
  }

  *error = "didnt find RGB primaries";
  babl_free (state);
  return NULL;
}

/* NOTE: GIMP-2.10.0-4 releases depends on this symbol */
const Babl *
babl_icc_make_space (const char   *icc_data,
                     int           icc_length,
                     BablIccIntent intent,
                     const char  **error)
{
  return babl_space_from_icc (icc_data, icc_length, intent, error);
}

static void symmetry_test (ICC *state)
{
  icc_write (s8, 8,-2);
  assert (icc_read (s8, 8) == -2);
  icc_write (s8, 8, 3);
  assert (icc_read (s8, 8) == 3);

  icc_write (u8, 8, 2);
  assert (icc_read (u8, 8) == 2);

  icc_write (u16, 8, 3);
  assert (icc_read (u16, 8) == 3);

  icc_write (s16, 8, -3);
  assert (icc_read (s16, 8) == -3);

  icc_write (s16, 8, 9);
  assert (icc_read (s16, 8) == 9);

  icc_write (u32, 8, 4);
  assert (icc_read (u32, 8) == 4);
}

char *
babl_icc_get_key (const char *icc_data,
                  int         icc_length,
                  const char *key,
                  const char *language,
                  const char *country)
{
  char *ret = NULL;
  ICC *state = icc_state_new ((void*)icc_data, icc_length, 0);

  if (!state)
    return ret;

  if (!strcmp (key, "copyright") ||
      !strcmp (key, "cprt"))
  {
    ret = decode_string (state, "cprt", language, country);

  } else if (!strcmp (key, "description") ||
             !strcmp (key, "profileDescriptionTag") ||
             !strcmp (key, "desc"))
  {
    ret = decode_string (state, "desc", language, country);

  } else if (!strcmp (key, "manufacturer") ||
             !strcmp (key, "deviceMfgDescTag") ||
             !strcmp (key, "dmnd"))
  {
    ret = decode_string (state, "dmnd", language, country);

  } else if (!strcmp (key, "device") ||
             !strcmp (key, "deviceModelDescTag") ||
             !strcmp (key, "dmdd"))
  {
    ret = decode_string (state, "dmdd", language, country);
  } else if (!strcmp (key, "class") ||
             !strcmp (key, "profile-class"))
  {
    sign_t tag = icc_read (sign, 12);
    return strdup (tag.str);
  } else if (!strcmp (key, "color-space"))
  {
    sign_t tag = icc_read (sign, 16);
    return strdup (tag.str);
  } else if (!strcmp (key, "pcs"))
  {
    sign_t tag = icc_read (sign, 20);
    return strdup (tag.str);
  } else if (!strcmp (key, "intent"))
  {
    char tag[5];
    int val = icc_read (u32, 64);
    snprintf (tag, sizeof (tag), "%i", val);
    return strdup (tag);
  } else if (!strcmp (key, "tags"))
  {
    char tag[4096]="NYI";
    return strdup (tag);
  }
  babl_free (state);
  return ret;
}


/*
 * Copyright 2001-2004 Unicode, Inc.
 *
 * Disclaimer
 *
 * This source code is provided as is by Unicode, Inc. No claims are
 * made as to fitness for any particular purpose. No warranties of any
 * kind are expressed or implied. The recipient agrees to determine
 * applicability of information provided. If this file has been
 * purchased on magnetic or optical media from Unicode, Inc., the
 * sole remedy for any claim will be exchange of defective media
 * within 90 days of receipt.
 *
 * Limitations on Rights to Redistribute This Code
 *
 * Unicode, Inc. hereby grants the right to freely use the information
 * supplied in this file in the creation of products supporting the
 * Unicode Standard, and to make copies of this file in any form
 * for internal or external distribution as long as this notice
 * remains attached.
 */
/* ---------------------------------------------------------------------

    Conversions between UTF32, UTF-16, and UTF-8. Source code file.
    Author: Mark E. Davis, 1994.
    Rev History: Rick McGowan, fixes & updates May 2001.
    Sept 2001: fixed const & error conditions per
	mods suggested by S. Parent & A. Lillich.
    June 2002: Tim Dodd added detection and handling of incomplete
	source sequences, enhanced error detection, added casts
	to eliminate compiler warnings.
    July 2003: slight mods to back out aggressive FFFE detection.
    Jan 2004: updated switches in from-UTF8 conversions.
    Oct 2004: updated to use UNI_MAX_LEGAL_UTF32 in UTF-32 conversions.
    Sep 2017: copied only the bits neccesary for utf16toutf8 into babl,
              otherwise unchanged from upstream.

    See the header file "ConvertUTF.h" for complete documentation.

------------------------------------------------------------------------ */

typedef uint32_t        UTF32;  /* at least 32 bits */
typedef unsigned short  UTF16;  /* at least 16 bits */
typedef unsigned char   UTF8;   /* typically 8 bits */
typedef unsigned char   Boolean; /* 0 or 1 */

typedef enum {
  conversionOK,           /* conversion successful */
  sourceExhausted,        /* partial character in source, but hit end */
  targetExhausted,        /* insuff. room in target for conversion */
  sourceIllegal           /* source sequence is illegal/malformed */
} ConversionResult;

#define UNI_REPLACEMENT_CHAR (UTF32)0x0000FFFD


#define UNI_SUR_HIGH_START  (UTF32)0xD800
#define UNI_SUR_HIGH_END    (UTF32)0xDBFF
#define UNI_SUR_LOW_START   (UTF32)0xDC00
#define UNI_SUR_LOW_END     (UTF32)0xDFFF
static const int halfShift  = 10; /* used for shifting by 10 bits */

static const UTF32 halfBase = 0x0010000UL;
static const UTF8 firstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

static int 
ConvertUTF16toUTF8 (const UTF16   **sourceStart, 
                    const UTF16    *sourceEnd,
	                 UTF8          **targetStart, 
	                 UTF8           *targetEnd, 
	                 ConversionFlags flags)
{
    ConversionResult result = conversionOK;
    const UTF16* source = *sourceStart;
    UTF8* target = *targetStart;
    while (source < sourceEnd) {
	UTF32 ch;
	unsigned short bytesToWrite = 0;
	const UTF32 byteMask = 0xBF;
	const UTF32 byteMark = 0x80;
	const UTF16* oldSource = source; /* In case we have to back up because of target overflow. */
	ch = *source++;
	/* If we have a surrogate pair, convert to UTF32 first. */
	if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END) {
	    /* If the 16 bits following the high surrogate are in the source buffer... */
	    if (source < sourceEnd) {
		UTF32 ch2 = *source;
		/* If it's a low surrogate, convert to UTF32. */
		if (ch2 >= UNI_SUR_LOW_START && ch2 <= UNI_SUR_LOW_END) {
		    ch = ((ch - UNI_SUR_HIGH_START) << halfShift)
			+ (ch2 - UNI_SUR_LOW_START) + halfBase;
		    ++source;
		} else if (flags == strictConversion) { /* it's an unpaired high surrogate */
		    --source; /* return to the illegal value itself */
		    result = sourceIllegal;
		    break;
		}
	    } else { /* We don't have the 16 bits following the high surrogate. */
		--source; /* return to the high surrogate */
		result = sourceExhausted;
		break;
	    }
	} else if (flags == strictConversion) {
	    /* UTF-16 surrogate values are illegal in UTF-32 */
	    if (ch >= UNI_SUR_LOW_START && ch <= UNI_SUR_LOW_END) {
		--source; /* return to the illegal value itself */
		result = sourceIllegal;
		break;
	    }
	}
	/* Figure out how many bytes the result will require */
	if (ch < (UTF32)0x80) {	     bytesToWrite = 1;
	} else if (ch < (UTF32)0x800) {     bytesToWrite = 2;
	} else if (ch < (UTF32)0x10000) {   bytesToWrite = 3;
	} else if (ch < (UTF32)0x110000) {  bytesToWrite = 4;
	} else {			    bytesToWrite = 3;
					    ch = UNI_REPLACEMENT_CHAR;
	}

	target += bytesToWrite;
	if (target > targetEnd) {
	    source = oldSource; /* Back up source pointer! */
	    target -= bytesToWrite; result = targetExhausted; break;
	}
	switch (bytesToWrite) { /* note: everything falls through. */
	    case 4: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
	    case 3: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
	    case 2: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
	    case 1: *--target =  (UTF8)(ch | firstByteMark[bytesToWrite]);
	}
	target += bytesToWrite;
    }
    *sourceStart = source;
    *targetStart = target;
    return result;
}

/* Trademarks:
 *
 * International Color Consortium is a registered trademarks of the.
 * International Color Consortium.
 */
