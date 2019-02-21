/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2017 Øyvind Kolås.
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

#define MAX_TRCS   100

/* FIXME: choose parameters more intelligently */
#define POLY_GAMMA_X0     (  0.5 / 255.0)
#define POLY_GAMMA_X1     (254.5 / 255.0)
#define POLY_GAMMA_DEGREE 6
#define POLY_GAMMA_SCALE  2

#include "config.h"
#include "babl-internal.h"
#include "base/util.h"

static BablTRC trc_db[MAX_TRCS];

static inline float 
_babl_trc_linear (const Babl *trc_, 
                  float       value)
{
  return value;
}

static inline float 
babl_trc_lut_from_linear (const Babl *trc_, 
                          float       x)
{
  BablTRC *trc = (void*)trc_;
  int entry;
  float ret, diff;

  entry = x * (trc->lut_size-1);
  diff =  ( (x * (trc->lut_size-1)) - entry);

  if (entry >= trc->lut_size -1)
  {
    entry = trc->lut_size - 1;
    diff = 0.0;
  }
  else if (entry < 0) entry = 0;

  if (diff > 0.0)
  {
    ret = trc->inv_lut[entry] * (1.0 - diff) + trc->inv_lut[entry+1] * diff;
  }
  else
  {
    ret = trc->inv_lut[entry];
  }
  return ret;
}

static inline float 
babl_trc_lut_to_linear (const Babl *trc_, 
                        float       x)
{
  BablTRC *trc = (void*)trc_;
  int entry;
  float ret, diff;

  entry = x * (trc->lut_size-1);
  diff =  ( (x * (trc->lut_size-1)) - entry);

  if (entry >= trc->lut_size) entry = trc->lut_size - 1;
  else if (entry < 0) entry = 0;

  if (diff > 0.0 && entry < trc->lut_size - 1)
  {
    ret = trc->lut[entry] * (1.0 - diff) + trc->lut[entry+1] * diff;
  }
  else
  {
    ret = trc->lut[entry];
  }
  return ret;
}

static inline float 
_babl_trc_gamma_to_linear (const Babl *trc_, 
                           float       value)
{
  BablTRC *trc = (void*)trc_;
  if (value >= trc->poly_gamma_to_linear_x0 &&
      value <= trc->poly_gamma_to_linear_x1)
    {
      return babl_polynomial_eval (&trc->poly_gamma_to_linear, value);
    }
  else if (value > 0.0f)
    {
      return powf (value, trc->gamma);
    }
  return 0.0f;
}

static inline float 
_babl_trc_gamma_from_linear (const Babl *trc_, 
                             float       value)
{
  BablTRC *trc = (void*)trc_;
  if (value >= trc->poly_gamma_from_linear_x0 &&
      value <= trc->poly_gamma_from_linear_x1)
    {
      return babl_polynomial_eval (&trc->poly_gamma_from_linear, value);
    }
  else if (value > 0.0f)
    {
      return powf (value, trc->rgamma);
    }
  return 0.0f;
}

static inline void 
_babl_trc_gamma_to_linear_buf (const Babl  *trc_, 
                               const float *in, 
                               float       *out, 
                               int          in_gap, 
                               int          out_gap, 
                               int          components, 
                               int          count)
{
  int i, c;
  for (i = 0; i < count; i ++)
    for (c = 0; c < components; c ++)
      out[out_gap * i + c] = _babl_trc_gamma_to_linear (trc_, in[in_gap *i + c]);
}

static inline void 
_babl_trc_gamma_from_linear_buf (const Babl  *trc_, 
                                 const float *in, 
                                 float       *out, 
                                 int          in_gap, 
                                 int          out_gap, 
                                 int          components, 
                                 int          count)
{
  int i, c;
  for (i = 0; i < count; i ++)
    for (c = 0; c < components; c ++)
      out[out_gap * i + c] = _babl_trc_gamma_from_linear (trc_, in[in_gap *i + c]);
}

