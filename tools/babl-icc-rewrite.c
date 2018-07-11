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
 * <https://www.gnu.org/licenses/>.
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
  BablICCFlags flags = 0;
  const Babl *babl;
  char *icc_data = NULL;
  long  icc_len;
  int genlen;
  char *description = NULL;
  char *copyright = NULL;
  const char *error;
  const char *la = NULL;
  const char *co = NULL;

  const char *input = NULL;
  const char *output = NULL;
  int i;

  babl_init ();


  for (i = 1; argv[i]; i++)
  {
    if (!strcmp (argv[i], "-d") ||
        !strcmp (argv[i], "--description"))
    {
      description = argv[++i];
    }
    else if (!strcmp (argv[i], "-c") ||
             !strcmp (argv[i], "--copyright"))
    {
      copyright = argv[++i];
    }
    else if (!strcmp (argv[i], "--compact-trc"))
    {
      flags |= BABL_ICC_COMPACT_TRC_LUT;
    }
    else if (argv[i][0] == '-')
    {
      fprintf (stderr, "unknown option %s\n", argv[i]);
      return -1;
    }
    else
    {
      if (!input)       input  = argv[i];
      else if (!output) output = argv[i];
    }
  }

  if (!input || !output)
  {
    fprintf (stderr, "usage: %s [options] <input.icc> <output.icc>\n", argv[0]);
    fprintf (stderr, " where recognized options are:  \n");
    fprintf (stderr, "    -d <description>\n");
    fprintf (stderr, "    -c <copyright>\n");
    fprintf (stderr, "    --compact-trc\n");
    return -1;
  }

  if (file_get_contents (input, &icc_data, &icc_len, NULL))
    return -1;

  if (!description)
  {
    description = babl_icc_get_key (icc_data, icc_len, "description", la, co);
    if (description)
      fprintf (stderr, "description: %s\n", description);
  }

  if (!copyright)
  {
    copyright = babl_icc_get_key (icc_data, icc_len, "copyright", la, co);
    if (copyright)
    {
      fprintf (stderr, "copyright: %s\n", copyright);
    }
  }
  {
    char *str = babl_icc_get_key (icc_data, icc_len, "device", la, co);
    if (str)
    {
      fprintf (stderr, "device: %s\n", str);
      free (str);
    }
  }
  {
    char *str = babl_icc_get_key (icc_data, icc_len, "manufacturer", la, co);
    if (str)
    {
      fprintf (stderr, "manufacturer: %s\n", str);
      free (str);
    }
  }
  babl = babl_icc_make_space (icc_data, icc_len, 0, &error);
  free (icc_data);
  if (error || !babl)
  {
    fprintf (stderr, "%s error %s", argv[0], error);
    return -1;
  }

  icc_data = (char *)babl_space_to_icc (babl, description,
                                        copyright, flags,
                                        &genlen);
  if (icc_data)
  {
    file_set_contents (output, icc_data, genlen);
  }
  fprintf (stderr, "[%s]\n", output);

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
