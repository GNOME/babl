#include "config.h"
#include <math.h>
#include "babl-internal.h"

int OK = 1;

int main (void)
{
  const Babl * trc;
  babl_init ();

  trc = babl_trc_new (NULL, BABL_TRC_FORMULA_GAMMA, 2.2, 0, NULL);
  for (float a = 0.0f; a <= 1.0f; a+=0.1f)
  {
    float b = babl_trc_to_linear (trc, a);
    float c = babl_trc_from_linear (trc, b);
    if (fabsf (a-c) > 0.01)
    {
      OK = 0;
      printf ("!!");
    }
    printf ("%s %f:%f:%f\n", babl_get_name (trc), a, b, c);
  }

  trc = babl_trc_new (NULL, BABL_TRC_FORMULA_GAMMA, 0.2, 0, NULL);
  for (float a = 0.0f; a <= 1.0f; a+=0.1f)
  {
    float b = babl_trc_to_linear (trc, a);
    float c = babl_trc_from_linear (trc, b);
    if (fabsf (a-c) > 0.01)
    {
      OK = 0;
      printf ("!!");
    }
    printf ("%s %f:%f:%f\n", babl_get_name (trc), a, b, c);
  }

  if(0){
     /*
         mostly symmetric, but fails on some of the low colors, test-case from #66

discont 0.000000:0.000000:0.000000
!!discont 0.025000:0.000000:0.000000
!!discont 0.050000:0.000000:0.000000
discont 0.075000:1.743565:0.075000
discont 0.100000:3.392027:0.100000
discont 0.125000:5.708076:0.125000
discont 0.150000:8.752512:0.150000
discont 0.175000:12.579302:0.175000
discont 0.200000:17.237312:0.200000
discont 0.225000:22.771437:0.225000
discont 0.250000:29.223341:0.250000

      */

    float params[8] = {2.4, 255.0 / 15 / 1.055, 0.055 / 1.055, 0, 15.0/255, 0, 0};
    trc = babl_trc_new ("discont", BABL_TRC_FORMULA_SRGB, params[0], 0, params);
    for (float a = 0.0f; a <= 1.0f; a+=0.025f)
    {
      float b = babl_trc_to_linear (trc, a);
      float c = babl_trc_from_linear (trc, b);
      if (fabsf (a-c) > 0.01)
      {
        OK = 0;
        printf ("!!");
      }
      printf ("%s %f:%f:%f\n", babl_get_name (trc), a, b, c);
    }
  }


  {
    float params[8] = {2.4, 0.947, 0.052, 0.077, 0.040, 0, 0};
    const Babl *srgb_trc = babl_trc ("sRGB");
    trc = babl_trc_new ("fake-srgb", BABL_TRC_FORMULA_SRGB, params[0], 0, params);
    for (float a = 0.0f; a <= 1.0f; a+=0.025f)
    {
      float b = babl_trc_to_linear (trc, a);
      float c = babl_trc_from_linear (trc, b);

      float d = babl_trc_to_linear (srgb_trc, a);
      float e = babl_trc_from_linear (srgb_trc, d);
      if (fabsf (a-c) > 0.01)
      {
        OK = 0;
        printf ("!!");
      }
      printf ("%s %f:  %f:%f    %s  %f:%f    \n", babl_get_name (trc), a, b, c,  babl_get_name (srgb_trc),   d, e);
    }
  }
  babl_exit ();

  return !OK;
}
