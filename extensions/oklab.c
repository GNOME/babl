/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2005, 2014, 2019 Øyvind Kolås.
 * Copyright (C) 2014, 2019 Elle Stone
 * Copyright (C) 2009, Martin Nordholts
 * Copyright (C) 2021, Mingye Wang <arthur2e5@aosc.io>
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

/*
 * Björn Ottosson (2020). Oklab, a perceptual color space for image
 * processing. https://bottosson.github.io/posts/oklab/
 */

#include "config.h"

#include <math.h>
#include <string.h>

#include "babl-internal.h"
#include "babl-matrix.h"
#include "babl.h"
#include "base/util.h"

#define DEGREES_PER_RADIAN (180 / 3.14159265358979323846)
#define RADIANS_PER_DEGREE (1 / DEGREES_PER_RADIAN)

static void components (void);
static void models (void);
static void conversions (void);
static void formats (void);

int init (void);

static int enable_lch = 0;
 // the Oklch conversions are not fully symmetric,
 // thus not allowing the tests to pass if we register
 // the code

int
init (void)
{
  components ();
  models ();
  formats ();
  conversions ();
  return 0;
}

static void
components (void)
{
  babl_component_new ("Ok L", "doc", "Luminance, range 0.0-100.0 in float",
                      NULL);
  babl_component_new ("Ok a", "chroma", "doc",
                      "chroma component 0.0 is no saturation", NULL);
  babl_component_new ("Ok b", "chroma", "doc",
                      "chroma component 0.0 is no saturation", NULL);
  babl_component_new ("Ok C", "chroma", "doc", "chrominance/saturation", NULL);
  babl_component_new ("Ok H", "chroma", "doc", "hue value range 0.0-360.0",
                      NULL);
}

static void
models (void)
{
  babl_model_new ("name", "Oklab", "doc",
                  "Oklab color model, a perceptually uniform space.",
                  babl_component ("Ok L"), babl_component ("Ok a"),
                  babl_component ("Ok b"), NULL);

  babl_model_new (
      "name", "OklabA", "doc", "Oklab color model with separate alpha.",
      babl_component ("Ok L"), babl_component ("Ok a"),
      babl_component ("Ok b"), babl_component ("A"), "alpha", NULL);

  if (enable_lch)
  {
    babl_model_new ("name", "Oklch", "doc",
                    "Cylindrical representation of Oklab.",
                    babl_component ("Ok L"), babl_component ("Ok C"),
                    babl_component ("Ok H"), NULL);

    babl_model_new (
        "name", "OklchA", "doc", "Oklch color model with separate alpha.",
        babl_component ("Ok L"), babl_component ("Ok C"),
        babl_component ("Ok H"), babl_component ("A"), "alpha", NULL);
  }
}

static void
formats (void)
{
  babl_format_new (
    "name", "Oklab float",
    babl_model ("Oklab"),
    babl_type ("float"),
    babl_component ("Ok L"),
    babl_component ("Ok a"),
    babl_component ("Ok b"),
    NULL
  );


  babl_format_new (
    "name", "Oklab alpha float",
    babl_model ("OklabA"),
    babl_type ("float"),
    babl_component ("Ok L"),
    babl_component ("Ok a"),
    babl_component ("Ok b"),
    babl_component ("A"),
    NULL
  );

  if (enable_lch)
  {
  babl_format_new (
    "name", "Oklch float",
    babl_model ("Oklch"),
    babl_type ("float"),
    babl_component ("Ok L"),
    babl_component ("Ok C"),
    babl_component ("Ok H"),
    NULL
  );

  babl_format_new (
    "name", "Oklch alpha float",
    babl_model ("OklchA"),
    babl_type ("float"),
    babl_component ("Ok L"),
    babl_component ("Ok C"),
    babl_component ("Ok H"),
    babl_component ("A"),
    NULL
  );
  }
}

