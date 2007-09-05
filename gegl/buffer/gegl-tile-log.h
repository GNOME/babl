/* This file is part of GEGL.
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
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Copyright 2006 Øyvind Kolås <pippin@gimp.org>
 */
#ifndef _GEGL_TILE_LOG_H
#define _GEGL_TILE_LOG_H

#include <glib.h>
#include "gegl-tile.h"
#include "gegl-handler.h"

G_BEGIN_DECLS

#define GEGL_TYPE_TILE_LOG            (gegl_tile_log_get_type ())
#define GEGL_TILE_LOG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GEGL_TYPE_TILE_LOG, GeglTileLog))
#define GEGL_TILE_LOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GEGL_TYPE_TILE_LOG, GeglTileLogClass))
#define GEGL_IS_TILE_LOG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GEGL_TYPE_TILE_LOG))
#define GEGL_IS_TILE_LOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GEGL_TYPE_TILE_LOG))
#define GEGL_TILE_LOG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GEGL_TYPE_TILE_LOG, GeglTileLogClass))


typedef struct _GeglTileLog      GeglTileLog;
typedef struct _GeglTileLogClass GeglTileLogClass;

struct _GeglTileLog
{
  GeglHandler  parent_instance;
};

struct _GeglTileLogClass
{
  GeglHandlerClass parent_class;
};

GType gegl_tile_log_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif
