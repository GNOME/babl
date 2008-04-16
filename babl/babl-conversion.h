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

#ifndef _BABL_H
#error  this file is only to be included by babl.h 
#endif

/****************************************************************/
/* BablConversion */
BABL_NAMED_CLASS (conversion);
/*
 * Babl * babl_conversion_new (<BablFormat *source, BablFormat *destination|
 *                              BablModel  *source, BablModel  *destination|
 *                              BablType   *source, BablType   *destination>,
 *                             <"linear"|"planar">, BablConversionFunc conv_func,
 *                              NULL);
 */
Babl *babl_conversion_id     (int id);
void  babl_conversion_each   (BablEachFunction  each_fun,
                        void             *user_data);
Babl * babl_conversion       (const char       *name);
Babl * babl_conversion_new   (void             *first_arg,
                        ...) BABL_ARG_NULL_TERMINATED;

/* Type and Format */
typedef long (*BablFuncLinear)    (char  *src,
                                   char  *dst,
                                   long   n);

/* Signature of functions registered for reference type
 * conversions,
 */
typedef long (*BablFuncPlane)     (char  *src,
                                   char  *dst,
                                   int    src_pitch,
                                   int    dst_pitch,
                                   long   n);

/* TypePlanar,ModelPlanar and FormatPlanar */
typedef long (*BablFuncPlanar)    (int    src_bands,
                                   char  *src[],
                                   int    src_pitch[],
                                   int    dst_bands,
                                   char  *dst[],
                                   int    dst_pitch[],
                                   long   n);

typedef struct
BablConversion {
  BablInstance           instance;
  const Babl            *source;
  const Babl            *destination;
  long                   cost;
  double                 error;
  union
    {
      BablFuncLinear     linear;
      BablFuncPlane      plane;
      BablFuncPlanar     planar;
    } function;
  int                    processings;
  long                   pixels;
} BablConversion;
