#include "babl.h"
#include "math.h"
#include "babl-internal.h"

int OK=1;

#define TOLERANCE 0.0046
#define samples   2048
double test[samples];

double r_interval (double min, double max)
{
  long int rand_i = random ();
  double ret;
  ret = (double) rand_i / RAND_MAX;
  ret*=(max-min);
  ret+=min;
  return ret;
}

void test_init (double min, double max)
{
  int i;
  for (i=0;i<samples;i++)
    {
      test [i]=r_interval(min,max);
    }
}
static Babl *double_vector_format (void)
{
  static Babl *self = NULL;
  
  if (!self)
     self = babl_format_new (
       babl_model ("Y"),
       babl_type ("double"),
       babl_component ("Y"),
       NULL);
  return self;
}

int type_check (Babl *babl,
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

  ref_fmt = double_vector_format ();
  fmt = babl_format_new (babl_model ("Y"),
                         babl,
                         babl_component ("Y"),
                         NULL);
  fish_to   = babl_fish (ref_fmt, fmt);
  fish_from = babl_fish (fmt, ref_fmt);
  
  original    = babl_calloc (1,babl->type.bits/8 * samples);
  clipped     = babl_calloc (1,64/8              * samples);
  destination = babl_calloc (1,babl->type.bits/8 * samples);
  transformed = babl_calloc (1,64/8              * samples);
  
  babl_process (fish_to,   test,        original,    samples);
  babl_process (fish_from, original,    clipped,     samples);
  babl_process (fish_to,   clipped,     destination, samples);
  babl_process (fish_from, destination, transformed, samples);

  {
    int cnt=0;
    int i;
    for (i=0;i<samples;i++)
      {
        if (fabs (clipped[i] - transformed[i])> TOLERANCE)
          {
            if (cnt++<4)
            babl_log ("%s:  %f %f %f)",
             babl->instance.name, test[i], clipped[i], transformed[i]
             );
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

  test_init (0.0, 182.0);

  babl_set_extender (babl_extension_quiet_log ());
  babl_type_each (type_check, NULL);

  babl_destroy ();

  return !OK;
}
