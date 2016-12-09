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

static inline long
float_pre_to_u32_pre (unsigned char *src_char, unsigned char *dst_char, long samples)
{
  float *src = (float *)src_char;
  uint32_t *dst = (uint32_t *)dst_char;
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
      
      dst[0] = (r >= 1.0f) ? 0xFFFFFFFF : ((r <= 0.0f) ? 0x0 : 0xFFFFFFFF * r + 0.5f);
      dst[1] = (g >= 1.0f) ? 0xFFFFFFFF : ((g <= 0.0f) ? 0x0 : 0xFFFFFFFF * g + 0.5f);
      dst[2] = (b >= 1.0f) ? 0xFFFFFFFF : ((b <= 0.0f) ? 0x0 : 0xFFFFFFFF * b + 0.5f);
      dst[3] = (a >= 1.0f) ? 0xFFFFFFFF : ((a <= 0.0f) ? 0x0 : 0xFFFFFFFF * a + 0.5f);
      
      dst += 4;
      src += 4;
    }
  return samples;
}


static inline long
float_to_u32 (unsigned char *src_char, unsigned char *dst_char, long samples)
{
  float *src    = (float *)src_char;
  uint32_t *dst = (uint32_t *)dst_char;
  long n = samples;
  while (n--)
    {
      float r = src[0];
      float g = src[1];
      float b = src[2];
      float a = src[3];
            
      dst[0] = (r >= 1.0f) ? 0xFFFFFFFF : ((r <= 0.0f) ? 0x0 : 0xFFFFFFFF * r + 0.5f);
      dst[1] = (g >= 1.0f) ? 0xFFFFFFFF : ((g <= 0.0f) ? 0x0 : 0xFFFFFFFF * g + 0.5f);
      dst[2] = (b >= 1.0f) ? 0xFFFFFFFF : ((b <= 0.0f) ? 0x0 : 0xFFFFFFFF * b + 0.5f);
      dst[3] = (a >= 1.0f) ? 0xFFFFFFFF : ((a <= 0.0f) ? 0x0 : 0xFFFFFFFF * a + 0.5f);
      
      dst += 4;
      src += 4;
    }
  return samples;
}

static inline long
u32_to_float (unsigned char *src_char, unsigned char *dst_char, long samples)
{
  uint32_t *src = (uint32_t *)src_char;
  float *dst    = (float *)dst_char;
  long n = samples;
  while (n--)
    {
      dst[0] = src[0] / 4294967296.0f;
      dst ++;
      src ++;
    }
  return samples;
}

static inline long
u32_to_float_x4 (unsigned char *src_char, unsigned char *dst_char, long samples)
{
  u32_to_float (src_char, dst_char, samples * 4);
  return samples;
}

static inline long
u32_to_float_x2 (unsigned char *src_char, unsigned char *dst_char, long samples)
{
  u32_to_float (src_char, dst_char, samples * 2);
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


  /* float and u32 */
  babl_conversion_new (babl_format ("R'G'B'A float"),
                       babl_format ("R'G'B'A u32"),
                      "linear", 
                       float_to_u32,
                       NULL);
  babl_conversion_new (babl_format ("RGBA float"),
                       babl_format ("RGBA u32"),
                      "linear", 
                       float_to_u32,
                       NULL);
  babl_conversion_new (babl_format ("R'aG'aB'aA float"),
                       babl_format ("R'aG'aB'aA u32"),
                      "linear", 
                       float_pre_to_u32_pre,
                       NULL);
  babl_conversion_new (babl_format ("RaGaBaA float"),
                       babl_format ("RaGaBaA u32"),
                      "linear", 
                       float_pre_to_u32_pre,
                       NULL);


  babl_conversion_new (babl_format ("YA u32"),
                       babl_format ("YA float"),
                      "linear", 
                       u32_to_float_x2,
                       NULL);

  babl_conversion_new (babl_format ("Y'A u32"),
                       babl_format ("Y'A float"),
                      "linear", 
                       u32_to_float_x2,
                       NULL);

  babl_conversion_new (babl_format ("Y u32"),
                       babl_format ("Y float"),
                      "linear", 
                       u32_to_float,
                       NULL);

  babl_conversion_new (babl_format ("Y' u32"),
                       babl_format ("Y' float"),
                      "linear", 
                       u32_to_float,
                       NULL);

  babl_conversion_new (babl_format ("RGBA u32"),
                       babl_format ("RGBA float"),
                      "linear", 
                       u32_to_float_x4,
                       NULL);

  babl_conversion_new (babl_format ("R'G'B'A u32"),
                       babl_format ("R'G'B'A float"),
                      "linear", 
                       u32_to_float_x4,
                       NULL);


  return 0;
}
