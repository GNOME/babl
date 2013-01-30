#include <stdlib.h>
#include <stdint.h>
#include "babl.h"

int init (void);

static inline long
float_to_u8 (unsigned char *src_char, unsigned char *dst, long samples)
{
  float *src = (float *)src_char;
  long n = samples;
  while (n--)
    {
      float r = src[0];
      float g = src[1];
      float b = src[2];
      float a = src[3];
            
      dst[0] = (r >= 1.0f) ? 0xFF : ((r <= 0.0f) ? 0x0 : 0xFF * r + 0.5f);
      dst[1] = (g >= 1.0f) ? 0xFF : ((g <= 0.0f) ? 0x0 : 0xFF * g + 0.5f);
      dst[2] = (b >= 1.0f) ? 0xFF : ((b <= 0.0f) ? 0x0 : 0xFF * b + 0.5f);
      dst[3] = (a >= 1.0f) ? 0xFF : ((a <= 0.0f) ? 0x0 : 0xFF * a + 0.5f);
      
      dst += 4;
      src += 4;
    }
  return samples;
}

static inline long
float_pre_to_u8_pre (unsigned char *src_char, unsigned char *dst, long samples)
{
  float *src = (float *)src_char;
  long n = samples;
  while (n--)
    {
      float r = src[0];
      float g = src[1];
      float b = src[2];
      float a = src[3];
      
      if (a > 1.0f) {
        r /= a;
        g /= a;
        b /= a;
        a /= a;
      }
      
      dst[0] = (r >= 1.0f) ? 0xFF : ((r <= 0.0f) ? 0x0 : 0xFF * r + 0.5f);
      dst[1] = (g >= 1.0f) ? 0xFF : ((g <= 0.0f) ? 0x0 : 0xFF * g + 0.5f);
      dst[2] = (b >= 1.0f) ? 0xFF : ((b <= 0.0f) ? 0x0 : 0xFF * b + 0.5f);
      dst[3] = (a >= 1.0f) ? 0xFF : ((a <= 0.0f) ? 0x0 : 0xFF * a + 0.5f);
      
      dst += 4;
      src += 4;

    }
  return samples;
}

static inline long
float_to_u16 (unsigned char *src_char, unsigned char *dst_char, long samples)
{
  float *src    = (float *)src_char;
  uint16_t *dst = (uint16_t *)dst_char;
  long n = samples;
  while (n--)
    {
      float r = src[0];
      float g = src[1];
      float b = src[2];
      float a = src[3];
            
      dst[0] = (r >= 1.0f) ? 0xFFFF : ((r <= 0.0f) ? 0x0 : 0xFFFF * r + 0.5f);
      dst[1] = (g >= 1.0f) ? 0xFFFF : ((g <= 0.0f) ? 0x0 : 0xFFFF * g + 0.5f);
      dst[2] = (b >= 1.0f) ? 0xFFFF : ((b <= 0.0f) ? 0x0 : 0xFFFF * b + 0.5f);
      dst[3] = (a >= 1.0f) ? 0xFFFF : ((a <= 0.0f) ? 0x0 : 0xFFFF * a + 0.5f);
      
      dst += 4;
      src += 4;
    }
  return samples;
}

static inline long
float_pre_to_u16_pre (unsigned char *src_char, unsigned char *dst_char, long samples)
{
  float *src = (float *)src_char;
  uint16_t *dst = (uint16_t *)dst_char;
  long n = samples;
  while (n--)
    {
      float r = src[0];
      float g = src[1];
      float b = src[2];
      float a = src[3];
      
      if (a > 1.0f) {
        r /= a;
        g /= a;
        b /= a;
        a /= a;
      }
      
      dst[0] = (r >= 1.0f) ? 0xFFFF : ((r <= 0.0f) ? 0x0 : 0xFFFF * r + 0.5f);
      dst[1] = (g >= 1.0f) ? 0xFFFF : ((g <= 0.0f) ? 0x0 : 0xFFFF * g + 0.5f);
      dst[2] = (b >= 1.0f) ? 0xFFFF : ((b <= 0.0f) ? 0x0 : 0xFFFF * b + 0.5f);
      dst[3] = (a >= 1.0f) ? 0xFFFF : ((a <= 0.0f) ? 0x0 : 0xFFFF * a + 0.5f);
      
      dst += 4;
      src += 4;
    }
  return samples;
}

int
init (void)
{
  /* float and u8 */
  babl_conversion_new (babl_format ("R'G'B'A float"),
                       babl_format ("R'G'B'A u8"),
                      "linear", 
                       float_to_u8,
                       NULL);
  babl_conversion_new (babl_format ("RGBA float"),
                       babl_format ("RGBA u8"),
                      "linear", 
                       float_to_u8,
                       NULL);
  babl_conversion_new (babl_format ("R'aG'aB'aA float"),
                       babl_format ("R'aG'aB'aA u8"),
                      "linear", 
                       float_pre_to_u8_pre,
                       NULL);
  babl_conversion_new (babl_format ("RaGaBaA float"),
                       babl_format ("RaGaBaA u8"),
                      "linear", 
                       float_pre_to_u8_pre,
                       NULL);

  /* float and u16 */
  babl_conversion_new (babl_format ("R'G'B'A float"),
                       babl_format ("R'G'B'A u16"),
                      "linear", 
                       float_to_u16,
                       NULL);
  babl_conversion_new (babl_format ("RGBA float"),
                       babl_format ("RGBA u16"),
                      "linear", 
                       float_to_u16,
                       NULL);
  babl_conversion_new (babl_format ("R'aG'aB'aA float"),
                       babl_format ("R'aG'aB'aA u16"),
                      "linear", 
                       float_pre_to_u16_pre,
                       NULL);
  babl_conversion_new (babl_format ("RaGaBaA float"),
                       babl_format ("RaGaBaA u16"),
                      "linear", 
                       float_pre_to_u16_pre,
                       NULL);

  return 0;
}
