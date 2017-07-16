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

#ifndef BABL_LIBRARY
#error "config.h must be included prior to babl-internal.h"
#endif

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

#include "babl-classes.h"
#include "babl-introspect.h"
#include "babl-class.h"
#include "babl-list.h"
#include "babl-hash-table.h"
#include "babl-db.h"
#include "babl-ids.h"
#include "babl-util.h"
#include "babl-memory.h"
#include "babl-mutex.h"
#include "babl-cpuaccel.h"

/* fallback to floor function when rint is not around */
#ifndef HAVE_RINT
# define rint(f)  (floor (((double) (f)) + 0.5))
#endif



Babl *   babl_conversion_find           (const void     *source,
                                         const void     *destination);
double   babl_conversion_error          (BablConversion *conversion);
long     babl_conversion_cost           (BablConversion *conversion);
long     babl_conversion_process        (const Babl     *conversion,
                                         const char     *source,
                                         char           *destination,
                                         long            n);

Babl   * babl_extension_base            (void);

Babl   * babl_extender                  (void);
void     babl_set_extender              (Babl           *new_extender);

Babl   * babl_extension_quiet_log       (void);
void     babl_extension_deinit          (void);

long     babl_fish_reference_process    (const Babl *babl,
                                         const char *source,
                                         char       *destination,
                                         long        n);

Babl   * babl_fish_reference            (const Babl     *source,
                                         const Babl     *destination);
Babl   * babl_fish_simple               (BablConversion *conversion);
Babl   * babl_fish_path                 (const Babl     *source,
                                         const Babl     *destination);

int      babl_fish_get_id               (const Babl     *source,
                                         const Babl     *destination);

double   babl_format_loss               (const Babl     *babl);
Babl   * babl_image_from_linear         (char           *buffer,
                                         const Babl     *format);
Babl   * babl_image_double_from_image   (const Babl     *source);

double   babl_model_is_symmetric        (const Babl     *babl);
void     babl_die                       (void);
int      babl_sanity                    (void);

void     babl_core_init                 (void);
const Babl *babl_format_with_model_as_type (const Babl     *model,
                                         const Babl     *type);
int      babl_formats_count             (void);                                     /* should maybe be templated? */
int      babl_type_is_symmetric         (const Babl     *babl);

/**** LOGGER ****/
#include <stdarg.h>

int babl_backtrack (void);

static inline void
real_babl_log_va(const char *file,
                 int         line,
                 const char *function,
                 const char *fmt,
                 va_list     varg)
{
  Babl *extender = babl_extender();

  if (extender != babl_extension_quiet_log())
    {
      if (babl_extender())
        fprintf (stdout, "When loading %s:\n\t", babl_extender()->instance.name);

      fprintf (stdout, "%s:%i %s()\n\t", file, line, function);
    }

  vfprintf (stdout, fmt, varg);

  fprintf (stdout, "\n");
  fflush (NULL);
  return;
}

static inline void
real_babl_log (const char *file,
               int         line,
               const char *function,
               const char *fmt, ...)
{
  va_list  varg;

  va_start (varg, fmt);
  real_babl_log_va (file, line, function, fmt, varg);
  va_end (varg);
}

/* Provide a string identifying the current function, non-concatenatable */
#ifndef G_STRFUNC
#if defined (__GNUC__)
#  define G_STRFUNC     ((const char*) (__PRETTY_FUNCTION__))
#elif defined (G_HAVE_ISO_VARARGS)
#  define G_STRFUNC     ((const char*) (__func__))
#else
#  define G_STRFUNC     ((const char*) ("???"))
#endif
#endif

#if defined(__cplusplus) && defined(BABL_ISO_CXX_VARIADIC_MACROS)
#  define BABL_ISO_VARIADIC_MACROS 1
#endif

#if defined(BABL_ISO_VARIADIC_MACROS)

#define babl_log(...)                                       \
  real_babl_log(__FILE__, __LINE__, G_STRFUNC, __VA_ARGS__)

