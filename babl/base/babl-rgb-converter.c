#include "config.h"
#include "babl-internal.h"
#include "base/util.h"
#include "babl-trc.h"
#include "babl-base.h"

static void
prep_conversion (const Babl *babl)
{
  Babl *conversion = (void*) babl;
  const Babl *source_space = babl_conversion_get_source_space (conversion);
  float *matrixf;
  unsigned int i;
  float *lut_red;
  float *lut_green;
  float *lut_blue;

  double matrix[9];
  babl_matrix_mul_matrix (
     (conversion->conversion.destination)->format.space->space.XYZtoRGB,
     (conversion->conversion.source)->format.space->space.RGBtoXYZ,
     matrix);

  matrixf = babl_calloc (sizeof (float), 9 + 256 * 3); // we leak this matrix , which is a singleton
  babl_matrix_to_float (matrix, matrixf);
  conversion->conversion.data = matrixf;

  lut_red = matrixf + 9;
  lut_green = lut_red + 256;
  lut_blue = lut_green + 256;
  for (i = 0; i < 256; i++)
  {
    lut_red[i] = babl_trc_to_linear (source_space->space.trc[0], i/255.0);
    lut_green[i] = babl_trc_to_linear (source_space->space.trc[1], i/255.0);
    lut_blue[i] = babl_trc_to_linear (source_space->space.trc[2], i/255.0);
  }
}

#define TRC_IN(rgba_in, rgba_out)  do{ int i;\
  for (i = 0; i < samples; i++) \
  { \
    rgba_out[i*4+3] = rgba_in[i*4+3]; \
  } \
  if ((source_space->space.trc[0] == source_space->space.trc[1]) && \
      (source_space->space.trc[1] == source_space->space.trc[2])) \
  { \
    const Babl *trc = (void*)source_space->space.trc[0]; \
    babl_trc_to_linear_buf(trc, rgba_in, rgba_out, 4, 4, 3, samples); \
  } \
  else \
  { \
    unsigned int c; \
    for (c = 0; c < 3; c ++) \
    { \
      const Babl *trc = (void*)source_space->space.trc[c]; \
      babl_trc_to_linear_buf(trc, rgba_in + c, rgba_out + c, 4, 4, 1, samples); \
    } \
  } \
}while(0)

#define TRC_OUT(rgba_in, rgba_out)  do{\
  { \
    if ((destination_space->space.trc[0] == destination_space->space.trc[1]) && \
        (destination_space->space.trc[1] == destination_space->space.trc[2])) \
    { \
      const Babl *trc = (void*)destination_space->space.trc[0]; \
      babl_trc_from_linear_buf(trc, rgba_in, rgba_out, 4, 4, 3, samples); \
    } \
    else \
    { \
      unsigned int c; \
      for (c = 0; c < 3; c ++) \
      { \
        const Babl *trc = (void*)destination_space->space.trc[c]; \
        babl_trc_from_linear_buf(trc, rgba_in + c, rgba_out + c, 4, 4, 1, samples); \
      } \
    } \
  }\
} while(0)


static inline void
universal_nonlinear_rgba_converter (const Babl    *conversion,
                                    unsigned char *__restrict__ src_char,
                                    unsigned char *__restrict__ dst_char,
                                    long           samples,
                                    void          *data)
{
  const Babl *source_space = babl_conversion_get_source_space (conversion);
  const Babl *destination_space = babl_conversion_get_destination_space (conversion);

  float * matrixf = data;
  float *rgba_in = (void*)src_char;
  float *rgba_out = (void*)dst_char;

  TRC_IN(rgba_in, rgba_out);

  babl_matrix_mul_vectorff_buf4 (matrixf, rgba_out, rgba_out, samples);

  TRC_OUT(rgba_out, rgba_out);
}

static inline void
universal_nonlinear_rgb_linear_converter (const Babl    *conversion,
                                          unsigned char *__restrict__ src_char,
                                          unsigned char *__restrict__ dst_char,
                                          long           samples,
                                          void          *data)
{
  const Babl *source_space = babl_conversion_get_source_space (conversion);
  float * matrixf = data;
  float *rgba_in = (void*)src_char;
  float *rgba_out = (void*)dst_char;

  TRC_IN(rgba_in, rgba_out);

  babl_matrix_mul_vectorff_buf4 (matrixf, rgba_out, rgba_out, samples);
}

