/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2005 Øyvind Kolås
 *               2013 Daniel Sabo
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

#include <stdlib.h>
#include <stdio.h>

#ifndef HAVE_SRANDOM
#define srandom srand
#define random  rand
#endif

#define BABL_PATH_NUM_TEST_PIXELS       3072
#define BABL_CONVERSION_NUM_TEST_PIXELS 128
#define BABL_FROMAT_NUM_TEST_PIXELS     256
#define BABL_MODEL_NUM_TEST_PIXELS      512
#define BABL_TYPE_NUM_TEST_PIXELS       512

#define BABL_COMPONENT_FMT_STR          "%.13f"
#define BABL_PIXEL_FMT_STR              BABL_COMPONENT_FMT_STR ", " \
                                        BABL_COMPONENT_FMT_STR ", " \
                                        BABL_COMPONENT_FMT_STR ", " \
                                        BABL_COMPONENT_FMT_STR

static double rand_double (void)
{
  return (double) random () / RAND_MAX;
}

static double rand_range_double (double min, double max)
{
  return rand_double () * (max - min) + min;
}

static void gen_path_pixels (void)
{
  int i;
  srandom (20050728);

  printf ("static const int babl_num_path_test_pixels = %d;\n\n", BABL_PATH_NUM_TEST_PIXELS);

  printf ("static const double babl_path_test_pixels[%d] = {\n", BABL_PATH_NUM_TEST_PIXELS * 4);

  /*  add 256 pixels in the valid range between 0.0 and 1.0  */
  for (i = 0; i < 256; i++)
    {
      printf (BABL_PIXEL_FMT_STR ",\n",
        rand_double (),
        rand_double (),
        rand_double (),
        rand_double ());
    }

  /*  add 16 pixels between -1.0 and 0.0  */
  for (i = 0; i < 16; i++)
    {
      printf (BABL_PIXEL_FMT_STR ",\n",
        rand_range_double (-1.0, 0.0),
        rand_range_double (-1.0, 0.0),
        rand_range_double (-1.0, 0.0),
        rand_range_double (-1.0, 0.0));
    }

  /*  add 16 pixels between 1.0 and 2.0  */
  for (i = 0; i < 16; i++)
    {
      printf (BABL_PIXEL_FMT_STR ",\n",
        rand_range_double (1.0, 2.0),
        rand_range_double (1.0, 2.0),
        rand_range_double (1.0, 2.0),
        rand_range_double (1.0, 2.0));
    }

  /*  add 16 pixels with an alpha of 0.0, to penalize conversions that
   *  destroy color information of fully-transparent pixels, when
   *  relevant.  see bug #780016.
   */
  for (i = 0; i < 16; i++)
    {
      printf (BABL_PIXEL_FMT_STR ",\n",
        rand_double (),
        rand_double (),
        rand_double (),
        0.0);
    }

  for (i = 304; i < BABL_PATH_NUM_TEST_PIXELS; i++)
    {
      printf (BABL_PIXEL_FMT_STR ",\n",
        rand_double (),
        rand_double (),
        rand_double (),
        rand_double ());
    }

  printf ("};\n\n");

  printf ("static const int babl_num_conversion_test_pixels = %d;\n\n", BABL_CONVERSION_NUM_TEST_PIXELS);

  printf ("static const double *babl_conversion_test_pixels = babl_path_test_pixels;\n\n");

  printf ("static const int babl_num_format_test_pixels = %d;\n\n", BABL_FROMAT_NUM_TEST_PIXELS);

  printf ("static const double *babl_format_test_pixels = babl_path_test_pixels;\n\n");
}

static void gen_model_pixels (void)
{
  int i;
  srandom (20050728);

  printf ("static const int babl_num_model_test_pixels = %d;\n\n", BABL_MODEL_NUM_TEST_PIXELS);

  printf ("static const double babl_model_test_pixels[%d] = {\n", BABL_MODEL_NUM_TEST_PIXELS * 4);

  /*  add 128 pixels in the valid range between 0.0 and 1.0  */
  for (i = 0; i < BABL_MODEL_NUM_TEST_PIXELS; i++)
    {
      printf (BABL_PIXEL_FMT_STR ",\n",
        rand_range_double (-0.2, 1.2),
        rand_range_double (-0.2, 1.2),
        rand_range_double (-0.2, 1.2),
        rand_range_double (-0.2, 1.2));
    }

  printf ("};\n\n");
}

static void gen_type_pixels (void)
{
  int i;
  srandom (20050728);

  printf ("static const int babl_num_type_test_pixels = %d;\n\n", BABL_TYPE_NUM_TEST_PIXELS);

  printf ("static const double babl_type_test_pixels[%d] = {\n", BABL_TYPE_NUM_TEST_PIXELS * 4);

  /*  add 128 pixels in the valid range between 0.0 and 1.0  */
  for (i = 0; i < BABL_MODEL_NUM_TEST_PIXELS; i++)
    {
      printf (BABL_PIXEL_FMT_STR ",\n",
        rand_range_double (0.0, 128.0),
        rand_range_double (0.0, 128.0),
        rand_range_double (0.0, 128.0),
        rand_range_double (0.0, 128.0));
    }

  printf ("};\n\n");
}

int main (int argc, char **argv)
{
  printf (
    "/* babl - dynamically extendable universal pixel conversion library.\n"
    " * Copyright (C) 2005 Øyvind Kolås\n"
    " *               2013 Daniel Sabo\n"
    " *\n"
    " * This library is free software; you can redistribute it and/or\n"
    " * modify it under the terms of the GNU Lesser General Public\n"
    " * License as published by the Free Software Foundation; either\n"
    " * version 3 of the License, or (at your option) any later version.\n"
    " *\n"
    " * This library is distributed in the hope that it will be useful,\n"
    " * but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
    " * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU\n"
    " * Lesser General Public License for more details.\n"
    " *\n"
    " * You should have received a copy of the GNU Lesser General\n"
    " * Public License along with this library; if not, see\n"
    " * <https://www.gnu.org/licenses/>.\n"
    " */\n"
    "\n");

  printf ("/* THIS IS A GENERATED FILE - DO NOT EDIT */\n\n");

  gen_path_pixels ();
  gen_model_pixels ();
  gen_type_pixels ();

  return 0;
}
