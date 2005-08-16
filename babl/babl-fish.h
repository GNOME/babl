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

#ifndef BABL_FISH_H
#define BABL_FISH_H

#include "babl-classes.h"

BABL_DEFINE_CLASS_NO_NEW_NO_ID(babl_fish)

Babl *
babl_fish (Babl *source,
           Babl *destination);

/* babl_fish_process will probably be a polymorph function
 * accepting source and destination buffer pointers will be
 * allowed as well as BablImage objects in their place
 */
int
babl_fish_process        (Babl *babl_fish,
                          void *source,
                          void *destination,
                          int   n);

/* whether the BablFish needs a BablImage to do the processing,
 * or void * are sufficient.
 */
int
babl_fish_needs_image (Babl *babl_fish);

#endif
