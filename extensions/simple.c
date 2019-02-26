#include <stdlib.h>
#include <stdint.h>
#include "babl.h"

int init (void);


static inline void
float_to_u8_x1 (const Babl    *conversion,
                unsigned char *src_char, 
                unsigned char *dst, 
                long           samples)
{
  float *src = (float *)src_char;
  long n = samples;
  while (n--)
    {
      float r = src[0];
      dst[0] = (r >= 1.0f) ? 0xFF : ((r <= 0.0f) ? 0x0 : 0xFF * r + 0.5f);
      dst += 1;
      src += 1;
    }
}

static inline void
float_to_u8_x4 (const Babl    *conversion,
                unsigned char *src_char, 
                unsigned char *dst, 
                long           samples)
{
  float_to_u8_x1 (conversion, src_char, dst, samples * 4);
}

static inline void
float_to_u8_x3 (const Babl    *conversion,
                unsigned char *src_char, 
                unsigned char *dst, 
                long           samples)
{
  float_to_u8_x1 (conversion, src_char, dst, samples * 3);
}

static inline void
float_to_u8_x2 (const Babl    *conversion,
                unsigned char *src_char, 
                unsigned char *dst, 
                long           samples)
{
  float_to_u8_x1 (conversion, src_char, dst, samples * 2);
}



static inline void
float_pre_to_u8_pre (const Babl    *conversion,
                     unsigned char *src_char, 
                     unsigned char *dst, 
                     long           samples)
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
}

static inline void
float_to_u16_x1 (const Babl    *conversion,
                 unsigned char *src_char, 
                 unsigned char *dst_char, 
                 long           samples)
{
  float *src    = (float *)src_char;
  uint16_t *dst = (uint16_t *)dst_char;
  long n = samples;
  while (n--)
    {
      float r = src[0];
      dst[0] = (r >= 1.0f) ? 0xFFFF : ((r <= 0.0f) ? 0x0 : 0xFFFF * r + 0.5f);
      dst += 1;
      src += 1;
    }
}
static inline void
float_to_u16_x2 (const Babl    *conversion,
                 unsigned char *src_char, 
                 unsigned char *dst_char, 
                 long           samples)
{
  float_to_u16_x1 (conversion, src_char, dst_char, samples * 2);
}
static inline void
float_to_u16_x3 (const Babl    *conversion,
                 unsigned char *src_char, 
                 unsigned char *dst_char, 
                 long           samples)
{
  float_to_u16_x1 (conversion, src_char, dst_char, samples * 3);
}
static inline void
float_to_u16_x4 (const Babl    *conversion,
                 unsigned char *src_char, 
                 unsigned char *dst_char, 
                 long           samples)
{
  float_to_u16_x1 (conversion, src_char, dst_char, samples * 4);
}

static inline void
float_pre_to_u16_pre (const Babl    *conversion,
                      unsigned char *src_char, 
                      unsigned char *dst_char, 
                      long           samples)
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
}

static inline void
float_pre_to_u32_pre (const Babl    *conversion,
                      unsigned char *src_char, 
                      unsigned char *dst_char, 
                      long           samples)
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
}


static inline void
float_to_u32_x1 (const Babl    *conversion,
                 unsigned char *src_char, 
                 unsigned char *dst_char, 
                 long           samples)
{
  float *src    = (float *)src_char;
  uint32_t *dst = (uint32_t *)dst_char;
  long n = samples;
  while (n--)
    {
      double r = src[0];
            
      dst[0] = (r >= 1.0f) ? 0xFFFFFFFF : ((r <= 0.0f) ? 0x0 : 0xFFFFFFFF * r + 0.5f);
      
      dst += 1;
      src += 1;
    }
}
static void
float_to_u32_x2 (const Babl    *conversion,
                 unsigned char *src_char, 
                 unsigned char *dst_char, 
                 long           samples)
{
  float_to_u32_x1 (conversion, src_char, dst_char, samples * 2);
}
static void
float_to_u32_x3 (const Babl    *conversion,
                 unsigned char *src_char, 
                 unsigned char *dst_char, 
                 long           samples)
{
  float_to_u32_x1 (conversion, src_char, dst_char, samples * 3);
}
static void
float_to_u32_x4 (const Babl    *conversion,
                 unsigned char *src_char, 
                 unsigned char *dst_char, 
                 long           samples)
{
  float_to_u32_x1 (conversion, src_char, dst_char, samples * 4);
}


