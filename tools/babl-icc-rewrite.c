/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2005, 2017 Øyvind Kolås.
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
#include <math.h>
#include "../babl/babl-internal.h"

static int
file_get_contents (const char  *path,
                   char       **contents,
                   long        *length,
                   void        *error);
void file_set_contents (const char *path, const char *data, long length);

int
main (int    argc,
      char **argv)
{
  const Babl *babl;
  char *icc = NULL;
  long  len;
  int genlen;
  char *error = NULL;
  babl_init ();

  if (!argv[1] || !argv[2])
  {
    fprintf (stderr, "usage: %s <input.icc> <output.icc>\n", argv[0]);
    return -1;
  }

  if (file_get_contents (argv[1], &icc, &len, NULL))
    return -1;

  babl = babl_space_rgb_icc (icc, len, &error);
  free (icc);

  icc = (char *)babl_space_rgb_to_icc (babl, &genlen);
  if (icc)
  {
    file_set_contents (argv[2], icc, genlen);
  }

  babl_exit ();
  return 0;
}

static int
file_get_contents (const char  *path,
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
  *length = size = ftell (file);
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

void file_set_contents (const char *path, const char *data, long length)
{
  FILE *fp = fopen (path, "wb");
  if (length == -1)
    length = strlen (data);
  fwrite(data, length, 1, fp);
  fclose (fp);
}
