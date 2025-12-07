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

#include <errno.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#include "config.h"
#include "babl-internal.h"
#include "babl-base.h"

static int ref_count = 0;

#ifndef _WIN32
#define BABL_PATH              LIBDIR BABL_DIR_SEPARATOR BABL_LIBRARY
#endif


static char * _babl_find_relocatable_exe (void);
static char * _babl_guess_libdir         (void);


/*
 * Returns a list of directories if the environment variable $BABL_PATH
 * is set, or the installation library directory by default.
 * This directory will be based on the compilation-time prefix for UNIX
 * and an actual DLL path for Windows.
 *
 * Returns: a string which must be freed after usage.
 */
static char *
babl_dir_list (void)
{
  char *ret = NULL;

#ifndef _WIN64
  ret = getenv ("BABL_PATH");
#else
  _dupenv_s (&ret, NULL, "BABL_PATH");
#endif

  if (!ret)
    {
#if defined(_WIN32)
      /* Figure it out from the location of this DLL */
      wchar_t w_filename[MAX_PATH];
      char *filename = NULL;
      char *sep1, *sep2;
      DWORD nSize = sizeof (w_filename) / sizeof ((w_filename)[0]);

      if (GetModuleFileNameW (get_libbabl_module (), w_filename, nSize) == 0)
        babl_fatal ("GetModuleFilenameW failed");

      filename = babl_convert_utf16_to_utf8 (w_filename);
      if (!filename)
        babl_fatal ("Converting module filename to UTF-8 failed");

      /* If the DLL file name is of the format
       * <foobar>\bin\*.dll, use <foobar>\lib\{BABL_LIBRARY}.
       * Otherwise, use the directory where the DLL is.
       */

      sep1 = strrchr (filename, BABL_DIR_SEPARATOR[0]);
      *sep1 = '\0';

      sep2 = strrchr (filename, BABL_DIR_SEPARATOR[0]);
      if (sep2 != NULL)
        {
          if (_stricmp (sep2 + 1, "bin") == 0)
            {
              char* filename_tmp;
              *(++sep2) = '\0';
              filename_tmp = babl_malloc (sizeof (char) * (strlen (filename) +
                                strlen (BABL_DIR_SEPARATOR BABL_LIBRARY) + 4));
              strcpy_s (filename_tmp, strlen(filename) + 1, filename);
              babl_free (filename);
              strcat (filename_tmp, "lib" BABL_DIR_SEPARATOR BABL_LIBRARY);
              filename = filename_tmp;
            }
        }

      ret = filename;
#else
      char *exe;

      exe = _babl_find_relocatable_exe ();
      if (exe)
        {
          char *sep1, *sep2;

          sep1 = strrchr (exe, BABL_DIR_SEPARATOR[0]);
          *sep1 = '\0';

          sep2 = strrchr (exe, BABL_DIR_SEPARATOR[0]);
          if (sep2 != NULL)
            {
              if (strcmp (sep2 + 1, "bin") == 0)
                {
                  char *tmp;
                  char *libdir = _babl_guess_libdir ();

                  *(++sep2) = '\0';
                  tmp = babl_malloc (sizeof (char) * (strlen (exe)                +
                                                      strlen (libdir)             +
                                                      strlen (BABL_DIR_SEPARATOR) +
                                                      strlen (BABL_LIBRARY) + 1));
                  strcpy (tmp, exe);
                  babl_free (exe);
                  strcat (tmp, libdir);
                  babl_free (libdir);
                  strcat (tmp, BABL_DIR_SEPARATOR BABL_LIBRARY);
                  exe = tmp;
                }
              else
                {
                  *sep1 = BABL_DIR_SEPARATOR[0];
                  /* This may happen when babl is loaded by uninstalled
                   * binaries, such as build-time tools, in which case, this is
                   * not an error, and we fallback to the build-time BABL_PATH.
                   * We assume that relocatable builds are not relocated during
                   * the build of a full bundle.
                   * This is why we output a message on stderr, but this is not
                   * a fatal error.
                   */
                  fprintf (stderr,
                           "Relocatable builds require the executable to be installed in bin/ unlike: %s\n"
                           "If this is a build-time tool, you may ignore this message.\n",
                           exe);
                  babl_free (exe);
                  exe = NULL;
                }
            }
          else
            {
              babl_free (exe);
              exe = NULL;
            }

          ret = exe;
        }

      if (! ret)
        {
          ret = babl_malloc (sizeof (char) * (strlen (BABL_PATH) + 1));
          strcpy (ret, BABL_PATH);
        }
#endif
    }
  else
    {
      char* ret_tmp = babl_malloc (sizeof (char) * (strlen (ret) + 1));

#ifndef _WIN64
      strcpy (ret_tmp, ret);
#else
      strcpy_s (ret_tmp, strlen (ret) + 1, ret);
#endif
      ret = ret_tmp;
    }

  return ret;
}