/* Convertion routine (space definition). */
/* It's all float. The original definition is in float. */
static double M1[9] = {
  +0.8189330101, +0.0329845436, +0.0482003018,
  +0.3618667424, +0.9293118715, +0.2643662691,
  -0.1288597137, +0.0361456387, +0.6338517070,
};

static double M2[9] = {
  +0.2104542553, +0.7936177850, - 0.0040720468,
  +1.9779984951, -2.4285922050, + 0.4505937099,
  +0.0259040371, +0.7827717662, - 0.8086757660,
};

static float M1f[9];
static float M2f[9];
static float inv_M1f[9];
static float inv_M2f[9];

static double inv_M1[9];
static double inv_M2[9];
static int mat_ready;

/* fast approximate cube root
 * origin: http://www.hackersdelight.org/hdcodetxt/acbrt.c.txt
 * permissions: http://www.hackersdelight.org/permissions.htm
 */
static inline float
_cbrtf (float x)
{
  union
  {
    float f;
    uint32_t i;
  } u = { x };

  u.i = u.i / 4 + u.i / 16;
  u.i = u.i + u.i / 16;
  u.i = u.i + u.i / 256;
  u.i = 0x2a5137a0 + u.i;
  u.f = 0.33333333f * (2.0f * u.f + x / (u.f * u.f));
  u.f = 0.33333333f * (2.0f * u.f + x / (u.f * u.f));

  return u.f;
}

static inline void
XYZ_to_Oklab_step (double *xyz, double *lab_out)
{
  double lms[3];
  babl_matrix_mul_vector (M1, xyz, lms);
  for (int i = 0; i < 3; i++)
    {
      lms[i] = cbrt (lms[i]);
    }
  babl_matrix_mul_vector (M2, lms, lab_out);
}

static inline void
XYZ_to_Oklab_stepf (float *xyz, float *lab_out)
{
  float lms[3];
  babl_matrix_mul_vectorff (M1f, xyz, lms);
  for (int i = 0; i < 3; i++)
    {
      lms[i] = _cbrtf (lms[i]);
    }
  babl_matrix_mul_vectorff (M2f, lms, lab_out);
}

static inline void
Oklab_to_XYZ_stepf (float *lab, float *xyz_out)
{
  float lms[3];
  babl_matrix_mul_vectorff (inv_M2f, lab, lms);
  for (int i = 0; i < 3; i++)
    {
      lms[i] = lms[i] * lms[i] * lms[i];
    }
  babl_matrix_mul_vectorff (inv_M1f, lms, xyz_out);
}

static inline void
Oklab_to_XYZ_step (double *lab, double *xyz_out)
{
  double lms[3];
  babl_matrix_mul_vector (inv_M2, lab, lms);
  for (int i = 0; i < 3; i++)
    {
      lms[i] = lms[i] * lms[i] * lms[i];
    }
  babl_matrix_mul_vector (inv_M1, lms, xyz_out);
}

static inline void
ab_to_ch_step (double *ab, double *ch_out)
{
  double a = ab[0], b = ab[1];

  ch_out[1] = sqrt (a * a + b * b);
  ch_out[2] = atan2 (b, a) * DEGREES_PER_RADIAN;

  // Keep H within the range 0-360
  if (ch_out[2] < 0.0)
    ch_out[2] += 360;
}

static inline void
ab_to_ch_stepf (float *ab, float *ch_out)
{
  float a = ab[0], b = ab[1];

  ch_out[1] = sqrtf (a * a + b * b);
  ch_out[2] = atan2f (b, a) * DEGREES_PER_RADIAN;

  // Keep H within the range 0-360
  if (ch_out[2] < 0.0)
    ch_out[2] += 360;
}

static inline void
ch_to_ab_step (double *ch, double *ab_out)
{
  double c = ch[0], h = ch[1];

  ab_out[0] = cos (h * RADIANS_PER_DEGREE) * c;
  ab_out[1] = sin (h * RADIANS_PER_DEGREE) * c;
}

