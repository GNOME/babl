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
#error  babl-internal.h must be included before babl-db.h, babl-db.h is strictly internal to babl core classes.
#endif

#include "babl-classes.h"

#include <string.h>

#define DB_DEF             static inline
#define DB_INITIAL_SIZE    0
#define DB_INCREMENT_SIZE  16


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
        sample_type = babl_class_name (db[0]->instance.type);
      babl_log ("%s(\"%s\"): failed (query performed on a %s database)",
       __FUNCTION__, name, sample_type);
    }
  return ret;
}


DB_DEF void
db_init(void)
{
  if (0==db_size)
    {
      db_size = DB_INITIAL_SIZE;
      db = babl_malloc (db_size * sizeof (BablInstance*));
      memset (db, 0, db_size * sizeof (BablInstance*));
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
    {
      if (collision->instance.id == babl->instance.id)
        babl_log ("%s: conflicting id: existing(%i:'%s') - new(%i:'%s')",
                __FUNCTION__, collision->instance.id, collision->instance.name,
                babl->instance.id, babl->instance.name);
      else if (!strcmp (collision->instance.name, babl->instance.name))
        babl_log ("%s: conflicting name: existing(%i:'%s') - new(%i:'%s')",
                __FUNCTION__, collision->instance.id, collision->instance.name,
                babl->instance.id, babl->instance.name);
      return collision;
    }

  if (db_entries + 1 > db_size)     /* must reallocate storage */
    {
      Babl **new_db;

      new_db = babl_realloc (db, (db_size + DB_INCREMENT_SIZE) * sizeof (BablInstance*));
      if (!new_db)
        {
          babl_log ("db_insert: unable to reallocate memory, element (%i:'%s') not inserted", babl->instance.id, babl->instance.name);
          return NULL;
        }
      db = new_db;

      /* null out the currently unused portions of memory */
      memset (db + db_size, 0, DB_INCREMENT_SIZE * sizeof (Babl*));
      db_size += DB_INCREMENT_SIZE;
    }
  db[db_entries++]=babl;
  return babl;
}


DB_DEF void
db_each (BablEachFunction  each_fun,
         void      *user_data)
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

typedef struct DbExistData
{
  unsigned int  type;
  int           id;
  const char   *name;
  Babl         *ret;
} DbExistData;

DB_DEF int 
db_each_exist (Babl *babl,
               void *void_data)
{
  DbExistData *data = void_data;

  if (data->id &&
      data->id == babl->instance.id)
    {
      data->ret = babl;
      return 1;
    }
  if (data->name &&
      !strcmp (babl->instance.name, data->name))
    {
      data->ret = babl;
      return 1;
    }
  return 0;  /* continue iterating */
}

DB_DEF Babl * 
db_exist (int         id,
          const char *name)
{
  DbExistData data;

  data.id = id;
  data.name = name;
  data.ret = NULL;

  db_each (db_each_exist, &data);

  return data.ret;
}

#endif
