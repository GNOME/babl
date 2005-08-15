/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2005, Øyvind Kolås.
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
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <string.h>
#include <math.h>
#include <assert.h>
#include "babl.h"

#include "util.h"

static void components    (void);
static void models        (void);
static void conversions   (void);
static void pixel_formats (void);

void
babl_base_model_ycbcr (void)
{
  components    ();
  models        ();
  conversions   ();
  pixel_formats ();
}

static void
components (void)
{
    babl_component_new (
   "Cb",
   "id",    BABL_CB,
   "chroma",
   NULL);

  babl_component_new (
   "Cr",
   "id",    BABL_CR,
   "chroma",
   NULL);
}

static void
models (void)
{
}

static void
conversions (void)
{
}

static void
pixel_formats (void)
{
  babl_pixel_format_new (
    "yuv420",
    "id",          BABL_YUV420,
    "planar",
    babl_model_id  (BABL_YCBCR),
    babl_type_id   (BABL_U8),
    babl_sampling  (1, 1), babl_component_id (BABL_LUMINANCE),
    babl_sampling  (2, 2), babl_component_id (BABL_CB), 
    babl_sampling  (2, 2), babl_component_id (BABL_CR),
    NULL);
  
#if 0
  babl_pixel_format_new (
    "yuv411",
    "id",          BABL_YUV411,
    "planar",
    babl_type      ("u8"),
    babl_sampling  (1, 1),
    babl_component ("Y"), 
    babl_sampling  (4, 1),
    babl_component_id (BABL_CB), 
    babl_sampling  (4, 1),
    babl_component_id (BABL_CR),
    NULL);

  babl_pixel_format_new (
    "yuv422",
    "id",          BABL_YUV422,
    "planar",
    babl_type      ("u8"),
    babl_sampling  (1, 1),
    babl_component ("Y"), 
    babl_sampling  (2, 1),
    babl_component_id (BABL_CB), 
    babl_sampling  (2, 1),
    babl_component_id (BABL_CR),
    NULL);
#endif
}
