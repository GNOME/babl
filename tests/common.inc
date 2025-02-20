#include <stdlib.h>
#include <math.h>
#include "babl/babl-introspect.h"

#define CHECK_CONV(test_name, componenttype, src_fmt, dst_fmt, src_pix, expected_pix) \
  {       \
  const Babl *fish = babl_fish (src_fmt, dst_fmt);       \
  if (!fish)       \
    {       \
      printf ("  %s failed to make fish\n", test_name);       \
      OK = 0;       \
    }       \
  for (size_t i = 0; i < sizeof(src_pix)/sizeof(src_pix[0]); i ++)       \
    {       \
      componenttype result[10];       \
      babl_process (fish, src_pix[i], result, 1);       \
      for (size_t c = 0; c < sizeof(expected_pix[i])/sizeof(expected_pix[i][0]); c++) \
      if (result[c] != expected_pix[i][c])       \
        {       \
          printf (" %s failed #%zu[%zu]  got %i expected %i\n", test_name, i, c, result[c], expected_pix[i][c]);       \
          OK = 0;          \
          babl_introspect((Babl *)fish); \
        }       \
    }       \
  }

#define CHECK_CONV_FLOAT(test_name, componenttype, max_error, src_fmt, dst_fmt, src_pix, expected_pix) \
  {       \
  const Babl *fish = babl_fish (src_fmt, dst_fmt);       \
  if (!fish)       \
    {       \
      printf ("  %s failed to make fish\n", test_name);       \
      OK = 0;       \
    }       \
  for (size_t i = 0; i < sizeof(src_pix)/sizeof(src_pix[0]); i ++)       \
    {       \
      componenttype result[10];       \
      babl_process (fish, src_pix[i], result, 1);       \
      for (size_t c = 0; c < sizeof(expected_pix[i])/sizeof(expected_pix[i][0]); c++) \
      if (fabs(result[c] - expected_pix[i][c]) > max_error)       \
        {       \
          printf (" %s failed #%zu[%zu]  got %lf expected %lf\n", test_name, i, c, result[c], expected_pix[i][c]);       \
          OK = 0;          \
          babl_introspect((Babl *)fish); \
        }       \
    }       \
  }