static inline void
ch_to_ab_stepf (float *ch, float *ab_out)
{
  float c = ch[0], h = ch[1];

  ab_out[0] = cosf (h * RADIANS_PER_DEGREE) * c;
  ab_out[1] = sinf (h * RADIANS_PER_DEGREE) * c;
}

static inline void
XYZ_to_Oklch_step (double *xyz, double *lch_out)
{
  XYZ_to_Oklab_step (xyz, lch_out);
  ab_to_ch_step (lch_out + 1, lch_out + 1);
}

static inline void
XYZ_to_Oklch_stepf (float *xyz, float *lch_out)
{
  XYZ_to_Oklab_stepf (xyz, lch_out);
  ab_to_ch_stepf (lch_out + 1, lch_out + 1);
}

static inline void
Oklch_to_XYZ_step (double *lch, double *xyz_out)
{
  double lab[3] = { lch[0], lch[1], lch[2] };
  ch_to_ab_step (lab + 1, lab + 1);
  Oklab_to_XYZ_step (lab, xyz_out);
}

static inline void
Oklch_to_XYZ_stepf (float *lch, float *xyz_out)
{
  float lab[3] = { lch[0], lch[1], lch[2] };
  ch_to_ab_stepf (lab + 1, lab + 1);
  Oklab_to_XYZ_stepf (lab, xyz_out);
}

static inline void
constants (void)
{
  double tmp[9];
  double D65[3] = { 0.95047, 1.0, 1.08883 };
  double D50[3] = { 0.96420288, 1.0, 0.82490540 };

  if (mat_ready)
    return;

  babl_chromatic_adaptation_matrix (D50, D65, tmp);
  babl_matrix_mul_matrix (tmp, M1, M1);

  babl_matrix_invert (M1, inv_M1);
  babl_matrix_invert (M2, inv_M2);

  babl_matrix_to_float (M1, M1f);
  babl_matrix_to_float (M2, M2f);
  babl_matrix_to_float (inv_M1, inv_M1f);
  babl_matrix_to_float (inv_M2, inv_M2f);

  mat_ready = 1;
}

/* Convertion routine (glue and boilerplate). */
static void
rgba_to_laba_float (const Babl *conversion, char *src_, char *dst_, long samples)
{
  long n = samples;
  float *src = (float *)src_, *dst = (float *)dst_;
  const Babl *space = babl_conversion_get_source_space (conversion);

  while (n--)
    {
      float xyz[3];
      babl_space_to_xyzf (space, src, xyz);
      XYZ_to_Oklab_stepf (xyz, dst);
      dst[3] = src[3];

      src += 4;
      dst += 4;
    }
}

static void
rgba_to_laba (const Babl *conversion, char *src_, char *dst_, long samples)
{
  long n = samples;
  double *src = (double*)src_, *dst = (double*)dst_;
  const Babl *space = babl_conversion_get_source_space (conversion);

  while (n--)
    {
      double xyz[3];
      babl_space_to_xyz (space, src, xyz);
      XYZ_to_Oklab_step (xyz, dst);
      dst[3] = src[3];

      src += 4;
      dst += 4;
    }
}

static void
rgba_to_lab_float (const Babl *conversion, char *src_, char *dst_, long samples)
{
  long n = samples;
  float *src = (float *)src_, *dst = (float *)dst_;
  const Babl *space = babl_conversion_get_source_space (conversion);

  while (n--)
    {
      float xyz[3];
      babl_space_to_xyzf (space, src, xyz);
      XYZ_to_Oklab_stepf (xyz, dst);

      src += 4;
      dst += 3;
    }
}

static void
rgba_to_lab (const Babl *conversion, char *src_, char *dst_, long samples)
{
  long n = samples;
  double *src = (double *)src_, *dst = (double *)dst_;
  const Babl *space = babl_conversion_get_source_space (conversion);

  while (n--)
    {
      double xyz[3];
      babl_space_to_xyz (space, src, xyz);
      XYZ_to_Oklab_step (xyz, dst);

      src += 4;
      dst += 3;
    }
}

