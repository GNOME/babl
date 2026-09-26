/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2017, Øyvind Kolås and others.
 *
 * babl-polynomial.h
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

#ifndef _BABL_POLYNOMIAL_H
#define _BABL_POLYNOMIAL_H


/* BablPolynomial is an opaque type representing a real polynomial of a real
 * variable.  In addition to a degree, polynomials have an associated *scale*,
 * which divides the exponents of the polynomial's terms.  For example, a
 * polynomial of degree 3 and of scale 1 has the form
 * `c0*x^0 + c1*x^1 + c2*x^2 + c3*x^3`, while a polynomial of degree 3 and of
 * scale 2 has the form `c0*x^0 + c1*x^0.5 + c2*x^1 + c3*x^1.5`.
 */


#define BABL_POLYNOMIAL_MIN_DEGREE  0
#define BABL_POLYNOMIAL_MAX_DEGREE 10

#define BABL_POLYNOMIAL_MIN_SCALE   1
#define BABL_POLYNOMIAL_MAX_SCALE   2


typedef struct BablPolynomial BablPolynomial;

typedef double (* BablPolynomialEvalFunc) (const BablPolynomial *poly,
                                           double                x);


struct BablPolynomial
{
  BablPolynomialEvalFunc eval;
  int                    degree;
  int                    scale;
  double                 coeff[BABL_POLYNOMIAL_MAX_DEGREE + 1];
};


/* evaluates `poly` at `x`. */
static inline double
babl_polynomial_eval (const BablPolynomial *poly,
                      double                x)
{
  return poly->eval (poly, x);
}


/* calculates the polynomial of maximal degree `degree` and of scale `scale`,
 * that minimizes the total error w.r.t. the gamma function `gamma`, over the
 * range `[x0, x1]`.
 */
void
babl_polynomial_approximate_gamma (BablPolynomial *poly,
                                   double          gamma,
                                   double          x0,
                                   double          x1,
                                   int             degree,
                                   int             scale);


#endif
