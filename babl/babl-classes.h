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

#ifndef _BABL_CLASSES_H
#define _BABL_CLASSES_H

/* Type and PixelFormat */
typedef void (*BablFuncLinear)    (void  *src,
                                   void  *dst,
                                   int    n);

/* TypePlanar, ModelPlanar and PixelFormatPlanar */
typedef void (*BablFuncPlanar)    (int    src_bands,
                                   void  *src[],
                                   int    src_pitch[],
                                   int    dst_bands,
                                   void  *dst[],
                                   int    dst_pitch[],
                                   int    n);

typedef void (*BablFuncPlanarBit) (int    src_bands,
                                   void  *src[],
                                   int    src_bit[],
                                   int    src_pitch[],
                                   int    src_bit_pitch[],
                                   int    dst_bands,
                                   void  *dst[],
                                   int    dst_bit[],
                                   int    dst_pitch[],
                                   int    dst_bit_pitch[],
                                   int    n);

/* magic number used at the start of all babl objects, used to do
 * differentiation in polymorphic functions. (as well as manual
 * type check assertions).
 */
#define BABL_MAGIC   0xbAb10000

typedef enum {
  BABL_INSTANCE = BABL_MAGIC,
  BABL_TYPE,
  BABL_TYPE_INTEGER,
  BABL_TYPE_FLOAT,
  BABL_SAMPLING,
  BABL_COMPONENT,
  BABL_MODEL,
  BABL_PIXEL_FORMAT,

  BABL_CONVERSION,
  BABL_CONVERSION_TYPE,
  BABL_CONVERSION_TYPE_PLANAR,
  BABL_CONVERSION_MODEL_PLANAR,
  BABL_CONVERSION_PIXEL_FORMAT,
  BABL_CONVERSION_PIXEL_FORMAT_PLANAR,

  BABL_FISH,
  BABL_FISH_REFERENCE,
  BABL_IMAGE,

  BABL_SKY
} BablClassType;

#define BABL_CLASS_TYPE_IS_VALID(klass_type) \
    (  ((klass_type)>=BABL_INSTANCE ) && ((klass_type)<=BABL_SKY) ?1:0 )

const char  *babl_class_name     (BablClassType klass);

/* common header for any item inserted into database */
typedef struct
{
  BablClassType  class_type;
  int            id;      /*< a numerical id, look at 'babl-ids.h' for the reserved
                              ones */
  char          *name;    /*< the name this type exists under         */
} BablInstance;

typedef struct
BablConversion {
  BablInstance           instance;
  union Babl            *source;
  union Babl            *destination;
  int                    time_cost;
  int                    loss;
  union
    {
      BablFuncLinear     linear;
      BablFuncPlanar     planar;
      BablFuncPlanarBit  planar_bit;
    } function;
} BablConversion;

typedef struct {
  BablConversion       conversion;
  BablConversion      *from_double;
  BablConversion      *to_double;
} BablConversionType;

typedef struct
{
  BablConversion conversion;
} BablConversionTypePlanar;

typedef struct
{
  BablConversion conversion;
} BablConversionModelPlanar;

typedef struct
{
  BablConversion conversion;
} BablConversionPixelFormat;

typedef struct
{
  BablConversion conversion;
} BablConversionPixelFormatPlanar;

typedef struct
{
  BablInstance     instance;
  BablConversion **from;  /*< NULL terminated list of conversions from class */
  BablConversion **to;    /*< NULL terminated list of conversions to class   */
  int              bits;  /*< number of bits used to represent the data type
                            (initially restricted to a multiple of 8) */
  double           min_val;
  double           max_val;
} BablType;

typedef struct
{
  BablType          type;
  int               is_signed;
  long              max;
  long              min;
} BablTypeInteger;

typedef struct
{
  BablType type;
  /* sign
   * biased_exponent
   * mantissa */
} BablTypeFloat;

typedef struct
{
  BablInstance instance;
  BablConversion **from; /*< NULL terminated list of conversions from class */
  BablConversion **to;   /*< NULL terminated list of conversions to class   */
  int          horizontal;
  int          vertical;
  char         name[4];
} BablSampling;

typedef struct
{
  BablInstance instance;
  BablConversion **from; /*< NULL terminated list of conversions from class */
  BablConversion **to;   /*< NULL terminated list of conversions to class   */
  int          luma;
  int          chroma;
  int          alpha;
} BablComponent;


typedef struct
{
  BablInstance     instance;
  BablConversion **from; /*< NULL terminated list of conversions from class */
  BablConversion **to;   /*< NULL terminated list of conversions to class   */
  int              components;
  BablComponent  **component;
} BablModel;

typedef struct
{
  BablInstance     instance;
  BablConversion **from; /*< NULL terminated list of conversions from class */
  BablConversion **to;   /*< NULL terminated list of conversions to class   */
  int              bands;
  int              planar;
  BablModel       *model;
  BablComponent  **component;
  BablType       **type;
  BablSampling   **sampling;
} BablPixelFormat;

typedef struct
{
  BablInstance    instance;
  int             bands;
  BablComponent **component;
  void          **data;
  int            *pitch;
  int            *stride;
} BablImage;

typedef struct
{
  BablInstance     instance;
  union Babl      *source;
  union Babl      *destination;
} BablFish;


/* a BablFish which is a reference babl fish relies on the double
 * versions that are required to exist for maximum sanity.
 *
 * A BablFishReference is not intended to be fast, thus the algorithm
 * encoded can use a multi stage approach, where some of the stages could
 * be completely removed for optimization reasons.
 *
 * This is not the intention of the "BablFishReference factory", it's
 * implementation is meant to be kept as small as possible wrt logic.
 *
 * One of the contributions that would be welcome are new fish factories.
 */


typedef struct
{
  BablFish         fish;

  BablConversion  *type_to_double;
  BablConversion  *model_to_rgba;
  BablConversion  *rgba_to_model;
  BablConversion  *double_to_type;
} BablFishReference;

typedef union
{
  BablClassType     class_type;
  BablInstance      instance;
  BablType          type;
  BablSampling      sampling;
  BablComponent     component;
  BablModel         model;
  BablPixelFormat   pixel_format;
  BablConversion    conversion;
  BablFish          fish;
  BablFishReference reference_fish;
  BablImage         image;
} Babl;


#define BABL_IS_BABL(obj)                              \
(NULL==(obj)?0:                                        \
 BABL_CLASS_TYPE_IS_VALID(((Babl*)(obj))->class_type)  \
)

#include "babl-instance.h"

#endif