static void
rgba_to_lcha_float (const Babl *conversion, char *src_, char *dst_, long samples)
{
  long n = samples;
  float *src = (float *)src_, *dst = (float *)dst_;
  const Babl *space = babl_conversion_get_source_space (conversion);

  while (n--)
    {
      float xyz[3];
      babl_space_to_xyzf (space, src, xyz);
      XYZ_to_Oklch_stepf (xyz, dst);
      dst[3] = src[3];

      src += 4;
      dst += 4;
    }
}

static void
rgba_to_lcha (const Babl *conversion, char *src_, char *dst_, long samples)
{
  long n = samples;
  double *src = (double *)src_, *dst = (double *)dst_;
  const Babl *space = babl_conversion_get_source_space (conversion);

  while (n--)
    {
      double xyz[3];
      babl_space_to_xyz (space, src, xyz);
      XYZ_to_Oklch_step (xyz, dst);
      dst[3] = src[3];

      src += 4;
      dst += 4;
    }
}

static void
rgba_to_lch_float (const Babl *conversion, char *src_, char *dst_, long samples)
{
  long n = samples;
  float *src = (float *)src_, *dst = (float *)dst_;
  const Babl *space = babl_conversion_get_source_space (conversion);

  while (n--)
    {
      float xyz[3];
      babl_space_to_xyzf (space, src, xyz);
      XYZ_to_Oklch_stepf (xyz, dst);

      src += 4;
      dst += 3;
    }
}

static void
rgba_to_lch (const Babl *conversion, char *src_, char *dst_, long samples)
{
  long n = samples;
  double *src = (double *)src_, *dst = (double *)dst_;
  const Babl *space = babl_conversion_get_source_space (conversion);

  while (n--)
    {
      double xyz[3];
      babl_space_to_xyz (space, src, xyz);
      XYZ_to_Oklch_step (xyz, dst);

      src += 4;
      dst += 3;
    }
}

static void
rgb_to_lab_float (const Babl *conversion, char *src_, char *dst_, long samples)
{
  long n = samples;
  float *src = (float *)src_, *dst = (float *)dst_;
  const Babl *space = babl_conversion_get_source_space (conversion);

  while (n--)
    {
      float xyz[3];
      babl_space_to_xyzf (space, src, xyz);
      XYZ_to_Oklab_stepf (xyz, dst);

      src += 3;
      dst += 3;
    }
}

static void
rgb_to_lch_float (const Babl *conversion, char *src_, char *dst_, long samples)
{
  long n = samples;
  float *src = (float *)src_, *dst = (float *)dst_;
  const Babl *space = babl_conversion_get_source_space (conversion);

  while (n--)
    {
      float xyz[3];
      babl_space_to_xyzf (space, src, xyz);
      XYZ_to_Oklch_stepf (xyz, dst);

      src += 3;
      dst += 3;
    }
}

static void
lab_to_rgb_float (const Babl *conversion, char *src_, char *dst_, long samples)
{
  long n = samples;
  float *src = (float *)src_, *dst = (float *)dst_;
  const Babl *space = babl_conversion_get_destination_space (conversion);

  while (n--)
    {
      float xyz[3];
      Oklab_to_XYZ_stepf (src, xyz);
      babl_space_from_xyzf (space, xyz, dst);

      src += 3;
      dst += 3;
    }
}

static void
lab_to_rgba_float (const Babl *conversion, char *src_, char *dst_, long samples)
{
  long n = samples;
  float *src = (float *)src_, *dst = (float *)dst_;
  const Babl *space = babl_conversion_get_destination_space (conversion);

  while (n--)
    {
      float xyz[3];
      Oklab_to_XYZ_stepf (src, xyz);
      babl_space_from_xyzf (space, xyz, dst);
      dst[3] = 1.0;

      src += 3;
      dst += 4;
    }
}

