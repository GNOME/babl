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

#include "babl.h"

void babl_base_pixel_formats (void)
{


  babl_pixel_format_new (
    "cmyk-float",
    "id",             BABL_CMYK_FLOAT,
    babl_model_id     (BABL_CMYK),
    babl_type_id      (BABL_FLOAT),
    babl_component_id (BABL_CYAN), 
    babl_component_id (BABL_MAGENTA), 
    babl_component_id (BABL_YELLOW),
    babl_component_id (BABL_KEY),
    NULL);

  babl_pixel_format_new (
    "cmyka-float",
    "id",             BABL_CMYKA_FLOAT,
    babl_model_id     (BABL_CMYKA),
    babl_type_id      (BABL_FLOAT),
    babl_component_id (BABL_CYAN), 
    babl_component_id (BABL_MAGENTA), 
    babl_component_id (BABL_YELLOW),
    babl_component_id (BABL_KEY),
    babl_component_id (BABL_ALPHA),
    NULL);

}
