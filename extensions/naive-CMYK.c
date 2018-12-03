/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2005, 2018 Øyvind Kolås.
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
 *
 * this is an interim file, to keep babl from complaining about double
 * registration of cmyk formats.
 */

#include "config.h"
#include <math.h>
#include <string.h>

#include "babl.h"
#include "base/util.h"

int init (void);

/* implement a hacky way to do cmyk with cairo  */
/* CMYKA                                        */
/* CMKA                                         */
/* CKYA                                         */


int
init (void)
{
  return 0;
}