static void
lab_to_rgba (const Babl *conversion, char *src_, char *dst_, long samples)
{
  long n = samples;
  double *src = (double *)src_, *dst = (double *)dst_;
  const Babl *space = babl_conversion_get_destination_space (conversion);

  while (n--)
    {
      double xyz[3];
      Oklab_to_XYZ_step (src, xyz);
      babl_space_from_xyz (space, xyz, dst);
      dst[3] = 1.0;

      src += 3;
      dst += 4;
    }
}

static void
lch_to_rgb_float (const Babl *conversion, char *src_, char *dst_, long samples)
{
  long n = samples;
  float *src = (float *)src_, *dst = (float *)dst_;
  const Babl *space = babl_conversion_get_destination_space (conversion);

  while (n--)
    {
      float xyz[3];
      Oklch_to_XYZ_stepf (src, xyz);
      babl_space_from_xyzf (space, xyz, dst);

      src += 3;
      dst += 3;
    }
}

static void
laba_to_rgba_float (const Babl *conversion, char *src_, char *dst_, long samples)
{
  long n = samples;
  float *src = (float *)src_, *dst = (float *)dst_;
  const Babl *space = babl_conversion_get_destination_space (conversion);

  while (n--)
    {
      float xyz[3];
      Oklab_to_XYZ_stepf (src, xyz);
      babl_space_from_xyzf (space, xyz, dst);
      dst[3] = src[3];

      src += 4;
      dst += 4;
    }
}

static void
laba_to_rgba (const Babl *conversion, char *src_, char *dst_, long samples)
{
  long n = samples;
  double *src = (double *)src_, *dst = (double *)dst_;
  const Babl *space = babl_conversion_get_destination_space (conversion);

  while (n--)
    {
      double xyz[3];
      Oklab_to_XYZ_step (src, xyz);
      babl_space_from_xyz (space, xyz, dst);
      dst[3] = src[3];

      src += 4;
      dst += 4;
    }
}

static void
lcha_to_rgba_float (const Babl *conversion, char *src_, char *dst_, long samples)
{
  long n = samples;
  float *src = (float *)src_, *dst = (float *)dst_;
  const Babl *space = babl_conversion_get_destination_space (conversion);

  while (n--)
    {
      float xyz[3];
      Oklch_to_XYZ_stepf (src, xyz);
      babl_space_from_xyzf (space, xyz, dst);
      dst[3] = src[3];

      src += 4;
      dst += 4;
    }
}

static void
lcha_to_rgba (const Babl *conversion, char *src_, char *dst_, long samples)
{
  long n = samples;
  double *src = (double *)src_, *dst = (double *)dst_;
  const Babl *space = babl_conversion_get_destination_space (conversion);

  while (n--)
    {
      double xyz[3];
      Oklch_to_XYZ_step (src, xyz);
      babl_space_from_xyz (space, xyz, dst);
      dst[3] = src[3];

      src += 4;
      dst += 4;
    }
}


static void
lch_to_rgba_float (const Babl *conversion, char *src_, char *dst_, long samples)
{
  long n = samples;
  float *src = (float *)src_, *dst = (float *)dst_;
  const Babl *space = babl_conversion_get_destination_space (conversion);

  while (n--)
    {
      float xyz[3];
      Oklch_to_XYZ_stepf (src, xyz);
      babl_space_from_xyzf (space, xyz, dst);
      dst[3] = 1.0f;

      src += 3;
      dst += 4;
    }
}

static void
lch_to_rgba (const Babl *conversion, char *src_, char *dst_, long samples)
{
  long n = samples;
  double *src = (double *)src_, *dst = (double *)dst_;
  const Babl *space = babl_conversion_get_destination_space (conversion);

  while (n--)
    {
      double xyz[3];
      Oklch_to_XYZ_step (src, xyz);
      babl_space_from_xyz (space, xyz, dst);
      dst[3] = 1.0f;

      src += 3;
      dst += 4;
    }
}


