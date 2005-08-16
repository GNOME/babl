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
#include "babl-db.h"
#include <string.h>

#include <stdarg.h>


static int 
each_babl_component_destroy (Babl *babl,
                             void *data)
{
  babl_free (babl->component.from);
  babl_free (babl->component.to);
  babl_free (babl);
  return 0;  /* continue iterating */
}

static Babl *
component_new (const char *name,
               int         id,
               int         luma,
               int         chroma,
               int         alpha)
{
  Babl *babl;

  babl                   = babl_calloc (sizeof (BablComponent) +
                                        strlen (name) + 1, 1);
  babl->instance.name    = (void *) babl + sizeof (BablComponent);
  strcpy (babl->instance.name, name);

  babl->class_type       = BABL_COMPONENT;
  babl->instance.id      = id;
  babl->component.luma   = luma;
  babl->component.chroma = chroma;
  babl->component.alpha  = alpha;

  return babl;
}

Babl *
babl_component_new (const char *name,
                    ...)
{
  va_list     varg;
  Babl       *babl;
  int         id         = 0;
  int         luma    = 0;
  int         chroma  = 0;
  int         alpha   = 0;
  const char *arg=name;

  va_start (varg, name);
  
  while (1)
    {
      arg = va_arg (varg, char *);
      if (!arg)
        break;
      
      if (BABL_IS_BABL (arg))
        {
          Babl *babl = (Babl*)arg;

          babl_log ("%s(): %s unexpected",
                    __FUNCTION__, babl_class_name (babl->class_type));
        }
      /* if we didn't point to a babl, we assume arguments to be strings */

      else if (!strcmp (arg, "id"))
        {
          id = va_arg (varg, int);
        }
      
      else if (!strcmp (arg, "luma"))
        {
          luma = 1;
        }
      
      else if (!strcmp (arg, "chroma"))
        {
          chroma = 1;
        }

      else if (!strcmp (arg, "alpha"))
        {
          alpha = 1;
        }
      
      else
        {
          babl_log ("%s(): unhandled parameter '%s' for pixel_format '%s'",
                    __FUNCTION__, arg, name);
          exit (-1);
        }
    }
    
  va_end   (varg);

  babl = component_new (name, id, luma, chroma, alpha);

  if (db_insert (babl) == babl)
    {
      return babl;
    }
  else
    {
      each_babl_component_destroy (babl, NULL);
      return NULL;
    }
}

BABL_CLASS_TEMPLATE (babl_component)
