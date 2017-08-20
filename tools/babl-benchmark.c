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
#include "babl-internal.h"

int ITERATIONS = 1;
#define  N_PIXELS (512*512)


#define  N_BYTES  N_PIXELS * (4 * 8)


static int
test (void)
{
  int i, j;
  int OK = 1;

  char *src_data = babl_malloc (N_BYTES);
  char *dst_data = babl_malloc (N_BYTES);
  double sum = 0;

  const Babl *formats[]={
#if 0
     babl_format("R'G'B'A u8"),
     babl_format("R'G'B'A u16"),
     babl_format_with_space("R'G'B'A u8",     babl_space("ProPhoto")),
     babl_format_with_space("RGBA float",     babl_space("ProPhoto")),
     babl_format_with_space("R'G'B' u16",     babl_space("ProPhoto")),
     babl_format("CIE Lab float"),
#endif
     babl_format("RGBA float"),
     babl_format("R'G'B'A float"),
     babl_format("R'G'B' u8"),
     babl_format_with_space("R'G'B' u8",     babl_space("Adobe")),
     babl_format_with_space("R'G'B' u8",     babl_space("ProPhoto")),
     babl_format_with_space("RGBA float",     babl_space("ProPhoto")),
     babl_format_with_space("R'G'B'A float",  babl_space("ProPhoto"))
     };
  int n_formats = sizeof (formats) / sizeof (formats[0]);

 for (i = 0; i < N_BYTES; i++)
   src_data[i] = random();

 fprintf (stdout,"%i iterations of %i pixels, mb/s is for sum of source and destinations bytes\n", ITERATIONS, N_PIXELS);

 for (i = 0; i < n_formats; i++)
   for (j = 0; j < n_formats; j++)
   {
      const Babl *fish = babl_fish (formats[i], formats[j]);
      long end, start;
      double megabytes_per_sec;
      int iters = ITERATIONS;

#if 1
      fprintf (stderr, "%s to %s\r", babl_get_name (formats[i]),
                                     babl_get_name (formats[j]));
#endif

      /* a quarter round of warmup */
      babl_process (fish, src_data, dst_data, N_PIXELS * 0.25);
      start = babl_ticks ();
      while (iters--)
      {
        babl_process (fish, src_data, dst_data, N_PIXELS);
      }
      end = babl_ticks ();
      megabytes_per_sec = (babl_format_get_bytes_per_pixel (formats[i]) +
                           babl_format_get_bytes_per_pixel (formats[j])) *
              (N_PIXELS * ITERATIONS / 1024.0 / 1024.0) / ((end-start)/(1000.0*1000.0));

      sum += megabytes_per_sec;
      fprintf (stdout, " %03.1f mb/s\t%s to %s\n",
                       megabytes_per_sec,
                      babl_get_name (formats[i]),
                      babl_get_name (formats[j]));
      fflush (0);
   }

 fprintf (stdout,"%3.1f mb/s\taverage\n", sum / (n_formats * n_formats));

  if (!OK)
    return -1;
  return 0;
}

int
main (int    argc,
      char **argv)
{
  if (argv[1]) ITERATIONS = atoi (argv[1]);
  babl_init ();
  if (test ())
    return -1;
  babl_exit ();
  return 0;
}
