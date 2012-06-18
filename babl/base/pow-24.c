/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2012, Red Hat, Inc.
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
 * <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <math.h>
#include "util.h"

/**
 * This implements pow(x, 2.4) and pow(x, 1/2.4) using a chebyshev based polynominal approximation.
 * This is based on the approach in:
 * http://stackoverflow.com/questions/6475373/optimizations-for-pow-with-const-non-integer-exponent/6478839#6478839
 *
 * The code includes checbychev constants for the 9th order, which seems to give a max error
 * of about 4e-09, but unless you define USE_MAX_POW24_ACCURACY the order has been limited to
 * give an error of about 1e-7 (i.e. single precission floats).
 */
 
/* Chebychev polynomial terms for x^(5/12) expanded around x=1.5
 * Non-zero terms calculated via
 * NIntegrate[(2/Pi)*ChebyshevT[N,u]/Sqrt [1-u^2]*((u+3)/2)^(5/12),{u,-1,1}, PrecisionGoal->20, WorkingPrecision -> 100]
 * Zeroth term is similar except it uses 1/pi rather than 2/pi.
 */
static const double Cn[] = { 
  1.1758200232996901923,
  0.16665763094889061230,
  -0.0083154894939042125035,
  0.00075187976780420279038,
  -0.000083240178519391795367,
  0.000010229209410070008679,
  -1.3401001466409860246e-6,
  1.8333422241635376682e-7,
  -2.5878596761348859722e-8
};


/* Returns x^(/12) for x in [1,2) */
static double pow512norm (double x)
{
   double Tn[9];
   double u;

   u = 2.0*x - 3.0;
   Tn[0] = 1.0;
   Tn[1] = u;
   Tn[2] = 2*u*Tn[2-1] - Tn[2-2];
   Tn[3] = 2*u*Tn[3-1] - Tn[3-2];
   Tn[4] = 2*u*Tn[4-1] - Tn[4-2];
   Tn[5] = 2*u*Tn[5-1] - Tn[5-2];
   Tn[6] = 2*u*Tn[6-1] - Tn[6-2];
#ifdef USE_MAX_POW24_ACCURACY
   Tn[7] = 2*u*Tn[7-1] - Tn[7-2];
   Tn[8] = 2*u*Tn[8-1] - Tn[8-2];
#endif

   return Cn[0]*Tn[0] + Cn[1]*Tn[1] + Cn[2]*Tn[2] + Cn[3]*Tn[3] + Cn[4]*Tn[4] + Cn[5]*Tn[5] + Cn[6]*Tn[6]
#ifdef USE_MAX_POW24_ACCURACY
     + Cn[7]*Tn[7] + Cn[8]*Tn[8]
#endif
     ;
}

/* Precalculated (2^N) ^ (5 / 12) */
static const double pow2_512[12] = {
  1.0,
  1.3348398541700343678,
  1.7817974362806785482,
  2.3784142300054420538,
  3.1748021039363991669,
  4.2378523774371812394,
  5.6568542494923805819,
  7.5509945014535482244,
  1.0079368399158985525e1,
  1.3454342644059433809e1,
  1.7959392772949968275e1,
  2.3972913230026907883e1
};


/* Returns x^(1/2.4) == x^(5/12) */
double babl_pow_1_24 (double x)
{
   double s;
   int iexp;
   div_t qr;

   s = frexp (x, &iexp);
   s *= 2.0;
   iexp -= 1;

   qr = div (iexp, 12);
   if (qr.rem < 0) {
      qr.quot -= 1;
      qr.rem += 12;
   }

   return ldexp (pow512norm (s) * pow2_512[qr.rem], 5 * qr.quot);
}

/* Chebychev polynomial terms for x^(7/5) expanded around x=1.5
 * Non-zero terms calculated via
 * NIntegrate[(2/Pi)*ChebyshevT[N,u]/Sqrt [1-u^2]*((u+3)/2)^(7/5),{u,-1,1}, PrecisionGoal->20, WorkingPrecision -> 100]
 * Zeroth term is similar except it uses 1/pi rather than 2/pi.
 */
static const double iCn[] = {
  1.7917488588043277509,
  0.82045614371976854984,
  0.027694100686325412819,
  -0.00094244335181762134018,
  0.000064355540911469709545,
  -5.7224404636060757485e-6,
  5.8767669437311184313e-7,
  -6.6139920053589721168e-8,
  7.9323242696227458163e-9
};

/* Returns x^(7/5) for x in [1,2) */
static double pow75norm (double x)
{
   double Tn[9];
   double u;

   u = 2.0*x - 3.0;
   Tn[0] = 1.0;
   Tn[1] = u;
   Tn[2] = 2*u*Tn[2-1] - Tn[2-2];
   Tn[3] = 2*u*Tn[3-1] - Tn[3-2];
   Tn[4] = 2*u*Tn[4-1] - Tn[4-2];
   Tn[5] = 2*u*Tn[5-1] - Tn[5-2];
#ifdef USE_MAX_POW24_ACCURACY
   Tn[6] = 2*u*Tn[6-1] - Tn[6-2];
   Tn[7] = 2*u*Tn[7-1] - Tn[7-2];
   Tn[8] = 2*u*Tn[8-1] - Tn[8-2];
#endif

   return iCn[0]*Tn[0] + iCn[1]*Tn[1] + iCn[2]*Tn[2] + iCn[3]*Tn[3] + iCn[4]*Tn[4] + iCn[5]*Tn[5]
#ifdef USE_MAX_POW24_ACCURACY
     + iCn[6]*Tn[6] + iCn[7]*Tn[7] + iCn[8]*Tn[8]
#endif
     ;
}

/* Precalculated (2^N) ^ (7 / 5) */
static const double pow2_75[5] = {
  1.0,
  2.6390158215457883983,
  6.9644045063689921093,
  1.8379173679952558018e+1,
  4.8502930128332728543e+1
};

/* Returns x^2.4 == x * x ^1.4 == x * x^(7/5) */
double babl_pow_24 (double x)
{
   double s;
   int iexp;
   div_t qr;

   s = frexp (x, &iexp);
   s *= 2.0;
   iexp -= 1;

   qr = div (iexp, 5);
   if (qr.rem < 0) {
      qr.quot -= 1;
      qr.rem += 5;
   }

   return x * ldexp (pow75norm (s) * pow2_75[qr.rem], 7 * qr.quot);
}

