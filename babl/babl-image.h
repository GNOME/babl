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

#ifndef _BABL_IMAGE_H
#define _BABL_IMAGE_H

#include "babl-classes.h"

/* babl images are allocated as a single chunk of memory, and
 * thus can be duplicated using  duplicate = babl_dup (original);
 *
 * NB: babl_fish_process () frees the images passed in by itself.
 */
Babl * babl_image_new             (void *first_component,
                                   ...);

/* create a new BablImage based on a packed BablFormat (or BablModel which
 * is a virtual pixelformat based on the BablModel using only doubles in the
 * order they are listed in the model.
 */
Babl * babl_image_from_linear (void *buffer,
                               Babl *format);

/* create a new babl image similar to the provided babl-image, but where all data
 * is in doubles,.. 
 */
Babl * babl_image_double_from_image (Babl *source);

#endif
