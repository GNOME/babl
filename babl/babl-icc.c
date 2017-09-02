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

typedef struct ICC {
  char *data;
  int   length;

  int   tags;
  int   headpos;
  int   o, no;
  int   p;
  int   psize;
} ICC;

ICC *icc_state_new (char *data, int length, int tags);

ICC *icc_state_new (char *data, int length, int tags)
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

static void write_u8 (ICC *state, int offset, uint8_t value)
{
  if (offset < 0 || offset >= state->length)
    return;
  *(uint8_t*) (&state->data[offset]) = value;
}

static void write_s8 (ICC *state, int offset, int8_t value)
{
  if (offset < 0 || offset >= state->length)
    return;
  *(int8_t*) (&state->data[offset]) = value;
}

static int read_u8 (ICC *state, int offset)
{
/* all reading functions take both the char *pointer and the length of the
 * buffer, and all reads thus gets protected by this condition.
 */
  if (offset < 0 || offset > state->length)
    return 0;

  return *(uint8_t*) (&state->data[offset]);
}

static int read_s8 (ICC *state, int offset)
{
  if (offset < 0 || offset > state->length)
    return 0;

  return *(int8_t*) (&state->data[offset]);
}

static void write_s16 (ICC *state, int offset, int16_t value)
{
  write_s8 (state, offset + 0, value >> 8);
  write_u8 (state, offset + 1, value & 0xff);
}

static int16_t read_s16 (ICC *state, int offset)
{
  return icc_read (u8, offset + 1) +
         (read_s8 (state, offset + 0) << 8); //XXX: transform to icc_read macro
}

static uint16_t read_u16 (ICC *state, int offset)
{
  return icc_read (u8, offset + 1) +
         (icc_read (u8, offset + 0) << 8);
}

static void write_u16 (ICC *state, int offset, uint16_t value)
{
  write_u8 (state, offset + 0, value >> 8);
  write_u8 (state, offset + 1, value & 0xff);
}

static u8f8_t read_u8f8_ (ICC *state, int offset)
{
  u8f8_t ret ={icc_read (u8, offset),
               icc_read (u8, offset + 1)};
  return ret;
}

static s15f16_t read_s15f16_ (ICC *state, int offset)
{
  s15f16_t ret ={icc_read (s16, offset),
                 icc_read (u16, offset + 2)};
  return ret;
}

static void write_u8f8_ (ICC *state, int offset, u8f8_t val)
{
  icc_write (u8, offset,     val.integer),
  icc_write (u8, offset + 1, val.fraction);
}

static void write_s15f16_ (ICC *state, int offset, s15f16_t val)
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

