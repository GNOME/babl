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

#include "config.h"
#include <math.h>
#include "babl-internal.h"

#ifdef __WIN32__
#include <windows.h>
#else
#include <sys/time.h>
#include <time.h>
#endif

#ifdef __WIN32__
static LARGE_INTEGER start_time;
static LARGE_INTEGER timer_freq;

static void
init_ticks (void)
{
  static int done = 0;

  if (done)
    return;
  done = 1;

  QueryPerformanceCounter(&start_time);
  QueryPerformanceFrequency(&timer_freq);
}

long
babl_ticks (void)
{
  LARGE_INTEGER end_time;

  init_ticks ();

  QueryPerformanceCounter(&end_time);
  return (end_time.QuadPart - start_time.QuadPart) * (1000000.0 / timer_freq.QuadPart);
}
#else
static struct timeval start_time;

#define usecs(time)    ((time.tv_sec - start_time.tv_sec) * 1000000 + time.tv_usec)

static void
init_ticks (void)
{
  static int done = 0;

  if (done)
    return;
  done = 1;
  gettimeofday (&start_time, NULL);
}

long
babl_ticks (void)
{
  struct timeval measure_time;
  init_ticks ();
  gettimeofday (&measure_time, NULL);
  return usecs (measure_time) - usecs (start_time);
}
#endif

double
babl_rel_avg_error (const double *imgA,
                    const double *imgB,
                    long          samples)
{
  double error = 0.0;
  long   i;

  for (i = 0; i < samples; i++)
    error += fabs (imgA[i] - imgB[i]);

  if (error >= 0.000001)
    error /= samples;
  else
    error = 0.0;

  return error;
}

