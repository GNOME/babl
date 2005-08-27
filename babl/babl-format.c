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
                  int             components,
                  BablModel      *model,
                  BablComponent **component,
                  BablSampling  **sampling,
                  BablType      **type)
{
  Babl *babl;

  {
    int i;
    /* i is desintation position */
    for (i=0 ; i<model->components; i++)
      {
        int j;

        for (j=0;j<components;j++)
          {
            if (component[j] == model->component[i])
              goto component_found;
          }
        babl_log ("matching source component for %s in model %s not found",
           model->component[i]->instance.name, model->instance.name);
        exit (-1);
        component_found:
        ;
      }
  }

  /* allocate all memory in one chunk */
  babl  = babl_malloc (sizeof (BablFormat) +
                       strlen (name) + 1 +
                       sizeof (BablComponent*) * (components) +
                       sizeof (BablSampling*)  * (components) +
                       sizeof (BablType*)      * (components) +
                       sizeof (int)            * (components) +
                       sizeof (int)            * (components));

  babl->format.from      = NULL;
  babl->format.to        = NULL;
  babl->format.component = ((void *)babl) + sizeof (BablFormat);
  babl->format.type      = ((void *)babl->format.component) + sizeof (BablComponent*) * (components);
  babl->format.sampling  = ((void *)babl->format.type)      + sizeof (BablType*) * (components);
  babl->instance.name    = ((void *)babl->format.sampling)  + sizeof (BablSampling*) * (components);
  
  babl->class_type    = BABL_FORMAT;
  babl->instance.id   = id;

  strcpy (babl->instance.name, name);

  babl->format.model      = model;
  babl->format.components = components;

  memcpy (babl->format.component, component, sizeof (BablComponent*) * components);
  memcpy (babl->format.type     , type     , sizeof (BablType*)      * components);
  memcpy (babl->format.sampling , sampling , sizeof (BablSampling*)  * components);

  babl->format.planar     = planar;

  babl->format.bytes_per_pixel = 0;
  {
    int i;
    for (i=0;i<components;i++)
      babl->format.bytes_per_pixel += type[i]->bits/8;
  }

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
  int              components  = 0;
  BablModel       *model  = NULL;
  BablComponent   *component [BABL_MAX_COMPONENTS];
  BablSampling    *sampling  [BABL_MAX_COMPONENTS];
  BablType        *type      [BABL_MAX_COMPONENTS];

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
                    babl_log ("no model specified before component %s",
                               babl->instance.name);
                  }
                component [components] = (BablComponent*) babl;
                type      [components] = current_type;
                sampling  [components] = current_sampling;
                components++;

                if (components>=BABL_MAX_COMPONENTS)
                  {
                    babl_log ("maximum number of components (%i) exceeded for %s",
                               BABL_MAX_COMPONENTS, name);
                  }
                break;
              case BABL_SAMPLING:
                  current_sampling = (BablSampling*)arg;
                  break;
              case BABL_MODEL:
                  if (model)
                    {
                    babl_log ("args=(%s): model %s already requested",
                      babl->instance.name, model->instance.name);
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
                babl_log ("%s unexpected",
                           babl_class_name (babl->class_type));
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
          babl_log ("unhandled parameter '%s' for format '%s'", arg, name);
          exit (-1);
        }
    }
    
  va_end   (varg);


  babl = format_new (name, id,
                           planar, components, model,
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
