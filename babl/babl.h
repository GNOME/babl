/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2005-2008, Øyvind Kolås.
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

/* magic number used at the start of all babl objects, used to do
 * differentiation in polymorphic functions. (as well as manual
 * type check assertions).
 */
#define BABL_MAGIC   0xbab100

/* Alpha threshold used in the reference implementation for
 * un-pre-multiplication of color data:
 *
 * 0.01 / (2^16 - 1)
 */
#define BABL_ALPHA_THRESHOLD 0.000000152590219

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

typedef union _Babl Babl;

typedef struct _BablList BablList;

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
const char * babl_name       (const Babl *babl);

void         babl_introspect (Babl       *babl); /* introspect a given BablObject     */

#include "babl-version.h"
#include "babl-type.h"
#include "babl-sampling.h"
#include "babl-component.h"
#include "babl-model.h"
#include "babl-format.h"
#include "babl-image.h"
#include "babl-conversion.h"
#include "babl-fish.h"
#include "babl-extension.h"

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
