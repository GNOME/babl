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
 * <http://www.gnu.org/licenses/>.
 */

#define MAX_TRCS   100

#include "config.h"
#include "babl-internal.h"
#include "base/util.h"

static BablTRC trc_db[MAX_TRCS];

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
  trc.gamma = gamma;
  if (gamma > 0.0001)
    trc.rgamma = 1.0 / gamma;

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
    sprintf (trc_db[i].name, "%s", name);
  else if (n_lut)
    sprintf (trc_db[i].name, "lut-trc");
  else
    sprintf (trc_db[i].name, "trc-%i-%f", type, gamma);

  if (n_lut)
  {
    int j;
    trc_db[i].lut_size = n_lut;
    trc_db[i].lut = babl_calloc (sizeof (float), n_lut);
    memcpy (trc_db[i].lut, lut, sizeof (float) * n_lut);
    trc_db[i].inv_lut = babl_calloc (sizeof (float), n_lut);
    for (j = 0; j < n_lut; j++)
      trc_db[i].inv_lut[j] =
        babl_trc_to_linear (BABL(&trc_db[i]), trc_db[i].lut[(int) ( j/(n_lut-1.0) * (n_lut-1))]);
  }

  switch (trc_db[i].type)
  {
    case BABL_TRC_LINEAR:
      trc_db[i].fun_to_linear = _babl_trc_linear;
      trc_db[i].fun_from_linear = _babl_trc_linear;
      break;
    case BABL_TRC_GAMMA:
      trc_db[i].fun_to_linear = _babl_trc_gamma_to_linear;
      trc_db[i].fun_from_linear = _babl_trc_gamma_from_linear;
      break;
    case BABL_TRC_GAMMA_2_2:
      trc_db[i].fun_to_linear = _babl_trc_gamma_2_2_to_linear;
      trc_db[i].fun_from_linear = _babl_trc_gamma_2_2_from_linear;
      break;
    case BABL_TRC_GAMMA_1_8:
      trc_db[i].fun_to_linear = _babl_trc_gamma_1_8_to_linear;
      trc_db[i].fun_from_linear = _babl_trc_gamma_1_8_from_linear;
      break;
    case BABL_TRC_SRGB:
      trc_db[i].fun_to_linear = _babl_trc_srgb_to_linear;
      trc_db[i].fun_from_linear = _babl_trc_srgb_from_linear;
      break;
    case BABL_TRC_LUT:
      trc_db[i].fun_to_linear = babl_trc_lut_to_linear;
      trc_db[i].fun_from_linear = babl_trc_lut_from_linear;
      break;
  }
  return (Babl*)&trc_db[i];
}

const Babl * babl_trc_lut   (const char *name, int n, float *entries)
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
babl_trc_gamma (double gamma)
{
  char name[32];
  int i;
  if (fabs (gamma - 1.0) < 0.01)
     return babl_trc_new ("linear", BABL_TRC_LINEAR, 1.0, 0, NULL);
  if (fabs (gamma - 1.8) < 0.01)
     return babl_trc_new ("1.8", BABL_TRC_GAMMA_1_8, 1.8, 0, NULL);
  if (fabs (gamma - 2.2) < 0.01)
     return babl_trc_new ("2.2", BABL_TRC_GAMMA_2_2, 2.2, 0, NULL);

  sprintf (name, "%.6f", gamma);
  for (i = 0; name[i]; i++)
    if (name[i] == ',') name[i] = '.';
  while (name[strlen(name)-1]=='0')
    name[strlen(name)-1]='\0';
  return babl_trc_new (name, BABL_TRC_GAMMA, gamma, 0, NULL);
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

float babl_trc_from_linear (const Babl *trc_, float value)
{
  return _babl_trc_from_linear (trc_, value);
}

float babl_trc_to_linear (const Babl *trc_, float value)
{
  return _babl_trc_to_linear (trc_, value);
}

const Babl *babl_trc_lut_find (float *lut, int lut_size)
{
  int i;
  int match = 1;

  /* look for linear match */
  for (i = 0; match && i < lut_size; i++)
    if (fabs (lut[i] - i / (lut_size-1.0)) > 0.015)
      match = 0;
  if (match)
    return babl_trc_gamma (1.0);

  /* look for 2.2 match: */
  match = 1;
  if (lut_size > 1024)
  {
  for (i = 0; match && i < lut_size; i++)
  {
#if 0
    fprintf (stderr, "%i %f %f\n", i,
                   lut[i],
                   pow ((i / (lut_size-1.0)), 2.2));
#endif
    if (fabs (lut[i] - pow ((i / (lut_size-1.0)), 2.2)) > 0.0001)
      match = 0;
  }
  }
  else
  {
    for (i = 0; match && i < lut_size; i++)
    {
    if (fabs (lut[i] - pow ((i / (lut_size-1.0)), 2.2)) > 0.001)
        match = 0;
    }
  }
  if (match)
    return babl_trc_gamma(2.2);


  /* look for 1.8 match: */
  match = 1;
  if (lut_size > 1024)
  {
  for (i = 0; match && i < lut_size; i++)
  {
#if 0
    fprintf (stderr, "%i %f %f\n", i,
                   lut[i],
                   pow ((i / (lut_size-1.0)), 1.8));
#endif
    if (fabs (lut[i] - pow ((i / (lut_size-1.0)), 1.8)) > 0.0001)
      match = 0;
  }
  }
  else
  {
    for (i = 0; match && i < lut_size; i++)
    {
    if (fabs (lut[i] - pow ((i / (lut_size-1.0)), 1.8)) > 0.001)
        match = 0;
    }
  }
  if (match)
    return babl_trc_gamma(2.2);


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

  return NULL;
}

