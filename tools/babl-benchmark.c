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

#ifdef _WIN32
/* On Windows setenv() does not exist, using _putenv_s() instead. The overwrite
 * arg is ignored (i.e. same as always 1).
 */
#define setenv(name,value,overwrite) _putenv_s(name, value)
#endif

int ITERATIONS = 4;
#define  N_PIXELS (1024*1024)  // a too small batch makes the test set live
                               // in l2 cache skewing results

                               // we could also add a cache purger..

int unit_pixels = 1; // use megapixels per second instead of bytes

int global_relative_scale = 1;

int exclude_identity = 1;
#define  N_BYTES  N_PIXELS * (4 * 8)

#define BAR_WIDTH 40

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

int show_details = 0;
int progress = 1;

#include <stdio.h>
#include <stdint.h>

#if 0
 // more accurate, the 2100 constant is roughly
 // what is needed on my system to convert to 1.5ghz
 // constant clock performance

inline uint64_t bench_ticks (void) {
    uint32_t lo, hi;
    __asm__ __volatile__ (
      "xorl %%eax, %%eax\n"
      "cpuid\n"
      "rdtsc\n"
      : "=a" (lo), "=d" (hi)
      :
      : "%ebx", "%ecx");
    return ((uint64_t)hi << 32 | lo) / 2100;
}
#else
static inline uint64_t bench_ticks (void) { return babl_ticks();}
#endif

#if 0
main()
{
    unsigned long long x;
    unsigned long long y;
    x = rdtsc();
    printf("%lld\n",x);
    y = rdtsc();
    printf("%lld\n",y);
    printf("it took this long to call printf: %lld\n",y-x);
}
#endif

