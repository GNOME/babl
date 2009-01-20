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

#ifndef _BABL_INTERNAL_H
#define _BABL_INTERNAL_H

#ifdef _BABL_H
#error babl-internal.h included after babl.h
#endif

#define BABL_MAX_COMPONENTS       32
#define BABL_CONVERSIONS           5

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "assert.h"

#undef  _BABL_INTERNAL_H
#include "babl.h"
#define _BABL_INTERNAL_H

#include "config.h"

#include "babl-list.h"
#include "babl-hash-table.h"
#include "babl-db.h"
#include "babl-ids.h"
#include "babl-util.h"
#include "babl-memory.h"
#include "babl-cpuaccel.h"

/* redefining some functions for the win32 platform */
#ifdef _WIN32
#define srandom srand
#define random  rand
#endif

/* fallback to floor function when rint is not around */
#ifndef HAVE_RINT
# define rint(f)  (floor (((double) (f)) + 0.5))
#endif



Babl *   babl_conversion_find           (const void     *source,
                                         const void     *destination);
double   babl_conversion_error          (BablConversion *conversion);
long     babl_conversion_cost           (BablConversion *conversion);
long     babl_conversion_process        (Babl           *conversion,
                                         char           *source,
                                         char           *destination,
                                         long            n);

Babl   * babl_extension_base            (void);

Babl   * babl_extender                  (void);
void     babl_set_extender              (Babl           *new_extender);

Babl   * babl_extension_quiet_log       (void);

long     babl_fish_process              (Babl           *babl,
                                         void           *source,
                                         void           *destination,
                                         long            n);
long     babl_fish_reference_process    (Babl           *babl,
                                         BablImage      *source,
                                         BablImage      *destination,
                                         long            n);

BablDb * babl_fish_db                   (void);
Babl   * babl_fish_reference            (const Babl     *source,
                                         const Babl     *destination);
Babl   * babl_fish_simple               (BablConversion *conversion);
void     babl_fish_stats                (FILE           *file);
Babl   * babl_fish_path                 (const Babl     *source,
                                         const Babl     *destination);

long     babl_fish_path_process         (Babl           *babl,
                                         void           *source,
                                         void           *destination,
                                         long            n);
int      babl_fish_get_id               (const Babl     *source,
                                         const Babl     *destination);

double   babl_format_loss               (Babl           *babl);
Babl   * babl_image_from_linear         (char           *buffer,
                                         Babl           *format);
Babl   * babl_image_double_from_image   (Babl           *source);

double   babl_model_is_symmetric        (Babl           *babl);
void     babl_die                       (void);
int      babl_sanity                    (void);

void     babl_core_init                 (void);
Babl   * babl_format_with_model_as_type (Babl           *model,
                                         Babl           *type);
int      babl_formats_count             (void);                                     /* should maybe be templated? */
int      babl_type_is_symmetric         (Babl           *babl);

/* FIXME: nasty,. including the symbol even in files where it is
 * not needed,. and a dummy function to use it in those cases
 */
static BablDb *db=NULL;
static void hack_hack (void)
{
  if (db==NULL)
    db=NULL;
}

/**** LOGGER ****/
#include <stdarg.h>

void babl_backtrack (void);

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
  fflush (NULL);
  return;
  hack_hack ();
}

#define babl_log(args...)                               \
  real_babl_log(__FILE__, __LINE__, __func__, args)

#define babl_fatal(args...) do{                         \
  real_babl_log(__FILE__, __LINE__, __func__, args);\
  babl_die();}                                          \
while(0)


#define babl_assert(expr) do{                              \
  if(!(expr))                                              \
    {                                                      \
      babl_fatal("Eeeeek! Assertion failed: `" #expr "`"); \
      assert(expr);                                        \
    }                                                      \
}while(0)
/***** LOGGER (end)**/

#define BABL_CLASS_TYPE_IS_VALID(klass_type)                            \
    (  ((klass_type)>=BABL_INSTANCE ) && ((klass_type)<=BABL_SKY) ?1:0 )

#define BABL_IS_BABL(obj)                                          \
(NULL==(obj)?0                                                     \
            :BABL_CLASS_TYPE_IS_VALID(((Babl*)(obj))->class_type)  \
)

extern int   babl_hmpf_on_name_lookups;

const char  *babl_class_name       (BablClassType klass);
void         babl_internal_init    (void);
void         babl_internal_destroy (void);


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

/* this template is expanded in the files including babl-internal.h,
 * generating code, the declarations for these functions are found in
 * the BABL_CLASS expansions done in babl.h as well, thus babl.h needs
 * to be kept in sync with the C files.
 */

#define BABL_CLASS_MINIMAL_IMPLEMENT(klass)                   \
void                                                          \
babl_##klass##_class_init (void)                              \
{                                                             \
  BABL_PRE_INIT_HOOK;                                         \
  if (!db)                                                    \
    db=babl_db_init ();                                       \
  BABL_INIT_HOOK;                                             \
}                                                             \
                                                              \
void                                                          \
babl_##klass##_class_destroy (void)                           \
{                                                             \
  BABL_DESTROY_PRE_HOOK;                                      \
  babl_db_each (db,each_babl_##klass##_destroy, NULL);        \
  babl_db_destroy (db);                                       \
  BABL_DESTROY_HOOK;                                          \
}                                                             \
                                                              \
void                                                          \
babl_##klass##_class_for_each (BablEachFunction  each_fun,    \
                               void             *user_data)   \
{                                                             \
  babl_db_each (db, each_fun, user_data);                     \
}                                                             \

#define BABL_CLASS_IMPLEMENT(klass)                           \
BABL_CLASS_MINIMAL_IMPLEMENT(klass)                           \
                                                              \
Babl *                                                        \
babl_##klass (const char *name)                               \
{                                                             \
  Babl *babl;                                                 \
                                                              \
  if (babl_hmpf_on_name_lookups)                              \
    {                                                         \
      babl_log ("%s(\"%s\"): hmpf!", __func__, name);         \
    }                                                         \
  babl = babl_db_exist_by_name (db, name);                    \
                                                              \
  if (!babl)                                                  \
    {                                                         \
      babl_fatal ("%s(\"%s\"): not found", __func__, name);   \
    }                                                         \
  return babl;                                                \
}                                                             \
                                                              \
Babl *                                                        \
babl_##klass##_from_id (int id)                               \
{                                                             \
  Babl *babl;                                                 \
  babl = babl_db_exist_by_id (db, id);                        \
  if (!babl)                                                  \
    {                                                         \
      babl_fatal ("%s(%i): not found", __func__, id);         \
    }                                                         \
  return babl;                                                \
}

#define BABL(obj)  ((Babl*)(obj))

#endif
