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

int ITERATIONS = 100;
#define  N_PIXELS (512*256)  // a too small batch makes the test set live
                               // in l2 cache skewing results

                               // we could also add a cache purger..

int unit_pixels = 1; // use megapixels per second instead of bytes

#define  N_BYTES  N_PIXELS * (4 * 8)

static const char *
unicode_hbar (int    width, 
              double fraction)
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
test (int set_no)
{
  int i, j;
  int OK = 1;

  char *src_data = babl_malloc (N_BYTES);
  char *dst_data = babl_malloc (N_BYTES);
  double sum = 0;

#define default_set(space, out_space) \
       babl_format_with_space("RGBA float", babl_space(space)), \
       babl_format_with_space("RaGaBaA float", babl_space(space)), \
       babl_format_with_space("R'G'B'A float", babl_space(space)), \
       babl_format_with_space("cairo-ARGB32", babl_space(out_space)) 

  const Babl **formats=NULL;
  const Babl *format_sets[][20]={
        { babl_format_with_space("R'G'B'A u8", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("R'G'B'A u16", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("R'G'B'A half", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("R'G'B'A float", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("RGBA u16", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("RGBA half", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("RGBA float", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("Y' u8", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("Y' u16", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("Y' half", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("Y' float", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("Y u8", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("Y u16", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("Y half", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("Y float", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },


        { babl_format_with_space("R'G'B'A u8", babl_space("sRGB")), default_set("sRGB", "ProPhoto"), NULL },
        { babl_format_with_space("R'G'B'A u16", babl_space("sRGB")), default_set("sRGB", "ProPhoto"), NULL },
        { babl_format_with_space("R'G'B'A half", babl_space("sRGB")), default_set("sRGB", "ProPhoto"), NULL },
        { babl_format_with_space("R'G'B'A float", babl_space("sRGB")), default_set("sRGB", "ProPhoto"), NULL },
        { babl_format_with_space("RGBA u16", babl_space("sRGB")), default_set("sRGB", "ProPhoto"), NULL },
        { babl_format_with_space("RGBA half", babl_space("sRGB")), default_set("sRGB", "ProPhoto"), NULL },
        { babl_format_with_space("RGBA float", babl_space("sRGB")), default_set("sRGB", "ProPhoto"), NULL },
        { babl_format_with_space("Y' u8", babl_space("sRGB")), default_set("sRGB", "ProPhoto"), NULL },
        { babl_format_with_space("Y' u16", babl_space("sRGB")), default_set("sRGB", "ProPhoto"), NULL },
        { babl_format_with_space("Y' half", babl_space("sRGB")), default_set("sRGB", "ProPhoto"), NULL },
        { babl_format_with_space("Y' float", babl_space("sRGB")), default_set("sRGB", "ProPhoto"), NULL },
        { babl_format_with_space("Y u8", babl_space("sRGB")), default_set("sRGB", "ProPhoto"), NULL },
        { babl_format_with_space("Y u16", babl_space("sRGB")), default_set("sRGB", "ProPhoto"), NULL },
        { babl_format_with_space("Y half", babl_space("sRGB")), default_set("sRGB", "ProPhoto"), NULL },
        { babl_format_with_space("Y float", babl_space("sRGB")), default_set("sRGB", "ProPhoto"), NULL },

        { babl_format_with_space("R'G'B'A u8", babl_space("Apple")), default_set("Apple", "ProPhoto"), NULL },
        { babl_format_with_space("R'G'B'A u16", babl_space("Apple")), default_set("Apple", "ProPhoto"), NULL },
        { babl_format_with_space("R'G'B'A half", babl_space("Apple")), default_set("Apple", "ProPhoto"), NULL },
        { babl_format_with_space("R'G'B'A float", babl_space("Apple")), default_set("Apple", "ProPhoto"), NULL },
        { babl_format_with_space("RGBA u16", babl_space("Apple")), default_set("Apple", "ProPhoto"), NULL },
        { babl_format_with_space("RGBA half", babl_space("Apple")), default_set("Apple", "ProPhoto"), NULL },
        { babl_format_with_space("RGBA float", babl_space("Apple")), default_set("Apple", "ProPhoto"), NULL },
        { babl_format_with_space("Y' u8", babl_space("Apple")), default_set("Apple", "ProPhoto"), NULL },
        { babl_format_with_space("Y' u16", babl_space("Apple")), default_set("Apple", "ProPhoto"), NULL },
        { babl_format_with_space("Y' half", babl_space("Apple")), default_set("Apple", "ProPhoto"), NULL },
        { babl_format_with_space("Y' float", babl_space("Apple")), default_set("Apple", "ProPhoto"), NULL },
        { babl_format_with_space("Y u8", babl_space("Apple")), default_set("Apple", "ProPhoto"), NULL },
        { babl_format_with_space("Y u16", babl_space("Apple")), default_set("Apple", "ProPhoto"), NULL },
        { babl_format_with_space("Y half", babl_space("Apple")), default_set("Apple", "ProPhoto"), NULL },
        { babl_format_with_space("Y float", babl_space("Apple")), default_set("Apple", "ProPhoto"), NULL },
     };

  int n_formats = 0;
  int n_sets = sizeof(format_sets)/sizeof(format_sets[0]);

  const Babl *fishes[50 * 50];
  double mbps[50 * 50] = {0,};
  long n;
  double max = 0.0;

  int show_details = 0;
  int set_iter = 0;
  int first_run = 1;

  if (set_no > n_sets-1) set_no = n_sets-1;

  while (set_iter < n_sets)
  {
  if (set_no >= 0)
    formats=&format_sets[set_no][0];
  else
    formats=&format_sets[set_iter][0];


 for (i = 0; i < N_BYTES; i++)
   src_data[i] = random();

 fprintf (stdout, "\n\n");
 fprintf (stdout, "set %i formats:\n", set_iter);
 for (i = 0; formats[i]; i++)
 {
   fprintf (stdout, "  %s\n", babl_get_name (formats[i]));
   n_formats++;
 }
 fprintf (stdout, "\n");

 //fprintf (stdout,"%i iterations of %i pixels, mp/s is for sum of source and destinations bytes\n", ITERATIONS, N_PIXELS);
 

 n = 0;
 for (i = 0; formats[i]; i++)
   for (j = 0; formats[j]; j++)
   if (i != j)
   {
      const Babl *fish = babl_fish (formats[i], formats[j]);
      long end, start;
      int iters = ITERATIONS;

      fprintf (stdout, "%s to %s          \r", babl_get_name (formats[i]),
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
      mbps [n] = (N_PIXELS * ITERATIONS / 1000.0 / 1000.0) / ((end-start)/(1000.0*1000.0));
      if (!unit_pixels)
        mbps [n] *= (babl_format_get_bytes_per_pixel (formats[i]) +
		      babl_format_get_bytes_per_pixel (formats[j]));

      sum += mbps[n];
      if (mbps[n] > max && first_run)
        max = mbps[n];
      n++;
   }


  fprintf (stdout, "\n%s %03.1f mb/s\taverage\n",
                      unicode_hbar(16, sum / (n_formats * n_formats - n_formats) / max),
                      sum / (n_formats * n_formats - n_formats));


 n = 0;
 for (i = 0; formats[i]; i++)
   for (j = 0; formats[j]; j++)
   if (i != j)
   {
      fprintf (stdout, "%s %03.1f m%s/s\t%s to %s %.9f",
                      unicode_hbar(16, mbps[n] / max),
                      mbps[n],
		      unit_pixels?"p":"b",
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
        fprintf (stdout, "[%d]", fishes[n]->fish_path.conversion_list->count);
        if (show_details)
        {
        for (k = 0; k < fishes[n]->fish_path.conversion_list->count; k++)
        {
          fprintf (stdout, "\n\t\t\t\t%s", babl_get_name (
                   fishes[n]->fish_path.conversion_list->items[k]));
        }
        }
      }
      fprintf (stdout, "\n");
      n++;
   }

  fflush (0);
  set_iter++;
  first_run = 0;
  }

  if (!OK)
    return -1;
  return 0;
}

int
main (int    argc,
      char **argv)
{
  //if (argv[1]) ITERATIONS = atoi (argv[1]);
  babl_init ();
  if (argv[1])
  {
    if (test (atoi(argv[1])))
      return -1;
  }
  else
    if (test (-1))
      return -1;
  babl_exit ();
  return 0;
}
