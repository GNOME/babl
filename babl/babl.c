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
#include "babl-internal.h"

static int ref_count = 0;

#ifdef _WIN32
static HMODULE libbabl_dll = NULL;

/* Minimal DllMain that just stores the handle to this DLL */

BOOL WINAPI			/* Avoid silly "no previous prototype" gcc warning */
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

static char *
babl_fish_serialize (Babl *fish, char *dest, int n)
{
  switch (fish->class_type)
  {
    case BABL_FISH_PATH:
      {
        char *d = dest;
                              
        snprintf (d, n, "%s\n%s\n",
        babl_get_name (fish->fish.source),
        babl_get_name (fish->fish.destination));
        n -= strlen (d);d += strlen (d);

        snprintf (d, n, "\terror=%f", fish->fish.error);
        n -= strlen (d);d += strlen (d);

        snprintf (d, n, " processings=%i", fish->fish.processings);
        n -= strlen (d);d += strlen (d);

        snprintf (d, n, " pixels=%li", fish->fish.pixels);
        n -= strlen (d);d += strlen (d);

        snprintf (d, n, " cost=%f", fish->fish_path.cost);
        n -= strlen (d);d += strlen (d);

        snprintf (d, n, " loss=%f", fish->fish_path.loss);
        n -= strlen (d);d += strlen (d);

        snprintf (d, n, "\n");
        n -= strlen (d);d += strlen (d);

        for (int i = 0; i < fish->fish_path.conversion_list->count; i++)
        {
          snprintf (d, n, "\t%s\n", 
            babl_get_name(fish->fish_path.conversion_list->items[i]  ));
          n -= strlen (d);
          d += strlen (d);
        }
      }
      break;
    case BABL_FISH_SIMPLE:   
      return NULL;
      break;
    case BABL_FISH_REFERENCE:
      return NULL; break;
    default:
      break;
  }

  return dest;
}

static const char *fish_cache_path (void)
{
  return "/tmp/babl.db"; // XXX: a $HOME/.cache/babl/fishes path might be better
}

static void babl_store_db (void)
{
  BablDb *db = babl_fish_db ();
  int i;
  FILE *dbfile = fopen (fish_cache_path (), "w");
  if (!dbfile)
    return;
  fprintf (dbfile, "#babl 0 %i fishes\n", db->babl_list->count);
  for (i = 0; i< db->babl_list->count; i++)
  {
    Babl *fish = db->babl_list->items[i];
    char tmp[8192];
    if (babl_fish_serialize (fish, tmp, 4096))
      fprintf (dbfile, "%s----\n", tmp);
  }
  fclose (dbfile);
}

static int
babl_file_get_contents (const char  *path,
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

  fseek (file, 0, SEEK_END);
  size = ftell (file);
  if (length) *length = size;
  rewind (file);
  buffer = malloc(size + 8);

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

int
_babl_fish_path_destroy (void *data);

char *
_babl_fish_create_name (char       *buf,
                        const Babl *source,
                        const Babl *destination,
                        int         is_reference);

static void babl_init_db (const char *path)
{
  long  length = -1;
  char  seps[] = "\n";
  Babl *babl   = NULL;
  char *contents = NULL;
  char *token;
  char *tokp;
  const Babl  *from_format = NULL;
  const Babl  *to_format   = NULL;
#ifdef _WIN32  // XXX: fixme - make this work on windows
  return;
#endif

  babl_file_get_contents (path, &contents, &length, NULL);
  if (!contents)
    return;

  token = strtok_r (contents, seps, &tokp);
  while( token != NULL )
    {
      switch (token[0])
      {
        case '-': /* finalize */
          //fprintf (stderr, "%p %p\n", from_format, to_format);
          if (babl)
            babl_db_insert (babl_fish_db(), babl);
          from_format = NULL;
          to_format = NULL;
          babl=NULL;
          break;
        case '#':
          break;
        case '\t':
          if (strchr (token, '='))
          {
            char seps2[] = " ";
            char *tokp2;
            char *token2;
            char name[4096];

            _babl_fish_create_name (name, from_format, to_format, 1);
            babl = babl_db_exist_by_name (babl_fish_db (), name);
            if (babl)
            {
              fprintf (stderr, "%s:%i: loading of cache failed\n",
                              __FUNCTION__, __LINE__);
              return;
            }

            babl = babl_calloc (1, sizeof (BablFishPath) +
                      strlen (name) + 1);
            babl_set_destructor (babl, _babl_fish_path_destroy);

            babl->class_type     = BABL_FISH_PATH;
            babl->instance.id    = babl_fish_get_id (from_format, to_format);
            babl->instance.name  = ((char *) babl) + sizeof (BablFishPath);
            strcpy (babl->instance.name, name);
            babl->fish.source               = from_format;
            babl->fish.destination          = to_format;
            babl->fish_path.conversion_list = babl_list_init_with_size (10);

            token2 = strtok_r (&token[1], seps2, &tokp2);
            while( token2 != NULL )
            {
              if (!strncmp (token2, "error=", 6))
              {
                babl->fish.error = strtod (token2 + 6, NULL);
              }
              else if (!strncmp (token2, "cost=", 5))
              {
                babl->fish_path.cost = strtod (token2 + 5, NULL);
              }
              else if (!strncmp (token2, "loss=", 5))
              {
                babl->fish_path.loss = strtod (token2 + 5, NULL);
              }
              else if (!strncmp (token2, "pixels=", 7))
              {
                babl->fish.pixels = strtol (token2 + 7, NULL, 10);
              }
              else if (!strncmp (token2, "processings=", 12))
              {
                babl->fish.processings = strtol (token2 + 12, NULL, 10);
              }
              token2 = strtok_r (NULL, seps2, &tokp2);
            }
          }
          else
          {
            Babl *conv =
                    (void*)babl_db_find(babl_conversion_db(), &token[1]);
            if (!conv)
            {
              return;
            }
            else
              babl_list_insert_last (babl->fish_path.conversion_list, conv);
          }
          break;
        default:
          if (!from_format)
          {
            from_format = (void*)babl_db_find(babl_format_db(), &token[1]);
            if (!from_format)
              return;
          }
          else
          {
            to_format = (void*)babl_db_find(babl_format_db(), &token[1]);
            if (!to_format)
              return;
          }

          break;
      }
      token = strtok_r (NULL, seps, &tokp);
    }
  if (contents)
    free (contents);
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

      babl_init_db (fish_cache_path());
    }
}

void
babl_exit (void)
{
  if (!-- ref_count)
    {
#ifdef _WIN32  // XXX: fixme - make this work on windows
#else
      babl_store_db ();
#endif

      if (getenv ("BABL_STATS"))
        {
          char  logfile_name[] = "/tmp/babl-stats.html";
          FILE *logfile;
          logfile = fopen (logfile_name, "w");
          if (logfile)
            {
              babl_fish_stats (logfile);
              fclose (logfile);
            }
        }

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
