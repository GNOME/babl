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

#ifndef _BABL_MODEL_H
#define _BABL_MODEL_H

BABL_CLASS_DECLARE (model);

typedef struct
{
  BablInstance      instance;
  BablList         *from_list;
  int               components;
  BablComponent   **component;
  BablType        **type; /*< must be doubles,
                              used here for convenience in code */
  void             *data;    /* user-data, used for palette */
  const Babl       *space;
  void             *model;   /* back pointer to model with sRGB space */
  BablModelFlag     flags;
} BablModel;

#endif