static inline void
u32_to_float (const Babl    *conversion,
              unsigned char *src_char, 
              unsigned char *dst_char, 
              long           samples)
{
  uint32_t *src = (uint32_t *)src_char;
  float *dst    = (float *)dst_char;
  long n = samples;
  while (n--)
    {
      dst[0] = src[0] / 4294967295.0;
      dst ++;
      src ++;
    }
}

static void
u32_to_float_x4 (const Babl    *conversion,
                 unsigned char *src_char, 
                 unsigned char *dst_char, 
                 long           samples)
{
  u32_to_float (conversion, src_char, dst_char, samples * 4);
}

static void
u32_to_float_x3 (const Babl    *conversion,
                 unsigned char *src_char, 
                 unsigned char *dst_char, 
                 long           samples)
{
  u32_to_float (conversion, src_char, dst_char, samples * 3);
}


static void
u32_to_float_x2 (const Babl    *conversion,
                 unsigned char *src_char, 
                 unsigned char *dst_char, 
                 long           samples)
{
  u32_to_float (conversion, src_char, dst_char, samples * 2);
}


static inline void
u16_to_float (const Babl    *conversion,
              unsigned char *src_char, 
              unsigned char *dst_char, 
              long           samples)
{
  uint16_t *src = (uint16_t *)src_char;
  float *dst    = (float *)dst_char;
  long n = samples;
  while (n--)
    {
      dst[0] = src[0] / 65535.0f;
      dst ++;
      src ++;
    }
}

static void
u16_to_float_x4 (const Babl    *conversion,
                 unsigned char *src_char, 
                 unsigned char *dst_char, 
                 long           samples)
{
  u16_to_float (conversion, src_char, dst_char, samples * 4);
}

static void
u16_to_float_x3 (const Babl    *conversion,
                 unsigned char *src_char, 
                 unsigned char *dst_char, 
                 long           samples)
{
  u16_to_float (conversion, src_char, dst_char, samples * 3);
}


static void
u16_to_float_x2 (const Babl    *conversion,
                 unsigned char *src_char, 
                 unsigned char *dst_char, 
                 long           samples)
{
  u16_to_float (conversion, src_char, dst_char, samples * 2);
}

static inline void
yau16_rgbaf (const Babl    *conversion,
             unsigned char *src_char, 
             unsigned char *dst_char, 
             long           samples)
{
  uint16_t *src = (uint16_t *)src_char;
  float *dst    = (float *)dst_char;
  long n = samples;
  while (n--)
    {
      dst[0] = src[0] / 65535.0f;
      dst[1] = src[0] / 65535.0f;
      dst[2] = src[0] / 65535.0f;
      dst[3] = src[1] / 65535.0f;
      dst +=4;
      src +=2;
    }
}