#define babl_fatal(...) do{                                  \
  real_babl_log(__FILE__, __LINE__, G_STRFUNC, __VA_ARGS__); \
  babl_die();}                                               \
while(0)

#elif defined(BABL_GNUC_VARIADIC_MACROS)

#define babl_log(args...)                               \
  real_babl_log(__FILE__, __LINE__, G_STRFUNC, args)

#define babl_fatal(args...) do{                         \
  real_babl_log(__FILE__, __LINE__, G_STRFUNC, args);    \
  babl_die();}                                          \
while(0)

#else

static inline void
babl_log (const char *format, ...)
{
  va_list args;
  va_start (args, format);
  real_babl_log_va (__FILE__, __LINE__, G_STRFUNC, format, args);
  va_end (args);
}
static inline void
babl_fatal (const char *format, ...)
{
  va_list args;
  va_start (args, format);
  real_babl_log_va (__FILE__, __LINE__, G_STRFUNC, format, args);
  va_end (args);
  babl_die();
}

#endif


#define babl_assert(expr) do{                              \
  if(!(expr))                                              \
    {                                                      \
      real_babl_log(__FILE__, __LINE__, G_STRFUNC, "Eeeeek! Assertion failed: `" #expr "`"); \
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
extern int   babl_in_fish_path;
extern BablMutex *babl_format_mutex;

#define BABL_DEBUG_MEM 0
#if BABL_DEBUG_MEM
extern BablMutex *babl_debug_mutex;
#endif

const char  *babl_class_name       (BablClassType klass);
void         babl_internal_init    (void);
void         babl_internal_destroy (void);


/* this template is expanded in the files including babl-internal.h,
 * generating code, the declarations for these functions are found in
 * the BABL_CLASS expansions done in babl.h as well, thus babl.h needs
 * to be kept in sync with the C files.
 */

#define BABL_CLASS_MINIMAL_IMPLEMENT(klass)                   \
                                                              \
BablDb *                                                      \
babl_##klass##_db (void)                                      \
{                                                             \
  if (!db)                                                    \
    db=babl_db_init ();                                       \
  return db;                                                  \
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
const Babl *                                                  \
babl_##klass (const char *name)                               \
{                                                             \
  Babl *babl;                                                 \
                                                              \
  if (babl_hmpf_on_name_lookups)                              \
    {                                                         \
      babl_log ("%s(\"%s\"): looking up", G_STRFUNC, name);   \
    }                                                         \
  if (!db)                                                    \
    {                                                         \
      babl_fatal ("%s(\"%s\"): you must call babl_init first", G_STRFUNC, name);  \
    }                                                         \
  babl = babl_db_exist_by_name (db, name);                    \
                                                              \
  if (!babl)                                                  \
    {                                                         \
      babl_fatal ("%s(\"%s\"): not found", G_STRFUNC, name);  \
    }                                                         \
  return babl;                                                \
}                                                             \
                                                              \
const Babl *                                                  \
babl_##klass##_from_id (int id)                               \
{                                                             \
  Babl *babl;                                                 \
  babl = babl_db_exist_by_id (db, id);                        \
  if (!babl)                                                  \
    {                                                         \
      babl_fatal ("%s(%i): not found", G_STRFUNC, id);        \
    }                                                         \
  return babl;                                                \
} \

#define BABL(obj)  ((Babl*)(obj))

static inline double babl_parse_double (const char *str)
{
  double result = 0;
  if (!str)
    return 0.0;
  result = atoi (str);
  if (strchr (str, '.'))
  {
    char *p = strchr (str, '.') + 1;
    double d = 10;
    for (;*p && *p >= '0' && *p <= '9';p++, d *= 10)
    {
      if (result >= 0)
        result += (*p - '0') / d;
      else
        result -= (*p - '0') / d;
    }
  }
  return result;
}


double _babl_legal_error (void);
void babl_init_db (void);
void babl_store_db (void);
int _babl_max_path_len (void);

#endif
