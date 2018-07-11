/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2005, Øyvind Kolås.
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

/* Verify that conversion of fully-transparent pixels preserves color
 * information, when relevant for the source and destination formats.
 * In particular, make sure that the conversion path doesn't pass
 * through a format with premultiplied alpha.  See bug #780016.
 *
 * Note that this test can, and will, result in false positives; i.e.,
 * if the test passes, it means nothing -- we only watch for the
 * occasional failure.  Fun, right?
 */

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "babl-internal.h"

#include "common.inc"


#define NUMBER_OF_TRIALS 50

static void
clear_fish_db (void)
{
  /* so ugly... */
  *babl_fish_db () = *babl_db_init ();
}

int
main (int    argc,
      char **argv)
{
  int OK = 1;
  int i;

  babl_init ();

  for (i = 0; OK && i < NUMBER_OF_TRIALS; i++)
    {
      clear_fish_db ();

      {
        uint16_t in [][4] = {{0xffff, 0xffff, 0xffff, 0}};
        float    out[][4] = {{1.0,    1.0,    1.0,    0.0}};

        /* this conversion is known to have been problematic.
         * see bug #780016.
         */
        CHECK_CONV_FLOAT ("u16' -> float", float, 0.001,
                          babl_format("R'G'B'A u16"),
                          babl_format("RGBA float"),
                          in, out);
      }
    }

  /* be nice and don't overwrite the fish cache, since we cleared all the
   * fishes.
   */
  /* babl_exit (); */

  return !OK;
}
