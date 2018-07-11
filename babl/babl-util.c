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

  if (error >= 0.0000001)
    error /= samples;
  else if (error <= 0.0)
    error = 0.0;
  else
    error = M_PI;

  return error;
}

int
_babl_file_get_contents (const char  *path,
                         char       **contents,
                         long        *length,
                         void        *error)
{
  FILE *file;
  long  size;
  char *buffer;

  file = fopen (path,"rb");

  if (!file)
    return -1;

  if (fseek (file, 0, SEEK_END) == -1 || (size = ftell (file)) == -1)
    {
      fclose (file);
      return -1;
    }
  if (length) *length = size;
  rewind (file);
  if ((size_t) size > SIZE_MAX - 8)
    {
      fclose (file);
      return -1;
    }
  buffer = calloc(size + 8, 1);

  if (!buffer)
    {
      fclose(file);
      return -1;
    }

  size -= fread (buffer, 1, size, file);
  if (size)
    {
      fclose (file);
      free (buffer);
      return -1;
    }
  fclose (file);
  *contents = buffer;
  return 0;
}

