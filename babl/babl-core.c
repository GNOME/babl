/* babl - dynamically extendable universal pixel conversion library.

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

#include "config.h"
#include <string.h>
#include "babl-internal.h"
#include "babl-ids.h"
#include "util.h"

static long
convert_double_double (const Babl *babl,
                       char       *src,
                       char       *dst,
                       int         src_pitch,
                       int         dst_pitch,
                       long        n)
{
  if (src_pitch == 64 &&
      dst_pitch == 64)
    {
      memcpy (dst, src, n / 8);
      return n;
    }

  while (n--)
    {
      (*(double *) dst) = (*(double *) src);
      dst              += dst_pitch;
      src              += src_pitch;
    }
  return n;
}

/*
   static long
   copy_strip_1 (int    src_bands,
              char **src,
              int   *src_pitch,
              int    dst_bands,
              char **dst,
              int   *dst_pitch,
              long   n)
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
   return n;
   }


 */
static long
rgba_to_rgba (const Babl *babl,
              char       *src,
              char       *dst,
              long        n)
{
  memcpy (dst, src, n * sizeof (double) * 4);
  return n;
}
void
babl_core_init (void)
{
  babl_type_new (
    "double",
    "id", BABL_DOUBLE,
    "bits", 64,
    "doc", "IEEE 754 double precision.",
    NULL);

  babl_component_new (
    "R",
    "id", BABL_RED,
    "luma",
    "chroma",
    NULL);

  babl_component_new (
    "G",
    "id", BABL_GREEN,
    "luma",
    "chroma",
    NULL);

  babl_component_new (
    "B",
    "id", BABL_BLUE,
    "luma",
    "chroma",
    NULL);

  babl_component_new (
    "A",
    "id", BABL_ALPHA,
    "alpha",
    NULL);

  babl_component_new (
    "PAD",
    "id", BABL_PADDING,
    NULL);

  babl_model_new (
    "id", BABL_RGBA,
    babl_component_from_id (BABL_RED),
    babl_component_from_id (BABL_GREEN),
    babl_component_from_id (BABL_BLUE),
    babl_component_from_id (BABL_ALPHA),
    "rgb",
    "linear",
    "alpha",
    NULL);

  /*
     babl_conversion_new (
     babl_model_from_id (BABL_RGBA),
     babl_model_from_id (BABL_RGBA),
     "planar",      copy_strip_1,
     NULL
     );
   */

  babl_conversion_new (
    babl_type_from_id (BABL_DOUBLE),
    babl_type_from_id (BABL_DOUBLE),
    "plane", convert_double_double,
    NULL
  );

  babl_conversion_new (
    babl_model_from_id (BABL_RGBA),
    babl_model_from_id (BABL_RGBA),
    "linear", rgba_to_rgba,
    NULL
  );
}


/////////////////// temporary here
///////////////////

const Babl * 
babl_trc_lut (const char *name, 
              int         n, 
              float      *entries)
{
  return babl_trc_new (name, BABL_TRC_LUT, 0, n, entries);
}


const Babl *
babl_trc_formula_srgb (double g, 
                       double a, 
                       double b, 
                       double c, 
                       double d,
                       double e,
                       double f)
{
  char name[128];
  float params[7]={g, a, b, c, d, e, f};

  if (fabs (g - 2.400) < 0.01 &&
      fabs (a - 0.947) < 0.01 &&
      fabs (b - 0.052) < 0.01 &&
      fabs (c - 0.077) < 0.01 &&
      fabs (d - 0.040) < 0.01 &&
      fabs (e - 0.000) < 0.01 &&
      fabs (f - 0.000) < 0.01
      )
    return babl_trc ("sRGB");

  snprintf (name, sizeof (name)-1, "%i.%06i %i.%06i %i.%04i %i.%04i %i.%04i %i.%04i %i.%04i",
            (int)(g), (int)((g-(int)g) * 1000000),
            (int)(a), (int)((a-(int)a) * 1000000),
            (int)(b), (int)((b-(int)b) * 10000),
            (int)(c), (int)((c-(int)c) * 10000),
            (int)(d), (int)((d-(int)d) * 10000),
            (int)(e), (int)((e-(int)e) * 10000),
            (int)(f), (int)((f-(int)f) * 10000));

  while (name[strlen(name)-1]=='0')
    name[strlen(name)-1]='\0';
  return babl_trc_new (name, BABL_TRC_FORMULA_SRGB, g, 0, params);
}