static int
test (int set_no)
{
  int i, j;
  int OK = 1;

  //printf("\e[3g");
  //printf("     \eH       \eH    \eH      \eH ");

  char *src_data = babl_malloc (N_BYTES);
  char *dst_data = babl_malloc (N_BYTES);

#define default_set(space, out_space) \
       babl_format_with_space("RGBA float", babl_space(space)), \
       babl_format_with_space("RaGaBaA float", babl_space(space)), \
       babl_format_with_space("R'G'B'A float", babl_space(space)), \
       babl_format_with_space("RGBA u16", babl_space(out_space)), \
       babl_format_with_space("R'G'B'A u8", babl_space(out_space)) 

#define cmyk_set(space, out_space) \
       babl_format_with_space("cmykA float", babl_space(space)), \
       babl_format_with_space("camayakaA float", babl_space(space)), \
       babl_format_with_space("RGBA u16", babl_space(out_space)), \
       babl_format_with_space("R'G'B'A u8", babl_space(out_space)) 

  const Babl **formats=NULL;
  const Babl *format_sets[][20]={
        { babl_format_with_space("R'G'B' u8", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("R'G'B' u16", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("R'G'B' half", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("R'G'B' float", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("RGB u8", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("RGB u16", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("RGB half", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("RGB float", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("R'G'B'A u8", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("R'G'B'A u16", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("R'G'B'A half", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("R'G'B'A float", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("RGBA u8", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("RGBA u16", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("RGBA half", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("RGBA float", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("Y u8", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("Y u16", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("Y half", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("Y float", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("Y' u8", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("Y' u16", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("Y' half", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("Y' float", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("Y'A u8", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("Y'A u16", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("Y'A half", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("Y'A float", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("YA u8", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("YA u16", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("YA half", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("YA float", babl_space("sRGB")), default_set("sRGB", "sRGB"), NULL },





        { babl_format_with_space("R'G'B' u8", babl_space("ProPhoto")), default_set("ProPhoto", "Rec2020"), NULL },
        { babl_format_with_space("R'G'B' u16", babl_space("ProPhoto")), default_set("ProPhoto", "Rec2020"), NULL },
        { babl_format_with_space("R'G'B' half", babl_space("ProPhoto")), default_set("ProPhoto", "Rec2020"), NULL },
        { babl_format_with_space("R'G'B' float", babl_space("ProPhoto")), default_set("ProPhoto", "Rec2020"), NULL },
        { babl_format_with_space("RGB u8", babl_space("ProPhoto")), default_set("ProPhoto", "Rec2020"), NULL },
        { babl_format_with_space("RGB u16", babl_space("ProPhoto")), default_set("ProPhoto", "Rec2020"), NULL },
        { babl_format_with_space("RGB half", babl_space("ProPhoto")), default_set("ProPhoto", "Rec2020"), NULL },
        { babl_format_with_space("RGB float", babl_space("ProPhoto")), default_set("ProPhoto", "Rec2020"), NULL },
        { babl_format_with_space("R'G'B'A u8", babl_space("ProPhoto")), default_set("ProPhoto", "Rec2020"), NULL },
        { babl_format_with_space("R'G'B'A u16", babl_space("ProPhoto")), default_set("ProPhoto", "Rec2020"), NULL },
        { babl_format_with_space("R'G'B'A half", babl_space("ProPhoto")), default_set("ProPhoto", "Rec2020"), NULL },
        { babl_format_with_space("R'G'B'A float", babl_space("ProPhoto")), default_set("ProPhoto", "Rec2020"), NULL },
        { babl_format_with_space("RGBA u8", babl_space("ProPhoto")), default_set("ProPhoto", "Rec2020"), NULL },
        { babl_format_with_space("RGBA u16", babl_space("ProPhoto")), default_set("ProPhoto", "Rec2020"), NULL },
        { babl_format_with_space("RGBA half", babl_space("ProPhoto")), default_set("ProPhoto", "Rec2020"), NULL },
        { babl_format_with_space("RGBA float", babl_space("ProPhoto")), default_set("ProPhoto", "Rec2020"), NULL },
        { babl_format_with_space("Y u8", babl_space("ProPhoto")), default_set("ProPhoto", "Rec2020"), NULL },
        { babl_format_with_space("Y u16", babl_space("ProPhoto")), default_set("ProPhoto", "Rec2020"), NULL },
        { babl_format_with_space("Y half", babl_space("ProPhoto")), default_set("ProPhoto", "Rec2020"), NULL },
        { babl_format_with_space("Y float", babl_space("ProPhoto")), default_set("ProPhoto", "Rec2020"), NULL },
        { babl_format_with_space("Y' u8", babl_space("ProPhoto")), default_set("ProPhoto", "Rec2020"), NULL },
        { babl_format_with_space("Y' u16", babl_space("ProPhoto")), default_set("ProPhoto", "Rec2020"), NULL },
        { babl_format_with_space("Y' half", babl_space("ProPhoto")), default_set("ProPhoto", "Rec2020"), NULL },
        { babl_format_with_space("Y' float", babl_space("ProPhoto")), default_set("ProPhoto", "Rec2020"), NULL },
        { babl_format_with_space("Y'A u8", babl_space("ProPhoto")), default_set("ProPhoto", "Rec2020"), NULL },
        { babl_format_with_space("Y'A u16", babl_space("ProPhoto")), default_set("ProPhoto", "Rec2020"), NULL },
        { babl_format_with_space("Y'A half", babl_space("ProPhoto")), default_set("ProPhoto", "Rec2020"), NULL },
        { babl_format_with_space("Y'A float", babl_space("ProPhoto")), default_set("ProPhoto", "Rec2020"), NULL },
        { babl_format_with_space("YA u8", babl_space("ProPhoto")), default_set("ProPhoto", "Rec2020"), NULL },
        { babl_format_with_space("YA u16", babl_space("ProPhoto")), default_set("ProPhoto", "Rec2020"), NULL },
        { babl_format_with_space("YA half", babl_space("ProPhoto")), default_set("ProPhoto", "Rec2020"), NULL },
        { babl_format_with_space("YA float", babl_space("ProPhoto")), default_set("ProPhoto", "Rec2020"), NULL },

        { babl_format_with_space("R'G'B' u8", babl_space("sRGB")), default_set("sRGB", "Rec2020"), NULL },
        { babl_format_with_space("R'G'B' u16", babl_space("sRGB")), default_set("sRGB", "Rec2020"), NULL },
        { babl_format_with_space("R'G'B' half", babl_space("sRGB")), default_set("sRGB", "Rec2020"), NULL },
        { babl_format_with_space("R'G'B' float", babl_space("sRGB")), default_set("sRGB", "Rec2020"), NULL },
        { babl_format_with_space("RGB u8", babl_space("sRGB")), default_set("sRGB", "Rec2020"), NULL },
        { babl_format_with_space("RGB u16", babl_space("sRGB")), default_set("sRGB", "Rec2020"), NULL },
        { babl_format_with_space("RGB half", babl_space("sRGB")), default_set("sRGB", "Rec2020"), NULL },
        { babl_format_with_space("RGB float", babl_space("sRGB")), default_set("sRGB", "Rec2020"), NULL },
        { babl_format_with_space("R'G'B'A u8", babl_space("sRGB")), default_set("sRGB", "Rec2020"), NULL },
        { babl_format_with_space("R'G'B'A u16", babl_space("sRGB")), default_set("sRGB", "Rec2020"), NULL },
        { babl_format_with_space("R'G'B'A half", babl_space("sRGB")), default_set("sRGB", "Rec2020"), NULL },
        { babl_format_with_space("R'G'B'A float", babl_space("sRGB")), default_set("sRGB", "Rec2020"), NULL },
        { babl_format_with_space("RGBA u8", babl_space("sRGB")), default_set("sRGB", "Rec2020"), NULL },
        { babl_format_with_space("RGBA u16", babl_space("sRGB")), default_set("sRGB", "Rec2020"), NULL },
        { babl_format_with_space("RGBA half", babl_space("sRGB")), default_set("sRGB", "Rec2020"), NULL },
        { babl_format_with_space("RGBA float", babl_space("sRGB")), default_set("sRGB", "Rec2020"), NULL },
        { babl_format_with_space("Y u8", babl_space("sRGB")), default_set("sRGB", "Rec2020"), NULL },
        { babl_format_with_space("Y u16", babl_space("sRGB")), default_set("sRGB", "Rec2020"), NULL },
        { babl_format_with_space("Y half", babl_space("sRGB")), default_set("sRGB", "Rec2020"), NULL },
        { babl_format_with_space("Y float", babl_space("sRGB")), default_set("sRGB", "Rec2020"), NULL },
        { babl_format_with_space("Y' u8", babl_space("sRGB")), default_set("sRGB", "Rec2020"), NULL },
        { babl_format_with_space("Y' u16", babl_space("sRGB")), default_set("sRGB", "Rec2020"), NULL },
        { babl_format_with_space("Y' half", babl_space("sRGB")), default_set("sRGB", "Rec2020"), NULL },
        { babl_format_with_space("Y' float", babl_space("sRGB")), default_set("sRGB", "Rec2020"), NULL },
        { babl_format_with_space("Y'A u8", babl_space("sRGB")), default_set("sRGB", "Rec2020"), NULL },
        { babl_format_with_space("Y'A u16", babl_space("sRGB")), default_set("sRGB", "Rec2020"), NULL },
        { babl_format_with_space("Y'A half", babl_space("sRGB")), default_set("sRGB", "Rec2020"), NULL },
        { babl_format_with_space("Y'A float", babl_space("sRGB")), default_set("sRGB", "Rec2020"), NULL },
        { babl_format_with_space("YA u8", babl_space("sRGB")), default_set("sRGB", "Rec2020"), NULL },
        { babl_format_with_space("YA u16", babl_space("sRGB")), default_set("sRGB", "Rec2020"), NULL },
        { babl_format_with_space("YA half", babl_space("sRGB")), default_set("sRGB", "Rec2020"), NULL },
        { babl_format_with_space("YA float", babl_space("sRGB")), default_set("sRGB", "Rec2020"), NULL },
        
        
        { babl_format_with_space("CMYKA float", babl_space("sRGB")), cmyk_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("CMYKA u16", babl_space("sRGB")), cmyk_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("CMYKA u8", babl_space("sRGB")), cmyk_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("cmykA float", babl_space("sRGB")), cmyk_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("cmykA u16", babl_space("sRGB")), cmyk_set("sRGB", "sRGB"), NULL },
        { babl_format_with_space("cmykA u8", babl_space("sRGB")), cmyk_set("sRGB", "sRGB"), NULL },

     };

  int n_formats = 0;
  int n_sets = sizeof(format_sets)/sizeof(format_sets[0]);

  const Babl *fishes[50 * 50];
  double mbps[50 * 50] = {0,};
  long n;

  int set_iter = 0;
  int first_run = 1;
  float max_throughput = 0;
  double max = 0.0;

  if (set_no > n_sets-1) set_no = n_sets-1;

  while (set_iter < n_sets)
  {
  double sum = 0;
          n_formats = 0;
  if (set_no >= 0)
    formats=&format_sets[set_no][0];
  else
    formats=&format_sets[set_iter][0];


 for (i = 0; i < N_BYTES; i++)
   src_data[i] = random();

 fprintf (stdout, "\n\n");
 //fprintf (stdout, "set %i:\n", set_iter);
 for (i = 0; formats[i]; i++)
 {
  // fprintf (stdout, "  %s\n", babl_get_name (formats[i]));
   n_formats++;
 }
 //fprintf (stdout, "\n");

 //fprintf (stdout,"%i iterations of %i pixels, mp/s is for sum of source and destinations bytes\n", ITERATIONS, N_PIXELS);
 

 n = 0;
 for (i = 0; formats[i]; i++)
   for (j = 0; formats[j]; j++)
   //if (i != j && i != (n_formats - 1) && (i==0 || j!=n_formats-1))
   if (i != j && i != (n_formats - 1) && (i==0 || j!=n_formats-1) && (j==0 || i==0) && (!exclude_identity || formats[i] != formats[j]))
   {
      const Babl *fish = babl_fish (formats[i], formats[j]);
      long end, start;
      int iters = ITERATIONS;
      if (progress)
      fprintf (stderr, "%s to %s               \r", babl_get_name (formats[i]),
                                                   babl_get_name (formats[j]));

      /* a round of warmup */
      babl_process (fish, src_data, dst_data, N_PIXELS/4);
      start = bench_ticks ();
      while (iters--)
      {
        babl_process (fish, src_data, dst_data, N_PIXELS);
      }
      end = bench_ticks ();
      fishes[n] = fish;
      mbps [n] = (N_PIXELS * ITERATIONS / 1000.0 / 1000.0) / ((end-start)/(1000.0*1000.0));
      if (!unit_pixels)
        mbps [n] *= (babl_format_get_bytes_per_pixel (formats[i]) +
		      babl_format_get_bytes_per_pixel (formats[j]));

      sum += mbps[n];
#if 1
      if (mbps[n] > max && first_run)
        max = mbps[n];
#endif
      n++;
   }

  if (progress)
  fprintf (stderr, "                                                       \r");

  {
  float throughput  = sum / n;
  if (throughput > max_throughput)
     max_throughput = throughput;
  fprintf (stdout, "%s %03.3f mp/s\tWorkflow: %s to %s\n\n",
                      unicode_hbar(BAR_WIDTH, throughput / max_throughput), throughput,
                      babl_get_name (formats[0]),
                      babl_get_name (formats[n_formats-1]));
  }

      if (mbps[n] > max && first_run)
        max = mbps[n];

  if (global_relative_scale) max = max_throughput;

 n = 0;
 for (i = 0; formats[i]; i++)
   for (j = 0; formats[j]; j++)
   //if (i != j && i != (n_formats - 1) && (i==0 || j!=n_formats-1))
   if (i != j && i != (n_formats - 1) && (i==0 || j!=n_formats-1) && (j==0 || i==0) && (!exclude_identity || formats[i] != formats[j]))
   {
      fprintf (stdout, "%s %03.3f m%s/s\t",
                      unicode_hbar(BAR_WIDTH, mbps[n] / max),
                      mbps[n],
		      unit_pixels?"p":"b");


      if (fishes[n]->class_type == BABL_FISH_REFERENCE)
      {
        fprintf (stdout, "REF ");
      }
      else if (fishes[n]->class_type == BABL_FISH_PATH)
      {
#if BABL_MICRO_VERSION>=89
        if (fishes[n]->fish_path.u8_lut)
          fprintf (stdout, "LUT ");
        else

#endif
        fprintf (stdout, "  %d ", fishes[n]->fish_path.conversion_list->count);
      }

      fprintf (stdout, "%.9f %s to %s ",
                      fishes[n]->fish.error,
                      babl_get_name (formats[i]),
                      babl_get_name (formats[j]));

      if (fishes[n]->class_type == BABL_FISH_PATH && show_details)
      {
      for (int k = 0; k < fishes[n]->fish_path.conversion_list->count; k++)
      {
        fprintf (stdout, "\n  %s", babl_get_name (
                 fishes[n]->fish_path.conversion_list->items[k]));
      }
      fprintf (stdout, "\n");
      }
      fprintf (stdout, "\n");
      n++;
   }

  fflush (0);
  set_iter++;
  first_run = 0;
  if (set_no>=0)
          return !OK;
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
  setenv ("BABL_INHIBIT_CACHE", "1", 1);
  babl_init ();
  if (argv[1] && argv[2]) show_details = 1;
  if (argv[1])
  {
    if (test (atoi(argv[1])))
      return -1;
 // if (test (atoi(argv[1])))
 //   return -1;
  }
  else
  {
    test (-1);
//  test (-1);
  }
  babl_exit ();
  return 0;
}