int
init (void)
{
  /* float and u8 */
  babl_conversion_new (babl_format ("R'G'B'A float"),
                       babl_format ("R'G'B'A u8"),
                      "linear", 
                       float_to_u8_x4,
                       NULL);
  babl_conversion_new (babl_format ("RGBA float"),
                       babl_format ("RGBA u8"),
                      "linear", 
                       float_to_u8_x4,
                       NULL);
  babl_conversion_new (babl_format ("R'G'B' float"),
                       babl_format ("R'G'B' u8"),
                      "linear", 
                       float_to_u8_x3,
                       NULL);
  babl_conversion_new (babl_format ("RGB float"),
                       babl_format ("RGB u8"),
                      "linear", 
                       float_to_u8_x3,
                       NULL);
  babl_conversion_new (babl_format ("Y'A float"),
                       babl_format ("Y'A u8"),
                      "linear", 
                       float_to_u8_x2,
                       NULL);
  babl_conversion_new (babl_format ("YA float"),
                       babl_format ("YA u8"),
                      "linear", 
                       float_to_u8_x2,
                       NULL);
  babl_conversion_new (babl_format ("YA float"),
                       babl_format ("YA u8"),
                      "linear", 
                       float_to_u8_x2,
                       NULL);
  babl_conversion_new (babl_format ("Y' float"),
                       babl_format ("Y' u8"),
                      "linear", 
                       float_to_u8_x1,
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
                       float_to_u16_x4,
                       NULL);
  babl_conversion_new (babl_format ("RGBA float"),
                       babl_format ("RGBA u16"),
                      "linear", 
                       float_to_u16_x4,
                       NULL);

  babl_conversion_new (babl_format ("R'G'B' float"),
                       babl_format ("R'G'B' u16"),
                      "linear", 
                       float_to_u16_x3,
                       NULL);
  babl_conversion_new (babl_format ("RGB float"),
                       babl_format ("RGB u16"),
                      "linear", 
                       float_to_u16_x3,
                       NULL);
  babl_conversion_new (babl_format ("Y'A float"),
                       babl_format ("Y'A u16"),
                      "linear", 
                       float_to_u16_x2,
                       NULL);
  babl_conversion_new (babl_format ("YA float"),
                       babl_format ("YA u16"),
                      "linear", 
                       float_to_u16_x2,
                       NULL);
  babl_conversion_new (babl_format ("Y' float"),
                       babl_format ("Y' u16"),
                      "linear", 
                       float_to_u16_x1,
                       NULL);
  babl_conversion_new (babl_format ("Y float"),
                       babl_format ("Y u16"),
                      "linear", 
                       float_to_u16_x1,
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
                       float_to_u32_x4,
                       NULL);
  babl_conversion_new (babl_format ("RGBA float"),
                       babl_format ("RGBA u32"),
                      "linear", 
                       float_to_u32_x4,
                       NULL);
  babl_conversion_new (babl_format ("R'G'B' float"),
                       babl_format ("R'G'B' u32"),
                      "linear", 
                       float_to_u32_x3,
                       NULL);
  babl_conversion_new (babl_format ("RGB float"),
                       babl_format ("RGB u32"),
                      "linear", 
                       float_to_u32_x3,
                       NULL);
  babl_conversion_new (babl_format ("Y'A float"),
                       babl_format ("Y'A u32"),
                      "linear", 
                       float_to_u32_x2,
                       NULL);
  babl_conversion_new (babl_format ("YA float"),
                       babl_format ("YA u32"),
                      "linear", 
                       float_to_u32_x2,
                       NULL);
  babl_conversion_new (babl_format ("Y' float"),
                       babl_format ("Y' u32"),
                      "linear", 
                       float_to_u32_x1,
                       NULL);
  babl_conversion_new (babl_format ("Y float"),
                       babl_format ("Y u32"),
                      "linear", 
                       float_to_u32_x1,
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

  babl_conversion_new (babl_format ("RGB u32"),
                       babl_format ("RGB float"),
                      "linear", 
                       u32_to_float_x3,
                       NULL);
  babl_conversion_new (babl_format ("R'G'B' u32"),
                       babl_format ("R'G'B' float"),
                      "linear", 
                       u32_to_float_x3,
                       NULL);



  babl_conversion_new (babl_format ("YA u16"),
                       babl_format ("YA float"),
                      "linear", 
                       u16_to_float_x2,
                       NULL);
  babl_conversion_new (babl_format ("Y'A u16"),
                       babl_format ("Y'A float"),
                      "linear", 
                       u16_to_float_x2,
                       NULL);
  babl_conversion_new (babl_format ("Y u16"),
                       babl_format ("Y float"),
                      "linear", 
                       u16_to_float,
                       NULL);
  babl_conversion_new (babl_format ("Y' u16"),
                       babl_format ("Y' float"),
                      "linear", 
                       u16_to_float,
                       NULL);
  babl_conversion_new (babl_format ("RGBA u16"),
                       babl_format ("RGBA float"),
                      "linear", 
                       u16_to_float_x4,
                       NULL);
  babl_conversion_new (babl_format ("R'G'B'A u16"),
                       babl_format ("R'G'B'A float"),
                      "linear", 
                       u16_to_float_x4,
                       NULL);

  babl_conversion_new (babl_format ("RGB u16"),
                       babl_format ("RGB float"),
                      "linear", 
                       u16_to_float_x3,
                       NULL);
  babl_conversion_new (babl_format ("R'G'B' u16"),
                       babl_format ("R'G'B' float"),
                      "linear", 
                       u16_to_float_x3,
                       NULL);
  babl_conversion_new (babl_format ("Y'A u16"),
                       babl_format ("R'G'B'A float"),
                      "linear", 
                       yau16_rgbaf,
                       NULL);
  return 0;
}