static inline float 
_babl_trc_formula_srgb_from_linear (const Babl *trc_, 
                                    float       value)
{
  BablTRC *trc = (void*)trc_;
  float x= value;
  float a = trc->lut[1];
  float b = trc->lut[2];
  float c = trc->lut[3];
  float d = trc->lut[4];
  if (x > c * d)  // XXX: verify that this math is the correct inverse
  {
    float v = _babl_trc_gamma_from_linear ((Babl *) trc, x);
    v = (v-b)/a;
    if (v < 0.0 || v >= 0.0)
      return v;
    return 0.0;
  }
  if (c > 0.0)
    return x / c;
  return 0.0;
}

static inline float 
_babl_trc_formula_srgb_to_linear (const Babl *trc_, 
                                  float       value)
{
  BablTRC *trc = (void*)trc_;
  float x= value;
  float a = trc->lut[1];
  float b = trc->lut[2];
  float c = trc->lut[3];
  float d = trc->lut[4];

  if (x >= d)
  {
    return _babl_trc_gamma_to_linear ((Babl *) trc, a * x + b);
  }
  return c * x;
}

static inline float 
_babl_trc_srgb_to_linear (const Babl *trc_, 
                          float       value)
{
  return babl_gamma_2_2_to_linearf (value);
}

static inline float 
_babl_trc_srgb_from_linear (const Babl *trc_, 
                            float       value)
{
  return babl_linear_to_gamma_2_2f (value);
}

static inline void 
_babl_trc_srgb_to_linear_buf (const Babl  *trc_, 
                              const float *in, 
                              float       *out, 
                              int          in_gap, 
                              int          out_gap, 
                              int          components, 
                              int          count)
{
  int i, c;
  for (i = 0; i < count; i ++)
    for (c = 0; c < components; c++)
      out[out_gap * i + c] = babl_gamma_2_2_to_linearf (in[in_gap * i + c]);
}

static inline void 
_babl_trc_srgb_from_linear_buf (const Babl  *trc_,
                                const float *in, 
                                float       *out,
                                int          in_gap,
                                int          out_gap,
                                int          components,
                                int          count)
{
  int i, c;
  for (i = 0; i < count; i ++)
    for (c = 0; c < components; c++)
      out[out_gap * i + c] = babl_linear_to_gamma_2_2f (in[in_gap * i + c]);
}

static inline void 
_babl_trc_to_linear_buf_generic (const Babl  *trc_, 
                                 const float *in, 
                                 float       *out, 
                                 int          in_gap, 
                                 int          out_gap, 
                                 int          components, 
                                 int          count)
{
  int i, c;
  BablTRC *trc = (void*)trc_;
  for (i = 0; i < count; i ++)
    for (c = 0; c < components; c ++)
      out[out_gap * i + c] = trc->fun_to_linear (trc_, in[in_gap * i + c]);
}

static inline void 
_babl_trc_from_linear_buf_generic (const Babl  *trc_,
                                   const float *in, 
                                   float       *out,
                                   int          in_gap, 
                                   int          out_gap,
                                   int          components,
                                   int          count)
{
  int i, c;
  BablTRC *trc = (void*)trc_;
  for (i = 0; i < count; i ++)
    for (c = 0; c < components; c ++)
      out[out_gap * i + c] = trc->fun_from_linear (trc_, in[in_gap * i + c]);
}

static inline void _babl_trc_linear_buf (const Babl  *trc_,
                                         const float *in, 
                                         float       *out,
                                         int          in_gap, 
                                         int          out_gap,
                                         int          components,
                                         int          count)
{
  int i, c;
  for (i = 0; i < count; i ++)
    for (c = 0; c < components; c ++)
      out[i * out_gap + c] = in[i * in_gap + c];
}


const Babl *
babl_trc (const char *name)
{
  int i;
  for (i = 0; trc_db[i].instance.class_type; i++)
    if (!strcmp (trc_db[i].instance.name, name))
    {
      return (Babl*)&trc_db[i];
    }
  babl_log("failed to find trc '%s'\n", name);
  return NULL;
}

