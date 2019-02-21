/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2016 Øyvind Kolås.
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

#ifdef _WIN32
#include <shlobj.h>
#endif

#include <time.h>
#include <sys/stat.h>
#include "config.h"
#include "babl-internal.h"
#include "git-version.h"

#ifdef _WIN32
#define FALLBACK_CACHE_PATH  "C:/babl-fishes.txt"
#else
#define FALLBACK_CACHE_PATH  "/tmp/babl-fishes.txt"
#endif

static int
mk_ancestry_iter (const char *path)
{
  char copy[4096];
  strncpy (copy, path, 4096);
  copy[sizeof (copy) - 1] = '\0';
  if (strrchr (copy, '/'))
    {
      *strrchr (copy, '/') = '\0';
      if (copy[0])
        {
          struct stat stat_buf;
          if ( ! (stat (copy, &stat_buf)==0 && S_ISDIR(stat_buf.st_mode)))
            {
              if (mk_ancestry_iter (copy) != 0)
                return -1;
#ifndef _WIN32
              return mkdir (copy, S_IRWXU);
#else
              return mkdir (copy);
#endif
            }
        }
    }
  return 0;
}

static int
mk_ancestry (const char *path)
{
  char copy[4096];
  strncpy (copy, path, 4096);
  copy[sizeof (copy) - 1] = '\0';
#ifdef _WIN32
  for (char *c = copy; *c; c++)
    if (*c == '\\')
      *c = '/';
#endif
  return mk_ancestry_iter (copy);
}

static const char *
fish_cache_path (void)
{
  struct stat stat_buf;
  static char path[4096];

  strncpy (path, FALLBACK_CACHE_PATH, 4096);
  path[sizeof (path) - 1] = '\0';
#ifndef _WIN32
  if (getenv ("XDG_CACHE_HOME"))
    snprintf (path, sizeof (path), "%s/babl/babl-fishes", getenv("XDG_CACHE_HOME"));
  else if (getenv ("HOME"))
    snprintf (path, sizeof (path), "%s/.cache/babl/babl-fishes", getenv("HOME"));
#else
{
  char win32path[4096];
  if (SHGetFolderPathA (NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, win32path) == S_OK)
    snprintf (path, sizeof (path), "%s\\%s\\babl-fishes.txt", win32path, BABL_LIBRARY);
  else if (getenv ("TEMP"))
    snprintf (path, sizeof (path), "%s\\babl-fishes.txt", getenv("TEMP"));
}
#endif

  if (stat (path, &stat_buf)==0 && S_ISREG(stat_buf.st_mode))
    return path;

  if (mk_ancestry (path) != 0)
    return FALLBACK_CACHE_PATH;

  return path;
}

static char *
babl_fish_serialize (Babl *fish, char *dest, int n)
{
  char *d = dest;
  if (fish->class_type != BABL_FISH &&
      fish->class_type != BABL_FISH_PATH)
  {
    return NULL;
  }

  snprintf (d, n, "%s\n%s\n",
  babl_get_name (fish->fish.source),
  babl_get_name (fish->fish.destination));
  n -= strlen (d);d += strlen (d);

  snprintf (d, n, "\tpixels=%li", fish->fish.pixels);
  n -= strlen (d);d += strlen (d);

  if (fish->class_type == BABL_FISH_PATH)
  {
    snprintf (d, n, " cost=%d", (int)fish->fish_path.cost);
    n -= strlen (d);d += strlen (d);
  }

  snprintf (d, n, " error=%f", fish->fish.error);
  n -= strlen (d);d += strlen (d);

  if (fish->class_type == BABL_FISH)
  {
    snprintf (d, n, " [reference]");
    n -= strlen (d);d += strlen (d);
  }

  snprintf (d, n, "\n");
  n -= strlen (d);d += strlen (d);

  if (fish->class_type == BABL_FISH_PATH)
  {
    for (int i = 0; i < fish->fish_path.conversion_list->count; i++)
    {
      snprintf (d, n, "\t%s\n",
        babl_get_name(fish->fish_path.conversion_list->items[i]  ));
      n -= strlen (d);d += strlen (d);
    }
  }

  return dest;
}

static int 
compare_fish_pixels (const void *a, const void *b)
{
  const Babl **fa = (void*)a;
  const Babl **fb = (void*)b;
  return ((*fb)->fish.pixels - (*fa)->fish.pixels);
}

static const char *
cache_header (void)
{
  static char buf[2048];
  if (strchr (BABL_GIT_VERSION, ' ')) // we must be building from tarball
    snprintf (buf, sizeof (buf),
             "#%i.%i.%i BABL_PATH_LENGTH=%d BABL_TOLERANCE=%f",
             BABL_MAJOR_VERSION, BABL_MINOR_VERSION, BABL_MICRO_VERSION,
             _babl_max_path_len (), _babl_legal_error ());
  else
    snprintf (buf, sizeof (buf), "#%s BABL_PATH_LENGTH=%d BABL_TOLERANCE=%f",
             BABL_GIT_VERSION, _babl_max_path_len (), _babl_legal_error ());
  return buf;
}

