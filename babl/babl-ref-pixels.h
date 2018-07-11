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

#ifndef _BABL_REF_PIXELS_H
#define _BABL_REF_PIXELS_H

int babl_get_num_path_test_pixels (void);
int babl_get_num_conversion_test_pixels (void);
int babl_get_num_format_test_pixels (void);
int babl_get_num_model_test_pixels (void);
int babl_get_num_type_test_pixels (void);

const double *babl_get_path_test_pixels (void);
const double *babl_get_conversion_test_pixels (void);
const double *babl_get_format_test_pixels (void);
const double *babl_get_model_test_pixels (void);
const double *babl_get_type_test_pixels (void);

#endif /* _BABL_REF_PIXELS_H */