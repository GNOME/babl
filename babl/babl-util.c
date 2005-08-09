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
#include "babl-memory.h"

static int list_length (void **list)
{
  void **ptr;
  int    len=0;
  
  ptr = list;
  while (NULL!=*ptr)
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
  int orig_len=0;
 
  if (*list)
    {
      orig_len = list_length (*list);
    }

  *list = babl_realloc ( (*list), 
                          sizeof(void *) * (orig_len + 2));

  if (!(*list))
    {
      fprintf (stderr, "%s(): eeek! failed to realloc", __FUNCTION__);
    }

  (*list)[orig_len]=new;
  (*list)[orig_len+1]=NULL;
}

