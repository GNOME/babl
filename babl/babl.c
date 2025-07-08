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
#include "babl-base.h"

static int ref_count = 0;

#ifndef _WIN32
#define BABL_PATH              LIBDIR BABL_DIR_SEPARATOR BABL_LIBRARY
#endif

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


static const char **simd_init (void);
void
babl_init (void)
{
  const char **exclusion_pattern;
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

      if (!getenv ("BABL_INHIBIT_CACHE"))
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

