/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2017, Øyvind Kolås and others.
 *
 * babl-polynomial.c
 * Copyright (C) 2017 Ell
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
 * <https://www.gnu.org/licenses/>.
 */

#ifdef BABL_POLYNOMIAL_DEGREE

BABL_POLYNOMIAL_DEGREE ( 0, __)
BABL_POLYNOMIAL_DEGREE ( 1,  0)
BABL_POLYNOMIAL_DEGREE ( 2,  1)
BABL_POLYNOMIAL_DEGREE ( 3,  2)
BABL_POLYNOMIAL_DEGREE ( 4,  3)
BABL_POLYNOMIAL_DEGREE ( 5,  4)
BABL_POLYNOMIAL_DEGREE ( 6,  5)
BABL_POLYNOMIAL_DEGREE ( 7,  6)
BABL_POLYNOMIAL_DEGREE ( 8,  7)
BABL_POLYNOMIAL_DEGREE ( 9,  8)
BABL_POLYNOMIAL_DEGREE (10,  9)
BABL_POLYNOMIAL_DEGREE (11, 10)
BABL_POLYNOMIAL_DEGREE (12, 11)
BABL_POLYNOMIAL_DEGREE (13, 12)
BABL_POLYNOMIAL_DEGREE (14, 13)
BABL_POLYNOMIAL_DEGREE (15, 14)
BABL_POLYNOMIAL_DEGREE (16, 15)
BABL_POLYNOMIAL_DEGREE (17, 16)
BABL_POLYNOMIAL_DEGREE (18, 17)
BABL_POLYNOMIAL_DEGREE (19, 18)
BABL_POLYNOMIAL_DEGREE (20, 19)
BABL_POLYNOMIAL_DEGREE (21, 20)
BABL_POLYNOMIAL_DEGREE (22, 21)

#undef BABL_POLYNOMIAL_DEGREE

#else

#include "config.h"
#include <string.h>
#include <math.h>
#include "babl-internal.h"


#define BABL_BIG_POLYNOMIAL_MAX_DEGREE (2 * BABL_POLYNOMIAL_MAX_DEGREE + BABL_POLYNOMIAL_MAX_SCALE)
#define EPSILON                        1e-10


typedef struct
{
  BablPolynomialEvalFunc eval;
  int                    degree;
  int                    scale;
  double                 coeff[BABL_BIG_POLYNOMIAL_MAX_DEGREE + 1];
} BablBigPolynomial;