static inline void
universal_linear_rgb_nonlinear_converter (const Babl    *conversion,
                                          unsigned char *__restrict__ src_char,
                                          unsigned char *__restrict__ dst_char,
                                          long           samples,
                                          void          *data)
{
  const Babl *destination_space = conversion->conversion.destination->format.space;
  float * matrixf = data;
  float *rgba_in = (void*)src_char;
  float *rgba_out = (void*)dst_char;

  babl_matrix_mul_vectorff_buf4 (matrixf, rgba_in, rgba_out, samples);

  TRC_OUT(rgba_out, rgba_out);
}

static inline void
universal_rgba_converter (const Babl    *conversion,
                          unsigned char *__restrict__ src_char,
                          unsigned char *__restrict__ dst_char,
                          long           samples,
                          void          *data)
{
  float *matrixf = data;
  float *rgba_in = (void*)src_char;
  float *rgba_out = (void*)dst_char;

  babl_matrix_mul_vectorff_buf4 (matrixf, rgba_in, rgba_out, samples);
}

static inline void
universal_rgb_converter (const Babl    *conversion,
                         unsigned char *__restrict__ src_char,
                         unsigned char *__restrict__ dst_char,
                         long           samples,
                         void          *data)
{
  float *matrixf = data;
  float *rgb_in = (void*)src_char;
  float *rgb_out = (void*)dst_char;

  babl_matrix_mul_vectorff_buf3 (matrixf, rgb_in, rgb_out, samples);
}


static inline void
universal_ya_converter (const Babl    *conversion,
                        unsigned char *__restrict__ src_char,
                        unsigned char *__restrict__ dst_char,
                        long           samples,
                        void          *data)
{
  memcpy (dst_char, src_char, samples * 4 * 2);
}

static inline void
universal_y_converter (const Babl    *conversion,
                       unsigned char *__restrict__ src_char,
                       unsigned char *__restrict__ dst_char,
                       long           samples,
                       void          *data)
{
  memcpy (dst_char, src_char, samples * 4);
}


static inline void
universal_nonlinear_rgb_u8_converter (const Babl    *conversion,
                                      unsigned char *__restrict__ src_char,
                                      unsigned char *__restrict__ dst_char,
                                      long           samples,
                                      void          *data)
{
  const Babl *destination_space = conversion->conversion.destination->format.space;

  float * matrixf = data;
  float * in_trc_lut_red = matrixf + 9;
  float * in_trc_lut_green = in_trc_lut_red + 256;
  float * in_trc_lut_blue = in_trc_lut_green + 256;
  unsigned int i;
  uint8_t *rgb_in_u8 = (void*)src_char;
  uint8_t *rgb_out_u8 = (void*)dst_char;

  float rgba_out[4*samples];

  for (i = 0; i < samples; i++)
  {
    rgba_out[i*4+0]=in_trc_lut_red[rgb_in_u8[i*3+0]];
    rgba_out[i*4+1]=in_trc_lut_green[rgb_in_u8[i*3+1]];
    rgba_out[i*4+2]=in_trc_lut_blue[rgb_in_u8[i*3+2]];
    rgba_out[i*4+3]=rgb_in_u8[i*3+2] * 255.0f;
  }

  babl_matrix_mul_vectorff_buf4 (matrixf, rgba_out, rgba_out, samples);

  {
    TRC_OUT(rgba_out, rgba_out);

    for (i = 0; i < samples; i++)
      for (unsigned int c = 0; c < 3; c ++)
        rgb_out_u8[i*3+c] = rgba_out[i*4+c] * 255.0f;
  }

}


#if defined(USE_SSE2)

#define m(matr, j, i)  matr[j*3+i]

#include <emmintrin.h>

