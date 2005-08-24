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
each_babl_format_destroy (Babl *babl,
                                void *data)
{
  babl_free (babl->format.from);
  babl_free (babl->format.to);
  babl_free (babl);

  return 0;  /* continue iterating */
}

static Babl *
format_new (const char     *name,
                  int             id,
                  int             planar,
                  int             bands,
                  BablModel      *model,
                  BablComponent **component,
                  BablSampling  **sampling,
                  BablType      **type)
{
  Babl *babl;
  int              band;

  /* allocate all memory in one chunk */
  babl  = babl_calloc (sizeof (BablFormat) +
                       strlen (name) + 1 +
                       sizeof (BablComponent*) * (bands+1) +
                       sizeof (BablSampling*)  * (bands+1) +
                       sizeof (BablType*)      * (bands+1) +
                       sizeof (int)            * (bands+1) +
                       sizeof (int)            * (bands+1),1);

  babl->format.component = ((void *)babl) + sizeof (BablFormat);
  babl->format.type      = ((void *)babl->format.component) + sizeof (BablComponent*) * (bands+1);
  babl->format.sampling  = ((void *)babl->format.type)      + sizeof (BablType*) * (bands+1);
  babl->instance.name          = ((void *)babl->format.sampling)  + sizeof (BablSampling*) * (bands+1);
  
  babl->class_type    = BABL_FORMAT;
  babl->instance.id   = id;
  strcpy (babl->instance.name, name);

  babl->format.model  = model;
  babl->format.bands  = bands;
  babl->format.planar = planar;

  for (band=0; band < bands; band++)
    {
      babl->format.component[band] = component[band];
      babl->format.type[band] = type[band];
      babl->format.sampling[band] = sampling[band];
    }
  babl->format.component[band] = NULL;
  babl->format.type[band]      = NULL;
  babl->format.sampling[band]  = NULL;

  return babl;
}

Babl *
babl_format_new (const char *name,
                       ...)
{
  va_list varg;
  Babl            *babl;
  int              id     = 0;
  int              planar = 0;
  int              bands  = 0;
  BablModel       *model  = NULL;
  BablComponent   *component [BABL_MAX_BANDS];
  BablSampling    *sampling  [BABL_MAX_BANDS];
  BablType        *type      [BABL_MAX_BANDS];

  BablSampling    *current_sampling = (BablSampling*) babl_sampling (1,1);
  BablType        *current_type     = (BablType*)     babl_type_id (BABL_U8);
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
                if (!model)
                  {
                    babl_log ("%s(): no model specified before component %s",
                              __FUNCTION__, babl->instance.name);
                  }
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
                  if (model)
                    {
                    babl_log ("%s(%s): model %s already requested",
                     __FUNCTION__, babl->instance.name, model->instance.name);
                    }
                  model = (BablModel*)arg;
                  break;
              case BABL_INSTANCE:
              case BABL_FORMAT:
              case BABL_CONVERSION:
              case BABL_CONVERSION_TYPE:
              case BABL_CONVERSION_TYPE_PLANAR:
              case BABL_CONVERSION_MODEL_PLANAR:
              case BABL_CONVERSION_FORMAT:
              case BABL_CONVERSION_FORMAT_PLANAR:
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
          babl_log ("%s: unhandled parameter '%s' for format '%s'",
                    __FUNCTION__, arg, name);
          exit (-1);
        }
    }
    
  va_end   (varg);


  babl = format_new (name, id,
                           planar, bands, model,
                           component, sampling, type);

  
  if (db_insert (babl) == babl)
    {
      return babl;
    }
  else
    {
      each_babl_format_destroy (babl, NULL);
      return NULL;
    }
}

BABL_CLASS_TEMPLATE (babl_format)
