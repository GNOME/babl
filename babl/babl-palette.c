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
 * <https://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <assert.h>
#include "config.h"
#include "babl-internal.h"
#include "babl.h"
#include "babl-memory.h"

#define HASH_TABLE_SIZE 1111


typedef struct BablPaletteRadius
{
  unsigned char  idx;
  unsigned short diff;
} BablPaletteRadius;

typedef struct BablPalette
{
  int                    count;  /* number of palette entries */
  const Babl            *format; /* the pixel format the palette is stored in */
  unsigned char         *data;   /* one linear segment of all the pixels
                                  * representing the palette, in order
                                  */
  double                *data_double;
  unsigned char         *data_u8;
  BablPaletteRadius     *radii;
  volatile unsigned int  hash[HASH_TABLE_SIZE];
} BablPalette;


static unsigned short ceil_sqrt_u8[3 * 255 * 255 + 1];

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
static double defpal_double[4*16];
static BablPaletteRadius defpal_radii[15 * 16];


static void
init_ceil_sqrt_u8 (void)
{
  int i;

  if (! ceil_sqrt_u8[1])
    {
      for (i = 0; i <= 3 * 255 * 255; i++)
        ceil_sqrt_u8[i] = ceil (sqrt (i));
    }
}

static inline int
diff2_u8 (const unsigned char *p1,
          const unsigned char *p2)
{
  return ((int) p1[0] - (int) p2[0]) * ((int) p1[0] - (int) p2[0]) +
         ((int) p1[1] - (int) p2[1]) * ((int) p1[1] - (int) p2[1]) +
         ((int) p1[2] - (int) p2[2]) * ((int) p1[2] - (int) p2[2]);
}

static int
babl_palette_radius_compare (const void *r1,
                             const void *r2)
{
  const BablPaletteRadius *radius1 = r1;
  const BablPaletteRadius *radius2 = r2;

  return (int) radius1->diff - (int) radius2->diff;
}

static void
babl_palette_init_radii (BablPalette *pal)
{
  int i, j;

  /* calculate the distance between each pair of colors in the palette, and, for
   * each color, construct a list of all other colors and their distances from
   * it, sorted by distance.  we use these lists in babl_palette_lookup() to
   * speed up the search, as described in the function.
   */

  for (i = 0; i < pal->count; i++)
    {
      BablPaletteRadius   *radii1 = pal->radii + (pal->count - 1) * i;
      const unsigned char *p1     = pal->data_u8 + 4 * i;

      for (j = i + 1; j < pal->count; j++)
        {
          BablPaletteRadius   *radii2 = pal->radii + (pal->count - 1) * j;
          const unsigned char *p2     = pal->data_u8 + 4 * j;
          unsigned short       diff;

          diff = floor (sqrt (diff2_u8 (p1, p2)));

          radii1[j - 1].idx  = j;
          radii1[j - 1].diff = diff;

          radii2[i].idx      = i;
          radii2[i].diff     = diff;
        }

      qsort (radii1, pal->count - 1, sizeof (BablPaletteRadius),
             babl_palette_radius_compare);
    }
}

static void
babl_palette_reset_hash (BablPalette *pal)
{
  int i;
  for (i = 0; i < HASH_TABLE_SIZE; i++)
    {
      pal->hash[i] = i + 1; /* always a miss */
    }
}

#define BABL_IDX_FACTOR 255.5

