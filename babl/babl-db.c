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

#define _BABL_DB_C

#include <string.h>
#include "babl-internal.h"

#define HASH_TABLE_SIZE      128
#define DB_INITIAL_SIZE      16
#define DB_INCREMENT_SIZE    16

static inline int hash (const char *str)
{
  int ret = 0;
  int i   = 1;

  while (*str)
    ret = (ret + (i++ * (*str++ & 31))) % (HASH_TABLE_SIZE - 1);
  return ret;
}

typedef struct _BablDb
{
  Babl  *hash [HASH_TABLE_SIZE];
  int    size;
  int    count;
  Babl **items;
} _BablDb;

Babl *
babl_db_find (BablDb     *db,
              const char *name)
{
  Babl *ret = babl_db_exist (db, 0, name);

  if (!ret)
    {
      const char *sample_type = "unknwon";

      if (db->items[0])
        sample_type = babl_class_name (db->items[0]->class_type);
      babl_log ("failed (query performed on a %s database)", sample_type);
    }
  return ret;
}


BablDb *
babl_db_init (void)
{
  BablDb *db = babl_calloc (sizeof (BablDb), 1);

  db->size  = DB_INITIAL_SIZE;
  db->count = 0;
  db->items = NULL;
  if (db->size)
    {
      db->items = babl_calloc (sizeof (BablInstance *), db->size);
    }
  return db;
}


int
babl_db_count (BablDb *db)
{
  return db->count;
}

void
babl_db_destroy (BablDb *db)
{
  babl_free (db->items);
  babl_free (db);
}

Babl *
babl_db_insert (BablDb *db,
                Babl   *item)
{
  Babl *collision;

  babl_assert (db && item);
  babl_assert (item->instance.name);

  collision = babl_db_exist (db, item->instance.id, item->instance.name);

  if (collision)
    return collision;

  if (db->count + 1 > db->size)     /* must reallocate storage */
    {
      Babl **new_items;

      new_items = babl_realloc (db->items, (db->size + DB_INCREMENT_SIZE) * sizeof (BablInstance *));
      babl_assert (new_items);

      db->items = new_items;

      /* null out the currently unused portions of memory */
      memset (db->items + db->size, 0, DB_INCREMENT_SIZE * sizeof (Babl *));
      db->size += DB_INCREMENT_SIZE;
    }

  {
    int key = hash (item->instance.name);
    if (db->hash[key] == NULL)
      db->hash[key] = item;
  }
  db->items[db->count++] = item;

  /* this point all registered items pass through, a nice
  * place to brand them with where the item came from. */
  item->instance.creator = babl_extender ();
  return item;
}

void
babl_db_each (BablDb          *db,
              BablEachFunction each_fun,
              void            *user_data)
{
  int i;

  for (i = 0; i < db->count; i++)
    {
      if (db->items[i])
        {
          if (each_fun ((Babl *) db->items[i], user_data))
            break;
        }
    }
}

typedef struct BablDbExistData
{
  int         id;
  const char *name;
  Babl       *ret;
} BablDbExistData;

static int
babl_db_each_exist (Babl *babl,
                    void *void_data)
{
  BablDbExistData *data = void_data;

  if (data->id && data->id == babl->instance.id)
    {
      data->ret = babl;
      return 1;     /* stop iterating */
    }
  else if (data->name && !strcmp (babl->instance.name, data->name))
    {
      data->ret = babl;
      return 1; /* stop iterating */
    }
  return 0;  /* continue iterating */
}

Babl *
babl_db_exist (BablDb     *db,
               int         id,
               const char *name)
{
  Babl *ret = NULL;

  if (name)
    ret = db->hash[hash (name)];
  if (ret &&
      name[0] == ret->instance.name[0] &&
      !strcmp (name, ret->instance.name))
    return ret;

  {
    BablDbExistData data;

    data.id   = id;
    data.name = name;
    data.ret  = NULL;

    babl_db_each (db, babl_db_each_exist, &data);

    return data.ret;
  }
}
