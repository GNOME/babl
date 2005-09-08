/* perform a symmetricality of conversion test on a set of randomized
 * RGBA data */

#include <stdlib.h>
#include <math.h>
#include "babl-internal.h"

int OK=1;

#define pixels    102400
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
  double r_min  = 0.0,
         r_max  = 1.0,
         g_min  = 0.0,
         g_max  = 1.0,
         b_min  = 0.0,
         b_max  = 1.0,
         a_min  = 0.0,
         a_max  = 1.0;
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

static void
validate_conversion (BablConversion *conversion)
{
  Babl *fmt_rgba_double = reference_format ();
  Babl *fmt_source      = BABL(conversion->source);
  Babl *fmt_destination = BABL(conversion->destination);

  void    *source;
  void    *destination;
  double  *destination_rgba_double;
  void    *ref_destination;
  double  *ref_destination_rgba_double;

  source          = babl_calloc (pixels, fmt_source->format.bytes_per_pixel);
  destination     = babl_calloc (pixels, fmt_destination->format.bytes_per_pixel);
  ref_destination = babl_calloc (pixels, fmt_destination->format.bytes_per_pixel);
  destination_rgba_double     = babl_calloc (pixels, fmt_rgba_double->format.bytes_per_pixel);
  ref_destination_rgba_double = babl_calloc (pixels, fmt_rgba_double->format.bytes_per_pixel);
  
  babl_process (babl_fish_reference (fmt_rgba_double, fmt_source),
      test, source, pixels);
  babl_process (babl_fish_simple (conversion),
      source, destination, pixels);

  babl_process (babl_fish_reference (fmt_source, fmt_destination),
      source, ref_destination, pixels);

  babl_process (babl_fish_reference (fmt_destination, fmt_rgba_double),
      ref_destination, ref_destination_rgba_double, pixels);
  babl_process (babl_fish_reference (fmt_destination, fmt_rgba_double),
      destination, destination_rgba_double, pixels);

  {
    int i;
    int log=0;

    for (i=0;i<pixels;i++)
      {
        int j;
        for (j=0;j<4;j++)
           if (fabs (destination_rgba_double[i*4+j] - 
                     ref_destination_rgba_double[i*4+j])>TOLERANCE)
             {
                if (!log)
                  log=1;
                OK=0;
             }
        if (log && log < 5)
          {
            babl_log ("%s", conversion->instance.name);
            babl_log ("\ttest:           %2.3f %2.3f %2.3f %2.3f", test [i*4+0],
                                                                   test [i*4+1],
                                                                   test [i*4+2],
                                                                   test [i*4+3]);
            babl_log ("\tconversion:     %2.3f %2.3f %2.3f %2.3f", destination_rgba_double [i*4+0],
                                                                   destination_rgba_double [i*4+1],
                                                                   destination_rgba_double [i*4+2],
                                                                   destination_rgba_double [i*4+3]);
            babl_log ("\tref_conversion: %2.3f %2.3f %2.3f %2.3f", ref_destination_rgba_double [i*4+0],
                                                                   ref_destination_rgba_double [i*4+1],
                                                                   ref_destination_rgba_double [i*4+2],
                                                                   ref_destination_rgba_double [i*4+3]);
            log++;
            OK=0;
          }
      }
  }

  
  babl_free (source);
  babl_free (destination);
  babl_free (destination_rgba_double);
  babl_free (ref_destination);
  babl_free (ref_destination_rgba_double);
}

static int
each_conversion (Babl *babl,
                 void *userdata)
{
  Babl *source = BABL(babl->conversion.source);
  Babl *destination = BABL(babl->conversion.destination);

  if (source->instance.id      != BABL_RGBA   &&
      destination->instance.id != BABL_RGBA   &&
      source->instance.id      != BABL_DOUBLE &&
      destination->instance.id != BABL_DOUBLE &&
      source->class_type       == BABL_FORMAT &&
      destination->class_type  == BABL_FORMAT)
  {
    validate_conversion ((BablConversion*)babl);
  }
  return 0;
}

int main (void)
{
  babl_init ();
  test_init ();

  babl_set_extender (babl_extension_quiet_log ());
  babl_conversion_each (each_conversion, NULL);

  babl_destroy ();

  return !OK;
}
