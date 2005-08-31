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


#ifndef _DB_H
#define _DB_H

#ifndef _BABL_INTERNAL_H
#error  babl-db.h is only to be included after babl-internal.h
#endif

#include <string.h>

#define DB_DEF             static inline

#ifndef DB_INITIAL_SIZE
#define DB_INITIAL_SIZE    16 
#endif
#ifndef DB_INCREMENT_SIZE
#define DB_INCREMENT_SIZE  16
#endif

static inline int hash (char *str)
{
  int ret = 0;
  int i   = 1;

  while (*str)
    ret = (ret + ( i++ * (*str ++ & 31 ))) % 199;
  return ret;
}

/* file scope variables, for this .c file's database */
static int    db_size     = 0;
static int    db_entries  = 0;
static Babl **db;

DB_DEF void   db_init    (void);
DB_DEF void   db_destroy (void);
DB_DEF void   db_each    (BablEachFunction  each_fun,
                          void             *user_data);
DB_DEF Babl * db_insert  (Babl  *entry);
DB_DEF Babl * db_exist   (int    id,
                          const char *name);

DB_DEF Babl * db_find    (const char *name)
{
  Babl *ret = db_exist (0, name);

  if (!ret)
    {
      const char *sample_type = "unknwon";

      if (db[0])
        sample_type = babl_class_name (db[0]->class_type);
      babl_log ("failed (query performed on a %s database)", sample_type);
    }
  return ret;
}


DB_DEF void
db_init(void)
{
  if (0==db_size)
    {
      db_size = DB_INITIAL_SIZE;
      db = NULL;
      if (db_size)
        {
          db = babl_calloc (sizeof (BablInstance*), db_size);
        }
    }
}


DB_DEF void
db_destroy (void)
{
  babl_free (db);
  db_size    = 0;
  db_entries = 0;
  db         = NULL;
}

DB_DEF Babl *
db_insert (Babl *babl)
{
  Babl *collision;

  if (!babl)
    {
      babl_log ("%s(babl=%p)", __FUNCTION__, babl);
      return NULL;
    }

  collision = db_exist (babl->instance.id, babl->instance.name);

  if (collision)
     return collision;

  if (db_entries + 1 > db_size)     /* must reallocate storage */
    {
      Babl **new_db;

      new_db = babl_realloc (db, (db_size + DB_INCREMENT_SIZE) * sizeof (BablInstance*));
      if (!new_db)
        {
          babl_log ("unable to reallocate memory, element (%i:'%s') not inserted",
                    babl->instance.id, babl->instance.name);
          return NULL;
        }
      db = new_db;

      /* null out the currently unused portions of memory */
      memset (db + db_size, 0, DB_INCREMENT_SIZE * sizeof (Babl*));
      db_size += DB_INCREMENT_SIZE;
    }
  babl->instance.creator = babl_extender ();
  db[db_entries++]=babl;
  return babl;
}


DB_DEF void
db_each (BablEachFunction  each_fun,
         void             *user_data)
{
  int i;

  for (i=0; i< db_entries; i++)
    {
      if (db[i])
        {
          if (each_fun ((Babl*) db[i], user_data))
            break;
        }
    }
}

typedef struct BablDbExistData
{
  unsigned int  type;
  int           id;
  const char   *name;
  Babl         *ret;
} BablDbExistData;

DB_DEF int 
db_each_exist (Babl *babl,
               void *void_data)
{
  BablDbExistData *data = void_data;

  if (data->id && data->id == babl->instance.id)
    {
          data->ret = babl;
          return 1; /* stop iterating */
    }
  else if (data->name && !strcmp (babl->instance.name, data->name))
    {
      data->ret = babl;
      return 1; /* stop iterating */
    }
  return 0;  /* continue iterating */
}

DB_DEF Babl * 
db_exist (int         id,
          const char *name)
{
  BablDbExistData data;

  data.id = id;
  data.name = name;
  data.ret = NULL;

  db_each (db_each_exist, &data);

  return data.ret;
}

#endif
