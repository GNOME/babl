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
 * <https://www.gnu.org/licenses/>.
 */

#ifndef _BABL_UTIL_H
#define _BABL_UTIL_H

#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef _WIN32
typedef struct stat BablStat;
#else
typedef struct _stat64 BablStat;
#endif

long
babl_ticks     (void);

double
babl_rel_avg_error (const double *imgA,
                    const double *imgB,
                    long          samples);

size_t
add_check_overflow (size_t numbers_count, ...);

size_t
mul_check_overflow (size_t numbers_count, ...);

FILE *
_babl_fopen (const char *path,
             const char *mode);

int
_babl_remove (const char *path);

int
_babl_rename (const char *oldname,
              const char *newname);

int
_babl_stat (const char *path,
            BablStat   *buffer);

int
_babl_mkdir (const char *path,
             int         mode);

typedef void
(*_babl_dir_foreach_cb_t) (const char *base_path,
                           const char *entry,
                           void       *data);

void
_babl_dir_foreach (const char             *path,
                   _babl_dir_foreach_cb_t  callback,
                   void                   *user_data);

#ifdef _WIN32

wchar_t *
babl_convert_utf8_to_utf16 (const char *str);

char *
babl_convert_utf16_to_utf8 (const wchar_t *wstr);

void *
get_libbabl_module (void);

#endif /* _WIN32 */

#endif
