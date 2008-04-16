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
/* BablComponent */
BABL_NAMED_CLASS (component);
/*
 * Babl * babl_component_new (const char *name,
 *                            NULL);
 */
Babl *babl_component_id     (int id);
void  babl_component_each   (BablEachFunction  each_fun,
                        void             *user_data);
Babl * babl_component       (const char       *name);
Babl * babl_component_new   (void             *first_arg,
                        ...) BABL_ARG_NULL_TERMINATED;

typedef struct
{
  BablInstance     instance;
  int              luma;
  int              chroma;
  int              alpha;
} BablComponent;
