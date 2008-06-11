#include "config.h"
#include <string.h>
#include <math.h>

#include <glib-object.h>
#include <glib/gprintf.h>

#include "gegl-types.h"
#include "gegl-buffer-types.h"
#include "gegl-buffer-private.h"
#include "gegl-tile-storage.h"
#include "gegl-tile-handler-cache.h"
#include "gegl-utils.h"

GeglBuffer *
gegl_buffer_linear_new (const GeglRectangle *extent,
                        const Babl          *format)
{
  GeglRectangle empty={0,0,0,0};

  if (extent==NULL)
    extent = &empty;

  if (format==NULL)
    format = babl_format ("RGBA float");

  return g_object_new (GEGL_TYPE_BUFFER,
                       "x", extent->x,
                       "y", extent->y,
                       "width", extent->width,
                       "height", extent->height,
                       "tile-width", extent->width,
                       "tile-height", extent->height,
                       "format", format,
                       NULL);
}


void gegl_tile_handler_cache_insert (GeglTileHandlerCache *cache,
                                     GeglTile             *tile,
                                     gint                  x,
                                     gint                  y,
                                     gint                  z);

GeglBuffer *
gegl_buffer_linear_new_from_data (const gpointer data,
                                  const Babl   *format,
                                  gint width,
                                  gint height,
                                  GCallback destroy_fn,
                                  gpointer destroy_fn_data)
{
  GeglBuffer *buffer;
  GeglRectangle extent={0,0,width, height};

  buffer = gegl_buffer_linear_new (&extent, format);

  {
    GeglTile *tile = g_object_new (GEGL_TYPE_TILE, NULL);

    tile->rev        = 1;
    tile->stored_rev = 1;
    tile->tile_storage = buffer->tile_storage;
    tile->x = 0;
    tile->y = 0;
    tile->z = 0;
    tile->data       = (gpointer)data;
    tile->size       = format->format.bytes_per_pixel * width * height;
    tile->next_shared = tile;
    tile->prev_shared = tile;

    {
      GeglTileHandlerCache *cache = g_object_get_data (G_OBJECT (buffer->tile_storage), "cache");
      if (cache)
        gegl_tile_handler_cache_insert (cache, tile, 0, 0, 0);
    }
    g_object_unref (tile);
  }

  return buffer;
}



gpointer       *gegl_buffer_linear_open       (GeglBuffer          *buffer,
                                               gint                *width,
                                               gint                *height,
                                               gint                *rowstride)
{
  if (buffer->extent.width == buffer->tile_width &&
      buffer->extent.height == buffer->tile_height)
    {
      GeglTile *tile;

      g_assert (buffer->tile_width == buffer->tile_storage->tile_width);
      g_assert (buffer->tile_height == buffer->tile_storage->tile_height);

      tile = g_object_get_data (G_OBJECT (buffer), "linear-tile");
      g_assert (tile == NULL);
      tile = gegl_tile_source_get_tile ((GeglTileSource*) (buffer),
                                        0,0,0);
      g_assert (tile);
      gegl_buffer_lock (buffer);
      gegl_tile_lock (tile);

      g_object_set_data (G_OBJECT (buffer), "linear-tile", tile);

      *width = buffer->extent.width;
      *height = buffer->extent.height;
      *rowstride = buffer->extent.width * buffer->format->format.bytes_per_pixel;
      return (gpointer)gegl_tile_get_data (tile);
    }

  g_warning ("doesn't seem to be a linear buffer");
  return NULL;
}

void            gegl_buffer_linear_close      (GeglBuffer          *buffer)
{
  GeglTile *tile;
  tile = g_object_get_data (G_OBJECT (buffer), "linear-tile");
  if (!tile)
    return;
  gegl_tile_unlock (tile);
  gegl_buffer_unlock (buffer);
  g_object_set_data (G_OBJECT (buffer), "linear-tile", NULL);
  return;
}
