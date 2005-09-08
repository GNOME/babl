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
        babl_fatal ("matching source component for %s in model %s not found",
                    model->component[i]->instance.name, model->instance.name);
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


static char buf[512]="";

static const char *
create_name (BablModel      *model,
             int             components,
             BablComponent **component,
             BablType      **type)
{
  char *p = &buf[0];

  sprintf (p, "%s ", model->instance.name);
  p+=strlen (model->instance.name) + 1;

  while (components--)
    {
      sprintf (p, "(%s as %s) ", 
         (*component)->instance.name,
         (*type)->instance.name);
      p+=strlen ((*component)->instance.name) +
         strlen ((*type)->instance.name     ) + strlen("( as ) ");
      component++;
      type++;
    }
  return buf;
}

Babl *
babl_format_new (void *first_arg,
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
  char            *name             = NULL;
  void            *arg              = first_arg;

  va_start (varg, first_arg);
  
  while (1)
    {
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
                    babl_fatal ("no model specified before component %s",
                                babl->instance.name);
                  }
                component [components] = (BablComponent*) babl;
                type      [components] = current_type;
                sampling  [components] = current_sampling;
                components++;

                if (components>=BABL_MAX_COMPONENTS)
                  {
                    babl_fatal ("maximum number of components (%i) exceeded for %s",
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
              case BABL_CONVERSION_LINEAR:
              case BABL_CONVERSION_PLANE:
              case BABL_CONVERSION_PLANAR:
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

      else if (!strcmp (arg, "name"))
        {
          name = va_arg (varg, char *);
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
          babl_fatal ("unhandled argument '%s' for format '%s'", arg, name);
        }

      arg = va_arg (varg, char *);
      if (!arg)
        break;
    }
    
  va_end   (varg);

  babl = format_new (name?name:create_name (model, components, component, type),
                     id,
                     planar, components, model,
                     component, sampling, type);
 
  { 
    Babl *ret = babl_db_insert (db, babl);
    if (ret!=babl)
        babl_free (babl);
    return ret;
  }
}

BABL_CLASS_TEMPLATE (format)