const Babl *
babl_trc_new (const char *name,
              BablTRCType type,
              double      gamma,
              int         n_lut,
              float      *lut)
{
  int i=0;
  static BablTRC trc;
  trc.instance.class_type = BABL_TRC;
  trc.instance.id         = 0;
  trc.type = type;
  trc.gamma  = gamma > 0.0    ? gamma       : 0.0;
  trc.rgamma = gamma > 0.0001 ? 1.0 / gamma : 0.0;

  if (n_lut )
  {
    for (i = 0; trc_db[i].instance.class_type; i++)
    {
    if ( trc_db[i].lut_size == n_lut &&
         (memcmp (trc_db[i].lut, lut, sizeof (float) * n_lut)==0)
       )
      {
        return (void*)&trc_db[i];
      }
    }
  }
  else
  for (i = 0; trc_db[i].instance.class_type; i++)
  {
    int offset = ((char*)&trc_db[i].type) - (char*)(&trc_db[i]);
    int size   = ((char*)&trc_db[i].gamma + sizeof(double)) - ((char*)&trc_db[i].type);

    if (memcmp ((char*)(&trc_db[i]) + offset, ((char*)&trc) + offset, size)==0)
      {
        return (void*)&trc_db[i];
      }
  }
  if (i >= MAX_TRCS-1)
  {
    babl_log ("too many BablTRCs");
    return NULL;
  }
  trc_db[i]=trc;
  trc_db[i].instance.name = trc_db[i].name;
  if (name)
    snprintf (trc_db[i].name, sizeof (trc_db[i].name), "%s", name);
  else if (n_lut)
    snprintf (trc_db[i].name, sizeof (trc_db[i].name), "lut-trc");
  else
    snprintf (trc_db[i].name, sizeof (trc_db[i].name), "trc-%i-%f", type, gamma);

  if (n_lut)
  {
    int j;
    trc_db[i].lut_size = n_lut;
    trc_db[i].lut = babl_calloc (sizeof (float), n_lut);
    memcpy (trc_db[i].lut, lut, sizeof (float) * n_lut);
    trc_db[i].inv_lut = babl_calloc (sizeof (float), n_lut);

    for (j = 0; j < n_lut; j++)
    {
      int k;
      double min = 0.0;
      double max = 1.0;
      for (k = 0; k < 16; k++)
      {
        double guess = (min + max) / 2;
        float reversed_index = babl_trc_lut_to_linear (BABL(&trc_db[i]), guess) * (n_lut-1.0);

        if (reversed_index < j)
        {
          min = guess;
        }
        else if (reversed_index > j)
        {
          max = guess;
        }
      }
      trc_db[i].inv_lut[j] = (min + max) / 2;
    }
  }

  trc_db[i].fun_to_linear_buf = _babl_trc_to_linear_buf_generic;
  trc_db[i].fun_from_linear_buf = _babl_trc_from_linear_buf_generic;

  switch (trc_db[i].type)
  {
    case BABL_TRC_LINEAR:
      trc_db[i].fun_to_linear = _babl_trc_linear;
      trc_db[i].fun_from_linear = _babl_trc_linear;
      trc_db[i].fun_from_linear_buf = _babl_trc_linear_buf;
      trc_db[i].fun_to_linear_buf = _babl_trc_linear_buf;
      break;
    case BABL_TRC_FORMULA_GAMMA:
      trc_db[i].fun_to_linear = _babl_trc_gamma_to_linear;
      trc_db[i].fun_from_linear = _babl_trc_gamma_from_linear;
      trc_db[i].fun_to_linear_buf = _babl_trc_gamma_to_linear_buf;
      trc_db[i].fun_from_linear_buf = _babl_trc_gamma_from_linear_buf;

      trc_db[i].poly_gamma_to_linear_x0 = POLY_GAMMA_X0;
      trc_db[i].poly_gamma_to_linear_x1 = POLY_GAMMA_X1;
      babl_polynomial_approximate_gamma (&trc_db[i].poly_gamma_to_linear,
                                         trc_db[i].gamma,
                                         trc_db[i].poly_gamma_to_linear_x0,
                                         trc_db[i].poly_gamma_to_linear_x1,
                                         POLY_GAMMA_DEGREE, POLY_GAMMA_SCALE);

      trc_db[i].poly_gamma_from_linear_x0 = POLY_GAMMA_X0;
      trc_db[i].poly_gamma_from_linear_x1 = POLY_GAMMA_X1;
      babl_polynomial_approximate_gamma (&trc_db[i].poly_gamma_from_linear,
                                         trc_db[i].rgamma,
                                         trc_db[i].poly_gamma_from_linear_x0,
                                         trc_db[i].poly_gamma_from_linear_x1,
                                         POLY_GAMMA_DEGREE, POLY_GAMMA_SCALE);
      break;
    case BABL_TRC_FORMULA_SRGB:
      trc_db[i].lut = babl_calloc (sizeof (float), 5);
      {
        int j;
        for (j = 0; j < 5; j++)
          trc_db[i].lut[j] = lut[j];
      }
      trc_db[i].fun_to_linear = _babl_trc_formula_srgb_to_linear;
      trc_db[i].fun_from_linear = _babl_trc_formula_srgb_from_linear;

      trc_db[i].poly_gamma_to_linear_x0 = lut[4];
      trc_db[i].poly_gamma_to_linear_x1 = POLY_GAMMA_X1;
      babl_polynomial_approximate_gamma (&trc_db[i].poly_gamma_to_linear,
                                         trc_db[i].gamma,
                                         trc_db[i].poly_gamma_to_linear_x0,
                                         trc_db[i].poly_gamma_to_linear_x1,
                                         POLY_GAMMA_DEGREE, POLY_GAMMA_SCALE);

      trc_db[i].poly_gamma_from_linear_x0 = lut[3] * lut[4];
      trc_db[i].poly_gamma_from_linear_x1 = POLY_GAMMA_X1;
      babl_polynomial_approximate_gamma (&trc_db[i].poly_gamma_from_linear,
                                         trc_db[i].rgamma,
                                         trc_db[i].poly_gamma_from_linear_x0,
                                         trc_db[i].poly_gamma_from_linear_x1,
                                         POLY_GAMMA_DEGREE, POLY_GAMMA_SCALE);
      break;
    case BABL_TRC_SRGB:
      trc_db[i].fun_to_linear = _babl_trc_srgb_to_linear;
      trc_db[i].fun_from_linear = _babl_trc_srgb_from_linear;
      trc_db[i].fun_from_linear_buf = _babl_trc_srgb_from_linear_buf;
      trc_db[i].fun_to_linear_buf = _babl_trc_srgb_to_linear_buf;
      break;
    case BABL_TRC_LUT:
      trc_db[i].fun_to_linear = babl_trc_lut_to_linear;
      trc_db[i].fun_from_linear = babl_trc_lut_from_linear;
      break;
  }
  return (Babl*)&trc_db[i];
}

