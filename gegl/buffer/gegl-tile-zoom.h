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
#ifndef _GEGL_TILE_ZOOM_H
#define _GEGL_TILE_ZOOM_H

#include <glib.h>
#include "gegl-buffer-types.h"
#include "gegl-tile.h"
#include "gegl-handler.h"
#include "gegl-storage.h"

G_BEGIN_DECLS

#define GEGL_TYPE_TILE_ZOOM            (gegl_tile_zoom_get_type ())
#define GEGL_TILE_ZOOM(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GEGL_TYPE_TILE_ZOOM, GeglTileZoom))
#define GEGL_TILE_ZOOM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GEGL_TYPE_TILE_ZOOM, GeglTileZoomClass))
#define GEGL_IS_TILE_ZOOM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GEGL_TYPE_TILE_ZOOM))
#define GEGL_IS_TILE_ZOOM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GEGL_TYPE_TILE_ZOOM))
#define GEGL_TILE_ZOOM_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GEGL_TYPE_TILE_ZOOM, GeglTileZoomClass))


typedef struct _GeglTileZoom      GeglTileZoom;
typedef struct _GeglTileZoomClass GeglTileZoomClass;

struct _GeglTileZoom
{
  GeglHandler      parent_instance;
  GeglTileBackend *backend;
  GeglStorage     *storage;
};

struct _GeglTileZoomClass
{
  GeglHandlerClass parent_class;
};

GType gegl_tile_zoom_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif
