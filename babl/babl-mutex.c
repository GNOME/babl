/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2009, Øyvind Kolås.
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

#include "config.h"
#include "babl-mutex.h"

#include <stdlib.h>

#ifndef _WIN32

static const pthread_mutexattr_t *
get_mutex_attr (void)
{
  static pthread_mutexattr_t mutexattr;
  static int initialized = 0;

  if (!initialized)
    {
      /* On some platforms, this will keep an allocation till process
         termination, but it isn't a growing leak. */
      pthread_mutexattr_init (&mutexattr);
      pthread_mutexattr_settype (&mutexattr, PTHREAD_MUTEX_RECURSIVE);
      initialized = 1;
    }

  return &mutexattr;
}

#endif

BablMutex *
babl_mutex_new (void)
{
  BablMutex *mutex = malloc (sizeof (BablMutex));
#ifdef _WIN32
  InitializeCriticalSection (mutex);
#else
  pthread_mutex_init (mutex, get_mutex_attr ());
#endif
  return mutex;
}

void
babl_mutex_destroy (BablMutex *mutex)
{
#ifdef _WIN32
  DeleteCriticalSection (mutex);
#else
  pthread_mutex_destroy(mutex);
#endif
  free (mutex);
}

void
babl_mutex_lock (BablMutex *mutex)
{
#ifdef _WIN32
  EnterCriticalSection (mutex);
#else
  pthread_mutex_lock (mutex);
#endif
}

void
babl_mutex_unlock (BablMutex *mutex)
{
#ifdef _WIN32
  LeaveCriticalSection (mutex);
#else
  pthread_mutex_unlock (mutex);
#endif
}