static const char **simd_init (void);
void
babl_init (void)
{
  const char **exclusion_pattern;
  char* env = NULL;

  babl_cpu_accel_set_use (1);
  exclusion_pattern = simd_init ();

  if (ref_count++ == 0)
    {
      char * dir_list;

      babl_internal_init ();
      babl_sampling_class_init ();
      babl_type_db ();
      babl_trc_class_init ();
      babl_space_class_init ();
      _babl_legal_error ();
      babl_component_db ();
      babl_model_db ();
      babl_format_db ();
      babl_conversion_db ();
      babl_extension_db ();
      babl_fish_db ();
      babl_core_init ();
      babl_sanity ();
      babl_extension_base ();
      babl_sanity ();

      dir_list = babl_dir_list ();
      babl_extension_load_dir_list (dir_list, exclusion_pattern);
      babl_free (dir_list);

#ifndef _WIN64
      env = getenv ("BABL_INHIBIT_CACHE");
#else
      _dupenv_s (&env, NULL, "BABL_INHIBIT_CACHE");
#endif
      if (!env)
        babl_init_db ();
#ifdef _WIN64
      free (env);
#endif
    }
}

void
babl_exit (void)
{
  if (!-- ref_count)
    {
      babl_store_db ();

      babl_extension_deinit ();
      babl_free (babl_extension_db ());;
      babl_free (babl_fish_db ());;
      babl_free (babl_conversion_db ());;
      babl_free (babl_format_db ());;
      babl_free (babl_model_db ());;
      babl_free (babl_component_db ());;
      babl_free (babl_type_db ());;

      babl_internal_destroy ();
#if BABL_DEBUG_MEM
      babl_memory_sanity ();
#endif
    }
}

#undef babl_model_is

int 
babl_model_is (const Babl *babl,
               const char *model)
{
  return babl && ((babl)==babl_model_with_space(model, babl));
}


#include "babl-cpuaccel.h"
void (*babl_base_init)  (void) = babl_base_init_generic;

const Babl * babl_trc_lookup_by_name_generic (const char *name);


const Babl *
babl_trc_new_generic (const char *name,
                      BablTRCType type,
                      double      gamma,
                      int         n_lut,
                      float      *lut);

void _babl_space_add_universal_rgb_generic (const Babl *space);
void (*_babl_space_add_universal_rgb) (const Babl *space) =
  _babl_space_add_universal_rgb_generic;

const Babl *
(*babl_trc_lookup_by_name) (const char *name) = babl_trc_lookup_by_name_generic;
const Babl *
(*babl_trc_new) (const char *name,
              BablTRCType type,
              double      gamma,
              int         n_lut,
              float      *lut) = babl_trc_new_generic;

#ifdef ARCH_X86_64
void babl_base_init_x86_64_v2 (void);
void babl_base_init_x86_64_v3 (void);
void _babl_space_add_universal_rgb_x86_64_v2 (const Babl *space);
void _babl_space_add_universal_rgb_x86_64_v3 (const Babl *space);

const Babl *
babl_trc_lookup_by_name_x86_64_v2 (const char *name);
const Babl *
babl_trc_lookup_by_name_x86_64_v3 (const char *name);

const Babl *
babl_trc_new_x86_64_v2 (const char *name,
                        BablTRCType type,
                        double      gamma,
                        int         n_lut,
                        float      *lut);
const Babl *
babl_trc_new_x86_64_v3 (const char *name,
                        BablTRCType type,
                        double      gamma,
                        int         n_lut,
                        float      *lut);

#endif
#ifdef ARCH_ARM
void babl_base_init_arm_neon (void);
void _babl_space_add_universal_rgb_arm_neon (const Babl *space);

const Babl *
babl_trc_lookup_by_name_arm_neon (const char *name);

const Babl *
babl_trc_new_arm_neon (const char *name,
                       BablTRCType type,
                       double      gamma,
                       int         n_lut,
                       float      *lut);

#endif

