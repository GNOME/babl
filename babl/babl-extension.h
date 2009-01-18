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

#ifndef _BABL_EXTENSION_H
#define _BABL_EXTENSION_H

#ifndef _BABL_H
#error  this file is only to be included by babl.h
#endif

/****************************************************************/
/* BablExtension */
BABL_NAMED_CLASS_DECLARE (extension);
/*
 * BablExtension objects are only used internally in babl.
 */

#ifndef BABL_DISABLE_DEPRECATED

#define babl_extension babl_extension_from_name

#endif /* BABL_DISABLE_DEPRECATED */


typedef struct
{
  BablInstance   instance; /* path to .so / .dll is stored in instance name */
  void          *dl_handle;
  void         (*destroy) (void);
} BablExtension;

#endif