#define BABL_POLYNOMIAL_EVAL___(poly, x) 0.0
#define BABL_POLYNOMIAL_EVAL_0(poly, x)  (                                          (poly)->coeff[0])
#define BABL_POLYNOMIAL_EVAL_1(poly, x)  (                                          (poly)->coeff[1])
#define BABL_POLYNOMIAL_EVAL_2(poly, x)  (BABL_POLYNOMIAL_EVAL_0  (poly, x) * (x) + (poly)->coeff[2])
#define BABL_POLYNOMIAL_EVAL_3(poly, x)  (BABL_POLYNOMIAL_EVAL_1  (poly, x) * (x) + (poly)->coeff[3])
#define BABL_POLYNOMIAL_EVAL_4(poly, x)  (BABL_POLYNOMIAL_EVAL_2  (poly, x) * (x) + (poly)->coeff[4])
#define BABL_POLYNOMIAL_EVAL_5(poly, x)  (BABL_POLYNOMIAL_EVAL_3  (poly, x) * (x) + (poly)->coeff[5])
#define BABL_POLYNOMIAL_EVAL_6(poly, x)  (BABL_POLYNOMIAL_EVAL_4  (poly, x) * (x) + (poly)->coeff[6])
#define BABL_POLYNOMIAL_EVAL_7(poly, x)  (BABL_POLYNOMIAL_EVAL_5  (poly, x) * (x) + (poly)->coeff[7])
#define BABL_POLYNOMIAL_EVAL_8(poly, x)  (BABL_POLYNOMIAL_EVAL_6  (poly, x) * (x) + (poly)->coeff[8])
#define BABL_POLYNOMIAL_EVAL_9(poly, x)  (BABL_POLYNOMIAL_EVAL_7  (poly, x) * (x) + (poly)->coeff[9])
#define BABL_POLYNOMIAL_EVAL_10(poly, x) (BABL_POLYNOMIAL_EVAL_8  (poly, x) * (x) + (poly)->coeff[10])
#define BABL_POLYNOMIAL_EVAL_11(poly, x) (BABL_POLYNOMIAL_EVAL_9  (poly, x) * (x) + (poly)->coeff[11])
#define BABL_POLYNOMIAL_EVAL_12(poly, x) (BABL_POLYNOMIAL_EVAL_10 (poly, x) * (x) + (poly)->coeff[12])
#define BABL_POLYNOMIAL_EVAL_13(poly, x) (BABL_POLYNOMIAL_EVAL_11 (poly, x) * (x) + (poly)->coeff[13])
#define BABL_POLYNOMIAL_EVAL_14(poly, x) (BABL_POLYNOMIAL_EVAL_12 (poly, x) * (x) + (poly)->coeff[14])
#define BABL_POLYNOMIAL_EVAL_15(poly, x) (BABL_POLYNOMIAL_EVAL_13 (poly, x) * (x) + (poly)->coeff[15])
#define BABL_POLYNOMIAL_EVAL_16(poly, x) (BABL_POLYNOMIAL_EVAL_14 (poly, x) * (x) + (poly)->coeff[16])
#define BABL_POLYNOMIAL_EVAL_17(poly, x) (BABL_POLYNOMIAL_EVAL_15 (poly, x) * (x) + (poly)->coeff[17])
#define BABL_POLYNOMIAL_EVAL_18(poly, x) (BABL_POLYNOMIAL_EVAL_16 (poly, x) * (x) + (poly)->coeff[18])
#define BABL_POLYNOMIAL_EVAL_19(poly, x) (BABL_POLYNOMIAL_EVAL_17 (poly, x) * (x) + (poly)->coeff[19])
#define BABL_POLYNOMIAL_EVAL_20(poly, x) (BABL_POLYNOMIAL_EVAL_18 (poly, x) * (x) + (poly)->coeff[20])
#define BABL_POLYNOMIAL_EVAL_21(poly, x) (BABL_POLYNOMIAL_EVAL_19 (poly, x) * (x) + (poly)->coeff[21])
#define BABL_POLYNOMIAL_EVAL_22(poly, x) (BABL_POLYNOMIAL_EVAL_20 (poly, x) * (x) + (poly)->coeff[22])

#define BABL_POLYNOMIAL_DEGREE(i, i_1)                                         \
  static double                                                                \
  babl_polynomial_eval_1_##i (const BablPolynomial *poly,                      \
                              double                x)                         \
  {                                                                            \
    /* quiet clang warnings */                                                 \
    const BablBigPolynomial *const big_poly = (const BablBigPolynomial *) poly;\
                                                                               \
    const double x2 = x * x;                                                   \
    (void) x2;                                                                 \
                                                                               \
    return BABL_POLYNOMIAL_EVAL_##i   (big_poly, x2) +                         \
           BABL_POLYNOMIAL_EVAL_##i_1 (big_poly, x2) * x;                      \
  }
#include "babl-polynomial.c"

#define BABL_POLYNOMIAL_DEGREE(i, i_1)                                         \
  static double                                                                \
  babl_polynomial_eval_2_##i (const BablPolynomial *poly,                      \
                              double                x)                         \
  {                                                                            \
    /* quiet clang warnings */                                                 \
    const BablBigPolynomial *const big_poly = (const BablBigPolynomial *) poly;\
                                                                               \
    return BABL_POLYNOMIAL_EVAL_##i   (big_poly, x) +                          \
           BABL_POLYNOMIAL_EVAL_##i_1 (big_poly, x) * sqrt (x);                \
  }
#include "babl-polynomial.c"

