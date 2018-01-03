/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2012, Øyvind Kolås.
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <assert.h>
#include "config.h"
#include "babl-internal.h"
#include "babl.h"
#include "babl-memory.h"

#define HASH_TABLE_SIZE 1111

/* A default palette, containing standard ANSI / EGA colors
 *
 */
static unsigned char defpal_data[4*16] = 
{  
0  ,0  ,0  ,255,
127,0  ,0  ,255,
0  ,127,0  ,255,
127,127,0  ,255,
0  ,0  ,127,255,
127,0  ,127,255,
0  ,127,127,255,
127,127,127,255,
63 ,63 ,63 ,255,
255,0  ,0  ,255,
0  ,255,0  ,255,
255,255,0  ,255,
0  ,0  ,255,255,
255,0  ,255,255,
0  ,255,255,255,
255,255,255,255,
};
static double defpal_double[4*8*16];


typedef struct BablPalette
{
  int                    count;  /* number of palette entries */
  const Babl            *format; /* the pixel format the palette is stored in */
  unsigned char         *data;   /* one linear segment of all the pixels
                                  * representing the palette, in order
                                  */
  double                *data_double;
  unsigned char         *data_u8;
  volatile unsigned int  hash[HASH_TABLE_SIZE];
} BablPalette;

static void
babl_palette_reset_hash (BablPalette *pal)
{
  int i;
  for (i = 0; i < HASH_TABLE_SIZE; i++)
    {
      pal->hash[i] = i + 1; /* always a miss */
    }
}

static int
babl_palette_lookup (BablPalette *pal, int r, int g, int b, int a)
{
  unsigned int pixel      = (r << 16) | (g << 8) | b;
  int          hash_index = pixel % HASH_TABLE_SIZE;
  unsigned int hash_value = pal->hash[hash_index];
  unsigned int hash_pixel = hash_value & 0x00ffffffu;
  int          idx        = hash_value >> 24;

  /* note:  we're assuming the palette has no more than 256 colors, otherwise
   * the index doesn't fit in the top 8 bits of the hash-table value.  since
   * we're only using this functions with u8 palette formats, there's no need
   * to actually verify this, but if we add wider formats in the future, it's
   * something to be aware of.
   */

  if (pixel == hash_pixel)
    {
      return idx;
    }
  else
    {
      int best_idx = 0;
      int best_diff = INT_MAX;

      for (idx = 0; idx < pal->count; idx++)
        {
          unsigned char *palpx = pal->data_u8 + idx * 4;
          int pr = palpx[0];
          int pg = palpx[1];
          int pb = palpx[2];

          int diff = (r - pr) * (r - pr) +
                     (g - pg) * (g - pg) +
                     (b - pb) * (b - pb);
          if (diff < best_diff)
            {
              best_diff = diff;
              best_idx  = idx;
            }
        }
      pal->hash[hash_index] = ((unsigned int) best_idx << 24) | pixel;
      return best_idx;
    }
}

static BablPalette *make_pal (const Babl *format, const void *data, int count)
{
  BablPalette *pal = NULL;
  int bpp = babl_format_get_bytes_per_pixel (format);

  pal = babl_malloc (sizeof (BablPalette));
  pal->count = count;
  pal->format = format;
  pal->data = babl_malloc (bpp * count);
  pal->data_double = babl_malloc (4 * sizeof(double) * count);
  pal->data_u8 = babl_malloc (4 * sizeof(char) * count);
  memcpy (pal->data, data, bpp * count);

  babl_process (babl_fish (format, babl_format ("RGBA double")),
                data, pal->data_double, count);
  babl_process (babl_fish (format, babl_format ("R'G'B'A u8")),
                data, pal->data_u8, count);

  babl_palette_reset_hash (pal);

  return pal;
}

static void babl_palette_free (BablPalette *pal)
{
  babl_free (pal->data);
  babl_free (pal->data_double);
  babl_free (pal->data_u8);
  babl_free (pal);
}