static void
lch_to_lab_float (const Babl *conversion, char *src_, char *dst_, long samples)
{
  long n = samples;
  float *src = (float *)src_, *dst = (float *)dst_;

  while (n--)
    {
      dst[0] = src[0];
      ch_to_ab_stepf (src + 1, dst + 1);

      src += 3;
      dst += 3;
    }
}

static void
lab_to_lch_float (const Babl *conversion, char *src_, char *dst_, long samples)
{
  long n = samples;
  float *src = (float *)src_, *dst = (float *)dst_;

  while (n--)
    {
      dst[0] = src[0];
      ab_to_ch_stepf (src + 1, dst + 1);

      src += 3;
      dst += 3;
    }
}

static void
lcha_to_laba_float (const Babl *conversion, char *src_, char *dst_, long samples)
{
  long n = samples;
  float *src = (float *)src_, *dst = (float *)dst_;

  while (n--)
    {
      dst[0] = src[0];
      ch_to_ab_stepf (src + 1, dst + 1);
      dst[3] = src[3];

      src += 4;
      dst += 4;
    }
}

static void
laba_to_lcha_float (const Babl *conversion, char *src_, char *dst_, long samples)
{
  long n = samples;
  float *src = (float *)src_, *dst = (float *)dst_;

  while (n--)
    {
      dst[0] = src[0];
      ab_to_ch_stepf (src + 1, dst + 1);
      dst[3] = src[3];

      src += 4;
      dst += 4;
    }
}

/* End conversion routines. */

static void
conversions (void)
{
  constants ();

#define _pair(f1, f2, fwd, rev)                                               \
  do                                                                          \
    {                                                                         \
      babl_conversion_new (babl_format (f1), babl_format (f2), "linear", fwd, \
                           NULL);                                             \
      babl_conversion_new (babl_format (f2), babl_format (f1), "linear", rev, \
                           NULL);                                             \
    }                                                                         \
  while (0)

  babl_conversion_new (babl_model("RGBA"),
                       babl_model("OklabA"),
                       "linear", rgba_to_laba,
                       NULL);
  babl_conversion_new (babl_model("OklabA"),
                       babl_model("RGBA"),
                       "linear", laba_to_rgba,
                       NULL);

  babl_conversion_new (babl_model("RGBA"),
                       babl_model("Oklab"),
                       "linear", rgba_to_lab,
                       NULL);
  babl_conversion_new (babl_model("Oklab"),
                       babl_model("RGBA"),
                       "linear", lab_to_rgba,
                       NULL);

  _pair ("RGB float", "Oklab float", rgb_to_lab_float, lab_to_rgb_float);
  _pair ("RGBA float", "Oklab alpha float", rgba_to_laba_float, laba_to_rgba_float);
  _pair ("RGBA float", "Oklab float", rgba_to_lab_float, lab_to_rgba_float);

  if (enable_lch)
  {
  babl_conversion_new (babl_model("RGBA"),
                       babl_model("OklchA"),
                       "linear", rgba_to_lcha,
                       NULL);
  babl_conversion_new (babl_model("OklchA"),
                       babl_model("RGBA"),
                       "linear", lcha_to_rgba,
                       NULL);

  babl_conversion_new (babl_model("RGBA"),
                       babl_model("Oklch"),
                       "linear", rgba_to_lch,
                       NULL);
  babl_conversion_new (babl_model("Oklch"),
                       babl_model("RGBA"),
                       "linear", lch_to_rgba,
                       NULL);
  _pair ("RGBA float", "Oklch float", rgba_to_lch_float, lch_to_rgba_float);
  _pair ("RGB float", "Oklch float", rgb_to_lch_float, lch_to_rgb_float);
  _pair ("RGBA float", "Oklch alpha float", rgba_to_lcha_float, lcha_to_rgba_float);
  
  _pair ("Oklab float", "Oklch float", lab_to_lch_float, lch_to_lab_float);
  _pair ("Oklab alpha float", "Oklch alpha float", laba_to_lcha_float, lcha_to_laba_float);
  }
  #undef _pair
}