static const BablPolynomialEvalFunc babl_polynomial_eval_funcs[BABL_POLYNOMIAL_MAX_SCALE]
                                                              [BABL_BIG_POLYNOMIAL_MAX_DEGREE + 1] =
{
  {
    #define BABL_POLYNOMIAL_DEGREE(i, i_1) babl_polynomial_eval_1_##i,
    #include "babl-polynomial.c"
  },
  {
    #define BABL_POLYNOMIAL_DEGREE(i, i_1) babl_polynomial_eval_2_##i,
    #include "babl-polynomial.c"
  }
};


static void
babl_polynomial_set_degree (BablPolynomial *poly,
                            int             degree,
                            int             scale)
{
  babl_assert (degree >= BABL_POLYNOMIAL_MIN_DEGREE &&
               degree <= BABL_BIG_POLYNOMIAL_MAX_DEGREE);
  babl_assert (scale >= BABL_POLYNOMIAL_MIN_SCALE &&
               scale <= BABL_POLYNOMIAL_MAX_SCALE);

  poly->eval   = babl_polynomial_eval_funcs[scale - 1][degree];
  poly->degree = degree;
  poly->scale  = scale;
}

static double
babl_polynomial_get (const BablPolynomial *poly,
                     int                   i)
{
  return poly->coeff[poly->degree - i];
}

static void
babl_polynomial_set (BablPolynomial *poly,
                     int             i,
                     double          c)
{
  poly->coeff[poly->degree - i] = c;
}

static void
babl_polynomial_copy (BablPolynomial       *poly,
                      const BablPolynomial *rpoly)
{
  poly->eval   = rpoly->eval;
  poly->degree = rpoly->degree;
  poly->scale  = rpoly->scale;
  memcpy (poly->coeff, rpoly->coeff, (rpoly->degree + 1) * sizeof (double));
}

static void
babl_polynomial_reset (BablPolynomial *poly,
                       int             scale)
{
  babl_polynomial_set_degree (poly, 0, scale);
  babl_polynomial_set (poly, 0, 0.0);
}

static void
babl_polynomial_shrink (BablPolynomial *poly)
{
  int i;

  for (i = 0; i <= poly->degree; i++)
    {
      if (fabs (poly->coeff[i]) > EPSILON)
        break;
    }

  if (i == poly->degree + 1)
    {
      babl_polynomial_reset (poly, poly->scale);
    }
  else if (i > 0)
    {
      memmove (poly->coeff, &poly->coeff[i],
               (poly->degree - i + 1) * sizeof (double));

      babl_polynomial_set_degree (poly, poly->degree - i, poly->scale);
    }
}

static void
babl_polynomial_add (BablPolynomial       *poly,
                     const BablPolynomial *rpoly)
{
  int i;

  babl_assert (poly->scale == rpoly->scale);

  if (poly->degree >= rpoly->degree)
    {
      for (i = 0; i <= rpoly->degree; i++)
        {
          babl_polynomial_set (poly, i, babl_polynomial_get (poly, i) +
                                        babl_polynomial_get (rpoly, i));
        }
    }
  else
    {
      int orig_degree = poly->degree;

      babl_polynomial_set_degree (poly, rpoly->degree, poly->scale);

      for (i = 0; i <= orig_degree; i++)
        {
          babl_polynomial_set (poly, i, poly->coeff[orig_degree - i] +
                                        babl_polynomial_get (rpoly, i));
        }

      for (; i <= rpoly->degree; i++)
        babl_polynomial_set (poly, i, babl_polynomial_get (rpoly, i));
    }
}