static u8f8_t d_to_u8f8 (double value)
{
  u8f8_t ret;
  ret.integer = floor (value);
  ret.fraction = fmod(value, 1.0) * 255.999;
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

static void write_s15f16 (ICC *state, int offset, double value)
{
   write_s15f16_ (state, offset, d_to_s15f16 (value));
}

static void write_u8f8 (ICC *state, int offset, double value)
{
  write_u8f8_ (state, offset, d_to_u8f8 (value));
}


static double read_s15f16 (ICC *state, int offset)
{
  return s15f16_to_d (read_s15f16_ (state, offset));
}

static double read_u8f8 (ICC *state, int offset)
{
  return u8f8_to_d (read_u8f8_ (state, offset));
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

static void write_u32 (ICC *state, int offset, uint32_t value)
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

static uint32_t read_u32 (ICC *state, int offset)
{
  return icc_read (u8, offset + 3) +
         (icc_read (u8, offset + 2) << 8) +
         (icc_read (u8, offset + 1) << 16) +
         (icc_read (u8, offset + 0) << 24);
}

static sign_t read_sign (ICC *state, int offset)
{
  sign_t ret;
  ret.str[0]=icc_read (u8, offset);
  ret.str[1]=icc_read (u8, offset + 1);
  ret.str[2]=icc_read (u8, offset + 2);
  ret.str[3]=icc_read (u8, offset + 3);
  ret.str[4]=0;
  return ret;
}

static void write_sign (ICC *state, int offset, const char *sign)
{
  int i;
  for (i = 0; i < 4; i ++)
    icc_write (u8, offset + i, sign[i]);
}

/* looks up offset and length for a specific icc tag
 */
static int icc_tag (ICC *state,
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

static const Babl *babl_trc_from_icc (ICC  *state, int offset,
                                      char **error)
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
              g = icc_read (s15f16, offset + 12 + 2 * 0);
              return babl_trc_gamma (g);
              break;
            case 3:
              {
                float a,b,c,d;
                g = icc_read (s15f16, offset + 12 + 2 * 0);
                a = icc_read (s15f16, offset + 12 + 2 * 1);
                b = icc_read (s15f16, offset + 12 + 2 * 2);
                c = icc_read (s15f16, offset + 12 + 2 * 3);
                d = icc_read (s15f16, offset + 12 + 2 * 4);
                //fprintf (stderr, "%f %f %f %f %f\n", g, a, b, c, d);
                if (fabs (g - 2.40)     < 0.01 &&
                    fabs (a - 26214)    < 0.01 &&
                    fabs (b - 0.947875) < 0.01 &&
                    fabs (c - (-3417))  < 0.01)
                  return babl_trc ("sRGB");
                else
                  return babl_trc_formula_srgb (g, a, b, c, d);
              }
              break;
            case 4:
              {
                float a,b,c,d,e,f;
                g = icc_read (s15f16, offset + 12 + 2 * 0);
                a = icc_read (s15f16, offset + 12 + 2 * 1);
                b = icc_read (s15f16, offset + 12 + 2 * 2);
                c = icc_read (s15f16, offset + 12 + 2 * 3);
                d = icc_read (s15f16, offset + 12 + 2 * 4);
                e = icc_read (s15f16, offset + 12 + 2 * 5);
                f = icc_read (s15f16, offset + 12 + 2 * 6);
                fprintf (stderr, "%f %f %f %f %f %f %f\n",
                              g, a, b, c, d, e, f);
            {
              fprintf (stdout, "unhandled parametric sRGB formula TRC type %i\n", function_type);
              *error = "unhandled sRGB formula like TRC";
              return babl_trc_gamma (2.2);
            }
                              }
              break;
            default:
              *error = "unhandled parametric TRC";
              fprintf (stdout, "unhandled parametric TRC type %i\n", function_type);
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

static void icc_allocate_tag (ICC *state, const char *tag, int size)
{
    state->no+=((4-state->o)%4);state->o = state->no;state->psize = size;
    icc_write (sign, 128 + 4 + 4 * state->headpos++, tag);
    icc_write (u32,  128 + 4 + 4 * state->headpos++, state->o);
    icc_write (u32,  128 + 4 + 4 * state->headpos++, size);
    state->p = state->no;\
    state->no+=size;
}

static void icc_duplicate_tag(ICC *state, const char *tag)
{
    icc_write (sign, 128 + 4 + 4 * state->headpos++, tag);
    icc_write (u32,  128 + 4 + 4 * state->headpos++, state->p);
    icc_write (u32,  128 + 4 + 4 * state->headpos++, state->psize);
}

void write_trc (ICC *state, const char *name, const BablTRC *trc);
void write_trc (ICC *state, const char *name, const BablTRC *trc)
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
  case BABL_TRC_GAMMA_1_8:
    icc_allocate_tag (state, name, 14);
    icc_write (sign, state->o, "curv");
    icc_write (u32, state->o + 4, 0);
    icc_write (u32, state->o + 8, 1);
    icc_write (u8f8, state->o + 12, 1.8);
    break;
  case BABL_TRC_GAMMA_2_2:
    icc_allocate_tag (state, name, 14);
    icc_write (sign, state->o, "curv");
    icc_write (u32, state->o + 4, 0);
    icc_write (u32, state->o + 8, 1);
    icc_write (u8f8, state->o + 12, 2.2);
    break;
  case BABL_TRC_LUT:
    icc_allocate_tag (state, name, 13 + trc->lut_size * 2);
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
  case BABL_TRC_FORMULA_SRGB:
//  default:
    {
      int lut_size = 512;
      icc_allocate_tag (state, name, 13 + lut_size * 2);
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

static void symmetry_test (ICC *state);

const char *babl_space_to_icc (const Babl *babl, int *ret_length)
{
  const BablSpace *space = &babl->space;
  static char icc[65536];
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

    write_trc (state, "rTRC", &space->trc[0]->trc);

    if (space->trc[0] == space->trc[1] &&
        space->trc[0] == space->trc[2])
    {
      icc_duplicate_tag (state, "gTRC");
      icc_duplicate_tag (state, "bTRC");
    }
    else
    {
      write_trc (state, "gTRC", &space->trc[1]->trc);
      write_trc (state, "bTRC", &space->trc[2]->trc);
    }

    {
      char str[128];
      int i;
      sprintf (str, "babl");
      icc_allocate_tag(state, "desc", 90 + strlen (str) + 1);
      icc_write (sign, state->o,"desc");
      icc_write (u32, state->o + 4, 0);
      icc_write (u32, state->o + 8, strlen(str) + 1);
      for (i = 0; str[i]; i++)
        icc_write (u8, state->o + 12 + i, str[i]);
    }

    {
      char str[128];
      int i;
      sprintf (str, "CC0/public domain");
      icc_allocate_tag(state, "cprt", 8 + strlen (str) + 1);
      icc_write (sign, state->o, "text");
      icc_write (u32, state->o + 4, 0);
      for (i = 0; str[i]; i++)
        icc_write (u8, state->o + 8 + i, str[i]);
    }

    icc_write (u32, 0, state->no + 3);
    length = state->no + 3;
  }

  if (ret_length)
    *ret_length = length;

  babl_free (state);
  return icc;
}

static char *icc_decode_mluc (ICC *state, int offset, int element_length, char *lang, char *country)
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
      int slength = icc_read(u32, offset + o + 4);
      int soffset = icc_read(u32, offset + o + 8);
      char *ret = babl_malloc (slength * 2);
      int j;

      for (j = 0; j < slength/2; j++)
      {
        int hi = icc_read(u8, offset + soffset + j * 2 + 0);
        int lo = icc_read(u8, offset + soffset + j * 2 + 1);

        ret[j] = lo + hi * 0; // only ASCII survives this
                              // brute utf16 decoding, so it is
                              // good we ask for english.
      }
      ret[j] = 0;
      return ret;
    }
    o+=record_size;
  }
  return babl_strdup ("");
}

