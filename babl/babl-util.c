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

#ifdef _WIN32

wchar_t *
babl_convert_utf8_to_utf16 (const char *str)
{
  int wchar_count = 0;
  wchar_t *wstr = NULL;

  if (!str)
    return NULL;

  wchar_count = MultiByteToWideChar (CP_UTF8,
                                     MB_ERR_INVALID_CHARS,
                                     str, -1,
                                     NULL, 0);
  if (wchar_count <= 0)
    return NULL;

  wstr = babl_malloc (wchar_count * sizeof (wchar_t));
  if (!wstr)
    return NULL;

  wchar_count = MultiByteToWideChar (CP_UTF8,
                                     MB_ERR_INVALID_CHARS,
                                     str, -1,
                                     wstr, wchar_count);
  if (wchar_count <= 0)
    {
      babl_free (wstr);
      return NULL;
    }

  return wstr;
}

char *
babl_convert_utf16_to_utf8 (const wchar_t *wstr)
{
  int char_count = 0;
  char *str = NULL;

  if (!wstr)
    return NULL;

  char_count = WideCharToMultiByte (CP_UTF8,
                                    WC_ERR_INVALID_CHARS,
                                    wstr, -1,
                                    NULL, 0,
                                    NULL, NULL);
  if (char_count <= 0)
    return NULL;

  str = babl_malloc (char_count);
  if (!str)
    return NULL;

  char_count = WideCharToMultiByte (CP_UTF8,
                                    WC_ERR_INVALID_CHARS,
                                    wstr, -1,
                                    str, char_count,
                                    NULL, NULL);
  if (char_count <= 0)
    {
      babl_free (str);
      return NULL;
    }

  return str;
}

extern IMAGE_DOS_HEADER __ImageBase;

void *
get_libbabl_module (void)
{
  return &__ImageBase;
}

#endif /* _WIN32 */