const Babl *
babl_trc_formula_cie (double g, 
                      double a, 
                      double b, 
                      double c)
{
  char name[128];
  float params[4]={g, a, b, c};

  snprintf (name, sizeof (name)-1, "%i.%06i  %i.%06i %i.%04i %i.%04i",
            (int)(g), (int)((g-(int)g) * 1000000),
            (int)(a), (int)((a-(int)a) * 1000000),
            (int)(b), (int)((b-(int)b) * 10000),
            (int)(c), (int)((c-(int)c) * 10000));

  while (name[strlen(name)-1]=='0')
    name[strlen(name)-1]='\0';
  return babl_trc_new (name, BABL_TRC_FORMULA_CIE, g, 0, params);
}


const Babl *
babl_trc_gamma (double gamma)
{
  char name[32];
  if (fabs (gamma - 1.0) < 0.01)
     return babl_trc_new ("linear", BABL_TRC_LINEAR, 1.0, 0, NULL);

  snprintf (name, sizeof (name)-1, "%i.%06i",
            (int)(gamma), (int)((gamma-(int)gamma) * 1000000));

  while (name[strlen(name)-1]=='0')
    name[strlen(name)-1]='\0';
  return babl_trc_new (name, BABL_TRC_FORMULA_GAMMA, gamma, 0, NULL);
}

void
babl_trc_class_init (void)
{
  babl_trc_new ("sRGB",  BABL_TRC_SRGB, 2.2, 0, NULL);
  babl_trc_gamma (2.2);
  babl_trc_gamma (1.8);
  babl_trc_gamma (1.0);
  babl_trc_new ("linear", BABL_TRC_LINEAR, 1.0, 0, NULL);
}

#if 0
float 
babl_trc_from_linear (const Babl *trc_, 
                      float       value)
{
  return babl_trc_from_linear (trc_, value);
}

float 
babl_trc_to_linear (const Babl *trc_,
                    float       value)
{
  return babl_trc_to_linear (trc_, value);
}
#endif

static int
babl_lut_match_gamma (float *lut, 
                      int    lut_size, 
                      float  gamma)
{
  int match = 1;
  int i;
  if (lut_size > 1024)
  {
    for (i = 0; match && i < lut_size; i++)
    {
      if (fabs (lut[i] - pow ((i / (lut_size-1.0)), gamma)) > 0.0001)
        match = 0;
    }
  }
  else
  {
    for (i = 0; match && i < lut_size; i++)
    {
      if (fabs (lut[i] - pow ((i / (lut_size-1.0)), gamma)) > 0.001)
        match = 0;
    }
  }
  return match;
}

const Babl *
babl_trc_lut_find (float *lut, 
                   int    lut_size)
{
  int i;
  int match = 1;

  /* look for linear match */
  for (i = 0; match && i < lut_size; i++)
    if (fabs (lut[i] - i / (lut_size-1.0)) > 0.015)
      match = 0;
  if (match)
    return babl_trc_gamma (1.0);

  /* look for sRGB match: */
  match = 1;
  if (lut_size > 1024)
  {
    for (i = 0; match && i < lut_size; i++)
    {
      if (fabs (lut[i] - gamma_2_2_to_linear (i / (lut_size-1.0))) > 0.0001)
        match = 0;
    }
  }
  else
  {
    for (i = 0; match && i < lut_size; i++)
    {
      if (fabs (lut[i] - gamma_2_2_to_linear (i / (lut_size-1.0))) > 0.001)
        match = 0;
    }
  }
  if (match)
    return babl_trc ("sRGB");

  if (babl_lut_match_gamma (lut, lut_size, 2.2))
    return babl_trc_gamma(2.2);

  if (babl_lut_match_gamma (lut, lut_size, 1.8))
    return babl_trc_gamma(1.8);

  return NULL;
}

const Babl * babl_trc (const char *name)
{
  return babl_trc_lookup_by_name (name);
}

