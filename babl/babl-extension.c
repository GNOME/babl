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

#define BABL_DYNAMIC_EXTENSIONS

#define BABL_INIT_HOOK     init_hook();
#define BABL_DESTROY_HOOK  destroy_hook();

#ifdef BABL_DYNAMIC_EXTENSIONS
        /* must be defined before inclusion of babl-internal.h */ 
#undef  BABL_INIT_HOOK
#define BABL_INIT_HOOK  init_hook();dynamic_init_hook();
#endif

#include "babl-internal.h"
#include "babl-db.h"
#include "babl-base.h"
#include <string.h>
#include <stdarg.h>


static int   each_babl_extension_destroy (Babl *babl, void *data);

Babl *babl_extension_current_extender = NULL;

Babl *
babl_extender (void)
{
  if (babl_extension_current_extender)
    return babl_extension_current_extender;
  return NULL;
}

static void
babl_set_extender (Babl *new_extender)
{
  babl_extension_current_extender = new_extender;
}

static Babl *
extension_new (const char *path,
               void       *dl_handle,
               void      (*destroy) (void))
{
  Babl *babl;

  babl                       = babl_malloc (sizeof (BablExtension) + strlen (path) + 1);
  babl->instance.name        = (void *) babl + sizeof (BablExtension);
  strcpy (babl->instance.name, path);
  babl->instance.id          = 0;
  babl->class_type           = BABL_EXTENSION;
  babl->extension.dl_handle  = dl_handle;
  babl->extension.destroy    = destroy;

  return babl;
}

static Babl *babl_quiet = NULL;

Babl *
babl_extension_quiet_log (void)
{
  if (babl_quiet)
    return babl_quiet;
  babl_quiet = extension_new ("", NULL, NULL);
  db_insert (babl_quiet);
  return babl_quiet;
}

Babl *
babl_extension_base (void)
{
  Babl *babl;
  void *dl_handle        = NULL;
  void (*destroy) (void) = NULL;

  babl = extension_new ("BablBase",
                        dl_handle,
                        destroy);
  babl_set_extender (babl);
  babl_base_init ();

  if (db_insert (babl) == babl)
    {
      babl_set_extender (NULL);
      return babl;
    }
  else
    {
      each_babl_extension_destroy (babl, NULL);
      babl_set_extender (NULL);
      return NULL;
    }
}

static void
init_hook (void)
{
  babl_extension_quiet_log ();
  babl_set_extender (NULL);
}

static void
destroy_hook (void)
{
  babl_quiet=NULL;
}

#ifdef BABL_DYNAMIC_EXTENSIONS

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>

#include <dlfcn.h>
#ifndef RTLD_NOW
#define RTLD_NOW 0
#endif


static Babl *
load_failed (Babl *babl)
{
  if (babl)
    {
      each_babl_extension_destroy (babl, NULL);
    }
  babl_set_extender (NULL);
  return NULL;
}

Babl *
babl_extension_load (const char *path)
{
  Babl *babl             = NULL;

  /* do the actual loading thing */
  void *dl_handle        = NULL;
  int  (*init)    (void) = NULL;
  void (*destroy) (void) = NULL;
 

  dl_handle = dlopen (path, RTLD_NOW);
  if (!dl_handle)
    {
      babl_log ("dlopen() failed:\n\t%s", dlerror ());
      return load_failed (babl);
    }
  init    = dlsym (dl_handle, "init");
  if (!init)
    {
      babl_log ("\n\tint babl_extension_init() function not found in extenstion '%s'", path);
      return load_failed (babl);
    }
 
  destroy = dlsym (dl_handle, "destroy");
  babl = extension_new (path,
                        dl_handle,
                        destroy);

  babl_set_extender (babl);
  if(init())
    {
      babl_log ("babl_extension_init() in extension '%s' failed (return!=0)", path);
      return load_failed (babl);
    }


  if (db_insert (babl) == babl)
    {
      babl_set_extender (NULL);
      return babl;
    }
  else
    {
      return load_failed (babl);
    }
}

static void
dynamic_init_hook (void)
{
  babl_extension_load ("/home/pippin/.babl/naive-CMYK.so");
  babl_extension_load ("/home/pippin/.babl/CIE-Lab.so");
}

#endif

static int 
each_babl_extension_destroy (Babl *babl,
                             void *data)
{
  if (babl->extension.destroy)
    babl->extension.destroy();
#ifdef BABL_DYNAMIC_EXTENSIONS
  if (babl->extension.dl_handle)
    dlclose (babl->extension.dl_handle);
#endif

  babl_free (babl);
  return 0;  /* continue iterating */
}


BABL_CLASS_TEMPLATE (babl_extension)
