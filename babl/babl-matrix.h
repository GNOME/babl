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


static inline void babl_matrix_mul_matrixf (const float *matA_,
                                            const float *matB_,
                                            float *out)
{
  int i, j;
  float matA[9];
  float matB[9];
  float t1, t2, t3;
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
  double a = v_in[0], b = v_in[1], c = v_in[2];
  double m_0_0 = m(mat, 0, 0);
  double m_0_1 = m(mat, 0, 1);
  double m_0_2 = m(mat, 0, 2);
  double m_1_0 = m(mat, 1, 0);
  double m_1_1 = m(mat, 1, 1);
  double m_1_2 = m(mat, 1, 2);
  double m_2_0 = m(mat, 2, 0);
  double m_2_1 = m(mat, 2, 1);
  double m_2_2 = m(mat, 2, 2);

  v_out[0] = m_0_0 * a + m_0_1 * b + m_0_2 * c;
  v_out[1] = m_1_0 * a + m_1_1 * b + m_1_2 * c;
  v_out[2] = m_2_0 * a + m_2_1 * b + m_2_2 * c;
}

static inline void babl_matrix_mul_vectorf (const double *mat, const float *v_in, float *v_out)
{
  float a = v_in[0], b = v_in[1], c = v_in[2];
  float m_0_0 = m(mat, 0, 0);
  float m_0_1 = m(mat, 0, 1);
  float m_0_2 = m(mat, 0, 2);
  float m_1_0 = m(mat, 1, 0);
  float m_1_1 = m(mat, 1, 1);
  float m_1_2 = m(mat, 1, 2);
  float m_2_0 = m(mat, 2, 0);
  float m_2_1 = m(mat, 2, 1);
  float m_2_2 = m(mat, 2, 2);

  v_out[0] = m_0_0 * a + m_0_1 * b + m_0_2 * c;
  v_out[1] = m_1_0 * a + m_1_1 * b + m_1_2 * c;
  v_out[2] = m_2_0 * a + m_2_1 * b + m_2_2 * c;
}

static inline void babl_matrix_mul_vectorff (const float *mat, const float *v_in, float *v_out)
{
  const float a = v_in[0], b = v_in[1], c = v_in[2];
  const float m_0_0 = m(mat, 0, 0);
  const float m_0_1 = m(mat, 0, 1);
  const float m_0_2 = m(mat, 0, 2);
  const float m_1_0 = m(mat, 1, 0);
  const float m_1_1 = m(mat, 1, 1);
  const float m_1_2 = m(mat, 1, 2);
  const float m_2_0 = m(mat, 2, 0);
  const float m_2_1 = m(mat, 2, 1);
  const float m_2_2 = m(mat, 2, 2);

  v_out[0] = m_0_0 * a + m_0_1 * b + m_0_2 * c;
  v_out[1] = m_1_0 * a + m_1_1 * b + m_1_2 * c;
  v_out[2] = m_2_0 * a + m_2_1 * b + m_2_2 * c;
}

static inline void babl_matrix_mul_vectorff_buf3 (const float *mat, const float *v_in, float *v_out,
                                                  int samples)
{
  int i;
  const float m_0_0 = m(mat, 0, 0);
  const float m_0_1 = m(mat, 0, 1);
  const float m_0_2 = m(mat, 0, 2);
  const float m_1_0 = m(mat, 1, 0);
  const float m_1_1 = m(mat, 1, 1);
  const float m_1_2 = m(mat, 1, 2);
  const float m_2_0 = m(mat, 2, 0);
  const float m_2_1 = m(mat, 2, 1);
  const float m_2_2 = m(mat, 2, 2);
  for (i = 0; i < samples; i ++)
  {
    const float a = v_in[0], b = v_in[1], c = v_in[2];

    v_out[0] = m_0_0 * a + m_0_1 * b + m_0_2 * c;
    v_out[1] = m_1_0 * a + m_1_1 * b + m_1_2 * c;
    v_out[2] = m_2_0 * a + m_2_1 * b + m_2_2 * c;
    v_in  += 3;
    v_out += 3;
  }
}

static inline void babl_matrix_mul_vectorff_buf4 (const float *mat, const float *v_in, float *v_out,
                                                  int samples)
{
  const float m_0_0 = m(mat, 0, 0);
  const float m_0_1 = m(mat, 0, 1);
  const float m_0_2 = m(mat, 0, 2);
  const float m_1_0 = m(mat, 1, 0);
  const float m_1_1 = m(mat, 1, 1);
  const float m_1_2 = m(mat, 1, 2);
  const float m_2_0 = m(mat, 2, 0);
  const float m_2_1 = m(mat, 2, 1);
  const float m_2_2 = m(mat, 2, 2);
  int i;
  for (i = 0; i < samples; i ++)
  {
    float a = v_in[0], b = v_in[1], c = v_in[2];

    v_out[0] = m_0_0 * a + m_0_1 * b + m_0_2 * c;
    v_out[1] = m_1_0 * a + m_1_1 * b + m_1_2 * c;
    v_out[2] = m_2_0 * a + m_2_1 * b + m_2_2 * c;
    v_out[3] = v_in[3];
    v_in  += 4;
    v_out += 4;
  }
}

static inline void babl_matrix_mul_vector_buf4 (const double *mat, const double *v_in, double *v_out,
                                                int samples)
{
  int i;
  const double m_0_0 = m(mat, 0, 0);
  const double m_0_1 = m(mat, 0, 1);
  const double m_0_2 = m(mat, 0, 2);
  const double m_1_0 = m(mat, 1, 0);
  const double m_1_1 = m(mat, 1, 1);
  const double m_1_2 = m(mat, 1, 2);
  const double m_2_0 = m(mat, 2, 0);
  const double m_2_1 = m(mat, 2, 1);
  const double m_2_2 = m(mat, 2, 2);
  for (i = 0; i < samples; i ++)
  {
    const double a = v_in[0], b = v_in[1], c = v_in[2];

    v_out[0] = m_0_0 * a + m_0_1 * b + m_0_2 * c;
    v_out[1] = m_1_0 * a + m_1_1 * b + m_1_2 * c;
    v_out[2] = m_2_0 * a + m_2_1 * b + m_2_2 * c;
    v_out[3] = v_in[3];
    v_in  += 4;
    v_out += 4;
  }
}


#undef m
#endif
