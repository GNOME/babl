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
 * <http://www.gnu.org/licenses/>.
 */

/* Implementation of list data structure. This is a bit superior
 * to the list implementation in babl-util.c.
 * Copyright (C) 2008, Jan Heller
 *
 * TODO: migrate babl to BablList
 */

#include "babl-internal.h"

#define BABL_LIST_INITIAL_SIZE 0x7F

BablList *
babl_list_init (void)
{
  BablList *list = babl_calloc (sizeof (BablList), 1);

  babl_assert (list);

  list->size = BABL_LIST_INITIAL_SIZE;
  list->count = 0;
  list->items = NULL;
  if (list->size) 
    {
      list->items = babl_calloc (sizeof (BablInstance *), list->size);
    }

  return list;
}

void
babl_list_destroy (BablList *list)
{
    babl_assert (list);

    babl_free (list->items);
    babl_free (list);
}

int
babl_list_size (BablList *list)
{
    babl_assert (list);
    return list->count;
}

void
babl_list_insert (BablList *list,
                  Babl     *item)
{
  babl_assert(list);
  babl_assert(BABL_IS_BABL(item));

  if (list->size < list->count + 1)
    {
        Babl **new_items;

        new_items = babl_realloc (list->items, (list->size * 2) * sizeof (BablInstance *));
        babl_assert (new_items);
        list->items = new_items;
        memset (list->items + list->size, 0, list->size * sizeof (BablInstance *));
        list->size *= 2;
    }
    list->items[list->count++] = item;
}

/* TODO: Rename babl_list_each_temp to babl_list_each after the list migration
 */

void
babl_list_each_temp (BablList         *list,
                     BablEachFunction each_fun,
                     void             *user_data)
{
  int i;

  babl_assert(list);
  babl_assert(each_fun);
 
  for (i = 0; i < list->count; i++)
    {
      if (list->items[i])
        {
          if (each_fun ((Babl *) list->items[i], user_data))
            break;
        }
    }
}