static int
babl_palette_lookup (BablPalette         *pal,
                     const unsigned char *p,
                     int                  best_idx)
{
  unsigned int pixel      = p[0] | (p[1] << 8) | (p[2] << 16);
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
      const BablPaletteRadius *radii = pal->radii + (pal->count - 1) * best_idx;
      const unsigned char     *q;
      int                      best_diff2;
      int                      best_diff;
      int                      diff0;
      int                      i;

      /* best_idx is the closest palette entry to the previous pixel (referred
       * to as the source color).  based on the assumption that nearby pixels
       * have similar color, we start the search for the current closest entry
       * at best_idx, and iterate over the entry's color list, as calculated in
       * babl_palette_init_radii(), in search for a better match.
       */

      q          = pal->data_u8 + 4 * best_idx;
      best_diff2 = diff2_u8 (p, q);
      best_diff  = ceil_sqrt_u8[best_diff2];
      diff0      = best_diff;

      for (i = 0; i < pal->count - 1; i++)
        {
          const BablPaletteRadius *radius = &radii[i];
          int                      min_diff;
          int                      diff2;

          /* radius->diff is the distance from the source color to the current
           * color.  diff0 is the distance from the source color to the input
           * color.  according to the triangle inequality, the distance from
           * the current color to the input color is at least
           * radius->diff - diff0.  if the shortest distance found so far is
           * less than that, then the best match found so far is necessarily
           * better than the current color, and we can stop the search, since
           * the color list is sorted in ascending radius->diff order.
           */

          idx      = radius->idx;
          min_diff = radius->diff - diff0;

          if (best_diff < min_diff || (best_diff == min_diff && best_idx < idx))
            break;

          q     = pal->data_u8 + 4 * idx;
          diff2 = diff2_u8 (p, q);

          if (diff2 < best_diff2 || (diff2 == best_diff2 && idx < best_idx))
            {
              best_idx   = idx;
              best_diff2 = diff2;
              best_diff  = ceil_sqrt_u8[diff2];
            }
        }

      pal->hash[hash_index] = ((unsigned int) best_idx << 24) | pixel;

      return best_idx;
    }
}

static BablPalette *
make_pal (const Babl *pal_space, 
          const Babl *format, 
          const void *data, 
          int         count)
{
  BablPalette *pal = NULL;
  int bpp = babl_format_get_bytes_per_pixel (format);

  babl_assert (count > 0);

  pal = babl_malloc (sizeof (BablPalette));
  pal->count = count;
  pal->format = format;
  pal->data = babl_malloc (bpp * count);
  pal->data_double = babl_malloc (4 * sizeof(double) * count);
  pal->data_u8 = babl_malloc (4 * sizeof(char) * count);
  pal->radii = babl_malloc (sizeof (BablPaletteRadius) *
                            (pal->count - 1)           *
                            pal->count);

  memcpy (pal->data, data, bpp * count);

  babl_process (babl_fish (format, babl_format_with_space ("RGBA double", pal_space)),
                data, pal->data_double, count);
  babl_process (babl_fish (format, babl_format_with_space ("R'G'B'A u8", pal_space)),
                data, pal->data_u8, count);

  babl_palette_init_radii (pal);

  babl_palette_reset_hash (pal);

  return pal;
}

static void 
babl_palette_free (BablPalette *pal)
{
  babl_free (pal->data);
  babl_free (pal->data_double);
  babl_free (pal->data_u8);
  babl_free (pal->radii);
  babl_free (pal);
}

static BablPalette *
default_palette (void)
{
  static BablPalette pal;
  static int inited = 0;

  babl_mutex_lock (babl_format_mutex);

  if (inited)
    {
      babl_mutex_unlock (babl_format_mutex);

      return &pal;
    }

  init_ceil_sqrt_u8 ();

  memset (&pal, 0, sizeof (pal));
  pal.count = 16;
  pal.format = babl_format ("R'G'B'A u8"); /* dynamically generated, so
                                              the default palette can
                                              not be fully static.
                                            */
  pal.data = defpal_data;
  pal.data_double = defpal_double;
  pal.data_u8 = defpal_data;
  pal.radii = defpal_radii;

  babl_process (babl_fish (pal.format, babl_format ("RGBA double")),
                pal.data, pal.data_double, pal.count);

  babl_palette_init_radii (&pal);

  babl_palette_reset_hash (&pal);

  inited = 1;

  babl_mutex_unlock (babl_format_mutex);

  return &pal;
}

