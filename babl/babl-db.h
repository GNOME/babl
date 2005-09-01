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

typedef struct _BablDb BablDb;

BablDb * babl_db_init    (void);
void     babl_db_destroy (BablDb           *db);
void     babl_db_each    (BablDb           *db, 
                          BablEachFunction  each_fun,
                          void             *user_data);
Babl   * babl_db_insert  (BablDb           *db,
                          Babl             *entry);
Babl   * babl_db_exist   (BablDb           *db,
                          int               id,
                          const char       *name);
Babl   * babl_db_find    (BablDb           *db,
                          const char       *name);

#endif
