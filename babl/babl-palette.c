/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2005, Øyvind Kolås.
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
#include <assert.h>
#include "babl.h"

typedef struct BablPalette
{
  int         count; /* number of palette entries */
  const Babl *format;/* the pixel format the palette is stored in */
  void       *data;  /* one linear segment of all the pixels representing the palette, in   order */
  void       *data_double;
  void       *data_u8;
} BablPalette;


static BablPalette *make_pal (Babl *format, void *data, int count)
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
                pal->data, pal->data_double, count);
  babl_process (babl_fish (format, babl_format ("RGBA u8")),
                pal->data, pal->data_u8, count);
  return pal;
}

static void babl_palette_free (BablPalette *pal)
{
  babl_free (pal->data);
  babl_free (pal->data_double);
  babl_free (pal->data_u8);
  babl_free (pal);
}

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

static BablPalette *default_palette (void)
{
  static BablPalette pal;
  static int inited = 0;
  if (inited)
    return &pal;
  inited = 1;
  pal.count = 16;
  pal.format = babl_format ("RGBA u8"); /* dynamically generated, so
                                           the default palette can
                                           not be fully static.
                                         */
  pal.data = defpal_data;
  pal.data_double = defpal_double;
  pal.data_u8 = defpal_data;

  babl_process (babl_fish (pal.format, babl_format ("RGBA double")),
                pal.data, pal.data_double, pal.count);
  return &pal;
}

static long
rgba_to_pal (char *src,
             char *dst,
             long  n,
             void *foo,
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

      srcf = ((double *) src);

      for (idx = 0; idx<pal->count; idx++)
        {
          double diff;
          double *palpx = ((double *)pal->data_double) + idx * 4;

          diff = (palpx[0] - srcf[0]) * (palpx[0] - srcf[0]) +
                 (palpx[1] - srcf[1]) * (palpx[1] - srcf[1]) +
                 (palpx[2] - srcf[2]) * (palpx[2] - srcf[2]) +
                 (palpx[3] - srcf[3]) * (palpx[3] - srcf[3]);
          if (diff < best_diff)
            {
              best_diff = diff;
              best_idx = idx;
            }
        }

      ((double *) dst)[0] = best_idx / 256.0;

      src += sizeof (double) * 4;
      dst += sizeof (double) * 1;
    }
  return n;
}

static long
rgba_to_pala (char *src,
              char *dst,
              long  n,
              void *foo,
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
          if (diff < best_diff)
            {
              best_diff = diff;
              best_idx = idx;
            }
        }

      ((double *) dst)[0] = best_idx / 256.0;
      ((double *) dst)[1] = alpha;

      src += sizeof (double) * 4;
      dst += sizeof (double) * 2;
    }
  return n;
}

static long
pal_to_rgba (char *src,
             char *dst,
             long  n,
             void *src_model_data,
             void *foo)
{
  BablPalette **palptr = src_model_data;
  BablPalette *pal = *palptr;
  assert(pal);
  while (n--)
    {
      int idx = (((double *) src)[0]) * 256.0;
      double *palpx;

      if (idx < 0) idx = 0;
      if (idx >= pal->count) idx = pal->count-1;

      palpx = ((double *)pal->data_double) + idx * 4;
      memcpy (dst, palpx, sizeof(double)*4);

      src += sizeof (double) * 1;
      dst += sizeof (double) * 4;
    }
  return n;
}

static long
pal_u8_to_rgba_u8 (char *src,
                   char *dst,
                   long  n,
                   void *src_model_data,
                   void *foo)
{
  BablPalette **palptr = src_model_data;
  assert(palptr);
  BablPalette *pal;
  pal = *palptr;
  assert(pal);
  while (n--)
    {
      int idx = (((unsigned char *) src)[0]);
      unsigned char *palpx;

      if (idx < 0) idx = 0;
      if (idx >= pal->count) idx = pal->count-1;

      palpx = ((unsigned char*)pal->data_u8) + idx * 4;
      memcpy (dst, palpx, sizeof(char)*4);

      src += sizeof (char) * 1;
      dst += sizeof (char) * 4;
    }
  return n;
}

