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

#ifndef _BABL_IDS_H
#define _BABL_IDS_H

enum {
  BABL_UNDEFINED = 0,
  BABL_TYPE_BASE = 100,
  BABL_U8,
  BABL_U16,
  BABL_FLOAT,
  BABL_DOUBLE,
  BABL_HALF_FLOAT,

  BABL_COMPONENT_BASE = 1000,
  BABL_RED,
  BABL_GREEN,
  BABL_BLUE,
  BABL_ALPHA,
  BABL_LUMINANCE,
  BABL_LUMINANCE_GAMMA_2_2,
  BABL_LUMINANCE_MUL_ALPHA,
  BABL_RED_MUL_ALPHA,
  BABL_GREEN_MUL_ALPHA,
  BABL_BLUE_MUL_ALPHA,
  BABL_RED_GAMMA_2_2,
  BABL_GREEN_GAMMA_2_2,
  BABL_BLUE_GAMMA_2_2,
  BABL_CYAN,
  BABL_MAGENTA,
  BABL_YELLOW,
  BABL_KEY,
  BABL_CB,
  BABL_CR,
  BABL_LAB_L,
  BABL_LAB_A,
  BABL_LAB_B,
  BABL_X,
  BABL_Y,
  BABL_Z,
  BABL_Z_BUFFER,
  BABL_PADDING,

  BABL_MODEL_BASE = 10000,
  BABL_RGB,
  BABL_RGB_GAMMA_2_2,
  BABL_RGBA,
  BABL_RGBA_GAMMA_2_2,
  BABL_RGBA_PREMULTIPLIED,
  BABL_CMY,
  BABL_CMYK,
  BABL_CMYKA,
  BABL_YCBCR,
  BABL_GRAYSCALE,
  BABL_GRAYSCALE_GAMMA_2_2,
  BABL_GRAYSCALE_ALPHA,
  BABL_GRAYSCALE_ALPHA_PREMULTIPLIED,

  BABL_PIXEL_FORMAT_BASE = 100000,
  BABL_SRGB,
  BABL_SRGBA,
  BABL_RGB_FLOAT,
  BABL_RGBA_FLOAT,
  BABL_CMYK_FLOAT,
  BABL_CMYKA_FLOAT,
  BABL_YUV420,
#if 0
  BABL_RGB_U8,
  BABL_RGBA_U8,
  BABL_RGBA_U16,
  BABL_RGBA_FLOAT,
  BABL_YUV411,
  BABL_YUV422,
#endif

  BABL_PIXEL_USER_BASE,
};


#endif