static const char **simd_init (void)
{
  static const char *exclude[] = {"neon-", "x86-64-v3", "x86-64-v2", NULL};
#ifdef ARCH_X86_64
  BablCpuAccelFlags accel = babl_cpu_accel_get_support ();
  if ((accel & BABL_CPU_ACCEL_X86_64_V3) == BABL_CPU_ACCEL_X86_64_V3)
  {
    static const char *exclude[] = {NULL};
    babl_base_init = babl_base_init_x86_64_v2; /// !!
                                               // this is correct,
                                               // it performs better
                                               // as observed in benchmarking
    babl_trc_new = babl_trc_new_x86_64_v2;
    babl_trc_lookup_by_name = babl_trc_lookup_by_name_x86_64_v2;
    _babl_space_add_universal_rgb = _babl_space_add_universal_rgb_x86_64_v3;
    return exclude;
  }
  else if ((accel & BABL_CPU_ACCEL_X86_64_V2) == BABL_CPU_ACCEL_X86_64_V2)
  {
    static const char *exclude[] = {"x86-64-v3-", NULL};
    babl_base_init = babl_base_init_x86_64_v2;
    babl_trc_new = babl_trc_new_x86_64_v2;
    babl_trc_lookup_by_name = babl_trc_lookup_by_name_x86_64_v2;
    _babl_space_add_universal_rgb = _babl_space_add_universal_rgb_x86_64_v2;
    return exclude;
  }
  else
  {
    static const char *exclude[] = {"x86-64-v3-", "x86-64-v2-", NULL};
    return exclude;
  }
#endif
#ifdef ARCH_ARM
  BablCpuAccelFlags accel = babl_cpu_accel_get_support ();
  if ((accel & BABL_CPU_ACCEL_ARM_NEON) == BABL_CPU_ACCEL_ARM_NEON)
  {
    static const char *exclude[] = {NULL};
    babl_base_init = babl_base_init_arm_neon;
    babl_trc_new = babl_trc_new_arm_neon;
    babl_trc_lookup_by_name = babl_trc_lookup_by_name_arm_neon;
    _babl_space_add_universal_rgb = _babl_space_add_universal_rgb_arm_neon;
    return exclude;
  }
  else
  {
    static const char *exclude[] = {"arm-neon-", NULL};
    return exclude;
  }
#endif
  return exclude;
}


/* Private functions */

static char *
_babl_find_relocatable_exe (void)
{
#if ! defined(ENABLE_RELOCATABLE) || defined(_WIN32) || defined(__APPLE__)
  return NULL;
#else
  char   *path;
  char   *sym_path;
  FILE   *file;
  char   *maps_line      = NULL;
  size_t  maps_line_size = 0;

  sym_path = babl_strdup ("/proc/self/exe");

  while (1)
    {
      size_t      bufsiz;
      ssize_t     nbytes;
      struct stat stat_buf;

#ifdef PATH_MAX
#define BABL_PATH_MAX PATH_MAX
#else
#define BABL_PATH_MAX 4096
#endif

      bufsiz = BABL_PATH_MAX;
      path   = babl_malloc (bufsiz);
      nbytes = readlink (sym_path, path, bufsiz);
      /* Some systems actually allow paths of bigger size than PATH_MAX. Thus
       * this macro is kind of bogus so we need to verify if we didn't get a
       * truncated value.
       * Other systems like Hurd will not even define it (see MR gimp!424).
       */
      while (nbytes == bufsiz && nbytes != -1)
        {
          /* The path was truncated. */
          babl_free (path);
          bufsiz += BABL_PATH_MAX;
          path    = babl_malloc (bufsiz);

          nbytes = readlink (sym_path, path, bufsiz);
        }
      babl_free (sym_path);

#undef BABL_PATH_MAX

      if (nbytes == -1)
        {
          babl_free (path);
          path = NULL;
          break;
        }

      /* Check whether the symlink's target is also a symlink.
       * We want to get the final target.
       */
      if (stat (path, &stat_buf) == -1)
        {
          babl_free (path);
          path = NULL;
          break;
        }

      if (! S_ISLNK (stat_buf.st_mode))
        return path;

      /* path is a symlink. Continue loop and resolve this. */
      sym_path = path;
    }

  /* readlink() or stat() failed; this can happen when the program is
   * running in Valgrind 2.2.
   * Read from /proc/self/maps as fallback.
   */

  file = _babl_fopen ("/proc/self/maps", "rb");

  if (! file)
    babl_fatal ("Failed to read /proc/self/maps: %s", strerror (errno));

  /* The first entry with r-xp permission should be the executable name. */
  while ((getline (&maps_line, &maps_line_size, file) != -1))
    {
      /* Extract the filename; it is always an absolute path. */
      path = strchr (maps_line, '/');

      /* Sanity check. */
      if (path && strstr (maps_line, " r-xp "))
        {
          /* We found the executable name. */
          path = babl_strdup (path);
          break;
        }

      path = NULL;
    }
  free (maps_line);

  fclose (file);

  if (path == NULL)
    babl_fatal ("Failed to find the executable's path for relocatability.");

  return path;
#endif
}

/* Returns the relative libdir, which may be lib/ or lib64/ or again
 * some subdirectory with multiarch.
 */
static char *
_babl_guess_libdir (void)
{
  char   *libdir;
  char   *rel_libdir;
  char   *sep;
  size_t  len;

  libdir = babl_strdup (LIBDIR);
  len = strlen (libdir);

  sep = strrchr (libdir, BABL_DIR_SEPARATOR[0]);
  while (sep != NULL && strstr (sep + 1, "lib") != sep + 1)
    {
      *sep = '\0';
      sep = strrchr (libdir, BABL_DIR_SEPARATOR[0]);
    }

  if (sep == NULL)
    babl_fatal ("Relocatable builds require LIBDIR to start with 'lib' unlike: %s", LIBDIR);

  while (strlen (libdir) < len)
    libdir[strlen (libdir)] = BABL_DIR_SEPARATOR[0];

  rel_libdir = babl_strdup (sep + 1);
  babl_free (libdir);

  return rel_libdir;
}
