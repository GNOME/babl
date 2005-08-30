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

/* Type and Format */
typedef void (*BablFuncLinear)    (void  *src,
                                   void  *dst,
                                   int    src_pitch,
                                   int    dst_pitch,
                                   int    n);

/* TypePlanar, ModelPlanar and FormatPlanar */
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
  BABL_FORMAT,

  BABL_CONVERSION,
  BABL_CONVERSION_TYPE,
  BABL_CONVERSION_TYPE_PLANAR,
  BABL_CONVERSION_MODEL_PLANAR,
  BABL_CONVERSION_FORMAT,
  BABL_CONVERSION_FORMAT_PLANAR,

  BABL_FISH,
  BABL_FISH_REFERENCE,
  BABL_IMAGE,

  BABL_EXTENSION,

  BABL_SKY
} BablClassType;



/* common header for any item inserted into database */
typedef struct
{
  BablClassType  class_type;
  int            id;      /*< a numerical id, look at 'babl-ids.h' for the reserved
                              ones */
  void          *creator;
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
  int                    processings;
  long                   pixels;
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
} BablConversionFormat;

typedef struct
{
  BablConversion conversion;
} BablConversionFormatPlanar;

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
  BablInstance     instance;
  BablConversion **from; /*< NULL terminated list of conversions from class */
  BablConversion **to;   /*< NULL terminated list of conversions to class   */
  int              horizontal;
  int              vertical;
  char             name[4];
} BablSampling;

typedef struct
{
  BablInstance     instance;
  BablConversion **from; /*< NULL terminated list of conversions from class */
  BablConversion **to;   /*< NULL terminated list of conversions to class   */
  int              luma;
  int              chroma;
  int              alpha;
} BablComponent;


typedef struct
{
  BablInstance     instance;
  BablConversion **from; /*< NULL terminated list of conversions from class */
  BablConversion **to;   /*< NULL terminated list of conversions to class   */
  int              components;
  BablComponent  **component;
  BablType       **type; /*< must be doubles, used here for convenience in code */
} BablModel;

typedef struct
{
  BablInstance     instance;
  BablConversion **from; /*< NULL terminated list of conversions from class */
  BablConversion **to;   /*< NULL terminated list of conversions to class   */
  int              components;
  BablComponent  **component;
  BablType       **type;
  BablSampling   **sampling;
  BablModel       *model;
  int              bytes_per_pixel;
  int              planar;
} BablFormat;

typedef struct
{
  BablInstance    instance;
  BablFormat     *format;    /*< (if known) */
  BablModel      *model;     /*< (always known) */
  int             components;
  BablComponent **component;
  BablSampling  **sampling;
  BablType      **type;
  void          **data;
  int            *pitch;
  int            *stride;
} BablImage;

typedef struct
{
  BablInstance    instance;
  union Babl     *source;
  union Babl     *destination;
  int   processings;
  long  pixels;
} BablFish;

/* BablFishReference on the double versions of conversions
 * that are required to exist for maximum sanity.
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
} BablFishReference;

typedef struct
{
  BablInstance   instance;               /* path to .so / .dll is stored in instance name */
  void          *dl_handle;
  void         (*destroy) (void); 
} BablExtension;

typedef union
{
  BablClassType     class_type;
  BablInstance      instance;
  BablType          type;
  BablSampling      sampling;
  BablComponent     component;
  BablModel         model;
  BablFormat        format;
  BablConversion    conversion;
  BablImage         image;
  BablFish          fish;
  BablFishReference reference_fish;
  BablExtension     extension;
} Babl;

#endif

