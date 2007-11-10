/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2005, Øyvind Kolås.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
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

#ifndef _BABL_UTIL_H
#define _BABL_UTIL_H

#include <math.h>

void   babl_add_ptr_to_list (void       ***list,
                             void         *new);

void
babl_list_each (void             **list,
                BablEachFunction   each_fun,
                void              *user_data);

long
babl_ticks     (void);

double
babl_rel_avg_error (double *imgA,
                    double *imgB,
                    long    samples);
#endif