const Babl * 
babl_trc_lut (const char *name, 
              int         n, 
              float      *entries)
{
  return babl_trc_new (name, BABL_TRC_LUT, 0, n, entries);
}

void
babl_trc_class_for_each (BablEachFunction each_fun,
                         void            *user_data)
{
  int i=0;
  for (i = 0; trc_db[i].instance.class_type; i++)
    if (each_fun (BABL (&trc_db[i]), user_data))
      return;
}

const Babl *
babl_trc_formula_srgb (double g, 
                       double a, 
                       double b, 
                       double c, 
                       double d)
{
  char name[128];
  int i;
  float params[5]={g, a, b, c, d};

  if (fabs (g - 2.400) < 0.01 &&
      fabs (a - 0.947) < 0.01 &&
      fabs (b - 0.052) < 0.01 &&
      fabs (c - 0.077) < 0.01 &&
      fabs (d - 0.040) < 0.01)
    return babl_trc ("sRGB");

  snprintf (name, sizeof (name), "%.6f %.6f %.4f %.4f %.4f", g, a, b, c, d);
  for (i = 0; name[i]; i++)
    if (name[i] == ',') name[i] = '.';
  while (name[strlen(name)-1]=='0')
    name[strlen(name)-1]='\0';
  return babl_trc_new (name, BABL_TRC_FORMULA_SRGB, g, 0, params);
}

const Babl *
babl_trc_gamma (double gamma)
{
  char name[32];
  int i;
  if (fabs (gamma - 1.0) < 0.01)
     return babl_trc_new ("linear", BABL_TRC_LINEAR, 1.0, 0, NULL);

  snprintf (name, sizeof (name), "%.6f", gamma);
  for (i = 0; name[i]; i++)
    if (name[i] == ',') name[i] = '.';
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
