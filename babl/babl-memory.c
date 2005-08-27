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

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "babl-internal.h"

static char *signature = "So long and thanks for all the fish.";

typedef struct
{
  char   *signature;
  size_t  size;
} BablAllocInfo;

#define OFFSET   (sizeof(BablAllocInfo))

#define BAI(ptr)    ((BablAllocInfo*)(((void*)ptr)-OFFSET))
#define IS_BAI(ptr) (BAI(ptr)->signature == signature)

static int mallocs  = 0;
static int frees    = 0;
static int strdups  = 0;
static int reallocs = 0;
static int callocs  = 0;
static int dups     = 0;

static const char *
mem_stats (void)
{
  static char buf[128];
  sprintf (buf, "mallocs:%i callocs:%i strdups:%i dups:%i allocs:%i frees:%i reallocs:%i\t|",
    mallocs, callocs, strdups, dups, mallocs+callocs+strdups+dups, frees, reallocs);
  return buf;
}

void *
babl_malloc (size_t size)
{
  void *ret;

  assert (size); 
  ret = malloc (size + OFFSET);
  if (!ret)
    babl_log ("args=(%i): failed",  size);

  BAI(ret + OFFSET)->signature = signature;
  BAI(ret + OFFSET)->size      = size;
  mallocs++;
  return ret + OFFSET;
}

char *
babl_strdup (const char *s)
{
  char *ret;

  ret = babl_malloc (strlen (s)+1);
  if (!ret)
    babl_log ("args=(%s): failed",  s);
  strcpy (ret, s); 

  strdups++;
  mallocs--;
  return ret;
}

void *
babl_dup (void *ptr)
{
  void *ret;
 
  assert (IS_BAI (ptr));

  ret = babl_malloc (BAI(ptr)->size);
  memcpy (ret, ptr, BAI(ptr)->size);

  dups++;
  mallocs--;
  return NULL;
}

void
babl_free (void *ptr)
{
  if (!ptr)
    return;
  assert(IS_BAI(ptr));
  free (BAI(ptr));
  frees++;
}

void *
babl_realloc (void   *ptr,
              size_t  size)
{
  void *ret;

  if (!ptr)
    {
      return babl_malloc (size);
    }

  assert (IS_BAI (ptr));
  ret = realloc (BAI(ptr), size + OFFSET);

  if (!ret)
    babl_log ("args=(%p, %i): failed",  ptr, size);
  
  BAI(ret+OFFSET)->signature = signature;
  BAI(ret+OFFSET)->size      = size;

  reallocs++;
  return ret + OFFSET;
}

void *
babl_calloc (size_t nmemb,
             size_t size)
{
  void *ret = babl_malloc (nmemb*size);

  if (!ret)
    babl_log ("args=(%i, %i): failed",  nmemb, size);

  memset (ret, 0, nmemb*size);

  callocs++;
  mallocs--;
  return ret;
}

void
babl_memory_sanity (void)
{
  if (frees != mallocs + strdups + callocs)
    {
      babl_log ("memory usage does not add up!\n"
"%s\n"
"\tbalance: %i-%i=%i\n",
  mem_stats(), (strdups+mallocs+callocs),frees, (strdups+mallocs+callocs)-frees);
    }
}
