#include <stdlib.h>
#include <stdint.h>
#include "babl.h"

#include "base/util.h"

int init (void);

static long
float_pre_to_u16 (unsigned char *src_char, unsigned char *dst_char, long samples)
{
  float *src    = (float *)src_char;
  uint16_t *dst = (uint16_t *)dst_char;
  long n = samples;
  while (n--)
    {
      if (src[3] < BABL_ALPHA_THRESHOLD)
        {
          dst[0] = 0;
          dst[1] = 0;
          dst[2] = 0;
          dst[3] = 0;
        }
      else
        {
          float a_recip = 65535.0f * (1.0f / src[3]);
          float r = src[0] * a_recip + 0.5;
          float g = src[1] * a_recip + 0.5;
          float b = src[2] * a_recip + 0.5;
          float a = 65535.0f * src[3] + 0.5;
          dst[0] = r >= 65535.0f ? 0xFFFF : r;
          dst[1] = g >= 65535.0f ? 0xFFFF : g;
          dst[2] = b >= 65535.0f ? 0xFFFF : b;
          dst[3] = a >= 65535.0f ? 0xFFFF : a;
        }
      
      src += 4;
      dst += 4;
    }
  return samples;
}

static long
u16_to_float_pre (unsigned char *src_char, unsigned char *dst_char, long samples)
{
  uint16_t *src = (uint16_t *)src_char;
  float *dst    = (float *)dst_char;
  long n = samples;
  while (n--)
    {
      float a_term = src[3] / (65535.0f * 65535.0f);
      dst[0] = src[0] * a_term;
      dst[1] = src[1] * a_term;
      dst[2] = src[2] * a_term;
      dst[3] = src[3] / 65535.0;
      
      src += 4;
      dst += 4;
    }
  return samples;
}

int
init (void)
{
  babl_conversion_new (babl_format ("RaGaBaA float"),
                       babl_format ("RGBA u16"),
                      "linear", 
                       float_pre_to_u16,
                       NULL);
  babl_conversion_new (babl_format ("RGBA u16"),
                       babl_format ("RaGaBaA float"),
                      "linear", 
                       u16_to_float_pre,
                       NULL);

  return 0;
}
