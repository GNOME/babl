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

#include "babl-internal.h"
#include <string.h>
#include <stdarg.h>
#include "babl-db.h"


static int 
each_babl_model_destroy (Babl *babl,
                         void         *data)
{
  babl_free (babl->model.from);
  babl_free (babl->model.to);
  babl_free (babl->model.component);
  babl_free (babl->instance.name);
  babl_free (babl);
  return 0;  /* continue iterating */
}


#define BABL_MAX_COMPONENTS 32

static BablModel *
model_new (const char     *name,
           int             id,
           int             components,
           BablComponent **component)
{
  BablModel *self;
  int        i; 

  self                     = babl_calloc (sizeof (BablModel), 1);

  self->instance.type = BABL_MODEL;
  self->instance.id   = id;
  self->instance.name = babl_strdup (name);

  self->components    = components;

  self->component     = babl_malloc (sizeof (BablComponent*) * (components+1));

  for (i=0; i < components; i++)
    {
      self->component[i] = component[i];
    }
  self->component[i] = NULL;

  return self;
}

BablModel *
babl_model_new (const char *name,
                       ...)
{
  va_list varg;
  BablModel *self;
  int              id     = 0;
  int              components  = 0;
  BablComponent   *band_component [BABL_MAX_COMPONENTS];
  const char      *arg=name;

  va_start (varg, name);

  
  while (1)
    {
      arg = va_arg (varg, char *);
      if (!arg)
        break;


      if (BABL_IS_BABL (arg))
        {
          Babl *babl = (Babl*)arg;

          switch (BABL_INSTANCE_TYPE (arg))
            {
              case BABL_COMPONENT:
                band_component [components] = (BablComponent*) babl;
                components++;

                if (components>=BABL_MAX_COMPONENTS)
                  {
                    babl_log ("%s(): maximum number of components (%i) exceeded for %s",
                     __FUNCTION__, BABL_MAX_COMPONENTS, name);
                  }
                break;
              case BABL_MODEL:
                  babl_log ("%s(): submodels not handled yet", __FUNCTION__);
                  break;
              case BABL_TYPE:
              case BABL_SAMPLING:
              case BABL_INSTANCE:
              case BABL_PIXEL_FORMAT:


              case BABL_CONVERSION:
              case BABL_CONVERSION_TYPE:
              case BABL_CONVERSION_TYPE_PLANAR:
              case BABL_CONVERSION_MODEL_PLANAR:
              case BABL_CONVERSION_PIXEL_FORMAT:
              case BABL_CONVERSION_PIXEL_FORMAT_PLANAR:
              case BABL_FISH:
                babl_log ("%s(): %s unexpected",
                          __FUNCTION__, babl_class_name (babl->instance.type));
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
      
      else
        {
          babl_log ("%s: unhandled parameter '%s' for babl_model '%s'",
                    __FUNCTION__, arg, name);
          exit (-1);
        }
    }
    
  va_end   (varg);

  self = model_new (name, id, components, band_component);
  
  if ((BablModel*) db_insert ((Babl*)self) == self)
    {
      return self;
    }
  else
    {
      each_babl_model_destroy ((Babl*)self, NULL);
      return NULL;
    }
}

BABL_CLASS_TEMPLATE (BablModel, babl_model, "BablModel")
