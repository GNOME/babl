/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2005, 2017 Øyvind Kolås.
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

#include "config.h"
#include <math.h>
#include "babl-internal.h"

static int
test2 (void)
{
  int OK = 1;
  const Babl *sRGB = babl_space ("sRGB");
  const Babl *fmt;

  fmt = babl_format ("R'G'B' u8");
  if (babl_format_get_space (fmt) != sRGB)
  {
    babl_log ("created space %s doesn't have sRGB when it should", babl_get_name (fmt));
    OK = 0;
  }
  fmt = babl_format_with_space ("R'G'B' u8", NULL);
  if (babl_format_get_space (fmt) != sRGB)
  {
    babl_log ("created space %s doesn't have sRGB when it should", babl_get_name (fmt));
    OK = 0;
  }
  fmt = babl_format_with_space ("R'G'B' u8", sRGB);
  if (babl_format_get_space (fmt) != sRGB)
  {
    babl_log ("created space %s doesn't have sRGB when it should", babl_get_name (fmt));
    OK = 0;
  }
  fmt = babl_format_with_space ("CIE Lab float", sRGB);
  if (babl_format_get_space (fmt) != sRGB)
  {
    babl_log ("created space %s doesn't have sRGB when it should", babl_get_name (fmt));
    OK = 0;
  }

  if (!OK)
    return -1;
  return 0;
}

static int
test3 (void)
{
  int OK = 1;
  const Babl *apple  = babl_space ("Apple");
  const Babl *sRGB = babl_space ("sRGB");
  const Babl *fmt;

  fmt = babl_format ("R'G'B' u8");
  if (babl_format_get_space (fmt) != sRGB)
  {
    babl_log ("created space %s doesn't have sRGB when it should", babl_get_name (fmt));
    OK = 0;
  }
  fmt = babl_format_with_space ("R'G'B' u8", NULL);
  if (babl_format_get_space (fmt) != sRGB)
  {
    babl_log ("created space %s doesn't have sRGB when it should", babl_get_name (fmt));
    OK = 0;
  }
  fmt = babl_format_with_space ("R'G'B' u8", apple);
  if (babl_format_get_space (fmt) != apple)
  {
    babl_log ("created space %s doesn't have apple when it should", babl_get_name (fmt));
    OK = 0;
  }
  fmt = babl_format_with_space ("CIE Lab float", apple);
  if (babl_format_get_space (fmt) != apple)
  {
    babl_log ("created space %s doesn't have apple when it should", babl_get_name (fmt));
    OK = 0;
  }

  if (!OK)
    return -1;
  return 0;
}

int
main (int    argc,
      char **argv)
{
  babl_init ();
  if (test2 ())
    return -1;
  if (test3 ())
    return -1;
  babl_exit ();
  return 0;
}
