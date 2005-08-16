/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2005, Øyvind Kolås.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <string.h>
#include <stdarg.h>

#include "babl-internal.h"
#include "babl-sampling.h"
#include "babl-type.h"
#include "babl-component.h"
#include "babl-db.h"

#define BABL_MAX_BANDS 32

static int 
each_babl_pixel_format_destroy (Babl *babl,
                                void *data)
{
  babl_free (babl->pixel_format.from);
  babl_free (babl->pixel_format.to);
  babl_free (babl->pixel_format.component);
  babl_free (babl->pixel_format.type);
  babl_free (babl->pixel_format.sampling);
  babl_free (babl->pixel_format.model);
  babl_free (babl->instance.name);
  babl_free (babl);

  return 0;  /* continue iterating */
}

static Babl *
pixel_format_new (const char     *name,
                  int             id,
                  int             planar,
                  int             bands,
                  BablModel     **model,
                  BablComponent **component,
                  BablSampling  **sampling,
                  BablType      **type)
{
  Babl *babl;
  int              band;

  babl                     = babl_calloc (sizeof (BablPixelFormat), 1);

  babl->class_type    = BABL_PIXEL_FORMAT;
  babl->instance.id   = id;
  babl->instance.name = babl_strdup (name);

  babl->pixel_format.bands    = bands;
  babl->pixel_format.planar   = planar;

  babl->pixel_format.model     = babl_malloc (sizeof (BablModel*)     * (bands+1));
  babl->pixel_format.component = babl_malloc (sizeof (BablComponent*) * (bands+1));
  babl->pixel_format.type      = babl_malloc (sizeof (BablType*)      * (bands+1));
  babl->pixel_format.sampling  = babl_malloc (sizeof (BablSampling*)  * (bands+1));

  for (band=0; band < bands; band++)
    {
      babl->pixel_format.model[band] = model[band];
      babl->pixel_format.component[band] = component[band];
      babl->pixel_format.type[band] = type[band];
      babl->pixel_format.sampling[band] = sampling[band];
    }
  babl->pixel_format.model[band] = NULL;
  babl->pixel_format.component[band] = NULL;
  babl->pixel_format.type[band]      = NULL;
  babl->pixel_format.sampling[band]  = NULL;

  return babl;
}

Babl *
babl_pixel_format_new (const char *name,
                       ...)
{
  va_list varg;
  Babl            *babl;
  int              id     = 0;
  int              planar = 0;
  int              bands  = 0;
  BablModel       *model     [BABL_MAX_BANDS];
  BablComponent   *component [BABL_MAX_BANDS];
  BablSampling    *sampling  [BABL_MAX_BANDS];
  BablType        *type      [BABL_MAX_BANDS];

  BablSampling    *current_sampling = (BablSampling*) babl_sampling (1,1);
  BablType        *current_type     = (BablType*)     babl_type_id (BABL_U8);
  BablModel       *current_model    = NULL;
  const char      *arg              = name;

  va_start (varg, name);

  
  while (1)
    {
      arg = va_arg (varg, char *);
      if (!arg)
        break;


      if (BABL_IS_BABL (arg))
        {
          Babl *babl = (Babl*)arg;

          switch (babl->class_type)
            {
              case BABL_TYPE:
                current_type = (BablType*) babl;
                break;
              case BABL_COMPONENT:
                if (!current_model)
                  {
                    babl_log ("%s(): no model specified before component %s",
                              __FUNCTION__, babl->instance.name);
                  }
                model     [bands] = current_model;
                component [bands] = (BablComponent*) babl;
                type      [bands] = current_type;
                sampling  [bands] = current_sampling;
                bands++;

                if (bands>=BABL_MAX_BANDS)
                  {
                    babl_log ("%s(): maximum number of bands (%i) exceeded for %s",
                              __FUNCTION__, BABL_MAX_BANDS, name);
                  }
                break;
              case BABL_SAMPLING:
                  current_sampling = (BablSampling*)arg;
                  break;
              case BABL_MODEL:
                  current_model = (BablModel*)arg;
                  break;
              case BABL_INSTANCE:
              case BABL_PIXEL_FORMAT:
              case BABL_CONVERSION:
              case BABL_CONVERSION_TYPE:
              case BABL_CONVERSION_TYPE_PLANAR:
              case BABL_CONVERSION_MODEL_PLANAR:
              case BABL_CONVERSION_PIXEL_FORMAT:
              case BABL_CONVERSION_PIXEL_FORMAT_PLANAR:
              case BABL_FISH:
              case BABL_FISH_REFERENCE:
              case BABL_IMAGE:
                babl_log ("%s(): %s unexpected",
                          __FUNCTION__, babl_class_name (babl->class_type));
                break;
              case BABL_SKY: /* shut up compiler */
                break;
            }
        }
      /* if we didn't point to a babl, we assume arguments to be strings */
      else if (!strcmp (arg, "id"))
        {
          id = va_arg (varg, int);
        }
      
      else if (!strcmp (arg, "packed"))
        {
          planar = 0;
        }
      
      else if (!strcmp (arg, "planar"))
        {
          planar = 1;
        }
      
      else
        {
          babl_log ("%s: unhandled parameter '%s' for pixel_format '%s'",
                    __FUNCTION__, arg, name);
          exit (-1);
        }
    }
    
  va_end   (varg);


  babl = pixel_format_new (name, id,
                           planar,
                           bands,
                           model, component, sampling, type);

  
  if (db_insert (babl) == babl)
    {
      return babl;
    }
  else
    {
      each_babl_pixel_format_destroy (babl, NULL);
      return NULL;
    }
}

BABL_CLASS_TEMPLATE (babl_pixel_format)
