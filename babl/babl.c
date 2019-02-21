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
#include "babl-internal.h"

static int ref_count = 0;

#ifdef _WIN32
static HMODULE libbabl_dll = NULL;

/* Minimal DllMain that just stores the handle to this DLL */

/* Avoid silly "no previous prototype" gcc warning */
BOOL WINAPI
DllMain (HINSTANCE hinstDLL,
         DWORD     fdwReason,
         LPVOID    lpvReserved);

BOOL WINAPI
DllMain (HINSTANCE hinstDLL,
         DWORD     fdwReason,
         LPVOID    lpvReserved)
{
  switch (fdwReason)
    {
      case DLL_PROCESS_ATTACH:
        libbabl_dll = hinstDLL;
        break;
    }

  return TRUE;
}

#else
#define BABL_PATH              LIBDIR BABL_DIR_SEPARATOR BABL_LIBRARY
#endif /* _WIN32 */

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
  char *ret;

  ret = getenv ("BABL_PATH");
  if (!ret)
    {
#ifdef _WIN32
      /* Figure it out from the location of this DLL */
      char *filename;
      int filename_size;
      char *sep1, *sep2;

      wchar_t w_filename[MAX_PATH];
      DWORD nSize = sizeof (w_filename) / sizeof ((w_filename)[0]);

      if (GetModuleFileNameW (libbabl_dll, w_filename, nSize) == 0)
        babl_fatal ("GetModuleFilenameW failed");

      filename_size = WideCharToMultiByte (CP_UTF8, 0, w_filename, -1, NULL, 0,
                                           NULL, NULL);
      filename = babl_malloc (sizeof (char) * filename_size);
      if (!WideCharToMultiByte (CP_UTF8, 0, w_filename, -1,
                                filename, filename_size, NULL, NULL))
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
          if (strcasecmp (sep2 + 1, "bin") == 0)
            {
              char* filename_tmp;
              *(++sep2) = '\0';
              filename_tmp = babl_malloc (sizeof (char) * (strlen (filename) +
                                strlen (BABL_DIR_SEPARATOR BABL_LIBRARY) + 4));
              strcpy (filename_tmp, filename);
              babl_free (filename);
              strcat (filename_tmp, "lib" BABL_DIR_SEPARATOR BABL_LIBRARY);
              filename = filename_tmp;
            }
        }

      ret = filename;
#else
      ret = babl_malloc (sizeof (char) * (strlen (BABL_PATH) + 1));
      strcpy (ret, BABL_PATH);
#endif
    }
  else
    {
      char* ret_tmp = babl_malloc (sizeof (char) * (strlen (ret) + 1));
      strcpy (ret_tmp, ret);
      ret = ret_tmp;
    }
  return ret;
}

void
babl_init (void)
{
  babl_cpu_accel_set_use (1);

  if (ref_count++ == 0)
    {
      char * dir_list;

      babl_internal_init ();
      babl_sampling_class_init ();
      babl_type_db ();
      babl_trc_class_init ();
      babl_space_class_init ();
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
      babl_extension_load_dir_list (dir_list);
      babl_free (dir_list);

      babl_init_db ();
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