static inline void babl_matrix_mul_vectorff_buf4_sse2 (const float *mat,
                                                       const float *v_in,
                                                       float       *v_out,
                                                       unsigned int samples)
{
  const __v4sf m___0 = {m(mat, 0, 0), m(mat, 1, 0), m(mat, 2, 0), 0};
  const __v4sf m___1 = {m(mat, 0, 1), m(mat, 1, 1), m(mat, 2, 1), 0};
  const __v4sf m___2 = {m(mat, 0, 2), m(mat, 1, 2), m(mat, 2, 2), 1};
  unsigned int i;
  for (i = 0; i < samples; i ++)
  {
    __v4sf a, b, c = _mm_load_ps(&v_in[0]);
    a = (__v4sf) _mm_shuffle_epi32((__m128i)c, _MM_SHUFFLE(0,0,0,0));
    b = (__v4sf) _mm_shuffle_epi32((__m128i)c, _MM_SHUFFLE(1,1,1,1));
    c = (__v4sf) _mm_shuffle_epi32((__m128i)c, _MM_SHUFFLE(3,2,2,2));
    _mm_store_ps (v_out, m___0 * a + m___1 * b + m___2 * c);
    v_out += 4;
    v_in  += 4;
  }
  _mm_empty ();
}

#undef m

static inline void
universal_nonlinear_rgba_converter_sse2 (const Babl    *conversion,
                                         unsigned char *__restrict__ src_char,
                                         unsigned char *__restrict__ dst_char,
                                         long           samples,
                                         void          *data)
{
  const Babl *source_space = babl_conversion_get_source_space (conversion);
  const Babl *destination_space = babl_conversion_get_destination_space (conversion);
  float * matrixf = data;
  float *rgba_in = (void*)src_char;
  float *rgba_out = (void*)dst_char;

  TRC_IN(rgba_in, rgba_out);

  babl_matrix_mul_vectorff_buf4_sse2 (matrixf, rgba_out, rgba_out, samples);

  TRC_OUT(rgba_out, rgba_out);
}


static inline void
universal_rgba_converter_sse2 (const Babl *conversion,
                               unsigned char *__restrict__ src_char,
                               unsigned char *__restrict__ dst_char,
                               long samples,
                               void *data)
{
  float *matrixf = data;
  float *rgba_in = (void*)src_char;
  float *rgba_out = (void*)dst_char;

  babl_matrix_mul_vectorff_buf4_sse2 (matrixf, rgba_in, rgba_out, samples);
}

static inline void
universal_nonlinear_rgb_u8_converter_sse2 (const Babl    *conversion,
                                           unsigned char *__restrict__ src_char,
                                           unsigned char *__restrict__ dst_char,
                                           long           samples,
                                           void          *data)
{
  const Babl *destination_space = conversion->conversion.destination->format.space;

  float * matrixf = data;
  float * in_trc_lut_red = matrixf + 9;
  float * in_trc_lut_green = in_trc_lut_red + 256;
  float * in_trc_lut_blue = in_trc_lut_green + 256;
  unsigned int i;
  uint8_t *rgb_in_u8 = (void*)src_char;
  uint8_t *rgb_out_u8 = (void*)dst_char;

  // The alignment is necessary for SIMD intrinsics in babl_matrix_mul_vectorff_buf4_sse2()
  float __attribute__ ((aligned (16))) rgba_out[4*samples];

  for (i = 0; i < samples; i++)
  {
    rgba_out[i*4+0]=in_trc_lut_red[rgb_in_u8[i*3+0]];
    rgba_out[i*4+1]=in_trc_lut_green[rgb_in_u8[i*3+1]];
    rgba_out[i*4+2]=in_trc_lut_blue[rgb_in_u8[i*3+2]];
  }

  babl_matrix_mul_vectorff_buf4_sse2 (matrixf, rgba_out, rgba_out, samples);

  {
    TRC_OUT(rgba_out, rgba_out);

    for (i = 0; i < samples; i++)
      for (unsigned c = 0; c < 3; c ++)
        rgb_out_u8[i*3+c] = rgba_out[i*4+c] * 255 + 0.5f;
  }
}


