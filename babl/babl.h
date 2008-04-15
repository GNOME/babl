/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2005, Øyvind Kolås.
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

/* NOTE! the babl API is polymorphic, it is probably easier to learn
 * it's usage by studying examples than this header file. The public
 * functions are attempted explained anyways.
 */

#ifndef _BABL_H
#define _BABL_H

#include <stdlib.h>

#ifdef _BABL_INTERNAL_H
#error babl.h included after babl-internal.h
#endif

typedef union _Babl Babl;
/* Union used for quick convenient access to any field of any BablInstance */

typedef struct _BablList BablList;

#ifndef BABL_HARD_MAX_PATH_LENGTH
#define BABL_HARD_MAX_PATH_LENGTH 16
#endif

/* magic number used at the start of all babl objects, used to do
 * differentiation in polymorphic functions. (as well as manual
 * type check assertions).
 */
#define BABL_MAGIC   0xbAb100

enum {
  BABL_INSTANCE = BABL_MAGIC,
  BABL_TYPE,
  BABL_TYPE_INTEGER,
  BABL_TYPE_FLOAT,
  BABL_SAMPLING,
  BABL_COMPONENT,
  BABL_MODEL,
  BABL_FORMAT,

  BABL_CONVERSION,
  BABL_CONVERSION_LINEAR,
  BABL_CONVERSION_PLANE,
  BABL_CONVERSION_PLANAR,

  BABL_FISH,
  BABL_FISH_REFERENCE,
  BABL_FISH_SIMPLE,
  BABL_FISH_PATH,
  BABL_IMAGE,

  BABL_EXTENSION,

  BABL_SKY
};
typedef unsigned int BablClassType;

/** Initialize the babl library */
void   babl_init       (void);

/** Deinitialize the babl library (frees any resources used, if the number
 *  of calls to babl_destroy() is is equal to the number of calls to
 *  babl_init()
 */
void   babl_destroy    (void);

#if     __GNUC__ >= 4
#define BABL_ARG_NULL_TERMINATED __attribute__((__sentinel__))
#else
#define BABL_ARG_NULL_TERMINATED
#endif

typedef int  (*BablEachFunction) (Babl *entry,
                                  void *data);

/* All Classes in babl have common functionality like the ability
 * to be iterated over, common functionality is defined through these
 * macros.
 */
#define BABL_CLASS(klass)                                    \
                                                             \
void   babl_##klass##_init    (void);                        \
void   babl_##klass##_destroy (void);                        \
Babl * babl_##klass##_id      (int id);                      \
void   babl_##klass##_each    (BablEachFunction  each_fun,   \
                              void             *user_data)

/* creates a class that has a specific name connected to it, that
 * also allows defining a new instance. These classes share common
 * functionality with the non_name classes but have two extra methods,
 * the means to lookup by name, as well as to create new named objects
 * that later can be looked up.
 */
#define BABL_NAMED_CLASS(klass)                              \
                                                             \
BABL_CLASS (klass);                                          \
Babl * babl_##klass           (const char       *name);      \
Babl * babl_##klass##_new     (void             *first_arg,  \
                              ...) BABL_ARG_NULL_TERMINATED



/* common header for any item inserted into database, the actual
 * implementation of babl-instance is in babl-internal
 */
typedef struct
{
  BablClassType  class_type;
  int            id;      /*< a numerical id, look at 'babl-ids.h' for the reserved
                              ones */
  void          *creator;
  char          *name;    /*< the name this type exists under         */
} BablInstance;

/**
 * babl_name:
 *
 * Return a string decsribing a BablInstance, might work better than
 * babl->instance.name when a good human readable name is desired.
 *
 * Returns: a name describing the instance.
 */
const char *babl_name       (const Babl *babl);
void        babl_introspect (Babl *babl); /* introspect a given BablObject     */

/****************************************************************/
/* BablType */
BABL_NAMED_CLASS (type);
/*
 * A data type that babl can have in it's buffers, requires
 * conversions to and from "double" to be registered before
 * passing sanity.
 *
 * Babl * babl_type_new (  const char *name,
 *                         "bits",     int bits,
 *                         ["min_val", double min_val,]
 *                         ["max_val", double max_val,]
 *                         NULL);
 */