static long
pala_to_rgba (char *src,
              char *dst,
              long  n,
              void *src_model_data,
              void *foo)
{
  BablPalette **palptr = src_model_data;
  BablPalette *pal = *palptr;

  assert(pal);
  while (n--)
    {
      int idx      = (((double *) src)[0]) * 256.0;
      double alpha = 255;//(((double *) src)[1]);
      double *palpx;

      if (idx < 0) idx = 0;
      if (idx >= pal->count) idx = pal->count-1;

      palpx = ((double *)pal->data_double) + idx * 4;
      memcpy (dst, palpx, sizeof(double)*4);

      ((double *)dst)[3] *= alpha; 

      src += sizeof (double) * 2;
      dst += sizeof (double) * 4;
    }
  return n;
}

/* should return the BablModel, permitting to fetch
 * other formats out of it?
 */
void babl_new_palette (const char *name, Babl **format_u8,
                                         Babl **format_u8_with_alpha)
{
  Babl *model;
  Babl *model_no_alpha;
  Babl *f_pal_double;
  Babl *f_pal_u8;
  Babl *f_pal_a_u8;
  BablPalette **palptr;

  char  cname[64];

  if (!name)
    {
      static int cnt = 0;
      sprintf (cname, "_babl-int-%i", cnt++);
      name = cname;
    }
  else
    {
      strcpy (cname, name);
      name = cname;
    }

  /* re-registering is a no-op */
  babl_component_new (
    "I",
    "luma",
    "chroma",
    "alpha",
    NULL);
  
  model = babl_model_new ("name", name,
                          babl_component ("I"),
                          babl_component ("A"),
                          NULL);
  palptr = malloc (sizeof (void*));
  *palptr = default_palette ();;
  cname[0] = 'v';
  model_no_alpha = babl_model_new ("name", name,
                                   babl_component ("I"),
                                   NULL);

  cname[0] = 'x';
  f_pal_a_u8 = babl_format_new ("name", name, model,
                                babl_type ("u8"),
                                babl_component ("I"),
                                babl_component ("A"),
                              NULL);
  cname[0] = 'y';
  f_pal_u8  = babl_format_new ("name", name, model_no_alpha,
                               babl_type ("u8"),
                               babl_component ("I"), NULL);

#if 0
  cname[0] = 'z';
  f_pal_double = babl_format_new ("name", name, model,
                                  babl_type ("double"),
                                  babl_component ("I"), NULL);
#endif
  babl_conversion_new (
     model,
     babl_model  ("RGBA"),
     "linear", pala_to_rgba,
     NULL
  );

  babl_conversion_new (
     babl_model  ("RGBA"),
     model,
     "linear", rgba_to_pala,
     NULL
  );

  babl_conversion_new (
     model_no_alpha,
     babl_model  ("RGBA"),
     "linear", pal_to_rgba,
     NULL
  );

  babl_conversion_new (
     babl_model  ("RGBA"),
     model_no_alpha,
     "linear", rgba_to_pal,
     NULL
  );

  babl_conversion_new (
     f_pal_u8,
     babl_format ("RGBA u8"),
     "linear", pal_u8_to_rgba_u8,
     NULL);

  babl_set_user_data (model, palptr);
  babl_set_user_data (model_no_alpha, palptr);

  if (format_u8)
    *format_u8 = f_pal_u8;
  if (format_u8_with_alpha)
    *format_u8_with_alpha = f_pal_a_u8;
  babl_sanity ();
}

void
babl_palette_set_palette (Babl *babl,
                          Babl *format,
                          void *data,
                          int   count)
{
  BablPalette **palptr = babl_get_user_data (babl);
  babl_palette_reset (babl);
  *palptr = make_pal (format, data, count);
}

void
babl_palette_reset (Babl *babl)
{
  BablPalette **palptr = babl_get_user_data (babl);
  if (*palptr != default_palette ())
    {
      babl_palette_free (*palptr);
    }
  *palptr = default_palette ();
}