static void
babl_polynomial_sub (BablPolynomial       *poly,
                     const BablPolynomial *rpoly)
{
  int i;

  babl_assert (poly->scale == rpoly->scale);

  if (poly->degree >= rpoly->degree)
    {
      for (i = 0; i <= rpoly->degree; i++)
        {
          babl_polynomial_set (poly, i, babl_polynomial_get (poly, i) -
                                        babl_polynomial_get (rpoly, i));
        }
    }
  else
    {
      int orig_degree = poly->degree;

      babl_polynomial_set_degree (poly, rpoly->degree, poly->scale);

      for (i = 0; i <= orig_degree; i++)
        {
          babl_polynomial_set (poly, i, poly->coeff[orig_degree - i] -
                                        babl_polynomial_get (rpoly, i));
        }

      for (; i <= rpoly->degree; i++)
        babl_polynomial_set (poly, i, -babl_polynomial_get (rpoly, i));
    }
}

static void
babl_polynomial_scalar_mul (BablPolynomial *poly,
                            double          a)
{
  int i;

  for (i = 0; i <= poly->degree; i++)
    poly->coeff[i] *= a;
}

static void
babl_polynomial_scalar_div (BablPolynomial *poly,
                            double          a)
{
  int i;

  for (i = 0; i <= poly->degree; i++)
    poly->coeff[i] /= a;
}

static void
babl_polynomial_mul_copy (BablPolynomial       *poly,
                          const BablPolynomial *poly1,
                          const BablPolynomial *poly2)
{
  int i;
  int j;

  babl_assert (poly1->scale == poly2->scale);

  babl_polynomial_set_degree (poly, poly1->degree + poly2->degree, poly1->scale);

  memset (poly->coeff, 0, (poly->degree + 1) * sizeof (double));

  for (i = poly1->degree; i >= 0; i--)
    {
      for (j = poly2->degree; j >= 0; j--)
        {
          babl_polynomial_set (poly, i + j, babl_polynomial_get (poly, i + j) +
                                            babl_polynomial_get (poly1, i)    *
                                            babl_polynomial_get (poly2, j));
        }
    }
}

static void
babl_polynomial_integrate (BablPolynomial *poly)
{
  int i;

  babl_polynomial_set_degree (poly, poly->degree + poly->scale, poly->scale);

  for (i = 0; i <= poly->degree - poly->scale; i++)
    {
      poly->coeff[i] *= poly->scale;
      poly->coeff[i] /= poly->degree - i;
    }

  for (; i <= poly->degree; i++)
    poly->coeff[i] = 0.0;
}

static void
babl_polynomial_gamma_integrate (BablPolynomial *poly,
                                 double          gamma)
{
  int i;

  babl_polynomial_set_degree (poly, poly->degree + poly->scale, poly->scale);

  gamma *= poly->scale;

  for (i = 0; i <= poly->degree - poly->scale; i++)
    {
      poly->coeff[i] *= poly->scale;
      poly->coeff[i] /= poly->degree - i + gamma;
    }

  for (; i <= poly->degree; i++)
    poly->coeff[i] = 0.0;
}

static double
babl_polynomial_inner_product (const BablPolynomial *poly1,
                               const BablPolynomial *poly2,
                               double                x0,
                               double                x1)
{
  BablBigPolynomial temp;

  babl_polynomial_mul_copy ((BablPolynomial *) &temp, poly1, poly2);
  babl_polynomial_integrate ((BablPolynomial *) &temp);

  return babl_polynomial_eval ((BablPolynomial *) &temp, x1) -
         babl_polynomial_eval ((BablPolynomial *) &temp, x0);
}

static double
babl_polynomial_gamma_inner_product (const BablPolynomial *poly,
                                     double                gamma,
                                     double                x0,
                                     double                x1)
{
  BablBigPolynomial temp;

  babl_polynomial_copy ((BablPolynomial *) &temp, poly);
  babl_polynomial_gamma_integrate ((BablPolynomial *) &temp, gamma);

  return babl_polynomial_eval ((BablPolynomial *) &temp, x1) * pow (x1, gamma) -
         babl_polynomial_eval ((BablPolynomial *) &temp, x0) * pow (x0, gamma);
}

static double
babl_polynomial_norm (const BablPolynomial *poly,
                      double                x0,
                      double                x1)
{
  return sqrt (babl_polynomial_inner_product (poly, poly, x0, x1));
}

