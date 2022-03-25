// utility program for validating lolremez approximation constants, and
// BablPolynomial based approximations, for TRCs
// the currently used apprimxations for 1.8 and 2.2 gamma pow functions are
// validated to be loss-less when coded for 8bit.

#include "config.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "babl-internal.h"

#if 0

#define GAMMA 2.2

static inline float from_linear (float x)
{
  if (x >= 0.01f && x < 0.25f)
  {
    double u = -1.1853049266795914e+8;
    u = u * x + 1.6235355750617304e+8;
    u = u * x + -9.6434183855508922e+7;
    u = u * x + 3.2595749146174438e+7;
    u = u * x + -6.9216734175519044e+6;
    u = u * x + 9.6337373983643336e+5;
    u = u * x + -8.9295299887376452e+4;
    u = u * x + 5.5387559329470092e+3;
    u = u * x + -2.3522564268245811e+2;
    u = u * x + 8.8234901614165394;
    return u * x + 5.3919966190648492e-2;
  } else if (x >= 0.25f && x < 1.0f)
  {
    double u = -2.1065242890384543e-1;
    u = u * x + 1.7554867367832886;
    u = u * x + -6.6371047248064382;
    u = u * x + 1.5049549954517457e+1;
    u = u * x + -2.279671781745644e+1;
    u = u * x + 2.4331499227325978e+1;
    u = u * x + -1.8839523095731037e+1;
    u = u * x + 1.0802279176589768e+1;
    u = u * x + -4.7776729355620852;
    u = u * x + 2.1410886948010769;
    return u * x + 1.817672123838504e-1;
  }
  return powf (x, 1.0f/2.2f);
}

static inline float to_linear (float x)
{
  if (x >= 0.01f && x < 1.0f)
  {
    double u = -1.7565198334207539;
    u = u * x + 9.4503605497836926;
    u = u * x + -2.2016178903082791e+1;
    u = u * x + 2.9177361786084179e+1;
    u = u * x + -2.4368251609523336e+1;
    u = u * x + 1.3522663223248737e+1;
    u = u * x + -5.253344907664925;
    u = u * x + 1.7182864905042889;
    u = u * x + 5.2860458501353106e-1;
    u = u * x + -3.0000031884069502e-3;
    return u * x + 1.6952727496833812e-5;
  }
  return powf (x, 2.2);
}
#endif

#if 0

#define GAMMA 1.8

static inline float from_linear (float x)
{
  if (x >= 0.01f && x < 0.25f)
  {
    double u = -7.0287082190390287e+7;
    u = u * x + 9.6393346352028194e+7;
    u = u * x + -5.734540040993472e+7;
    u = u * x + 1.9423130902481005e+7;
    u = u * x + -4.1360185772523716e+6;
    u = u * x + 5.7798684366021459e+5;
    u = u * x + -5.3914765738125787e+4;
    u = u * x + 3.3827381495697474e+3;
    u = u * x + -1.4758049734050082e+2;
    u = u * x + 6.34823684277896;
    return u * x + 2.5853366952641552e-2;
  } else if (x >= 0.25f && x < 1.1f)
  {
    double u = -1.0514013917303294;
    u = u * x + 7.7742547018698687;
    u = u * x + -2.5688463052927626e+1;
    u = u * x + 5.009448068094152e+1;
    u = u * x + -6.4160579394623318e+1;
    u = u * x + 5.6890996491836047e+1;
    u = u * x + -3.5956430472666212e+1;
    u = u * x + 1.6565821666356617e+1;
    u = u * x + -5.8508167212560416;
    u = u * x + 2.2859969154731878;
    return u * x + 9.6140522367339399e-2;
  }
  return powf (x, 1.0f/1.8f);
}

static inline float to_linear (float x)
{
  if (x >= 0.01f && x < 0.7f)
  {
    double u = -1.326432065236105e+1;
    u = u * x + 7.7192973347868776e+1;
    u = u * x + -1.9639662782311719e+2;
    u = u * x + 2.8719828602066411e+2;
    u = u * x + -2.6718118019754855e+2;
    u = u * x + 1.6562450069335532e+2;
    u = u * x + -6.9988172743274441e+1;
    u = u * x + 2.0568254985551865e+1;
    u = u * x + -4.5302829214271245;
    u = u * x + 1.7636048338730889;
    u = u * x + 1.3015451332543148e-2;
    return u * x + -5.4445726922508747e-5;
  }
  else if (x >= 0.7f && x < 1.4f)
  {
    double u = 2.4212422421184617e-3;
    u = u * x + -2.0853930731707795e-2;
    u = u * x + 8.2416801461966525e-2;
    u = u * x + -2.1755799369117727e-1;
    u = u * x + 1.0503926510667593;
    u = u * x + 1.1196374095271941e-1;
    return u * x + -8.7825075945914206e-3;
  }
  return powf (x, 1.8);
}
#endif

#if 1

#define GAMMA  2.2
#define X0     (  0.5f / 255.0f)
#define X1     (254.5f / 255.0f)
#define DEGREE 6
#define SCALE  2

static inline float 
from_linear (float x)
{
  if (x >= X0 && x <= X1)
  {
    BablPolynomial poly;

    babl_polynomial_approximate_gamma (&poly,
                                       1.0 / GAMMA, X0, X1, DEGREE, SCALE);

    return babl_polynomial_eval (&poly, x);
  }
  return powf (x, 1.0f/GAMMA);
}

static inline float 
to_linear (float x)
{
  if (x >= X0 && x <= X1)
  {
    BablPolynomial poly;

    babl_polynomial_approximate_gamma (&poly,
                                       GAMMA, X0, X1, DEGREE, SCALE);

    return babl_polynomial_eval (&poly, x);
  }
  return powf (x, GAMMA);
}
#endif

static inline float 
from_linear_ref (float x)
{
  return powf (x, 1.0/GAMMA);
}

static inline float 
to_linear_ref (float x)
{
  return powf (x, GAMMA);
}

int 
main (void)
{
  int i;
  float max_diff = 0.0;
  int   max_diff_u8 = 0;
  int   u8_diff_count = 0;

  for (i = 0; i < 256; i++)
  {
     float val = i / 255.0;
     float from_ref = from_linear_ref (val);
     float to_ref   = to_linear_ref (val);
     float from     = from_linear (val);
     float to       = to_linear (val);
     int from_ref_u8 = from_ref * 255.5;
     int   to_ref_u8 =   to_ref * 255.5;
     int     from_u8 =     from * 255.5;
     int       to_u8 =       to * 255.5;
     float from_diff = fabs (from_ref - from);
     float to_diff      = fabs (to_ref - to);
     int   from_diff_u8 = abs (from_u8 -from_ref_u8);
     int   to_diff_u8   = abs (to_u8 -to_ref_u8);

     if (max_diff < from_diff) max_diff = from_diff;
     if (max_diff < to_diff)   max_diff = to_diff;

     if (from_diff_u8 || to_diff_u8)
     {
       u8_diff_count ++;
       if (from_diff_u8 > max_diff_u8) max_diff_u8 = from_diff_u8;
       if (to_diff_u8 > max_diff_u8) max_diff_u8 = to_diff_u8;
     }
  }
  fprintf (stderr, "diffs: %i max-u8-diff: %i: max-diff: %f(%f)\n", u8_diff_count, max_diff_u8, max_diff, max_diff * 256.0);
}


