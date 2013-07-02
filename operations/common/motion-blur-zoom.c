/* This file is an image processing operation for GEGL
 *
 * GEGL is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * GEGL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GEGL; if not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright (C) 2013 Téo Mazars  <teo.mazars@ensimag.fr>
 *
 * This operation is inspired by and uses parts of blur-motion.c
 * from GIMP 2.8.4:
 *
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 * Copyright (C) 1996 Torsten Martinsen       <torsten@danbbs.dk>
 * Copyright (C) 1996 Federico Mena Quintero  <federico@nuclecu.unam.mx>
 * Copyright (C) 1996 Heinz W. Werntges       <quartic@polloux.fciencias.unam.mx>
 * Copyright (C) 1997 Daniel Skarda           <0rfelyus@atrey.karlin.mff.cuni.cz>
 * Copyright (C) 2007 Joerg Gittinger         <sw@gittingerbox.de>
 *
 * This operation is also inspired by GEGL's blur-motion-linear.c :
 *
 * Copyright (C) 2006 Øyvind Kolås  <pippin@gimp.org>
 */

#include "config.h"

#include <glib/gi18n-lib.h>
#include <math.h>

#ifdef GEGL_CHANT_PROPERTIES

gegl_chant_double_ui (center_x, _("X"),
                      -G_MAXDOUBLE, G_MAXDOUBLE, 20.0,
                      -100000.0, 100000.0, 1.0,
                      _("Horizontal center position"))

gegl_chant_double_ui (center_y, _("Y"),
                      -G_MAXDOUBLE, G_MAXDOUBLE, 20.0,
                      -100000.0, 100000.0, 1.0,
                      _("Vertical center position"))

gegl_chant_double_ui (factor, _("Factor"),
                      -10.0, 1.0, 0.1,
                      -0.5, 1.0, 2.0,
                      _("Bluring factor"))

#else

#define GEGL_CHANT_TYPE_AREA_FILTER
#define GEGL_CHANT_C_FILE "motion-blur-zoom.c"

#include "gegl-chant.h"

#define SQR(c) ((c) * (c))

#define MAX_NUM_IT 200
#define NOMINAL_NUM_IT 100

static void
prepare (GeglOperation *operation)
{
  GeglOperationAreaFilter *op_area = GEGL_OPERATION_AREA_FILTER (operation);
  GeglChantO              *o       = GEGL_CHANT_PROPERTIES (operation);
  GeglRectangle           *whole_region;

  whole_region = gegl_operation_source_get_bounding_box (operation, "input");

  if (whole_region != NULL)
    {
      op_area->left = op_area->right
        = MAX (fabs (whole_region->x - o->center_x),
               fabs (whole_region->width + whole_region->x - o->center_x)) * fabs (o->factor) +1;

      op_area->top = op_area->bottom
        = MAX (fabs (whole_region->y - o->center_y),
               fabs (whole_region->height + whole_region->y - o->center_y)) * fabs (o->factor) +1;
    }
  else
    {
      op_area->left   =
      op_area->right  =
      op_area->top    =
      op_area->bottom = 0;
    }

  gegl_operation_set_format (operation, "input",  babl_format ("RaGaBaA float"));
  gegl_operation_set_format (operation, "output", babl_format ("RaGaBaA float"));
}

static inline gfloat *
get_pixel_color (gfloat              *in_buf,
                 const GeglRectangle *rect,
                 gint                 x,
                 gint                 y)
{
  gint ix = x - rect->x;
  gint iy = y - rect->y;

  ix = CLAMP (ix, 0, rect->width  - 1);
  iy = CLAMP (iy, 0, rect->height - 1);

  return &in_buf[(iy * rect->width + ix) * 4];
}


