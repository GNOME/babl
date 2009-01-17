/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2005-2008, Øyvind Kolås and others.
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
 * <http://www.gnu.org/licenses/>.
 */

#ifndef _BABL_H
#error  this file is only to be included by babl.h 
#endif

/****************************************************************/
/* BablType */
BABL_NAMED_CLASS (type);
/*
 * A data type that babl can have in it's buffers, requires
 * conversions to and from "double" to be registered before
 * passing sanity.
 *
 * Babl * babl_type_new (  const char *name,
 *                         "bits",     int bits,
 *                         ["min_val", double min_val,]
 *                         ["max_val", double max_val,]
 *                         NULL);
 */

typedef struct
{
  BablInstance     instance;
  BablList         *from_list;
  int              bits;  /*< number of bits used to represent the data type
                            (initially restricted to a multiple of 8) */
  double           min_val;
  double           max_val;
} BablType;

typedef struct
{
  BablType          type;
  int               is_signed;
  long              max;
  long              min;
} BablTypeInteger;

typedef struct
{
  BablType type;
  /* sign
   * biased_exponent
   * mantissa */
} BablTypeFloat;
