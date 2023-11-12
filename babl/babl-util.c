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
#include <stdarg.h>
#include <limits.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "babl-internal.h"

#ifdef __WIN32__
#include <windows.h>
#include <wchar.h>
#else
#include <sys/time.h>
#include <time.h>
#include <dirent.h>
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

size_t
add_check_overflow (size_t numbers_count, ...)
{
  size_t result = 0;
  va_list args;

  assert (numbers_count > 0);

  va_start (args, numbers_count);
  while (numbers_count--)
    {
      size_t addendum = va_arg (args, size_t);

      if ((SIZE_MAX - result) < addendum)
        {
          result = 0;
          break;
        }

      result += addendum;
    }
  va_end (args);

  return result;
}

size_t
mul_check_overflow (size_t numbers_count, ...)
{
  size_t result = 1;
  va_list args;

  assert (numbers_count > 0);

  va_start (args, numbers_count);
  while (numbers_count--)
    {
      size_t factor = va_arg (args, size_t);

      if ((SIZE_MAX / result) < factor)
        {
          result = 0;
          break;
        }

      result *= factor;
    }
  va_end (args);

  return result;
}

FILE *
_babl_fopen (const char *path,
             const char *mode)
{
#ifndef _WIN32
  return fopen (path, mode);
#else
  wchar_t *path_utf16 = babl_convert_utf8_to_utf16 (path);
  wchar_t *mode_utf16 = babl_convert_utf8_to_utf16 (mode);
  FILE *result = NULL;

  result = _wfopen (path_utf16, mode_utf16);

  if (path_utf16)
    babl_free (path_utf16);

  if (mode_utf16)
    babl_free (mode_utf16);

  return result;
#endif
}

int
_babl_remove (const char *path)
{
#ifndef _WIN32
  return remove (path);
#else
  wchar_t *path_utf16 = babl_convert_utf8_to_utf16 (path);
  int result = 0;

  result = _wremove (path_utf16);

  if (path_utf16)
    babl_free (path_utf16);

  return result;
#endif
}

int
_babl_rename (const char *oldname,
              const char *newname)
{
#ifndef _WIN32
  return rename (oldname, newname);
#else
  wchar_t *oldname_utf16 = babl_convert_utf8_to_utf16 (oldname);
  wchar_t *newname_utf16 = babl_convert_utf8_to_utf16 (newname);
  int result = 0;

  result = _wrename (oldname_utf16, newname_utf16);

  if (oldname_utf16)
    babl_free (oldname_utf16);

  if (newname_utf16)
    babl_free (newname_utf16);

  return result;
#endif
}

int
_babl_stat (const char *path,
            BablStat   *buffer)
{
#ifndef _WIN32
  return stat (path, buffer);
#else
  wchar_t *path_utf16 = babl_convert_utf8_to_utf16 (path);
  int result = 0;

  result = _wstat64 (path_utf16, buffer);

  if (path_utf16)
    babl_free (path_utf16);

  return result;
#endif
}

int
_babl_mkdir (const char *path,
             int         mode)
{
#ifndef _WIN32
  return mkdir (path, (mode_t) mode);
#else
  wchar_t *path_utf16 = babl_convert_utf8_to_utf16 (path);
  int result = 0;
  (void) mode;

  result = _wmkdir (path_utf16);

  if (path_utf16)
    babl_free (path_utf16);

  return result;
#endif
}

void
_babl_dir_foreach (const char             *path,
                   _babl_dir_foreach_cb_t  callback,
                   void                   *user_data)
{
#ifndef _WIN32
  DIR *dir = opendir (path);

  if (!path)
    return;

  if (dir != NULL)
    {
      struct dirent *dentry;

      while ((dentry = readdir (dir)))
        callback (path, dentry->d_name, user_data);

      closedir (dir);
    }
#else
  char *search = NULL;
  wchar_t *search_utf16 = NULL;
  struct _wfinddata64_t info;
  intptr_t search_id = 0;

  if (!path)
    return;

  search = babl_strcat (search, path);
  search = babl_strcat (search, "\\*");
  search_utf16 = babl_convert_utf8_to_utf16 (search);
  if (!search_utf16)
    goto cleanup;

  memset (&info, 0, sizeof (info));
  if ((search_id = _wfindfirst64 (search_utf16, &info)) != (intptr_t)-1)
    {
      do
        {
          char *entry = babl_convert_utf16_to_utf8 (info.name);

          if (entry)
            {
              callback (path, entry, user_data);
              babl_free (entry);
            }
        }
      while (_wfindnext64 (search_id, &info) == 0);

      _findclose (search_id);
    }

cleanup:
  if (search_utf16)
    babl_free (search_utf16);

  if (search)
    babl_free (search);
#endif
}

