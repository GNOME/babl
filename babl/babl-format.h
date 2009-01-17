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
/* BablFormat */
BABL_NAMED_CLASS (format);
/*
 * Babl * babl_format_new (["name", const char *name,]
 *                          BablModel          *model,
 *                         [BablType           *type,]
 *                         [BablSampling,      *sampling,]
 *                          BablComponent      *component1,
 *                        [[BablType           *type,]
 *                         [BablSampling       *sampling,]
 *                          BablComponent      *componentN,
 *                        ...]
 *                          ["planar",]
 *                          NULL);
 *
 * Provided BablType and|or BablSampling is valid for the following
 * components as well. If no name is provided a (long) descriptive
 * name is used.
 */

typedef struct
{
  BablInstance     instance;
  BablList         *from_list;
  int              components;
  BablComponent  **component;
  BablType       **type;
  void            *image_template; /* image template for use with
                                      linear (non-planer) images */

  BablSampling   **sampling;
  BablModel       *model;
  int              bytes_per_pixel;
  int              planar;
  double           loss; /*< average relative error when converting
                             from and to RGBA double */
  int              visited; /* for convenience in code while searching
                               for conversion paths */
} BablFormat;
