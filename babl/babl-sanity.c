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

static int OK;
 
static int
foo (Babl *babl,
     void *user_data)
{
/*  babl_log ("%s", babl->instance.name);*/
  return 0;
}

static Babl *babl_conversion_destination (Babl *babl);

static int
type_sanity (Babl *babl,
             void *user_data)
{
  /* ensure that every type has reference conversions to
   * and from double */
  void **ptr;
  int  ok;

  ok = 0;
  if (babl->type.from)
    {
      ptr = (void **) babl->type.from;

      while (ptr && NULL!=*ptr)
        {
          if (babl_conversion_destination ((Babl *)(*ptr)) == babl_type_id (BABL_DOUBLE))
            {
              ok = 1;
              break;
            }
          ptr++;
        }
    }
  if (!ok)
    {
      OK = 0;
      babl_log ("lack of sanity! type '%s' has no conversion to double",
                babl->instance.name);
    }

  return 0;
}


static int
model_sanity (Babl *babl,
              void *user_data)
{
  /* ensure that every type has reference conversions to
   * and from rgba */
  void **ptr;
  int  ok;

  ok = 0;
  if (babl->model.from)
    {
      ptr = (void **) babl->model.from;

      while (ptr && NULL!=*ptr)
        {
          if (babl_conversion_destination ((Babl *)(*ptr)) == babl_model_id (BABL_RGBA))
            {
              ok = 1;
              break;
            }
          ptr++;
        }
    }
  if (!ok)
    {
      OK=0;
      babl_log ("lack of sanity! model '%s' has no conversion to 'rgba'",
                babl->instance.name);
    }

  return 0;
}

static int
id_sanity (Babl *babl,
           void *user_data)
{
  if (0 == babl->instance.id &&
      babl->instance.creator &&
      !strcmp(BABL(babl->instance.creator)->instance.name, "BablBase"))
    {
      OK=0;
      babl_log ("%s\t'%s' has id==0",
        babl_class_name (babl->class_type), babl->instance.name);
    }
  return 0;
}

int
babl_sanity (void)
{
  OK=1;
  
  babl_type_each         (id_sanity, NULL);
  babl_component_each    (id_sanity, NULL);
  babl_model_each        (id_sanity, NULL);
  babl_format_each       (id_sanity, NULL);

  babl_type_each         (type_sanity, NULL);
  babl_sampling_each     (foo, NULL);
  babl_component_each    (foo, NULL);
  babl_model_each        (model_sanity, NULL);
  babl_format_each       (foo, NULL);
  babl_conversion_each   (foo, NULL);

  return OK;
}

static Babl *babl_conversion_destination (Babl *babl)
{
  return (Babl *)babl->conversion.destination;
}