static void
babl_polynomial_normalize (BablPolynomial *poly,
                           double          x0,
                           double          x1)
{
  double norm;

  norm = babl_polynomial_norm (poly, x0, x1);

  if (norm > EPSILON)
    babl_polynomial_scalar_div (poly, norm);
}

static void
babl_polynomial_project_copy (BablPolynomial       *poly,
                              const BablPolynomial *rpoly,
                              const BablPolynomial *basis,
                              int                   basis_n,
                              double                x0,
                              double                x1)
{
  int i;

  babl_assert (basis_n > 0);

  babl_polynomial_reset (poly, basis[0].scale);

  for (i = 0; i < basis_n; i++)
    {
      BablPolynomial temp;

      babl_polynomial_copy (&temp, &basis[i]);
      babl_polynomial_scalar_mul (&temp,
                                  babl_polynomial_inner_product (&temp, rpoly,
                                                                 x0, x1));
      babl_polynomial_add (poly, &temp);
    }
}

static void
babl_polynomial_gamma_project_copy (BablPolynomial       *poly,
                                    double                gamma,
                                    const BablPolynomial *basis,
                                    int                   basis_n,
                                    double                x0,
                                    double                x1)
{
  int i;

  babl_assert (basis_n > 0);

  babl_polynomial_reset (poly, basis[0].scale);

  for (i = 0; i < basis_n; i++)
    {
      BablPolynomial temp;

      babl_polynomial_copy (&temp, &basis[i]);
      babl_polynomial_scalar_mul (&temp,
                                  babl_polynomial_gamma_inner_product (&temp,
                                                                       gamma,
                                                                       x0, x1));
      babl_polynomial_add (poly, &temp);
    }
}

static void
babl_polynomial_gram_schmidt (BablPolynomial *basis,
                              int             basis_n,
                              double          x0,
                              double          x1)
{
  int i;

  for (i = 0; i < basis_n; i++)
    {
      if (i > 0)
        {
          BablPolynomial temp;

          babl_polynomial_project_copy (&temp, &basis[i], basis, i, x0, x1);
          babl_polynomial_sub (&basis[i], &temp);
        }

      babl_polynomial_normalize (&basis[i], x0, x1);
    }
}

static void
babl_polynomial_basis (BablPolynomial *basis,
                       int             basis_n,
                       int             scale)
{
  int i;

  for (i = 0; i < basis_n; i++)
    {
      babl_polynomial_set_degree (&basis[i], i, scale);

      basis[i].coeff[0] = 1.0;
      memset (&basis[i].coeff[1], 0, i * sizeof (double));
    }
}

static void
babl_polynomial_orthonormal_basis (BablPolynomial *basis,
                                   int             basis_n,
                                   double          x0,
                                   double          x1,
                                   int             scale)
{
  babl_polynomial_basis (basis, basis_n, scale);
  babl_polynomial_gram_schmidt (basis, basis_n, x0, x1);
}

void
babl_polynomial_approximate_gamma (BablPolynomial *poly,
                                   double          gamma,
                                   double          x0,
                                   double          x1,
                                   int             degree,
                                   int             scale)
{
  BablPolynomial *basis;

  babl_assert (poly != NULL);
  babl_assert (gamma >= 0.0);
  babl_assert (x0 >= 0.0);
  babl_assert (x0 < x1);
  babl_assert (degree >= BABL_POLYNOMIAL_MIN_DEGREE &&
               degree <= BABL_POLYNOMIAL_MAX_DEGREE);
  babl_assert (scale >= BABL_POLYNOMIAL_MIN_SCALE &&
               scale <= BABL_POLYNOMIAL_MAX_SCALE);

  basis = alloca ((degree + 1) * sizeof (BablPolynomial));

  babl_polynomial_orthonormal_basis (basis, degree + 1, x0, x1, scale);

  babl_polynomial_gamma_project_copy (poly, gamma, basis, degree + 1, x0, x1);
  babl_polynomial_shrink (poly);
}

#endif
