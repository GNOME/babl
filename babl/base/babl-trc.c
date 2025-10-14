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
#include "babl-base.h"
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
    diff = 0.0f;
  }
  else if (entry < 0) entry = 0;

  if (diff > 0.0f)
  {
    ret = trc->inv_lut[entry] * (1.0f - diff) + trc->inv_lut[entry+1] * diff;
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

  if (diff > 0.0f && entry < trc->lut_size - 1)
  {
    ret = trc->lut[entry] * (1.0f - diff) + trc->lut[entry+1] * diff;
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
                               const float *__restrict__ in, 
                               float       *__restrict__ out, 
                               int          in_gap, 
                               int          out_gap, 
                               int          components, 
                               int          count)
{
  if (in_gap == out_gap && in_gap == 4 && components == 3)
  {
    for (int i = 0; i < count; i ++)
      for (int c = 0; c < 3; c ++)
        out[4 * i + c] = _babl_trc_gamma_to_linear (trc_, in[4 *i + c]);
  }
  else
  {
    for (int i = 0; i < count; i ++)
      for (int c = 0; c < components; c ++)
        out[out_gap * i + c] = _babl_trc_gamma_to_linear (trc_, in[in_gap *i + c]);
  }
}

static inline void 
_babl_trc_gamma_from_linear_buf (const Babl  *trc_, 
                                 const float *__restrict__ in, 
                                 float       *__restrict__ out, 
                                 int          in_gap, 
                                 int          out_gap, 
                                 int          components, 
                                 int          count)
{
  if (in_gap == out_gap && in_gap == 4 && components == 3)
  {
    for (int i = 0; i < count; i ++)
      for (int c = 0; c < 3; c ++)
        out[4 * i + c] = _babl_trc_gamma_from_linear (trc_, in[4 *i + c]);
  }
  else
  {
    for (int i = 0; i < count; i ++)
      for (int c = 0; c < components; c ++)
        out[out_gap * i + c] = _babl_trc_gamma_from_linear (trc_, in[in_gap *i + c]);
  }
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
  float e = trc->lut[5];
  float f = trc->lut[6];

  if (x - f > c * d)
  {
    float v = _babl_trc_gamma_from_linear ((Babl *) trc, x - f);
    v = (v-b)/a;
    if (!isnan(v))
      return v;
    return 0.0f;
  }
  if (c > 0.0f)
    return (x - e) / c;
  return 0.0f;
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
  float e = trc->lut[5];
  float f = trc->lut[6];

  if (x >= d)
  {
    return _babl_trc_gamma_to_linear ((Babl *) trc, a * x + b) + e;
  }
  return c * x + f;
}
static inline float 
_babl_trc_formula_cie_from_linear (const Babl *trc_, 
                                   float       value)
{
  BablTRC *trc = (void*)trc_;
  float x= value;
  float a = trc->lut[1];
  float b = trc->lut[2];
  float c = trc->lut[3];

  if (x > c)
  {
    float v = _babl_trc_gamma_from_linear ((Babl *) trc, x - c);
    v = (v-b)/a;
    if (!isnan(v))
      return v;
  }
  return 0.0f;
}

static inline float 
_babl_trc_formula_cie_to_linear (const Babl *trc_, 
                                 float       value)
{
  BablTRC *trc = (void*)trc_;
  float x= value;
  float a = trc->lut[1];
  float b = trc->lut[2];
  float c = trc->lut[3];

  if (x >= -b / a)
  {
    return _babl_trc_gamma_to_linear ((Babl *) trc, a * x + b) + c;
  }
  return c;
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
  if (in_gap == out_gap && in_gap == 4 && components == 3)
  {
  for (int i = 0; i < count; i ++)
    for (int c = 0; c < 3; c++)
      out[4 * i + c] = babl_gamma_2_2_to_linearf (in[4 * i + c]);
  }
  else
  {
  for (int i = 0; i < count; i ++)
    for (int c = 0; c < components; c++)
      out[out_gap * i + c] = babl_gamma_2_2_to_linearf (in[in_gap * i + c]);
  }
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
  if (in_gap == out_gap && in_gap == 4 && components == 3)
  {
      for (int i = 0; i < count; i ++)
       for (int c = 0; c < 3; c++)
         out[4 * i + c] = babl_linear_to_gamma_2_2f (in[4 * i + c]);
  }
  else
  {
     for (int i = 0; i < count; i ++)
       for (int c = 0; c < components; c++)
         out[out_gap * i + c] = babl_linear_to_gamma_2_2f (in[in_gap * i + c]);
  }
}

static inline void 
_babl_trc_to_linear_buf_generic (const Babl  *trc_, 
                                 const float *__restrict__ in, 
                                 float       *__restrict__ out, 
                                 int          in_gap, 
                                 int          out_gap, 
                                 int          components, 
                                 int          count)
{
  BablTRC *trc = (void*)trc_;
  if (in_gap == out_gap && in_gap == 4 && components == 3)
  {
    for (int i = 0; i < count; i ++)
      for (int c = 0; c < 3; c ++)
        out[4 * i + c] = trc->fun_to_linear (trc_, in[4 * i + c]);
  }
  else
  {
    for (int i = 0; i < count; i ++)
      for (int c = 0; c < components; c ++)
        out[out_gap * i + c] = trc->fun_to_linear (trc_, in[in_gap * i + c]);
  }
}

static inline void 
_babl_trc_from_linear_buf_generic (const Babl  *trc_,
                                   const float *__restrict__ in, 
                                   float       *__restrict__ out,
                                   int          in_gap, 
                                   int          out_gap,
                                   int          components,
                                   int          count)
{
  BablTRC *trc = (void*)trc_;
  if (in_gap == out_gap && in_gap == 4 && components == 3)
  {
    for (int i = 0; i < count; i ++)
      for (int c = 0; c < 3; c ++)
        out[4 * i + c] = trc->fun_from_linear (trc_, in[4 * i + c]);
  }
  else
  {
    for (int i = 0; i < count; i ++)
      for (int c = 0; c < components; c ++)
        out[out_gap * i + c] = trc->fun_from_linear (trc_, in[in_gap * i + c]);
  }
}



static inline void _babl_trc_linear_buf (const Babl  *trc_,
                                         const float *__restrict__ in, 
                                         float       *__restrict__ out,
                                         int          in_gap, 
                                         int          out_gap,
                                         int          components,
                                         int          count)
{
  if (in_gap == out_gap && in_gap == 4 && components == 3)
  {
     for (int i = 0; i < count; i ++)
       for (int c = 0; c < 3; c ++)
         out[i * 4 + c] = in[i * 4 + c];
  }
  else
  {
    for (int i = 0; i < count; i ++)
      for (int c = 0; c < components; c ++)
        out[i * out_gap + c] = in[i * in_gap + c];
  }
}

const Babl *
BABL_SIMD_SUFFIX (babl_trc_lookup_by_name) (const char *name);

const Babl *
BABL_SIMD_SUFFIX (babl_trc_lookup_by_name) (const char *name)
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
BABL_SIMD_SUFFIX (babl_trc_new) (const char *name,
              BablTRCType type,
              double      gamma,
              int         n_lut,
              float      *lut);


const Babl *
BABL_SIMD_SUFFIX (babl_trc_new) (const char *name,
              BablTRCType type,
              double      gamma,
              int         n_lut,
              float      *lut)
{
  int i=0;
  BablTRC trc;
  memset (&trc, 0, sizeof(trc));
  trc.instance.class_type = BABL_TRC;
  trc.instance.id         = 0;
  trc.type = type;
  trc.gamma  = gamma > 0.0    ? gamma       : 0.0;
  trc.rgamma = gamma > 0.0001 ? 1.0 / gamma : 0.0;
  if(name)
    strncpy (trc.name, name, sizeof (trc.name) - 1);

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
    snprintf (trc_db[i].name, sizeof (trc_db[i].name) - 1, "%s", name);
  else if (n_lut)
    snprintf (trc_db[i].name, sizeof (trc_db[i].name) - 1, "lut-trc");
  else
    snprintf (trc_db[i].name, sizeof (trc_db[i].name) - 1, "trc-%i-%f", type, gamma);

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
        float reversed_index = babl_trc_lut_to_linear (BABL(&trc_db[i]), guess) * (n_lut-1.0f);

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
    case BABL_TRC_FORMULA_CIE:
      trc_db[i].lut = babl_calloc (sizeof (float), 4);
      {
        int j;
        for (j = 0; j < 4; j++)
          trc_db[i].lut[j] = lut[j];
      }
      trc_db[i].fun_to_linear = _babl_trc_formula_cie_to_linear;
      trc_db[i].fun_from_linear = _babl_trc_formula_cie_from_linear;

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

    case BABL_TRC_FORMULA_SRGB:
      trc_db[i].lut = babl_calloc (sizeof (float), 7);
      {
        int j;
        for (j = 0; j < 7; j++)
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

void
BABL_SIMD_SUFFIX(babl_trc_class_for_each) (BablEachFunction each_fun,
                                           void            *user_data);

void
BABL_SIMD_SUFFIX(babl_trc_class_for_each) (BablEachFunction each_fun,
                                           void            *user_data)
{
  int i=0;
  for (i = 0; trc_db[i].instance.class_type; i++)
    if (each_fun (BABL (&trc_db[i]), user_data))
      return;
}