static BablPalette *default_palette (void)
{
  static BablPalette pal;
  static int inited = 0;
  if (inited)
    return &pal;
  memset (&pal, 0, sizeof (pal));
  inited = 1;
  pal.count = 16;
  pal.format = babl_format ("R'G'B'A u8"); /* dynamically generated, so
                                              the default palette can
                                              not be fully static.
                                            */
  pal.data = defpal_data;
  pal.data_double = defpal_double;
  pal.data_u8 = defpal_data;

  babl_process (babl_fish (pal.format, babl_format ("RGBA double")),
                pal.data, pal.data_double, pal.count);

  babl_palette_reset_hash (&pal);
  return &pal;
}

static void
rgba_to_pal (Babl *conversion,
             char *src,
             char *dst,
             long  n,
             void *dst_model_data)
{
  BablPalette **palptr = dst_model_data;
  BablPalette *pal = *palptr;
  while (n--)
    {
      int idx;

      int best_idx = 0;
      double best_diff = 100000;
      double *srcf;

      srcf = ((double *) src);

      for (idx = 0; idx<pal->count; idx++)
        {
          double diff;
          double *palpx = ((double *)pal->data_double) + idx * 4;

          diff = (palpx[0] - srcf[0]) * (palpx[0] - srcf[0]) +
                 (palpx[1] - srcf[1]) * (palpx[1] - srcf[1]) +
                 (palpx[2] - srcf[2]) * (palpx[2] - srcf[2]);
          if (diff <= best_diff)
            {
              best_diff = diff;
              best_idx = idx;
            }
        }

      ((double *) dst)[0] = best_idx / 255.5;

      src += sizeof (double) * 4;
      dst += sizeof (double) * 1;
    }
}

static void
rgba_to_pala (Babl *conversion,
              char *src,
              char *dst,
              long  n,
              void *dst_model_data)
{
  BablPalette **palptr = dst_model_data;
  BablPalette *pal = *palptr;
  
  assert(pal);
  while (n--)
    {
      int idx;

      int best_idx = 0;
      double best_diff = 100000;
      double *srcf;
      double alpha;

      srcf = ((double *) src);
      alpha = srcf[3];

      for (idx = 0; idx<pal->count; idx++)
        {
          double diff;
          double *palpx = ((double *)pal->data_double) + idx * 4;

          diff = (palpx[0] - srcf[0]) * (palpx[0] - srcf[0]) +
                 (palpx[1] - srcf[1]) * (palpx[1] - srcf[1]) +
                 (palpx[2] - srcf[2]) * (palpx[2] - srcf[2]);
          if (diff <= best_diff)
            {
              best_diff = diff;
              best_idx = idx;
            }
        }

      ((double *) dst)[0] = best_idx / 255.5;
      ((double *) dst)[1] = alpha;

      src += sizeof (double) * 4;
      dst += sizeof (double) * 2;
    }
}

static void
pal_to_rgba (Babl *conversion,
             char *src,
             char *dst,
             long  n,
             void *src_model_data)
{
  BablPalette **palptr = src_model_data;
  BablPalette *pal = *palptr;
  assert(pal);
  while (n--)
    {
      int idx = (((double *) src)[0]) * 255.5;
      double *palpx;

      if (idx < 0) idx = 0;
      if (idx >= pal->count) idx = pal->count-1;

      palpx = ((double *)pal->data_double) + idx * 4;
      memcpy (dst, palpx, sizeof(double)*4);

      src += sizeof (double) * 1;
      dst += sizeof (double) * 4;
    }
}

static void
pala_to_rgba (Babl *conversion,
              char *src,
              char *dst,
              long  n,
              void *src_model_data)
{
  BablPalette **palptr = src_model_data;
  BablPalette *pal = *palptr;

  assert(pal);
  while (n--)
    {
      int idx      = (((double *) src)[0]) * 255.5;
      double alpha = (((double *) src)[1]);
      double *palpx;

      if (idx < 0) idx = 0;
      if (idx >= pal->count) idx = pal->count-1;

      palpx = ((double *)pal->data_double) + idx * 4;
      memcpy (dst, palpx, sizeof(double)*4);

      ((double *)dst)[3] *= alpha; 

      src += sizeof (double) * 2;
      dst += sizeof (double) * 4;
    }
}

