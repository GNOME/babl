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
#include "babl-internal.h"

#ifndef HAVE_SRANDOM
#define srandom srand
#define random  rand
#endif

int ITERATIONS = 2;
#define  N_PIXELS (512*1024)  // a too small batch makes the test set live
                               // in l2 cache skewing results

                               // we could also add a cache purger..


#define  N_BYTES  N_PIXELS * (4 * 8)

static const char *unicode_hbar (int width, double fraction)
{
  static char ret[200]="";
  const char *block[9]= {" ", "▏", "▎", "▍", "▌", "▋", "▊", "▉","█"};
  int i;
  if (width > 100) width = 100;

  ret[0]=0;
  for (i = 0; i < width; i++)
  {
    double start = i * 1.0 / width;
    if (start < fraction)
      strcat (ret, block[8]);
    else
    {
      double miss = (start - fraction) * width;
      if (miss < 1.0)
        strcat (ret, block[(int)((1.0-miss) * 8.999)]);
      else
        strcat (ret, block[0]);
    }
  }
  return ret;
}

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
     babl_format("Y float"),
     babl_format("R'G'B'A u16"),
     babl_format_with_space("RGBA float",     babl_space("ProPhoto")),
     babl_format_with_space("R'G'B' u16",     babl_space("ProPhoto")),
#endif
     //babl_format("R'G'B'A u8"),
     //babl_format("R'G'B'A u16"),
       babl_format_with_space("R'G'B'A u8", babl_space("ProPhoto")),
       babl_format_with_space("R'G'B'A half", babl_space("ProPhoto")),
       babl_format_with_space("R'G'B'A float", babl_space("ProPhoto")),
       babl_format_with_space("R'G'B'A double", babl_space("ProPhoto")),
       babl_format_with_space("cairo-RGB24", babl_space("Adobe")),
       babl_format_with_space("cairo-ARGB32", babl_space("Adobe")),

     };
  int n_formats = sizeof (formats) / sizeof (formats[0]);
  const Babl *fishes[50 * 50];
  double mbps[50 * 50] = {0,};
  int n;
  double max = 0.0;

  assert (n_formats < 50);

 for (i = 0; i < N_BYTES; i++)
   src_data[i] = random();


 fprintf (stdout,"%i iterations of %i pixels, mb/s is for sum of source and destinations bytes\n", ITERATIONS, N_PIXELS);

 n = 0;
 for (i = 0; i < n_formats; i++)
   for (j = 0; j < n_formats; j++)
   if (i != j)
   {
      const Babl *fish = babl_fish (formats[i], formats[j]);
      long end, start;
      int iters = ITERATIONS;

      fprintf (stderr, "%s to %s          \r", babl_get_name (formats[i]),
                                               babl_get_name (formats[j]));
      fflush (0);

      /* a quarter round of warmup */
      babl_process (fish, src_data, dst_data, N_PIXELS * 0.25);
      start = babl_ticks ();
      while (iters--)
      {
        babl_process (fish, src_data, dst_data, N_PIXELS);
      }
      end = babl_ticks ();
      fishes[n] = fish;
      mbps [n] = (babl_format_get_bytes_per_pixel (formats[i]) +
                           babl_format_get_bytes_per_pixel (formats[j])) *
              (N_PIXELS * ITERATIONS / 1024.0 / 1024.0) / ((end-start)/(1000.0*1000.0));

      sum += mbps[n];
      if (mbps[n] > max)
        max = mbps[n];
      n++;
   }

 n = 0;
 for (i = 0; i < n_formats; i++)
   for (j = 0; j < n_formats; j++)
   if (i != j)
   {
      fprintf (stdout, "%s %03.1f mb/s\t%s to %s %.9f",
                      unicode_hbar(16, mbps[n] / max),
                      mbps[n],
                      babl_get_name (formats[i]),
                      babl_get_name (formats[j]),
                      fishes[n]->fish.error);
      if (fishes[n]->class_type == BABL_FISH_REFERENCE)
      {
        fprintf (stdout, "[R]");
      }
      else if (fishes[n]->class_type == BABL_FISH_PATH)
      {
        int k;
        //fprintf (stdout, "[%d]", fishes[n]->fish_path.conversion_list->count);
        for (k = 0; k < fishes[n]->fish_path.conversion_list->count; k++)
        {
          fprintf (stdout, "\n\t\t\t\t%s", babl_get_name (
                   fishes[n]->fish_path.conversion_list->items[k]));
        }
      }
      fprintf (stdout, "\n");
      n++;
   }
  fprintf (stdout, "\n%s %03.1f mb/s\taverage\n",
                      unicode_hbar(16, sum / (n_formats * n_formats - n_formats) / max),
                      sum / (n_formats * n_formats - n_formats));

  fflush (0);

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
