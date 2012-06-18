#include <stdio.h>
#include <math.h>

#include "pow-24.h"

int
main (int argc, char *argv[])
{
  double s, r1, r2, diff, max;
  double at_s, at_r1, at_r2;
  long i;

  s = 0.03;
  at_s = 0;
  max = 0;
  for (i = 0; i < 1100000000; i++)
    {
      r1 = babl_pow_24 (s);
      r2 = pow (s, 2.4);
      diff = fabs (r2-r1);
      if (diff > max) {
	max = diff;
	at_s = s;
	at_r1 = r1;
	at_r2 = r2;
      }
      s += 0.000000001;
    }
  printf ("x^2.4\n");
  printf ("max from 0 to %f is %e\n", s, max);
  printf ("at: %f %f %f\n", at_s, at_r1, at_r2);

  s = 0.03;
  at_s = 0;
  max = 0;
  for (i = 0; i < 1100000000; i++)
    {
      r1 = babl_pow_1_24 (s);
      r2 = pow (s, 1/2.4);
      diff = fabs (r2-r1);
      if (diff > max) {
	max = diff;
	at_s = s;
	at_r1 = r1;
	at_r2 = r2;
      }
      s += 0.000000001;
    }
  printf ("x^(1/2.4)\n");
  printf ("max from 0 to %f is %e\n", s, max);
  printf ("at: %f %f %f\n", at_s, at_r1, at_r2);

  return 0;
}
