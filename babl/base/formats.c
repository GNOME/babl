/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2006, Øyvind Kolås.
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

#include "config.h"
#include <stdlib.h>

#include "babl-classes.h"
#include "babl.h"
#include "babl-ids.h"
#include "babl-base.h"

void
babl_formats_init (void)
{
  const Babl *types[]={
    babl_type_from_id (BABL_DOUBLE),
    babl_type_from_id (BABL_FLOAT),
    babl_type_from_id (BABL_HALF),
    babl_type_from_id (BABL_U8),
    babl_type_from_id (BABL_U16),
    babl_type_from_id (BABL_U32)
  };
  for (int i = 0; i < sizeof (types)/sizeof(types[0]);i++)
  {
    const Babl *type = types[i];

    babl_format_new (
      babl_model_from_id (BABL_RGBA_NONLINEAR_PREMULTIPLIED),
      type,
      babl_component_from_id (BABL_RED_NONLINEAR_MUL_ALPHA),
      babl_component_from_id (BABL_GREEN_NONLINEAR_MUL_ALPHA),
      babl_component_from_id (BABL_BLUE_NONLINEAR_MUL_ALPHA),
      babl_component_from_id (BABL_ALPHA),
      NULL);

    babl_format_new (
      babl_model_from_id (BABL_RGBA_PERCEPTUAL_PREMULTIPLIED),
      type,
      babl_component_from_id (BABL_RED_PERCEPTUAL_MUL_ALPHA),
      babl_component_from_id (BABL_GREEN_PERCEPTUAL_MUL_ALPHA),
      babl_component_from_id (BABL_BLUE_PERCEPTUAL_MUL_ALPHA),
      babl_component_from_id (BABL_ALPHA),
      NULL);

    babl_format_new (
      babl_model_from_id (BABL_RGBA_NONLINEAR),
      type,
      babl_component_from_id (BABL_RED_NONLINEAR),
      babl_component_from_id (BABL_GREEN_NONLINEAR),
      babl_component_from_id (BABL_BLUE_NONLINEAR),
      babl_component_from_id (BABL_ALPHA),
      NULL);

    babl_format_new (
      babl_model_from_id (BABL_RGBA_PERCEPTUAL),
      type,
      babl_component_from_id (BABL_RED_PERCEPTUAL),
      babl_component_from_id (BABL_GREEN_PERCEPTUAL),
      babl_component_from_id (BABL_BLUE_PERCEPTUAL),
      babl_component_from_id (BABL_ALPHA),
      NULL);

  babl_format_new (
    babl_model_from_id (BABL_RGB_NONLINEAR),
    type,
    babl_component_from_id (BABL_RED_NONLINEAR),
    babl_component_from_id (BABL_GREEN_NONLINEAR),
    babl_component_from_id (BABL_BLUE_NONLINEAR),
    NULL);
  babl_format_new (
    babl_model_from_id (BABL_RGB_NONLINEAR),
    type,
    babl_component_from_id (BABL_RED_NONLINEAR),
    babl_component_from_id (BABL_GREEN_NONLINEAR),
    babl_component_from_id (BABL_BLUE_NONLINEAR),
    NULL);

  babl_format_new (
    babl_model_from_id (BABL_RGB_PERCEPTUAL),
    type,
    babl_component_from_id (BABL_RED_PERCEPTUAL),
    babl_component_from_id (BABL_GREEN_PERCEPTUAL),
    babl_component_from_id (BABL_BLUE_PERCEPTUAL),
    NULL);
  babl_format_new (
    babl_model_from_id (BABL_RGB_PERCEPTUAL),
    type,
    babl_component_from_id (BABL_RED_PERCEPTUAL),
    babl_component_from_id (BABL_GREEN_PERCEPTUAL),
    babl_component_from_id (BABL_BLUE_PERCEPTUAL),
    NULL);

  babl_format_new (
    babl_model_from_id (BABL_MODEL_GRAY_NONLINEAR),
    type,
    babl_component_from_id (BABL_GRAY_NONLINEAR),
    NULL);

  babl_format_new (
    babl_model_from_id (BABL_MODEL_GRAY_PERCEPTUAL),
    type,
    babl_component_from_id (BABL_GRAY_PERCEPTUAL),
    NULL);

  babl_format_new (
    babl_model_from_id (BABL_MODEL_GRAY_NONLINEAR_ALPHA),
    type,
    babl_component_from_id (BABL_GRAY_NONLINEAR),
    babl_component_from_id (BABL_ALPHA),
    NULL);

  babl_format_new (
    babl_model_from_id (BABL_MODEL_GRAY_PERCEPTUAL_ALPHA),
    type,
    babl_component_from_id (BABL_GRAY_PERCEPTUAL),
    babl_component_from_id (BABL_ALPHA),
    NULL);

  babl_format_new (
    babl_model_from_id (BABL_MODEL_GRAY_NONLINEAR_ALPHA_PREMULTIPLIED),
    type,
    babl_component_from_id (BABL_GRAY_NONLINEAR_MUL_ALPHA),
    babl_component_from_id (BABL_ALPHA),
    NULL);


  babl_format_new (
    babl_model_from_id (BABL_RGBA),
    type,
    babl_component_from_id (BABL_RED),
    babl_component_from_id (BABL_GREEN),
    babl_component_from_id (BABL_BLUE),
    babl_component_from_id (BABL_ALPHA),
    NULL);

  babl_format_new (
    babl_model_from_id (BABL_RGB),
    type,
    babl_component_from_id (BABL_RED),
    babl_component_from_id (BABL_GREEN),
    babl_component_from_id (BABL_BLUE),
    NULL);

  babl_format_new (
    babl_model_from_id (BABL_RGBA_PREMULTIPLIED),
    type,
    babl_component_from_id (BABL_RED_MUL_ALPHA),
    babl_component_from_id (BABL_GREEN_MUL_ALPHA),
    babl_component_from_id (BABL_BLUE_MUL_ALPHA),
    babl_component_from_id (BABL_ALPHA),
    NULL);

  babl_format_new (
    babl_model_from_id (BABL_GRAY_ALPHA),
    type,
    babl_component_from_id (BABL_GRAY_LINEAR),
    babl_component_from_id (BABL_ALPHA),
    NULL);
  babl_format_new (
    babl_model_from_id (BABL_GRAY_ALPHA_PREMULTIPLIED),
    type,
    babl_component_from_id (BABL_GRAY_LINEAR_MUL_ALPHA),
    babl_component_from_id (BABL_ALPHA),
    NULL);
  babl_format_new (
    babl_model_from_id (BABL_GRAY),
    type,
    babl_component_from_id (BABL_GRAY_LINEAR),
    NULL);

  }

  /* overriding name, since the generated name would be wrong due
   * to differing types
   */
  babl_format_new (
    "name", "Y'CbCr u8",
    "planar",
    babl_model_from_id (BABL_YCBCR),
    babl_type_from_id (BABL_U8_LUMA),
    babl_sampling (1, 1),
    babl_component_from_id (BABL_GRAY_NONLINEAR),
    babl_type_from_id (BABL_U8_CHROMA),
    babl_sampling (2, 2),
    babl_component_from_id (BABL_CB),
    babl_sampling (2, 2),
    babl_component_from_id (BABL_CR),
    NULL);
  babl_format_new (
    babl_model_from_id (BABL_YCBCR),
    babl_type_from_id (BABL_FLOAT),
    babl_component_from_id (BABL_GRAY_NONLINEAR),
    babl_type_from_id (BABL_FLOAT),
    babl_component_from_id (BABL_CB),
    babl_component_from_id (BABL_CR),
    NULL);
  babl_format_new (
    babl_model_from_id (BABL_YCBCR_ALPHA),
    babl_type_from_id (BABL_FLOAT),
    babl_component_from_id (BABL_GRAY_NONLINEAR),
    babl_type_from_id (BABL_FLOAT),
    babl_component_from_id (BABL_CB),
    babl_component_from_id (BABL_CR),
    babl_component_from_id (BABL_ALPHA),
    NULL);
}