static gboolean
process (GeglOperation       *operation,
         GeglBuffer          *input,
         GeglBuffer          *output,
         const GeglRectangle *roi,
         gint                 level)
{
  GeglOperationAreaFilter *op_area = GEGL_OPERATION_AREA_FILTER (operation);
  GeglChantO              *o       = GEGL_CHANT_PROPERTIES (operation);
  gfloat                  *in_buf, *out_buf, *out_pixel;
  gint                     x, y;
  GeglRectangle            src_rect;

  src_rect = *roi;
  src_rect.x -= op_area->left;
  src_rect.y -= op_area->top;
  src_rect.width += op_area->left + op_area->right;
  src_rect.height += op_area->top + op_area->bottom;

  in_buf  = g_new  (gfloat, src_rect.width * src_rect.height * 4);
  out_buf = g_new0 (gfloat, roi->width * roi->height * 4);
  out_pixel = out_buf;

  gegl_buffer_get (input, &src_rect, 1.0, babl_format ("RaGaBaA float"),
                   in_buf, GEGL_AUTO_ROWSTRIDE, GEGL_ABYSS_NONE);

  for (y = roi->y; y < roi->height + roi->y; ++y)
    {
      for (x = roi->x; x < roi->width + roi->x; ++x)
        {
          gint c, i;
          gfloat dxx, dyy, ix, iy, inv_xy_len;
          gfloat sum[] = {0, 0, 0, 0};

          gfloat x_start = x;
          gfloat y_start = y;
          gfloat x_end   = x + (o->center_x - (gfloat) x) * o->factor;
          gfloat y_end   = y + (o->center_y - (gfloat) y) * o->factor;

          gint dist = ceil (sqrt (SQR (x_end - x_start) + SQR (y_end - y_start)) +1);

          /* ensure quality when near the center or with small factor */
          gint xy_len = MAX (dist, 3);

          /* performance concern */
          if (xy_len > NOMINAL_NUM_IT)
            xy_len = MIN (NOMINAL_NUM_IT + (gint) sqrt (xy_len - NOMINAL_NUM_IT), MAX_NUM_IT);

          inv_xy_len = 1.0 / (gfloat) xy_len;

          dxx = (x_end - x_start) * inv_xy_len;
          dyy = (y_end - y_start) * inv_xy_len;

          ix  = x_start;
          iy  = y_start;

          for (i = 0; i < xy_len; i++)
            {
              gfloat dx = ix - floor (ix);
              gfloat dy = iy - floor (iy);

              /* do bilinear interpolation to get a nice smooth result */
              gfloat *pix0, *pix1, *pix2, *pix3;
              gfloat mixy0[4];
              gfloat mixy1[4];

              pix0 = get_pixel_color (in_buf, &src_rect, ix, iy);
              pix1 = get_pixel_color (in_buf, &src_rect, ix+1, iy);
              pix2 = get_pixel_color (in_buf, &src_rect, ix, iy+1);
              pix3 = get_pixel_color (in_buf, &src_rect, ix+1, iy+1);

              for (c = 0; c < 4; ++c)
                {
                  mixy0[c] = dy * (pix2[c] - pix0[c]) + pix0[c];
                  mixy1[c] = dy * (pix3[c] - pix1[c]) + pix1[c];

                  sum[c] +=  dx * (mixy1[c] - mixy0[c]) + mixy0[c];
                }

              ix += dxx;
              iy += dyy;
            }

          for (c = 0; c < 4; ++c)
            *out_pixel++ = sum[c] * inv_xy_len;
        }
    }

  gegl_buffer_set (output, roi, 1.0, babl_format ("RaGaBaA float"),
                   out_buf, GEGL_AUTO_ROWSTRIDE);

  g_free (in_buf);
  g_free (out_buf);

  return  TRUE;
}

static void
gegl_chant_class_init (GeglChantClass *klass)
{
  GeglOperationClass       *operation_class;
  GeglOperationFilterClass *filter_class;

  operation_class = GEGL_OPERATION_CLASS (klass);
  filter_class    = GEGL_OPERATION_FILTER_CLASS (klass);

  operation_class->prepare = prepare;
  filter_class->process    = process;

  gegl_operation_class_set_keys (operation_class,
                                 "name",        "gegl:motion-blur-zoom",
                                 "categories",  "blur",
                                 "description", _("Zoom motion blur"),
                                 NULL);
}

#endif
