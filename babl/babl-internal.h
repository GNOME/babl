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

#ifndef _BABL_INTERNAL_H
#define _BABL_INTERNAL_H

#define BABL_MAX_COMPONENTS 32

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "babl-classes.h"
#include "babl-instance.h"

#include "babl-ids.h"
#include "babl-util.h"
#include "babl-memory.h"
#include "babl-introspect.h"

#include "babl-classes.h"

/* internal classes */
#include "babl-conversion.h"
#include "babl-extension.h"
/* */

/**** LOGGER ****/
#include <stdarg.h>

static inline void
real_babl_log (const char *file,
               int         line,
               const char *function,
               const char *fmt, ...)
{
  Babl *extender = babl_extender();
  va_list  varg;
  

  if (extender != babl_extension_quiet_log())
    {
      if (babl_extender())
        fprintf (stdout, "When loading %s:\n\t", babl_extender()->instance.name);

      fprintf (stdout, "%s:%i %s()\n\t", file, line, function);
    }

  va_start (varg, fmt);
  vfprintf (stdout, fmt, varg);
  va_end (varg);

  fprintf (stdout, "\n");
}

#define babl_log(args...)  real_babl_log(__FILE__, __LINE__, __FUNCTION__, args)

/********************/

#define BABL_CLASS_TYPE_IS_VALID(klass_type) \
    (  ((klass_type)>=BABL_INSTANCE ) && ((klass_type)<=BABL_SKY) ?1:0 )

#define BABL_IS_BABL(obj)                              \
(NULL==(obj)?0:                                        \
 BABL_CLASS_TYPE_IS_VALID(((Babl*)(obj))->class_type)  \
)

extern int   babl_hmpf_on_name_lookups;

const char  *babl_class_name     (BablClassType klass);
void         babl_internal_init    (void);
void         babl_internal_destroy (void);

extern int babl_hmpf_on_name_lookups;

#define BABL_DEFINE_EACH(type_name)                           \
void                                                          \
type_name##_each (BablEachFunction  each_fun,                 \
                  void             *user_data)                \
{                                                             \
  db_each (each_fun, user_data);                              \
}                                                             \

#define BABL_DEFINE_LOOKUP_BY_ID(type_name)                   \
Babl *                                                        \
type_name##_id (int id)                                       \
{                                                             \
  Babl *babl;                                                 \
  babl = db_exist (id, NULL);                                 \
  if (!babl)                                                  \
    {                                                         \
      babl_log ("%s(%i): not found", __FUNCTION__, id);   \
    }                                                         \
  return babl;                                                \
}

#define BABL_DEFINE_LOOKUP_BY_NAME(type_name)                 \
Babl *                                                        \
type_name (const char *name)                                  \
{                                                             \
  Babl *babl;                                                 \
                                                              \
  if (babl_hmpf_on_name_lookups)                              \
    {                                                         \
      babl_log ("%s(\"%s\"): hmpf!", __FUNCTION__, name);     \
    }                                                         \
  babl = db_exist (0, name);                                  \
                                                              \
  if (!babl)                                                  \
    {                                                         \
      babl_log ("%s(\"%s\"): not found", __FUNCTION__, name); \
    }                                                         \
  return babl;                                                \
}

#ifndef BABL_INIT_HOOK
#define BABL_INIT_HOOK
#endif
#ifndef BABL_PRE_INIT_HOOK
#define BABL_PRE_INIT_HOOK
#endif
#ifndef BABL_DESTROY_HOOK
#define BABL_DESTROY_HOOK
#endif
#ifndef BABL_DESTROY_PRE_HOOK
#define BABL_DESTROY_PRE_HOOK
#endif

#define BABL_DEFINE_INIT(type_name)                           \
void                                                          \
type_name##_init (void)                                       \
{                                                             \
  BABL_PRE_INIT_HOOK;                                         \
  db_init ();                                                 \
  BABL_INIT_HOOK;                                             \
}

#define BABL_DEFINE_DESTROY(type_name)                        \
void                                                          \
type_name##_destroy (void)                                    \
{                                                             \
  BABL_DESTROY_PRE_HOOK;                                      \
  db_each (each_##type_name##_destroy, NULL);                 \
  db_destroy ();                                              \
  BABL_DESTROY_HOOK;                                          \
}

#define BABL_CLASS_TEMPLATE(type_name)                        \
BABL_DEFINE_INIT           (type_name)                        \
BABL_DEFINE_DESTROY        (type_name)                        \
BABL_DEFINE_LOOKUP_BY_NAME (type_name)                        \
BABL_DEFINE_EACH           (type_name)                        \
BABL_DEFINE_LOOKUP_BY_ID   (type_name)               

#define BABL(obj)  ((Babl*)(obj))

#endif
