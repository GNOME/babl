/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2005, Øyvind Kolås.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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
#include "babl-internal.h"

static int mallocs=0;
static int frees=0;
static int strdups=0;
static int reallocs=0;
static int callocs=0;

static const char *
mem_stats (void)
{
  static char buf[128];
  sprintf (buf, "mallocs:%i callocs:%i strdups:%i allocs:%i frees:%i reallocs:%i\t|",
    mallocs, callocs, strdups, mallocs+callocs+strdups, frees, reallocs);
  return buf;
}

void *
babl_malloc (size_t size)
{
  void *ret;
 
  ret = malloc (size);
  if (!ret)
    babl_log ("%s(%i): failed", __FUNCTION__, size);
  mallocs++;
  return ret;
}

char *
babl_strdup (const char *s)
{
  char *ret;
 
  ret = strdup (s);
  if (!ret)
    babl_log ("%s(%s): failed", __FUNCTION__, s);
  strdups++;
  return ret;
}

void
babl_free (void *ptr)
{
  if (!ptr)
    return;
  free (ptr);
  frees++;
}

void *
babl_realloc (void   *ptr,
              size_t  size)
{
  void *ret;
 
  ret = realloc (ptr, size);

  if (!ret)
    babl_log ("%s(%p, %i): failed", __FUNCTION__, ptr, size);
  
  reallocs++;

  if (!ptr)     /* to make the statistics work out */
    mallocs++;

  return ret;
}

void *
babl_calloc (size_t nmemb,
             size_t size)
{
  void *ret = calloc (nmemb, size);

  if (!ret)
    babl_log ("%s(%i, %i): failed", __FUNCTION__, nmemb, size);

  callocs++;
  return ret;
}

void
babl_memory_sanity (void)
{
  if (frees != mallocs + strdups + callocs)
    {
      fprintf (stderr,
"babl_memory usage does not add up!!!!!!!!\n"
"%s\n"
"balance: %i-%i=%i\n",
  mem_stats(), (strdups+mallocs+callocs),frees, (strdups+mallocs+callocs)-frees);
    }
}
