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

#ifndef _BABL_CLASS_H
#define _BABL_CLASS_H

#ifndef _BABL_H
#error  this file is only to be included by babl.h
#endif


/* magic number used at the start of all babl objects, used to do
 * differentiation in polymorphic functions. (as well as manual
 * type check assertions).
 */
#define BABL_MAGIC   0xbab100

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
typedef int BablClassType;

typedef union _Babl Babl;

typedef int  (*BablEachFunction) (Babl *entry,
                                  void *data);

/* All Classes in babl have common functionality like the ability
 * to be iterated over, common functionality is defined through these
 * macros.
 */
#define BABL_CLASS_DECLARE(klass)                            \
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
#define BABL_NAMED_CLASS_DECLARE(klass)                      \
                                                             \
BABL_CLASS_DECLARE (klass);                                  \
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


#endif