static void
rgba_u8_to_pal (Babl          *conversion,
                unsigned char *src,
                unsigned char *dst,
                long           n,
                void          *src_model_data)
{
  BablPalette **palptr = src_model_data;
  BablPalette *pal;
  assert (palptr);
  pal = *palptr;
  assert(pal);
  while (n--)
    {
      dst[0] = babl_palette_lookup (pal, src[0], src[1], src[2], src[3]);

      src += sizeof (char) * 4;
      dst += sizeof (char) * 1;
    }
}

static void
rgba_u8_to_pal_a (Babl          *conversion,
                  unsigned char *src,
                  unsigned char *dst,
                  long           n,
                  void          *src_model_data)
{
  BablPalette **palptr = src_model_data;
  BablPalette *pal;
  assert (palptr);
  pal = *palptr;
  assert(pal);
  while (n--)
    {
      dst[0] = babl_palette_lookup (pal, src[0], src[1], src[2], src[3]);
      dst[1] = src[3];

      src += sizeof (char) * 4;
      dst += sizeof (char) * 2;
    }
}

static long
pal_u8_to_rgba_u8 (Babl          *conversion,
                   unsigned char *src,
                   unsigned char *dst,
                   long           n,
                   void          *src_model_data)
{
  BablPalette **palptr = src_model_data;
  BablPalette *pal;
  assert (palptr);
  pal = *palptr;
  assert(pal);
  while (n--)
    {
      int idx = src[0];
      unsigned char *palpx;

      if (idx < 0) idx = 0;
      if (idx >= pal->count) idx = pal->count-1;

      palpx = pal->data_u8 + idx * 4;
      memcpy (dst, palpx, sizeof(char)*4);

      src += sizeof (char) * 1;
      dst += sizeof (char) * 4;
    }
  return n;
}

static long
pala_u8_to_rgba_u8 (Babl          *conversion,
                    unsigned char *src,
                    unsigned char *dst,
                    long           n,
                    void          *src_model_data)
{
  BablPalette **palptr = src_model_data;
  BablPalette *pal;
  assert (palptr);
  pal = *palptr;
  assert(pal);
  while (n--)
    {
      int idx = src[0];
      unsigned char *palpx;

      if (idx < 0) idx = 0;
      if (idx >= pal->count) idx = pal->count-1;

      palpx = pal->data_u8 + idx * 4;
      memcpy (dst, palpx, sizeof(char)*4);
      dst[3] = (dst[3] * src[1] + 128) / 255;

      src += sizeof (char) * 2;
      dst += sizeof (char) * 4;
    }
  return n;
}


#include "base/util.h"

static inline long
conv_pal8_pala8 (Babl *conversion,
                 unsigned char *src, unsigned char *dst, long samples)
{
  long n = samples;

  while (n--)
    {
      dst[0] = src[0];
      dst[1] = 255;
      src   += 1;
      dst   += 2;
    }
  return samples;
}

static inline long
conv_pala8_pal8 (Babl *conversion,
                 unsigned char *src, unsigned char *dst, long samples)
{
  long n = samples;

  while (n--)
    {
      dst[0] = src[0];
      src   += 2;
      dst   += 1;
    }
  return samples;
}

int
babl_format_is_palette (const Babl *format)
{
  if (format->class_type == BABL_FORMAT)
    return format->format.palette;
  return 0;
}

/* should return the BablModel, permitting to fetch
 * other formats out of it?
 */