static inline void
universal_nonlinear_rgb_linear_converter_sse2 (const Babl    *conversion,
                                               unsigned char *__restrict__ src_char,
                                               unsigned char *__restrict__ dst_char,
                                               long           samples,
                                               void          *data)
{
  const Babl *source_space = babl_conversion_get_source_space (conversion);
  float * matrixf = data;
  float *rgba_in  = (void*)src_char;
  float *rgba_out = (void*)dst_char;

  TRC_IN(rgba_in, rgba_out);

  babl_matrix_mul_vectorff_buf4_sse2 (matrixf, rgba_out, rgba_out, samples);
}


static inline void
universal_linear_rgb_nonlinear_converter_sse2 (const Babl    *conversion,
                                               unsigned char *__restrict__ src_char,
                                               unsigned char *__restrict__ dst_char,
                                               long           samples,
                                               void          *data)
{
  const Babl *destination_space = conversion->conversion.destination->format.space;
  float * matrixf = data;
  float *rgba_in = (void*)src_char;
  float *rgba_out = (void*)dst_char;

  if (((uintptr_t) rgba_in & 0xF) == 0)
    {
      babl_matrix_mul_vectorff_buf4_sse2 (matrixf, rgba_in, rgba_out, samples);
    }
  else
    {
      /* babl_matrix_mul_vectorff_buf4_sse2() requires source pointer address
       * to be 16-bytes aligned.
       */
      float __attribute__ ((aligned (16))) aligned_rgba_in[4 * 4 * samples];

      memcpy (aligned_rgba_in, rgba_in, 4 * 4 * samples);
      babl_matrix_mul_vectorff_buf4_sse2 (matrixf, aligned_rgba_in, rgba_out, samples);
    }

  TRC_OUT(rgba_out, rgba_out);
}
#endif


