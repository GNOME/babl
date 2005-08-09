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

typedef enum {
  BABL_INSTANCE = 0xBAB10000,
  BABL_TYPE,
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
  BABL_IMAGE,

  BABL_SKY
} BablClassType;

#define BABL_CLASS_TYPE_IS_VALID(klass_type) \
    (  ((klass_type)>=BABL_INSTANCE ) && ((klass_type)<=BABL_SKY) ?1:0 )

/* common header for any item inserted into database */
typedef struct
{
  BablClassType  type;
  int            id;      /*< a numerical id, look at 'babl-ids.h' for the reserved
                              ones */
  char          *name;    /*< the name this type exists under         */
} BablInstance;

typedef struct
{
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

typedef struct
{
  BablConversion conversion;
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
} BablType;

typedef struct
{
  BablInstance instance;
  BablConversion **from; /*< NULL terminated list of conversions from class */
  BablConversion **to;   /*< NULL terminated list of conversions to class   */
  int          horizontal;
  int          vertical;
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
  int              planar;
  int              bands;
  BablComponent  **component;
  BablType       **type;
  BablSampling   **sampling;
  BablConversion  *to_conversions;
  BablConversion  *from_conversions;
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

typedef union
{
  BablClassType   kind;
  BablInstance    instance;
  BablType        type;
  BablSampling    sampling;
  BablComponent   component;
  BablModel       model;
  BablPixelFormat pixel_format;
  BablConversion  conversion;
  BablFish        fish;
  BablImage        image;
} Babl;


#define BABL_NAME(obj)          (((Babl*)(obj))->instance.name)
#define BABL_INSTANCE_TYPE(obj) (((Babl*)(obj))->kind)

#define BABL_IS_BABL(obj)\
(NULL==(obj)?0:BABL_CLASS_TYPE_IS_VALID(BABL_INSTANCE_TYPE(obj)))

#define BABL_IS_OF_KIND(obj,kind) \
    (!BABL_IS_BABL(obj)?0:kind==BABL_INSTANCE_TYPE(obj)?1:0)

#define BABL_IS_INSTANCE(obj)      BABL_IS_OF_KIND(obj,BABL_TYPE)
#define BABL_IS_TYPE(obj)          BABL_IS_OF_KIND(obj,BABL_TYPE)
#define BABL_IS_COMPONENT(obj)     BABL_IS_OF_KIND(obj,BABL_COMPONENT)
#define BABL_IS_SAMPLING(obj)      BABL_IS_OF_KIND(obj,BABL_SAMPLING)
#define BABL_IS_PIXEL_FORMAT(obj)  BABL_IS_OF_KIND(obj,BABL_PIXEL_FORMAT)

typedef int  (*BablEachFunction) (Babl *entry,
                                  void *data);

const char  *babl_class_name     (BablClassType klass);



#define BABL_DEFINE_CLASS(TypeName, type_name)                   \
                                                                 \
void       type_name##_init       (void);                        \
void       type_name##_destroy    (void);                        \
void       type_name##_each       (BablEachFunction  each_fun,   \
                                   void             *user_data); \
TypeName * type_name              (const char       *name);      \
TypeName * type_name##_id         (int               id);        \
TypeName * type_name##_new        (const char       *name,       \
                                   ...);


#define BABL_DEFINE_CLASS_NO_NEW_NO_ID(TypeName, type_name)      \
                                                                 \
void       type_name##_init       (void);                        \
void       type_name##_destroy    (void);                        \
void       type_name##_each       (BablEachFunction  each_fun,   \
                                   void             *user_data);

#endif

