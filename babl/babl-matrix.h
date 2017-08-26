#ifndef _BABL_MATRIX_H_
#define _BABL_MATRIX_H_

#include <stdio.h>

#define m(matr, j, i)  matr[j*3+i]

static inline void babl_matrix_mul_matrix (const double *matA_,
                                           const double *matB_,
                                           double *out)
{
  int i, j;
  double matA[9];
  double matB[9];
  double t1, t2, t3;
  memcpy (matA, matA_, sizeof (matA));
  memcpy (matB, matB_, sizeof (matB));

  for (i = 0; i < 3; i++)
  {
    t1 = m(matA, i, 0);
    t2 = m(matA, i, 1);
    t3 = m(matA, i, 2);

    for (j = 0; j < 3; j ++)
    {
      m(out,i,j) = t1 * m(matB, 0, j);
      m(out,i,j) += t2 * m(matB, 1, j);
      m(out,i,j) += t3 * m(matB, 2, j);
    }
  }
}

static inline void babl_matrix_to_float (const double *in, float *out)
{
  int i;
  for (i = 0; i < 9; i ++)
    out[i] = in[i];
}

static inline void babl_matrix_invert (const double *in, double *out)
{
  double mat[9];
  double det, invdet;
  memcpy (mat, in, sizeof (mat));
  det = m(mat, 0, 0) * (m(mat, 1, 1) *m(mat, 2, 2) - m(mat, 2, 1)*m(mat, 1, 2)) -
        m(mat, 0, 1) * (m(mat, 1, 0) *m(mat, 2, 2) - m(mat, 1, 2)*m(mat, 2, 0)) +
        m(mat, 0, 2) * (m(mat, 1, 0) *m(mat, 2, 1) - m(mat, 1, 1)*m(mat, 2, 0));
  invdet = 1.0 / det;
  m(out, 0, 0) = (m(mat, 1, 1) * m(mat, 2, 2) - m(mat, 2, 1) * m(mat, 1, 2)) * invdet;
  m(out, 0, 1) = (m(mat, 0, 2) * m(mat, 2, 1) - m(mat, 0, 1) * m(mat, 2, 2)) * invdet;
  m(out, 0, 2) = (m(mat, 0, 1) * m(mat, 1, 2) - m(mat, 0, 2) * m(mat, 1, 1)) * invdet;
  m(out, 1, 0) = (m(mat, 1, 2) * m(mat, 2, 0) - m(mat, 1, 0) * m(mat, 2, 2)) * invdet;
  m(out, 1, 1) = (m(mat, 0, 0) * m(mat, 2, 2) - m(mat, 0, 2) * m(mat, 2, 0)) * invdet;
  m(out, 1, 2) = (m(mat, 1, 0) * m(mat, 0, 2) - m(mat, 0, 0) * m(mat, 1, 2)) * invdet;
  m(out, 2, 0) = (m(mat, 1, 0) * m(mat, 2, 1) - m(mat, 2, 0) * m(mat, 1, 1)) * invdet;
  m(out, 2, 1) = (m(mat, 2, 0) * m(mat, 0, 1) - m(mat, 0, 0) * m(mat, 2, 1)) * invdet;
  m(out, 2, 2) = (m(mat, 0, 0) * m(mat, 1, 1) - m(mat, 1, 0) * m(mat, 0, 1)) * invdet;
}


static inline void babl_matrix_mul_vector (const double *mat, const double *v_in, double *v_out)
{
  double val[3]={v_in[0], v_in[1], v_in[2]};

  v_out[0] = m(mat, 0, 0) * val[0] + m(mat, 0, 1) * val[1] + m(mat, 0, 2) * val[2];
  v_out[1] = m(mat, 1, 0) * val[0] + m(mat, 1, 1) * val[1] + m(mat, 1, 2) * val[2];
  v_out[2] = m(mat, 2, 0) * val[0] + m(mat, 2, 1) * val[1] + m(mat, 2, 2) * val[2];
}

static inline void babl_matrix_mul_vectorf (const double *mat, const float *v_in, float *v_out)
{
  float val[3]={v_in[0], v_in[1], v_in[2]};

  v_out[0] = m(mat, 0, 0) * val[0] + m(mat, 0, 1) * val[1] + m(mat, 0, 2) * val[2];
  v_out[1] = m(mat, 1, 0) * val[0] + m(mat, 1, 1) * val[1] + m(mat, 1, 2) * val[2];
  v_out[2] = m(mat, 2, 0) * val[0] + m(mat, 2, 1) * val[1] + m(mat, 2, 2) * val[2];
}

static inline void babl_matrix_mul_vectorff (const float *mat, const float *v_in, float *v_out)
{
  float val[3]={v_in[0], v_in[1], v_in[2]};

  v_out[0] = m(mat, 0, 0) * val[0] + m(mat, 0, 1) * val[1] + m(mat, 0, 2) * val[2];
  v_out[1] = m(mat, 1, 0) * val[0] + m(mat, 1, 1) * val[1] + m(mat, 1, 2) * val[2];
  v_out[2] = m(mat, 2, 0) * val[0] + m(mat, 2, 1) * val[1] + m(mat, 2, 2) * val[2];
}


#undef m
#endif