static void
rgba_to_pal (Babl *conversion,
             char *src_b,
             char *dst,
             long  n,
             void *dst_model_data)
{
  const Babl *space = babl_conversion_get_source_space (conversion);
  BablPalette **palptr = dst_model_data;
  BablPalette *pal;
  int best_idx = 0;
  assert (palptr);
  pal = *palptr;
  assert(pal);

  while (n--)
    {
      double *src_d = (void*) src_b;
      unsigned char src[4];
      int c;
      for (c = 0; c < 3; c++)
      {
        if (src_d[c] >= 1.0f)
          src[c] = 255;
        else if (src_d[c] <= 0.0f)
          src[c] = 0;
        else
          src[c] = babl_trc_from_linear (space->space.trc[0],
                                         src_d[c]) * 255 + 0.5f;
      }
      if (src_d[3] >= 1.0f)
        src[3] = 255;
      else if (src_d[3] <= 0.0f)
        src[3] = 0;
      else
        src[3] = src_d[3] * 255 + 0.5f;

      best_idx = babl_palette_lookup (pal, src, best_idx);

      ((double *) dst)[0] = best_idx / BABL_IDX_FACTOR;

      src_b += sizeof (double) * 4;
      dst += sizeof (double) * 1;
    }

}

static void
rgba_to_pala (Babl *conversion,
              char *src_i,
              char *dst,
              long  n,
              void *dst_model_data)
{
  const Babl *space = babl_conversion_get_destination_space (conversion);
  BablPalette **palptr = dst_model_data;
  BablPalette *pal;
  int best_idx = 0;
  assert (palptr);
  pal = *palptr;
  assert(pal);

  while (n--)
    {
      double *src_d = (void*) src_i;
      unsigned char src[4];
      int c;
      for (c = 0; c < 3; c++)
      {
        if (src_d[c] >= 1.0f)
          src[c] = 255;
        else if (src_d[c] <= 0.0f)
          src[c] = 0;
        else
          src[c] = babl_trc_from_linear (space->space.trc[0],
                                         src_d[c]) * 255 + 0.5f;
      }
      if (src_d[3] >= 1.0f)
        src[3] = 255;
      else if (src_d[3] <= 0.0f)
        src[3] = 0;
      else
        src[3] = src_d[3] * 255 + 0.5f;

      best_idx = babl_palette_lookup (pal, src, best_idx);

      ((double *) dst)[0] = best_idx / BABL_IDX_FACTOR;
      ((double *) dst)[1] = src_d[3];

      src_i += sizeof (double) * 4;
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
      int idx = (((double *) src)[0]) * BABL_IDX_FACTOR;
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
  BablPalette *pal;

  assert(palptr);
  pal  = *palptr;

  assert(pal);
  while (n--)
    {
      int idx      = (((double *) src)[0]) * BABL_IDX_FACTOR;
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
rgba_float_to_pal_a (Babl          *conversion,
                     unsigned char *src_b,
                     unsigned char *dst,
                     long           n,
                     void          *src_model_data)
{
  const Babl *space = babl_conversion_get_destination_space (conversion);
  BablPalette **palptr = src_model_data;
  BablPalette *pal;
  int best_idx = 0;
  assert (palptr);
  pal = *palptr;
  assert(pal);

  while (n--)
    {
      float *src_f = (void*) src_b;
      unsigned char src[4];
      int c;
      for (c = 0; c < 3; c++)
      {
        if (src_f[c] >= 1.0f)
          src[c] = 255;
        else if (src_f[c] <= 0.0f)
          src[c] = 0;
        else
          src[c] = babl_trc_from_linear (space->space.trc[0],
                                         src_f[c]) * 255 + 0.5f;
      }
      if (src_f[3] >= 1.0f)
        src[3] = 255;
      else if (src_f[3] <= 0.0f)
        src[3] = 0;
      else
        src[3] = src_f[3] * 255 + 0.5f;


      dst[0] = best_idx = babl_palette_lookup (pal, src, best_idx);
      dst[1] = src[3];

      src_b += sizeof (float) * 4;
      dst += sizeof (char) * 2;
    }
}


static void
rgba_float_to_pal (Babl          *conversion,
                   unsigned char *src_b,
                   unsigned char *dst,
                   long           n,
                   void          *src_model_data)
{
  const Babl *space = babl_conversion_get_destination_space (conversion);
  BablPalette **palptr = src_model_data;
  BablPalette *pal;
  int best_idx = 0;
  assert (palptr);
  pal = *palptr;
  assert(pal);

  while (n--)
    {
      float *src_f = (void*) src_b;
      unsigned char src[4];
      int c;
      for (c = 0; c < 3; c++)
      {
        if (src_f[c] >= 1.0f)
          src[c] = 255;
        else if (src_f[c] <= 0.0f)
          src[c] = 0;
        else
          src[c] = babl_trc_from_linear (space->space.trc[0],
                                         src_f[c]) * 255 + 0.5f;
      }
      if (src_f[3] >= 1.0f)
        src[3] = 255;
      else if (src_f[3] <= 0.0f)
        src[3] = 0;
      else
        src[3] = src_f[3] * 255 + 0.5f;

      dst[0] = best_idx = babl_palette_lookup (pal, src, best_idx);

      src_b += sizeof (float) * 4;
      dst += sizeof (char) * 1;
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
  int best_idx = 0;
  assert (palptr);
  pal = *palptr;
  assert(pal);

  while (n--)
    {
      dst[0] = best_idx = babl_palette_lookup (pal, src, best_idx);

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
  int best_idx = 0;
  assert (palptr);
  pal = *palptr;
  assert(pal);
  while (n--)
    {
      dst[0] = best_idx = babl_palette_lookup (pal, src, best_idx);
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
conv_pal8_pala8 (Babl          *conversion,
                 unsigned char *src, 
                 unsigned char *dst, 
                 long           samples)
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
conv_pala8_pal8 (Babl          *conversion,
                 unsigned char *src, 
                 unsigned char *dst, 
                 long           samples)
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


const Babl *
babl_new_palette_with_space (const char  *name,
                             const Babl  *space,
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

  if (!space)
    space = babl_space ("sRGB");

  if (!name)
    {
      static int cnt = 0;
      snprintf (cname, sizeof (cname), "_babl-int-%i", cnt++);
      name = cname;
    }
  else
    {
      snprintf (cname, sizeof (cname), "%s-%p", name, space);
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
  f_pal_a_u8 = (void*) babl_format_new ("name", name, model, space,
                                babl_type ("u8"),
                                component, alpha, NULL);
  cname[0] = ')';
  f_pal_u8  = (void*) babl_format_new ("name", name, model_no_alpha, space,
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

  babl_conversion_new (
     babl_format ("RGBA float"),
     f_pal_a_u8,
     "linear", rgba_float_to_pal_a,
     "data", palptr,
     NULL);
  babl_conversion_new (
     babl_format ("RGBA float"),
     f_pal_u8,
     "linear", rgba_float_to_pal,
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

/* should return the BablModel, permitting to fetch
 * other formats out of it?
 */
const Babl *
babl_new_palette (const char  *name,
                  const Babl **format_u8,
                  const Babl **format_u8_with_alpha)
{
  return babl_new_palette_with_space (name, NULL,
                                      format_u8, format_u8_with_alpha);
}

void
babl_palette_set_palette (const Babl *babl,
                          const Babl *format,
                          void       *data,
                          int         count)
{
  BablPalette **palptr = babl_get_user_data (babl);
  babl_palette_reset (babl);

  if (count > 256)
    {
      babl_log ("attempt to create a palette with %d colors. "
                "truncating to 256 colors.",
                count);

      count = 256;
    }

  if (count > 0)
    {
      *palptr = make_pal (babl_format_get_space (babl), format, data, count);
    }
  else
    {
      babl_log ("attempt to create a palette with %d colors. "
                "using default palette instead.",
                count);
    }
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