const Babl *
babl_space_from_icc (const char *icc_data,
                     int         icc_length,
                     char      **error)
{
  ICC  *state = icc_state_new ((char*)icc_data, icc_length, 0);
  int   profile_size    = icc_read (u32, 0);
  //int   icc_ver_major   = icc_read (u8, 8);
  const Babl *trc_red   = NULL;
  const Babl *trc_green = NULL;
  const Babl *trc_blue  = NULL;
  char *descr     = NULL;
  char *copyright = NULL;

  sign_t profile_class, color_space;

  if (profile_size != icc_length)
  {
    *error = "icc profile length inconsistency";
  }
#if 0
  else if (icc_ver_major > 2)
  {
    *error = "only ICC v2 profiles supported";
  }
#endif
  else
  {
  profile_class = icc_read (sign, 12);
  if (strcmp (profile_class.str, "mntr"))
    *error = "not a monitor-class profile";
  else
  {
  color_space = icc_read (sign, 16);
  if (strcmp (color_space.str, "RGB "))
    *error = "not defining an RGB space";
  }
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

  {
     int offset, element_size;
     icc_tag (state, "desc", &offset, &element_size);
     if (!strcmp (state->data + offset, "mluc"))
     {
       descr = icc_decode_mluc (state, offset, element_size, "en", NULL);
     }
     else
     if (!strcmp (state->data + offset, "desc"))
     {
       descr = babl_strdup (state->data + offset + 12);
     }
  }

  {
     int offset, element_size;
     icc_tag (state, "cprt", &offset, &element_size);
     if (!strcmp (state->data + offset, "mluc"))
     {
       copyright = icc_decode_mluc (state, offset, element_size, "en", NULL);
     }
     else
     if (!strcmp (state->data + offset, "desc"))
     {
       copyright = babl_strdup (state->data + offset + 8);
     }
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

     babl_free (state);

     {
       Babl *ret = (void*)babl_space_from_rgbxyz_matrix (NULL,
                wX, wY, wZ,
                rx, gx, bx,
                ry, gy, by,
                rz, gz, bz,
                trc_red, trc_green, trc_blue);
       ret->space.description = descr;
       ret->space.copyright = copyright;
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

     icc_tag (state, "wtpt", &offset, &element_size);
     {
       double wX = icc_read (s15f16, offset + 8);
       double wY = icc_read (s15f16, offset + 8 + 4);
       double wZ = icc_read (s15f16, offset + 8 + 4 * 2);
       babl_free (state);

       {
         Babl *ret = (void*) babl_space_from_chromaticities (NULL,
                       wX / (wX + wY + wZ),
                       wY / (wX + wY + wZ),
                       red_x, red_y,
                       green_x, green_y,
                       blue_x, blue_y,
                       trc_red, trc_green, trc_blue);
         ret->space.description = descr;
         ret->space.copyright = copyright;
         return ret;
       }

     }
  }

  *error = "didnt find RGB primaries";
  babl_free (state);
  return NULL;
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

