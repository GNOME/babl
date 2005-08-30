/* perform a symmetricality of conversion test on a set of randomized
 * RGBA data */

#include <stdlib.h>
#include "babl.h"
#include "math.h"
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
  double r_min  = -0.2,
         r_max  =  1.5,
         g_min  = -0.2,
         g_max  =  1.5,
         b_min  = -0.2,
         b_max  =  1.5,
         a_min  = -0.5,
         a_max  =  1.5;
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

static Babl *construct_double_format (Babl *model)
{
  void *argument[42+1];
  int   args = 0;
  int   i;

  argument[args++] = model;
  argument[args++] = babl_type ("double");

  for (i=0;i<model->model.components; i++)
    {
      argument[args++] = model->model.component[i];
    }
  argument[args++] = NULL;

#define o(argno) argument[argno],
  return babl_format_new (o(0)  o(1)  o(2)  o(3)
                          o(4)  o(5)  o(6)  o(7)
                          o(8)  o(9) o(10) o(11)
                         o(12) o(13) o(14) o(15)
                         o(16) o(17) o(18) o(19) 
                         o(20) o(21) o(22) o(23) 
                         o(24) o(25) o(26) o(27)
                         o(28) o(29) o(30) o(31)
                         o(32) o(33) o(34) o(35)
                         o(36) o(37) o(38) o(39) 
                         o(40) o(41) o(42) NULL);
#undef o
}

int model_check (Babl *babl,
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
  fmt       = construct_double_format (babl);
  fish_to   = babl_fish (ref_fmt, fmt);
  fish_from = babl_fish (fmt, ref_fmt);
  
  original    = babl_calloc (1,64/8 * babl->model.components * pixels);
  clipped     = babl_calloc (1,64/8 * 4 * pixels);
  destination = babl_calloc (1,64/8 * babl->model.components * pixels);
  transformed = babl_calloc (1,64/8 * 4 * pixels);

  babl_process (fish_to,   test,        original,    pixels);
  babl_process (fish_from, original,    clipped,     pixels);
  babl_process (fish_to,   clipped,     destination, pixels);
  babl_process (fish_from, destination, transformed, pixels);

  {
    int i;
    int log=0;

    for (i=0;i<pixels;i++)
      {
        int j;
        for (j=0;j<4;j++)
           if (fabs (clipped[i*4+j] - transformed[i*4+j])>TOLERANCE)
             {
                log=1;
                OK=0;
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
  babl_model_each (model_check, NULL);

  babl_destroy ();

  return !OK;
}
