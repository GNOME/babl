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
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "babl-memory.h"
#include "babl-internal.h"

static int list_length (void **list)
{
  void **ptr;
  int    len = 0;

  ptr = list;
  while (NULL != *ptr)
    {
      ptr++;
      len++;
    }
  return len;
}

void
babl_add_ptr_to_list (void ***list,
                      void   *new)
{
  int orig_len = 0;

  if (*list)
    {
      orig_len = list_length (*list);
    }

  *list = babl_realloc ((*list),
                        sizeof (void *) * (orig_len + 2));

  if (!(*list))
    {
      babl_fatal ("failed to realloc");
    }

  (*list)[orig_len]     = new;
  (*list)[orig_len + 1] = NULL;
}

void
babl_list_each (void           **list,
                BablEachFunction each_fun,
                void            *user_data)
{
  int i;

  if (!list)
    return;
  for (i = 0; i < list_length (list); i++)
    {
      if (each_fun ((Babl *) list[i], user_data))
        break;
    }
}

#include <sys/time.h>
#include <time.h>

static struct timeval start_time;
static struct timeval measure_time;

#define usecs(time)    ((time.tv_sec - start_time.tv_sec) * 1000000 + time.tv_usec)

static void
init_ticks (void)
{
  static int done = 0;

  if (done)
    return;
  done = 1;
  gettimeofday (&start_time, NULL);
}

long
babl_ticks (void)
{
  init_ticks ();
  gettimeofday (&measure_time, NULL);
  return usecs (measure_time) - usecs (start_time);
}

double
babl_rel_avg_error (double *imgA,
                    double *imgB,
                    long    samples)
{
  double error = 0.0;
  long   i;

  for (i = 0; i < samples; i++)
    error += fabs (imgA[i] - imgB[i]);

  if (error >= 0.000001)
    error /= samples;
  else
    error = 0.0;

  return error;
}

