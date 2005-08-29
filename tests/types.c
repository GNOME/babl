#include "babl.h"
#include "math.h"
#include "babl-internal.h"

int OK=1;

double test[] = {    
  0.0, 0.5, 1.0, 0.1, 0.9, 1.1, -0.1, -2, 2.0, 100, -100, 200, 200
};

int samples = sizeof(test) / sizeof(test[0]);

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

  Babl *fmt;

  
  original    = babl_calloc (1,babl->type.bits/8 * samples);
  clipped     = babl_calloc (1,64/8 * samples);
  destination = babl_calloc (1,babl->type.bits/8 * samples);
  transformed = babl_calloc (1,64/8 * samples);

  fmt = babl_format_new (babl_model ("Y"),
                         babl,
                         babl_component ("Y"),
                         NULL);
  
  babl_process (babl_fish (double_vector_format (), fmt),
                test, original, samples);
  babl_process (babl_fish (fmt, double_vector_format ()),
                original, clipped, samples);
  babl_process (babl_fish (double_vector_format (), fmt),
                clipped, destination, samples);
  babl_process (babl_fish (fmt, double_vector_format ()),
                destination, transformed, samples);
  {
    int i;
    for (i=0;i<samples;i++)
      {
        if (fabs (clipped[i] - transformed[i])>0.00001)
          babl_log ("%s:  %f %f %f)",
            babl->instance.name, test[i], clipped[i], transformed[i]
           );
          OK=0;
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

  babl_set_extender (babl_extension_quiet_log ());
  babl_type_each (type_check, NULL);

  babl_destroy ();

  return !OK;
}
