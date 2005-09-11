/* perform a symmetricality of conversion test on a set of randomized
 * RGBA data */

#include <stdlib.h>
#include <math.h>
#include "babl-internal.h"

#define pixels  1024

static double test[pixels * 4];

static void
test_init (void)
{
  int    i;

  for (i = 0; i < pixels * 4; i++)
     test [i] = (double)random () / RAND_MAX;
}

int format_check (Babl *babl,
                  void *userdata)
{
  void   *original;
  double *clipped;
  void   *destination;
  double *transformed;

  Babl *ref_fmt;
  Babl *fmt;
  Babl *fish_to;
  Babl *fish_from;

  ref_fmt   = babl_format_new (
       babl_model ("RGBA"),
       babl_type ("double"),
       babl_component ("R"),
       babl_component ("G"),
       babl_component ("B"),
       babl_component ("A"),
       NULL);

  fmt       = babl; 
  fish_to   = babl_fish (ref_fmt, fmt);
  fish_from = babl_fish (fmt, ref_fmt);
  
  original    = babl_calloc (pixels, fmt->format.bytes_per_pixel);
  clipped     = babl_calloc (pixels, ref_fmt->format.bytes_per_pixel);
  destination = babl_calloc (pixels, fmt->format.bytes_per_pixel);
  transformed = babl_calloc (pixels,  ref_fmt->format.bytes_per_pixel);

  babl_process (fish_to,   test,        original,    pixels);
  babl_process (fish_from, original,    clipped,     pixels);
  babl_process (fish_to,   clipped,     destination, pixels);
  babl_process (fish_from, destination, transformed, pixels);

  {
    int i;
    double loss=0.0;

    for (i=0;i<pixels*4;i++)
      {
        loss += fabs (clipped[i] - test[i]);
      }
    loss /= pixels;

    if (userdata)
      babl_log ("%s\tloss: %f", babl->instance.name, loss);
    babl->format.loss = loss;
  }
  
  babl_free (original);
  babl_free (clipped);
  babl_free (destination);
  babl_free (transformed);
  return 0;
}

int main (void)
{
  babl_init ();
  test_init ();

  babl_set_extender (babl_extension_quiet_log ());
  babl_format_each (format_check, (void*)1);

  babl_destroy ();

  return 0;
}
