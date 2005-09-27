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
typedef long (*BablFuncLinear)    (void  *src,
                                   void  *dst,
                                   long   n);

/* Signature of functions registered for reference type
 * conversions,
 */
typedef long (*BablFuncPlane)     (void  *src,
                                   void  *dst,
                                   int    src_pitch,
                                   int    dst_pitch,
                                   long   n);

/* TypePlanar,ModelPlanar and FormatPlanar */
typedef long (*BablFuncPlanar)    (int    src_bands,
                                   void  *src[],
                                   int    src_pitch[],
                                   int    dst_bands,
                                   void  *dst[],
                                   int    dst_pitch[],
                                   long   n);

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
  int                    cost;
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


typedef struct
{
  BablInstance     instance;
  BablConversion **from;  /*< NULL terminated list of conversions from class */
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
  int              horizontal;
  int              vertical;
  char             name[4];
} BablSampling;

typedef struct
{
  BablInstance     instance;
  int              luma;
  int              chroma;
  int              alpha;
} BablComponent;


typedef struct
{
  BablInstance     instance;
  BablConversion **from; /*< NULL terminated list of conversions from class */
  int              components;
  BablComponent  **component;
  BablType       **type; /*< must be doubles, used here for convenience in code */
} BablModel;

typedef struct
{
  BablInstance     instance;
  BablConversion **from; /*< NULL terminated list of conversions from class */
  int              components;
  BablComponent  **component;
  BablType       **type;
  BablSampling   **sampling;
  BablModel       *model;
  int              bytes_per_pixel;
  int              planar;
  double           loss; /*< average relative error when converting 
                             from and to RGBA double */
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

/* BablFish, common base class for various fishes.
 */
typedef struct
{
  BablInstance    instance;
  union Babl     *source;
  union Babl     *destination;

  double          error;    /* the amount of noise introduced by the fish */

  /* instrumentation */
  int             processings; /* number of times the fish has been used */
  long            pixels;      /* number of pixels translates */
  long            msecs;       /* msecs spent within this fish */
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

  BablConversion  *conversion[BABL_HARD_MAX_PATH_LENGTH];
  int              conversions;
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

typedef struct
{
  BablInstance   instance; /* path to .so / .dll is stored in instance name */
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
  BablFishReference fish_reference;
  BablFishSimple    fish_simple;
  BablFishPath      fish_path;
  BablExtension     extension;
} Babl;

#endif