const Babl *babl_new_palette (const char  *name,
                              const Babl **format_u8,
                              const Babl **format_u8_with_alpha)
{
  const Babl *model;
  const Babl *model_no_alpha;
  Babl *f_pal_u8;
  Babl *f_pal_a_u8;
  const Babl *component;
  const Babl *alpha;
  BablPalette **palptr;

  char  cname[64];

  if (!name)
    {
      static int cnt = 0;
      snprintf (cname, sizeof (cname), "_babl-int-%i", cnt++);
      name = cname;
    }
  else
    {
      strcpy (cname, name);
      name = cname;

      if ((model = babl_db_exist_by_name (babl_model_db (), name)))
        {
          cname[0] = ')';
          if (format_u8)
            *format_u8 = babl_db_exist_by_name (babl_format_db (), name);
          cname[0] = '\\';
          if (format_u8_with_alpha)
            *format_u8_with_alpha = babl_db_exist_by_name (babl_format_db (), name);
          return model;
        }
    }

  /* re-registering is a no-op */
  component = babl_component_new (
    "I",
    "luma",
    "chroma",
    NULL);
  alpha = babl_component ("A");
  
  model = babl_model_new ("name", name, component, alpha, NULL);
  palptr = malloc (sizeof (void*));
  *palptr = default_palette ();;
  cname[0] = 'v';
  model_no_alpha = babl_model_new ("name", name, component, NULL);
  cname[0] = '\\';
  f_pal_a_u8 = (void*) babl_format_new ("name", name, model,
                                babl_type ("u8"),
                                component, alpha, NULL);
  cname[0] = ')';
  f_pal_u8  = (void*) babl_format_new ("name", name, model_no_alpha,
                               babl_type ("u8"),
                               component, NULL);

  f_pal_a_u8->format.palette = 1;
  f_pal_u8->format.palette = 1;

  babl_conversion_new (
     model,
     babl_model ("RGBA"),
     "linear", pala_to_rgba,
     "data", palptr,
     NULL
  );

  babl_conversion_new (
     babl_model ("RGBA"),
     model,
     "linear", rgba_to_pala,
     "data", palptr,
     NULL
  );

  babl_conversion_new (
     model_no_alpha,
     babl_model ("RGBA"),
     "linear", pal_to_rgba,
     "data", palptr,
     NULL
  );
  babl_conversion_new (
     babl_model ("RGBA"),
     model_no_alpha,
     "linear", rgba_to_pal,
     "data", palptr,
     NULL
  );

  babl_conversion_new (
     f_pal_u8,
     f_pal_a_u8,
     "linear", conv_pal8_pala8,
     NULL
  );

  babl_conversion_new (
     f_pal_a_u8,
     f_pal_u8,
     "linear", conv_pala8_pal8,
     NULL
  );


  babl_conversion_new (
     f_pal_u8,
     babl_format ("R'G'B'A u8"),
     "linear", pal_u8_to_rgba_u8,
     "data", palptr,
     NULL);


  babl_conversion_new (
     f_pal_a_u8,
     babl_format ("R'G'B'A u8"),
     "linear", pala_u8_to_rgba_u8,
     "data", palptr,
     NULL);

  babl_conversion_new (
     babl_format ("R'G'B'A u8"),
     f_pal_a_u8,
     "linear", rgba_u8_to_pal_a,
     "data", palptr,
     NULL);
  babl_conversion_new (
     babl_format ("R'G'B'A u8"),
     f_pal_u8,
     "linear", rgba_u8_to_pal,
     "data", palptr,
     NULL);

  babl_set_user_data (model, palptr);
  babl_set_user_data (model_no_alpha, palptr);

  if (format_u8)
    *format_u8 = f_pal_u8;
  if (format_u8_with_alpha)
    *format_u8_with_alpha = f_pal_a_u8;
  babl_sanity ();
  return model;
}

void
babl_palette_set_palette (const Babl *babl,
                          const Babl *format,
                          void       *data,
                          int         count)
{
  BablPalette **palptr = babl_get_user_data (babl);
  babl_palette_reset (babl);
  *palptr = make_pal (format, data, count);
}

void
babl_palette_reset (const Babl *babl)
{
  BablPalette **palptr = babl_get_user_data (babl);
  if (*palptr != default_palette ())
    {
      babl_palette_free (*palptr);
    }
  *palptr = default_palette ();
}
