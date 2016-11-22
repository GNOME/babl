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
 * <http://www.gnu.org/licenses/>.
 */

#include <time.h>
#include <sys/stat.h>
#include "config.h"
#include "babl-internal.h"
#include "git-version.h"

static int
mk_ancestry_iter (const char *path)
{
  char copy[4096];
  strncpy (copy, path, 4096);
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
  return -1;
}

static int
mk_ancestry (const char *path)
{
  char copy[4096];
  strncpy (copy, path, 4096);
#ifdef _WIN32
  for (char *c = copy; *c; c++)
    if (*c == '\\')
      *c = '/';
#endif
  return mk_ancestry_iter (path);
}

static const char *fish_cache_path (void)
{
  struct stat stat_buf;
  static char resolved[4096];

#ifndef _WIN32 
  if (getenv ("HOME"))
    sprintf (resolved, "%s/.cache/babl/babl-fishes", getenv("HOME"));
  else
    strncpy (resolved, "/tmp/babl.db", 4096);
#else
  if (getenv ("TEMP"))
    sprintf (resolved, "%s\\babl-fishes.txt", getenv("TEMP"));
  else
    strncpy (resolved, "c:\\babl-fishes.txt", 4096);
#endif

  if (stat (resolved, &stat_buf)==0 && S_ISREG(stat_buf.st_mode))
    return resolved;

  mk_ancestry (resolved);

  return resolved;
}

static char *
babl_fish_serialize (Babl *fish, char *dest, int n)
{
  char *d = dest;
  if (fish->class_type != BABL_FISH_PATH)
    return NULL;
                              
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

  snprintf (d, n, "\n");
  n -= strlen (d);d += strlen (d);

  for (int i = 0; i < fish->fish_path.conversion_list->count; i++)
  {
    snprintf (d, n, "\t%s\n", 
      babl_get_name(fish->fish_path.conversion_list->items[i]  ));
    n -= strlen (d);d += strlen (d);
  }

  return dest;
}

static int compare_fish_pixels (const void *a, const void *b)
{
  const Babl **fa = (void*)a;
  const Babl **fb = (void*)b;
  return ((*fb)->fish.pixels - (*fa)->fish.pixels) +
         ((*fb)->fish.processings - (*fa)->fish.processings);
}

void babl_store_db (void)
{
  BablDb *db = babl_fish_db ();
  int i;
  FILE *dbfile = fopen (fish_cache_path (), "w");
  if (!dbfile)
    return;
  fprintf (dbfile, "#%s BABL_TOLERANCE=%f\n",
           BABL_GIT_VERSION, _babl_legal_error ());

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

void babl_init_db (void)
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
#ifdef _WIN32  // XXX: fixme - make this work on windows
  return;
#endif

  if (getenv ("BABL_DEBUG_CONVERSIONS"))
    return;

  babl_file_get_contents (path, &contents, &length, NULL);
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
            if ( ((babl->fish.pixels+1 + babl->fish.processings) % 
                  ((tim % 100)+1)) == 0)
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
            char buf[2048];
            sprintf (buf, "#%s BABL_TOLERANCE=%f",
                     BABL_GIT_VERSION, _babl_legal_error ());
            if (strcmp ( token, buf))
            {
              free (contents);
              return;
            }
          }
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
                babl->fish.error = babl_parse_double (token2 + 6);
              }
              else if (!strncmp (token2, "cost=", 5))
              {
                babl->fish_path.cost = babl_parse_double (token2 + 5);
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
            Babl *conv = (void*)babl_db_find(babl_conversion_db(), &token[1]);
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
            from_format = (void*)babl_db_find(babl_format_db(), token);
            if (!from_format)
              return;
          }
          else
          {
            to_format = (void*)babl_db_find(babl_format_db(), token);
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
