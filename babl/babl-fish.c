/* babl - dynamically extendable universal pixel fish library.
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
#include "assert.h"
#include <string.h>
#include <stdarg.h>

static int 
each_babl_fish_destroy (Babl *babl,
                        void *data)
{
  babl_free (babl->instance.name);
  babl_free (babl);
  return 0;  /* continue iterating */
}

static BablFish *
fish_new (const char              *name,
                int                id,
                Babl              *source,
                Babl              *destination)
{
  BablFish *self = NULL;

  self = babl_calloc (sizeof (BablFish), 1);
  self->instance.type = BABL_FISH;

  self->instance.id   = id;
  self->instance.name = babl_strdup (name);
  self->source        = (union Babl*)source;
  self->destination   = (union Babl*)destination;

  assert (BABL_IS_BABL (self->source));
  assert (BABL_IS_BABL (self->destination));

/*  Might make sense to allow a precalculated shortcut to
 *  participate in later checks for optimal conversions
  
    babl_add_ptr_to_list ((void ***)&(source->type.from), self);
    babl_add_ptr_to_list ((void ***)&(destination->type.to), self);
  */
  
  return (BablFish*)self;
}

BablFish *
babl_fish_new (const char *name,
                     ...)
{
  va_list            varg;
  BablFish    *self;

  int                id          = 0;
  Babl              *source      = NULL;
  Babl              *destination = NULL;

  const char        *arg         = name;

  va_start (varg, name);
  
  while (1)
    {
      arg = va_arg (varg, char *);
      if (!arg)
        break;
     
      else if (!strcmp (arg, "id"))
        {
          id = va_arg (varg, int);
        }

      else if (!strcmp (arg, "source"))
        {
          source = va_arg (varg, Babl*);
        }
      else if (!strcmp (arg, "destination"))
        {
          destination = va_arg (varg, Babl*);
        }
      else
        {
          babl_log ("%s(): unhandled parameter '%s' for pixel_format '%s'",
                    __FUNCTION__, arg, name);
          exit (-1);
        }
    }
    
  va_end   (varg);

  self = fish_new (name, id,
                         source, destination);

  if ((BablFish*) db_insert ( (Babl*)self) == self)
    {
      return self;
    }
  else
    {
      each_babl_fish_destroy ( (Babl*)self, NULL);
      return NULL;
    }
}

BABL_CLASS_TEMPLATE(BablFish, babl_fish, "BablFish")