void 
babl_store_db (void)
{
  BablDb *db = babl_fish_db ();
  int i;
  char *tmpp = calloc(8000,1);
  FILE *dbfile;

  if (!tmpp)
    return;
  snprintf (tmpp, 8000, "%s~", fish_cache_path ());
  dbfile  = fopen (tmpp, "w");
  if (!dbfile)
  {
    free (tmpp);
    return;
  }
  fprintf (dbfile, "%s\n", cache_header ());

  /* sort the list of fishes by usage, making next run more efficient -
   * and the data easier to approach as data for targeted optimization
   */
  qsort (db->babl_list->items, db->babl_list->count,
         sizeof (Babl*), compare_fish_pixels);

  for (i = 0; i< db->babl_list->count; i++)
  {
    Babl *fish = db->babl_list->items[i];
    char tmp[8192];
    if (babl_fish_serialize (fish, tmp, 4096))
      fprintf (dbfile, "%s----\n", tmp);
  }
  fclose (dbfile);

#ifdef _WIN32
  remove (fish_cache_path ());
#endif
  rename (tmpp, fish_cache_path());
  free (tmpp);
}

int
_babl_fish_path_destroy (void *data);

char *
_babl_fish_create_name (char       *buf,
                        const Babl *source,
                        const Babl *destination,
                        int         is_reference);

void 
babl_init_db (void)
{
  const char *path = fish_cache_path ();
  long  length = -1;
  char  seps[] = "\n\r";
  Babl *babl   = NULL;
  char *contents = NULL;
  char *token;
  char *tokp;
  const Babl  *from_format = NULL;
  const Babl  *to_format   = NULL;
  time_t tim = time (NULL);

  if (getenv ("BABL_DEBUG_CONVERSIONS"))
    return;

  _babl_file_get_contents (path, &contents, &length, NULL);
  if (!contents)
    return;

  token = strtok_r (contents, seps, &tokp);
  while( token != NULL )
    {
      switch (token[0])
      {
        case '-': /* finalize */
          if (babl)
          {
            if ((babl->fish.pixels) == (tim % 100))
            {
              /* 1% chance of individual cached conversions being dropped -
               * making sure mis-measured conversions do not
                 stick around for a long time*/
              babl_free (babl);
            }
            else
              babl_db_insert (babl_fish_db(), babl);
          }
          from_format = NULL;
          to_format = NULL;
          babl=NULL;
          break;
        case '#':
          /* if babl has changed in git .. drop whole cache */
          {
            if (strcmp ( token, cache_header ()))
            {
              free (contents);
              return;
            }
          }
          break;
        case '\t':
          if (to_format && strchr (token, '='))
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
              free (contents);
              return;
            }

            if (strstr (token, "[reference]"))
            {
              /* there isn't a suitable path for requested formats,
               * let's create a dummy BABL_FISH instance and insert
               * it into the fish database to indicate that such path
               * does not exist.
               */
              const char *name = "X"; /* name does not matter */
              babl = babl_calloc (1, sizeof (BablFish) + strlen (name) + 1);

              babl->class_type       = BABL_FISH;
              babl->instance.id      = babl_fish_get_id (from_format,
                                                         to_format);
              babl->instance.name    = ((char *) babl) + sizeof (BablFish);
              strcpy (babl->instance.name, name);
              babl->fish.source      = from_format;
              babl->fish.destination = to_format;
              babl->fish.data        = (void*) 1; /* signals babl_fish() to
                                                   * show a "missing fash path"
                                                   * warning upon the first
                                                   * lookup
                                                   */
            }
            else
            {
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
              _babl_fish_prepare_bpp (babl);
              _babl_fish_rig_dispatch (babl);
            }

            token2 = strtok_r (&token[1], seps2, &tokp2);
            while( token2 != NULL )
            {
              if (!strncmp (token2, "error=", 6))
              {
                babl->fish.error = babl_parse_double (token2 + 6);
              }
              else if (!strncmp (token2, "cost=", 5))
              {
                if (babl->class_type == BABL_FISH_PATH)
                  babl->fish_path.cost = babl_parse_double (token2 + 5);
              }
              else if (!strncmp (token2, "pixels=", 7))
              {
                babl->fish.pixels = strtol (token2 + 7, NULL, 10);
              }
              token2 = strtok_r (NULL, seps2, &tokp2);
            }
          }
          else if (to_format && babl && babl->class_type == BABL_FISH_PATH)
          {
            Babl *conv = (void*)babl_db_find(babl_conversion_db(), &token[1]);
            if (!conv)
            {
              babl_free (babl);
              babl = NULL;
            }
            else
              babl_list_insert_last (babl->fish_path.conversion_list, conv);
          }
          break;
        default:
          if (!from_format)
          {
            from_format = (void*)babl_db_find(babl_format_db(), token);
          }
          else
          {
            to_format = (void*)babl_db_find(babl_format_db(), token);
          }
          break;
      }
      token = strtok_r (NULL, seps, &tokp);
    }
  if (contents)
    free (contents);
}
