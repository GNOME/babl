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
#ifndef S_IRWXU
  #define S_IRWXU 0000700
#endif
#if !defined(S_ISREG) && defined(S_IFMT) && defined(S_IFREG)
  #define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#endif
#if !defined(S_ISDIR) && defined(S_IFMT) && defined(S_IFDIR)
  #define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif
#define strtok_r strtok_s
#endif

#ifdef _WIN32
#define FALLBACK_CACHE_PATH  "C:/babl-fishes.txt"
#else
#define FALLBACK_CACHE_PATH  "/tmp/babl-fishes.txt"
#endif

static int
mk_ancestry_iter (const char *path)
{
  char *copy = babl_strdup (path);
  char *rchr = NULL;
  int result = 0;

  if (!copy)
    return -1;

  rchr = strrchr (copy, '/');
  if (rchr)
    {
      *rchr = '\0';

      if (copy[0])
        {
          BablStat stat_buf;
          if ( ! (_babl_stat (copy, &stat_buf)==0 && S_ISDIR(stat_buf.st_mode)))
            result = mk_ancestry_iter (copy) == 0 ? _babl_mkdir (copy, S_IRWXU) : -1;
        }
    }

  babl_free (copy);
  return result;
}

static int
mk_ancestry (const char *path)
{
  char *copy = babl_strdup (path);
  int result = 0;

  if (!copy)
    return -1;

#ifdef _WIN32
  for (char *c = copy; *c; c++)
    if (*c == '\\')
      *c = '/';
#endif

  result = mk_ancestry_iter (copy);

  babl_free (copy);
  return result;
}

static char *
fish_cache_path (void)
{
  char *path = NULL;
  char buf[4096];
  BablStat stat_buf;

#ifndef _WIN32

  strncpy (buf, FALLBACK_CACHE_PATH, 4096);
  buf[sizeof (buf) - 1] = '\0';

  if (getenv ("XDG_CACHE_HOME"))
    snprintf (buf, sizeof (buf), "%s/babl/babl-fishes", getenv("XDG_CACHE_HOME"));
  else if (getenv ("HOME"))
    snprintf (buf, sizeof (buf), "%s/.cache/babl/babl-fishes", getenv("HOME"));

  path = babl_strdup (buf);

#else

  wchar_t *appdata_utf16 = NULL;

  if (SHGetKnownFolderPath (&FOLDERID_LocalAppData, KF_FLAG_DEFAULT, NULL, &appdata_utf16) == S_OK)
    {
      char *appdata = babl_convert_utf16_to_utf8 (appdata_utf16);

      if (appdata && appdata[0])
        {
          const char *fmt = "%s\\%s\\babl-fishes.txt";
          size_t sz = add_check_overflow (3, strlen (fmt), strlen (appdata), strlen (BABL_LIBRARY));

          if (sz > 0 && (path = babl_malloc (sz)) != NULL)
            _snprintf_s (path, sz, sz, fmt, appdata, BABL_LIBRARY);
        }

      if (appdata)
        babl_free (appdata);
    }
  else if (getenv ("TEMP"))
    {
      snprintf (buf, sizeof (buf), "%s\\babl-fishes.txt", getenv("TEMP"));
      path = babl_strdup (buf);
    }

  if (appdata_utf16)
    {
      CoTaskMemFree (appdata_utf16);
      appdata_utf16 = NULL;
    }

#endif

  if (!path)
    return babl_strdup (FALLBACK_CACHE_PATH);

  if (_babl_stat (path, &stat_buf) == 0 && S_ISREG(stat_buf.st_mode))
    return path;

  if (mk_ancestry (path) != 0)
    return babl_strdup (FALLBACK_CACHE_PATH);

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

  snprintf (d, n, " error=%.10f", fish->fish.error);
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
  char *cache_path = fish_cache_path ();
  char *tmpp = calloc(8000,1);
  FILE *dbfile = NULL;
  int i;

  if (!cache_path || !tmpp)
    goto cleanup;

  snprintf (tmpp, 8000, "%s~", cache_path);
  dbfile  = _babl_fopen (tmpp, "w");
  if (!dbfile)
    goto cleanup;

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
  dbfile = NULL;

#ifdef _WIN32
  _babl_remove (cache_path);
#endif
  _babl_rename (tmpp, cache_path);

cleanup:
  if (dbfile)
    fclose (dbfile);

  if (cache_path)
    babl_free (cache_path);

  if (tmpp)
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
  char *path = fish_cache_path ();
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
    goto cleanup;

  _babl_file_get_contents (path, &contents, &length, NULL);
  if (!contents)
    goto cleanup;

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
              goto cleanup;
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
              goto cleanup;
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
                {
                  babl->fish_path.cost = babl_parse_double (token2 + 5);
                  _babl_fish_prepare_bpp (babl);
                }
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

cleanup:
  if (contents)
    free (contents);

  if (path)
    babl_free (path);
}
