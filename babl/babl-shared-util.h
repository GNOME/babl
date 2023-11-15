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
 * Public License along with this library; if not, see
 * <https://www.gnu.org/licenses/>.
 */

#ifndef _BABL_SHARED_UTIL_H
#define _BABL_SHARED_UTIL_H


/* Util functions shared between the babl library and the babl CLI tool. */


void
_babl_float_to_half (void        *halfp,
                     const float *floatp,
                     int          numel);
void
_babl_half_to_float (float      *floatp,
                     const void *halfp,
                     int         numel);


#endif