static int
add_rgb_adapter (Babl *babl,
                 void *space)
{
  if (babl != space)
  {

#if defined(USE_SSE2)
    if ((babl_cpu_accel_get_support () & BABL_CPU_ACCEL_X86_SSE) &&
        (babl_cpu_accel_get_support () & BABL_CPU_ACCEL_X86_SSE2))
    {
       prep_conversion(babl_conversion_new(
                       babl_format_with_space("RGBA float", space),
                       babl_format_with_space("RGBA float", babl),
                       "linear", universal_rgba_converter_sse2,
                       NULL));
       prep_conversion(babl_conversion_new(
                       babl_format_with_space("RGBA float", babl),
                       babl_format_with_space("RGBA float", space),
                       "linear", universal_rgba_converter_sse2,
                       NULL));
       prep_conversion(babl_conversion_new(
                       babl_format_with_space("R'G'B'A float", space),
                       babl_format_with_space("R'G'B'A float", babl),
                       "linear", universal_nonlinear_rgba_converter_sse2,
                       NULL));
       prep_conversion(babl_conversion_new(
                       babl_format_with_space("R'G'B'A float", babl),
                       babl_format_with_space("R'G'B'A float", space),
                       "linear", universal_nonlinear_rgba_converter_sse2,
                       NULL));

       prep_conversion(babl_conversion_new(
                       babl_format_with_space("R'G'B'A float", space),
                       babl_format_with_space("RGBA float", babl),
                       "linear", universal_nonlinear_rgb_linear_converter_sse2,
                       NULL));
       prep_conversion(babl_conversion_new(
                       babl_format_with_space("R'G'B'A float", babl),
                       babl_format_with_space("RGBA float", space),
                       "linear", universal_nonlinear_rgb_linear_converter_sse2,
                       NULL));

       prep_conversion(babl_conversion_new(
                       babl_format_with_space("RGBA float", babl),
                       babl_format_with_space("R'G'B'A float", space),
                       "linear", universal_linear_rgb_nonlinear_converter_sse2,
                       NULL));
       prep_conversion(babl_conversion_new(
                       babl_format_with_space("RGBA float", space),
                       babl_format_with_space("R'G'B'A float", babl),
                       "linear", universal_linear_rgb_nonlinear_converter_sse2,
                       NULL));

       prep_conversion(babl_conversion_new(
                       babl_format_with_space("R'G'B' u8", space),
                       babl_format_with_space("R'G'B' u8", babl),
                       "linear", universal_nonlinear_rgb_u8_converter_sse2,
                       NULL));
       prep_conversion(babl_conversion_new(
                       babl_format_with_space("R'G'B' u8", babl),
                       babl_format_with_space("R'G'B' u8", space),
                       "linear", universal_nonlinear_rgb_u8_converter_sse2,
                       NULL));
    }
    else
#endif
    {
#if 1
       prep_conversion(babl_conversion_new(
                       babl_format_with_space("RGBA float", space),
                       babl_format_with_space("RGBA float", babl),
                       "linear", universal_rgba_converter,
                       NULL));
       prep_conversion(babl_conversion_new(
                       babl_format_with_space("RGBA float", babl),
                       babl_format_with_space("RGBA float", space),
                       "linear", universal_rgba_converter,
                       NULL));

       prep_conversion(babl_conversion_new(
                       babl_format_with_space("R'G'B'A float", space),
                       babl_format_with_space("R'G'B'A float", babl),
                       "linear", universal_nonlinear_rgba_converter,
                       NULL));
       prep_conversion(babl_conversion_new(
                       babl_format_with_space("R'G'B'A float", babl),
                       babl_format_with_space("R'G'B'A float", space),
                       "linear", universal_nonlinear_rgba_converter,
                       NULL));
#endif
#if 1
       prep_conversion(babl_conversion_new(
                       babl_format_with_space("R'G'B'A float", space),
                       babl_format_with_space("RGBA float", babl),
                       "linear", universal_nonlinear_rgb_linear_converter,
                       NULL));
       prep_conversion(babl_conversion_new(
                       babl_format_with_space("R'G'B'A float", babl),
                       babl_format_with_space("RGBA float", space),
                       "linear", universal_nonlinear_rgb_linear_converter,
                       NULL));
#endif

#if 1
       prep_conversion(babl_conversion_new(
                       babl_format_with_space("R'G'B' u8", space),
                       babl_format_with_space("R'G'B' u8", babl),
                       "linear", universal_nonlinear_rgb_u8_converter,
                       NULL));
       prep_conversion(babl_conversion_new(
                       babl_format_with_space("R'G'B' u8", babl),
                       babl_format_with_space("R'G'B' u8", space),
                       "linear", universal_nonlinear_rgb_u8_converter,
                       NULL));

       prep_conversion(babl_conversion_new(
                       babl_format_with_space("RGBA float", babl),
                       babl_format_with_space("R'G'B'A float", space),
                       "linear", universal_linear_rgb_nonlinear_converter,
                       NULL));
       prep_conversion(babl_conversion_new(
                       babl_format_with_space("RGBA float", space),
                       babl_format_with_space("R'G'B'A float", babl),
                       "linear", universal_linear_rgb_nonlinear_converter,
                       NULL));
#endif
    }
    prep_conversion(babl_conversion_new(
                    babl_format_with_space("RGB float", space),
                    babl_format_with_space("RGB float", babl),
                    "linear", universal_rgb_converter,
                    NULL));
    prep_conversion(babl_conversion_new(
                    babl_format_with_space("RGB float", babl),
                    babl_format_with_space("RGB float", space),
                    "linear", universal_rgb_converter,
                    NULL));
    prep_conversion(babl_conversion_new(
                    babl_format_with_space("Y float", space),
                    babl_format_with_space("Y float", babl),
                    "linear", universal_y_converter,
                    NULL));
    prep_conversion(babl_conversion_new(
                    babl_format_with_space("YaA float", babl),
                    babl_format_with_space("YaA float", space),
                    "linear", universal_ya_converter,
                    NULL));
    prep_conversion(babl_conversion_new(
                    babl_format_with_space("YA float", babl),
                    babl_format_with_space("YA float", space),
                    "linear", universal_ya_converter,
                    NULL));
  }
  return 0;
}

/* The first time a new Babl space is used - for creation of a fish, is when
 * this function is called, it adds conversions hooks that provides its formats
 * with conversions internally as well as for conversions to and from other RGB
 * spaces.
 */
void
BABL_SIMD_SUFFIX(_babl_space_add_universal_rgb) (const Babl *space);
void
BABL_SIMD_SUFFIX(_babl_space_add_universal_rgb) (const Babl *space)
{
  babl_space_class_for_each (add_rgb_adapter, (void*)space);
}