void
_babl_float_to_half (void        *halfp,
                     const float *floatp,
                     int          numel)
{
  uint16_t       *hp = (uint16_t *) halfp;
  const uint32_t *xp = (const uint32_t *) floatp;
  uint16_t        hs, he, hm;
  uint32_t        x, xs, xe, xm;
  int             hes;

  if (hp == NULL || xp == NULL)
    {
      /* Nothing to convert (e.g., imag part of pure real) */
      return;
    }

  while (numel--)
    {
      x = *xp++;
      if ((x & 0x7FFFFFFFu) == 0)
        {
          /* Return the signed zero */
          *hp++ = (uint16_t) (x >> 16);
        }
      else
        {
          /* Not zero */
          xs = x & 0x80000000u;  /* Pick off sign bit      */
          xe = x & 0x7F800000u;  /* Pick off exponent bits */
          xm = x & 0x007FFFFFu;  /* Pick off mantissa bits */
          if (xe == 0)
            {
              /* Denormal will underflow, return a signed zero */
              *hp++ = (uint16_t) (xs >> 16);
            }
          else if (xe == 0x7F800000u)
            {
              /* Inf or NaN (all the exponent bits are set) */
              if (xm == 0)
                {
                  /* If mantissa is zero ... */
                  *hp++ = (uint16_t) ((xs >> 16) | 0x7C00u); /* Signed Inf */
                }
              else
                {
                  *hp++ = (uint16_t) 0xFE00u; /* NaN, only 1st mantissa bit set */
                }
            }
          else
            {
              /* Normalized number */
              hs = (uint16_t) (xs >> 16); /* Sign bit */
              hes = ((int)(xe >> 23)) - 127 + 15; /* Exponent unbias the single, then bias the halfp */
              if (hes >= 0x1F)
                {
                  /* Overflow */
                  *hp++ = (uint16_t) ((xs >> 16) | 0x7C00u); /* Signed Inf */
                }
              else if (hes <= 0)
                {
                  /* Underflow */
                  if ((14 - hes) > 24)
                    {
                      /* Mantissa shifted all the way off & no rounding possibility */
                      hm = (uint16_t) 0u;  /* Set mantissa to zero */
                    }
                  else
                    {
                      xm |= 0x00800000u;  /* Add the hidden leading bit */
                      hm = (uint16_t) (xm >> (14 - hes)); /* Mantissa */
                      if ((xm >> (13 - hes)) & 0x00000001u) /* Check for rounding */
                        hm += (uint16_t) 1u; /* Round, might overflow into exp bit, but this is OK */
                    }
                  *hp++ = (hs | hm); /* Combine sign bit and mantissa bits, biased exponent is zero */
                }
              else
                {
                  he = (uint16_t) (hes << 10); /* Exponent */
                  hm = (uint16_t) (xm >> 13);  /* Mantissa */
                  if (xm & 0x00001000u) /* Check for rounding */
                    *hp++ = (hs | he | hm) + (uint16_t) 1u; /* Round, might overflow to inf, this is OK */
                  else
                    *hp++ = (hs | he | hm);  /* No rounding */
                }
            }
        }
    }
}

/*-----------------------------------------------------------------------------\
 *
 * Routine:  _babl_half_to_float (formerly halfp2singles)
 *
 * Input:  source = address of 16-bit data to convert
 *         numel  = Number of values at that address to convert
 *
 * Output: target = Address of 32-bit floating point data to hold output (numel values)
 *
 *
 * Programmer:  James Tursa
 *
\*-----------------------------------------------------------------------------*/
void
_babl_half_to_float (float      *floatp,
                     const void *halfp,
                     int         numel)
{
  uint32_t       *xp = (uint32_t *) floatp;
  const uint16_t *hp = (const uint16_t *) halfp;
  uint16_t        h, hs, he, hm;
  uint32_t        xs, xe, xm;
  int32_t         xes;
  int             e;

  if (xp == NULL || hp == NULL)
    /* Nothing to convert (e.g., imag part of pure real) */
    return;

  while (numel--)
    {
      h = *hp++;
      if ((h & 0x7FFFu) == 0)
        {
          /* Return the signed zero */
          *xp++ = ((uint32_t) h) << 16;
        }
      else
        {
          /* Not zero */
          hs = h & 0x8000u; /* Pick off sign bit      */
          he = h & 0x7C00u; /* Pick off exponent bits */
          hm = h & 0x03FFu; /* Pick off mantissa bits */
          if (he == 0)
            {
              /* Denormal will convert to normalized */
              e = -1; /* The following loop figures out how much extra to adjust the exponent */
              do
                {
                  e++;
                  hm <<= 1;
                }
              while ((hm & 0x0400u) == 0); /* Shift until leading bit overflows into exponent bit */
              xs = ((uint32_t) hs) << 16; /* Sign bit */
              xes = ((int32_t) (he >> 10)) - 15 + 127 - e; /* Exponent unbias the halfp, then bias the single */
              xe = (uint32_t) (xes << 23); /* Exponent */
              xm = ((uint32_t) (hm & 0x03FFu)) << 13; /* Mantissa */
              *xp++ = (xs | xe | xm); /* Combine sign bit, exponent bits, and mantissa bits */
            }
          else if (he == 0x7C00u)
            {
              /* Inf or NaN (all the exponent bits are set) */
              if (hm == 0)
                {
                  /* If mantissa is zero ... */
                  *xp++ = (((uint32_t) hs) << 16) | ((uint32_t) 0x7F800000u); /* Signed Inf */
                }
              else
                {
                  *xp++ = (uint32_t) 0xFFC00000u; /* NaN, only 1st mantissa bit set */
                }
            }
          else
            {
              /* Normalized number */
              xs = ((uint32_t) hs) << 16; /* Sign bit */
              xes = ((int32_t) (he >> 10)) - 15 + 127; /* Exponent unbias the halfp, then bias the single */
              xe = (uint32_t) (xes << 23); /* Exponent */
              xm = ((uint32_t) hm) << 13; /* Mantissa */
              *xp++ = (xs | xe | xm); /* Combine sign bit, exponent bits, and mantissa bits */
            }
        }
    }
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

  file = _babl_fopen (path, "rb");

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
