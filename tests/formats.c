/* perform a symmetricality of conversion test on a set of randomized
 * RGBA data */

#include <stdlib.h>
#include <math.h>
#include "babl-internal.h"

int OK=1;

#define pixels    1024
#define TOLERANCE 0.001

double test[pixels * 4];

double r_interval (double min, double max)
{
  long int rand_i = random ();
  double ret;
  ret = (double) rand_i / RAND_MAX;
  ret*=(max-min);
  ret+=min;
  return ret;
}

void test_init (void)
{
  double r_min  =  0.0,
         r_max  =  1.0,
         g_min  =  0.0,
         g_max  =  1.0,
         b_min  =  0.0,
         b_max  =  1.0,
         a_min  =  0.0,
         a_max  =  1.0;
  int i;
  double r,g,b,a;
  for (i=0;i<pixels;i++)
    {
      r=r_interval(r_min, r_max);
      g=r_interval(g_min, g_max);
      b=r_interval(b_min, b_max);
      a=r_interval(a_min, a_max);
      test [i*4 + 0]=r;
      test [i*4 + 1]=g;
      test [i*4 + 2]=b;
      test [i*4 + 3]=a;
    }
}


static Babl *reference_format (void)
{
  static Babl *self = NULL;
  
  if (!self)
     self = babl_format_new (
       babl_model ("RGBA"),
       babl_type ("double"),
       babl_component ("R"),
       babl_component ("G"),
       babl_component ("B"),
       babl_component ("A"),
       NULL);
  return self;
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

  ref_fmt   = reference_format ();
  fmt       = babl; /*construct_double_format (babl);*/
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
    int log=0;
    double loss=0.0;

    for (i=0;i<pixels;i++)
      {
        int j;
        for (j=0;j<4;j++)
          {
           loss += fabs (clipped[i*4+j] - test[i*4+j]);

           if (fabs (clipped[i*4+j] - transformed[i*4+j])>TOLERANCE)
             {
                if (!log)
                  log=1;
                OK=0;
             }
          }
        if (log && log < 5)
          {
            babl_log ("%s", babl->instance.name);
            babl_log ("\ttest:     %2.3f %2.3f %2.3f %2.3f", test [i*4+0],
                                                             test [i*4+1],
                                                             test [i*4+2],
                                                             test [i*4+3]);
            babl_log ("\tclipped:  %2.3f %2.3f %2.3f %2.3f", clipped [i*4+0],
                                                             clipped [i*4+1],
                                                             clipped [i*4+2],
                                                             clipped [i*4+3]);
            babl_log ("\ttrnsfrmd: %2.3f %2.3f %2.3f %2.3f", transformed [i*4+0],
                                                             transformed [i*4+1],
                                                             transformed [i*4+2],
                                                             transformed [i*4+3]);
            log++;
            OK=0;
          }
      }
    loss /= pixels;
       {
          babl_log ("%s\tloss:%f%%", babl->instance.name, loss * 100.0);
          OK = 0;
       }
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
  babl_format_each (format_check, NULL);

  babl_destroy ();

  return !OK;
}
