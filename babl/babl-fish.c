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
  babl_free (babl);
  return 0;  /* continue iterating */
}

BablFish *
babl_fish_new (const char        *name,
               Babl              *source,
               Babl              *destination)
{
  Babl *self = NULL;

  assert (BABL_IS_BABL (source));
  assert (BABL_IS_BABL (destination));

  self                   = babl_calloc (sizeof (BablFish), 1);
  self->class_type             = BABL_FISH;
  self->instance.id      = 0;
  self->instance.name    = "Fishy";
  self->fish.source      = (union Babl*)source;
  self->fish.destination = (union Babl*)destination;

  if (db_insert (self) == self)
    {
      return (BablFish*)self;
    }
  else
    {
      each_babl_fish_destroy (self, NULL);
      return NULL;
    }

/*  Might make sense to allow a precalculated shortcut to
 *  participate in later checks for optimal conversions, then we
 *  should also have better generated names,.   model + datatype 
 *  is a possibility , or even full single line serialization of
 *  components with types.
 *
    babl_add_ptr_to_list ((void ***)&(source->type.from), self);
    babl_add_ptr_to_list ((void ***)&(destination->type.to), self);
  */
  return (BablFish*)self;
}

BABL_CLASS_TEMPLATE(BablFish, babl_fish, "BablFish")
