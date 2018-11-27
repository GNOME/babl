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
 * <https://www.gnu.org/licenses/>.
 */

#ifndef _BABL_FORMAT_H
#define _BABL_FORMAT_H

BABL_CLASS_DECLARE (format);

typedef struct
{
  BablInstance     instance;
  BablList        *from_list;
  int              components;
  BablComponent  **component;
  BablType       **type;
  BablModel       *model;
  const Babl      *space;
  void            *model_data;
  void            *image_template; /* image template for use with
                                      linear (non-planer) images */

  BablSampling   **sampling;
  int              bytes_per_pixel;
  int              planar;
  double           loss; /*< average relative error when converting
                             from and to RGBA double */
  int              visited; /* for convenience in code while searching
                               for conversion paths */
  int              format_n; /* whether the format is a format_n type or not */
  int              palette;
  const char      *encoding;
} BablFormat;

#endif
