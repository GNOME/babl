
#include "babl/babl-introspect.h"

#define CHECK_CONV(test_name, componenttype, src_fmt, dst_fmt, src_pix, expected_pix) \
  {       \
  const Babl *fish;       \
  int i;       \
  fish = babl_fish (src_fmt, dst_fmt);       \
  if (!fish)       \
    {       \
      printf ("  %s failed to make fish\n", test_name);       \
      OK = 0;       \
    }       \
  for (i = 0; i < sizeof(src_pix)/sizeof(src_pix[0]); i ++)       \
    {       \
      int c;\
      componenttype result[10];       \
      babl_process (fish, src_pix[i], result, 1);       \
      for (c = 0; c < sizeof(expected_pix[i])/sizeof(expected_pix[i][0]); c++) \
      if (result[c] != expected_pix[i][c])       \
        {       \
          printf (" %s failed #%i[%i]  got %i expected %i\n", test_name, i, c, result[c], expected_pix[i][c]);       \
          OK = 0;          \
          babl_introspect((Babl *)fish); \
        }       \
    }       \
  }
