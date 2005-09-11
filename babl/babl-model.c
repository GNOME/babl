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
  babl_free (babl);
  return 0;  /* continue iterating */
}

static char buf[512]="";

static const char *
create_name (const char     *name,
             int             components,
             BablComponent **component)
{
  char *p = buf;
  if (name)
    return name;
  while (components--)
    {
      sprintf (p, (*component)->instance.name);
      p+=strlen ((*component)->instance.name);
      component++;
    }
  return buf;
}

static Babl *
model_new (const char     *name,
           int             id,
           int             components,
           BablComponent **component)
{
  Babl *babl;

  babl                   = babl_malloc (sizeof (BablModel) +
                                        sizeof (BablComponent*) * (components) +
                                        strlen (name) + 1);
  babl->model.component = ((void*)babl) + sizeof (BablModel);
  babl->instance.name   = ((void*)babl->model.component) + sizeof (BablComponent*) * (components);
  
  babl->class_type       = BABL_MODEL;
  babl->instance.id      = id;
  babl->model.components = components;
  strcpy (babl->instance.name, name);
  memcpy (babl->model.component, component, sizeof (BablComponent*)*components);

  babl->model.from         = NULL;
  return babl;
}

Babl *
babl_model_new (void *first_argument,
                ...)
{
  va_list        varg;
  Babl          *babl;
  int            id          = 0;
  int            components  = 0;
  const char    *arg         = first_argument;
  const char    *name        = NULL;
  BablComponent *component [BABL_MAX_COMPONENTS];

  va_start (varg, first_argument);
  
  while (1)
    {


      if (BABL_IS_BABL (arg))
        {
          Babl *babl = (Babl*)arg;

          switch (babl->class_type)
            {
              case BABL_COMPONENT:
                component [components] = (BablComponent*) babl;
                components++;

                if (components>=BABL_MAX_COMPONENTS)
                  {
                    babl_log ("maximum number of components (%i) exceeded for %s",
                      BABL_MAX_COMPONENTS, name);
                  }
                break;
              case BABL_MODEL:
                  babl_log ("submodels not handled yet");
                  break;
              case BABL_TYPE:
              case BABL_TYPE_INTEGER:
              case BABL_TYPE_FLOAT:
              case BABL_SAMPLING:
              case BABL_INSTANCE:
              case BABL_FORMAT:


              case BABL_CONVERSION:
              case BABL_CONVERSION_LINEAR:
              case BABL_CONVERSION_PLANE:
              case BABL_CONVERSION_PLANAR:
              case BABL_FISH:
              case BABL_FISH_REFERENCE:
              case BABL_FISH_SIMPLE:
              case BABL_IMAGE:
              case BABL_EXTENSION:
                babl_log ("%s unexpected", babl_class_name (babl->class_type));
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
      
      else
        {
          babl_fatal ("unhandled argument '%s' for babl_model '%s'", arg, name);
        }

      arg = va_arg (varg, char *);
      if (!arg)
        break;
    }
    
  va_end   (varg);

  babl = model_new (create_name (name, components, component), id, components, component);
  
  { 
    Babl *ret = babl_db_insert (db, babl);
    if (ret!=babl)
        babl_free (babl);
    return ret;
  }
}

BABL_CLASS_TEMPLATE (model)