Babl *babl_type_id     (int id);
void  babl_type_each   (BablEachFunction  each_fun,
                        void             *user_data);
Babl * babl_type       (const char       *name);
Babl * babl_type_new   (void             *first_arg,
                        ...) BABL_ARG_NULL_TERMINATED;

typedef struct
{
  BablInstance     instance;
  BablList         *from_list;
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


/****************************************************************/
/* BablSampling */
BABL_CLASS (sampling);
/**/
Babl * babl_sampling       (int horizontal,
                            int vertical);
Babl *babl_sampling_id     (int id);
void  babl_sampling_each   (BablEachFunction  each_fun,
                            void             *user_data);
typedef struct
{
  BablInstance     instance;
  BablList         *from_list;
  int              horizontal;
  int              vertical;
  char             name[4];
} BablSampling;


/****************************************************************/
/* BablComponent */
BABL_NAMED_CLASS (component);
/*
 * Babl * babl_component_new (const char *name,
 *                            NULL);
 */
Babl *babl_component_id     (int id);
void  babl_component_each   (BablEachFunction  each_fun,
                        void             *user_data);
Babl * babl_component       (const char       *name);
Babl * babl_component_new   (void             *first_arg,
                        ...) BABL_ARG_NULL_TERMINATED;

typedef struct
{
  BablInstance     instance;
  int              luma;
  int              chroma;
  int              alpha;
} BablComponent;


/****************************************************************/
/* BablModel */
BABL_NAMED_CLASS (model);
/*
 * Babl * babl_model_new (["name", const char *name,]
 *                        BablComponent *component1,
 *                       [BablComponent *componentN, ...]
 *                        NULL);
 *
 * If no name is provided a name is generated by concatenating the
 * name of all the involved components.
 *
 */
Babl *babl_model_id     (int id);
void  babl_model_each   (BablEachFunction  each_fun,
                        void             *user_data);
Babl * babl_model       (const char       *name);
Babl * babl_model_new   (void             *first_arg,
                        ...) BABL_ARG_NULL_TERMINATED;

typedef struct
{
  BablInstance     instance;
  BablList         *from_list;
  int              components;
  BablComponent  **component;
  BablType       **type; /*< must be doubles, used here for convenience in code */
} BablModel;


/****************************************************************/
/* BablFormat */
BABL_NAMED_CLASS (format);
/*
 * Babl * babl_format_new (["name", const char *name,]
 *                          BablModel          *model,
 *                         [BablType           *type,]
 *                         [BablSampling,      *sampling,]
 *                          BablComponent      *component1,
 *                        [[BablType           *type,]
 *                         [BablSampling       *sampling,]
 *                          BablComponent      *componentN,
 *                        ...]
 *                          ["planar",]
 *                          NULL);
 *
 * Provided BablType and|or BablSampling is valid for the following
 * components as well. If no name is provided a (long) descriptive
 * name is used.
 */
Babl *babl_format_id     (int id);
void  babl_format_each   (BablEachFunction  each_fun,
                        void             *user_data);
Babl * babl_format       (const char       *name);
Babl * babl_format_new   (void             *first_arg,
                        ...) BABL_ARG_NULL_TERMINATED;

typedef struct
{
  BablInstance     instance;
  BablList         *from_list;
  int              components;
  BablComponent  **component;
  BablType       **type;
  void            *image_template; /* image template for use with 
                                       linear (non-planer) images */

  BablSampling   **sampling;
  BablModel       *model;
  int              bytes_per_pixel;
  int              planar;
  double           loss; /*< average relative error when converting
                             from and to RGBA double */
  int              visited; /* for convenience in code while searching 
                               for conversion paths */
} BablFormat;


/****************************************************************/
/* BablImage */
BABL_CLASS (image);
/*
 * Babl images can be used for planar buffers instead of linear buffers for
 * babl_process(), BablImages are still experimental, for now BablImages can be
 * passed to babl_process, two different babl_process() functions will be
 * needed for this since the polymorphism cannot be trusted to work on linear
 * buffers that originate outside babl's control.
 * 
 * Babl * babl_image (BablComponent *component1,
 *                    void          *data,
 *                    int            pitch,
 *                    int            stride,
 *                   [BablComponent *component1,
 *                    void          *data,
 *                    int            pitch,
 *                    int            stride,
 *                    ...]
 *                    NULL);
 */
Babl * babl_image      (void *first_component,
                        ...) BABL_ARG_NULL_TERMINATED;
Babl *babl_image_id     (int id);
void  babl_image_each   (BablEachFunction  each_fun,
                        void             *user_data);
typedef struct
{
  BablInstance    instance;
  BablFormat     *format;    /*< (if known) */
  BablModel      *model;     /*< (always known) */
  int             components;
  BablComponent **component;
  BablType      **type;
  BablSampling  **sampling;
  char          **data;
  int            *pitch;
  int            *stride;
} BablImage;


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


/****************************************************************/
/* BablFish */
BABL_CLASS (fish);
/*  Create a babl fish capable of converting from source_format to
 *  destination_format, source and destination can be
 *  either strings with the names of the formats or BablFormat objects.
 */
Babl * babl_fish       (const void *source_format,
                        const void *destination_format);

/** Process n pixels from source to destination using babl_fish,
 *  returns number of pixels converted. 
 */
long   babl_process    (Babl *babl_fish,
                        void *source,
                        void *destination,
                        long  n);

Babl *babl_fish_id     (int id);
void  babl_fish_each   (BablEachFunction  each_fun,
                        void             *user_data);

/* BablFish, common base class for various fishes.
 */
typedef struct
{
  BablInstance    instance;
  const Babl     *source;
  const Babl     *destination;

  double          error;    /* the amount of noise introduced by the fish */

  /* instrumentation */
  int             processings; /* number of times the fish has been used */
  long            pixels;      /* number of pixels translates */
  long            usecs;       /* usecs spent within this fish */
} BablFish;

/* BablFishSimple is the simplest type of fish, wrapping a single
 * conversion function, (note this might not be the optimal chosen
 * conversion even if it exists)
 *
 * TODO: exterminate
 */
typedef struct
{
  BablFish         fish;
  BablConversion  *conversion;
} BablFishSimple;


/* BablFishPath is a combination of registered conversions, both
 * from the reference types / model conversions, and optimized format to
 * format conversion.
 *
 * This is the most advanced scheduled species of fish, some future
 * version of babl might even be evovling path fishes in a background
 * thread, based on the fish instrumentation. For this to work in a future
 * version transmogrification between the fish classes would be used.
 */
typedef struct
{
  BablFish         fish;
  double           cost;   /* number of  ticks *10 + chain_length */
  double           loss;   /* error introduced */
  BablList         *conversion_list;
} BablFishPath;

/* BablFishReference
 *
 * A BablFishReference is not intended to be fast, thus the algorithm
 * encoded can use a multi stage approach, based on the knowledge babl
 * has encoded in the pixel formats.
 *
 * One of the contributions that would be welcome are new fish factories.
 *
 * TODO:
 *   * make optimal use of a single allocation containing enough space
 *     for the maximum amount of memory needed in two adjecant buffers
 *     at any time.
 */
typedef struct
{
  BablFish         fish;
} BablFishReference;


/****************************************************************/
/* BablExtension */
BABL_NAMED_CLASS (extension);
/*
 * BablExtension objects are only used internally in babl.
 */
Babl *babl_extension_id     (int id);
void  babl_extension_each   (BablEachFunction  each_fun,
                        void             *user_data);
Babl * babl_extension       (const char       *name);
Babl * babl_extension_new   (void             *first_arg,
                        ...) BABL_ARG_NULL_TERMINATED;

typedef struct
{
  BablInstance   instance; /* path to .so / .dll is stored in instance name */
  void          *dl_handle;
  void         (*destroy) (void);
} BablExtension;



/* This union can be used for convenient access to any field without the need
 * to case if the variable already is of the type Babl *
 */
typedef union _Babl
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
  BablFishReference fish_reference;
  BablFishSimple    fish_simple;
  BablFishPath      fish_path;
  BablExtension     extension;
} _Babl;

#undef BABL_CLASS
#undef BABL_NAMED_CLASS

#endif
