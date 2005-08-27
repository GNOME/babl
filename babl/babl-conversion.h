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

#ifndef BABL_CONVERSION_H
#define BABL_CONVERSION_H

#include "babl-classes.h"
#include "babl-instance.h"

void   babl_conversion_process (BablConversion *conversion,
                                void           *source,
                                void           *destination,
                                long           n);

Babl * babl_conversion_new     (Babl           *source,
                                Babl           *destination,
                                void           *first_argument,
                                ...);

BABL_DEFINE_CLASS_NO_NEW (babl_conversion);
#endif
