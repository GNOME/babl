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

#include <string.h>
#include <stdarg.h>

#include "babl-internal.h"
#include "babl-image.h"
#include "babl-type.h"
#include "babl-component.h"

#define BABL_MAX_BANDS 32

static BablImage *db[100]={NULL,};

#if 0
static int 
each_babl_image_destroy (Babl *babl,
                         void *data)
{
  babl_free (babl->image.component);
  babl_free (babl->image.pitch);
  babl_free (babl->image.stride);
  babl_free (babl);

  return 0;  /* continue iterating */
}
#endif

static BablImage *
image_new (int             bands,
           BablComponent **component,
           void          **data,
           int            *pitch,
           int            *stride)
{
  BablImage *self;
  int        band;

  self                = babl_calloc (sizeof (BablImage), 1);

  self->instance.type = BABL_IMAGE;
  self->instance.id   = 0;
  self->instance.name = "babl image";

  self->bands         = bands;

  self->component     = babl_malloc (sizeof (BablComponent*) * (bands+1));
  self->data          = babl_malloc (sizeof (void*)          * (bands+1));
  self->pitch         = babl_malloc (sizeof (int)            * (bands+1));
  self->stride        = babl_malloc (sizeof (int)            * (bands+1));

  for (band=0; band < bands; band++)
    {
      self->component[band] = component[band];
      self->data[band]      = data[band];
      self->pitch[band]     = pitch[band];
      self->stride[band]    = stride[band];
    }
  self->component[band] = NULL;
  self->data[band]      = NULL;
  self->pitch[band]     = 0;
  self->stride[band]    = 0;

  return self;
}

BablImage *
babl_image_new (void *first,
                ...)
{
  va_list        varg;
  BablImage      *self;
  int            bands     = 0;
  BablComponent *component [BABL_MAX_BANDS];
  void          *data      [BABL_MAX_BANDS];
  int            pitch     [BABL_MAX_BANDS];
  int            stride    [BABL_MAX_BANDS];

  const char      *arg = first;

  va_start (varg, first);

  while (1)
    {
      BablComponent *new_component = NULL;
      if (!arg)
        break;

      if (BABL_IS_BABL (arg))
        {
          Babl *babl = (Babl*)arg;

          if (babl->instance.type == BABL_COMPONENT)
            {
              new_component = (BablComponent *)babl;
            }
          else
            {
                babl_log ("%s(): %s unexpected",
                          __FUNCTION__, babl_class_name (babl->instance.type));
                return NULL;
            }
        }
      else
        {
          new_component = babl_component (arg);
        }

      component [bands] = new_component;
      data      [bands] = va_arg (varg, void*);
      pitch     [bands] = va_arg (varg, int);
      stride    [bands] = va_arg (varg, int);
      bands++;
                
      if (bands>=BABL_MAX_BANDS)
        {
          babl_log ("%s(): maximum number of bands (%i) exceeded for BablImage",
                    __FUNCTION__, BABL_MAX_BANDS);
        }

      arg = va_arg (varg, char *);
    }
    
  va_end   (varg);


  self = image_new (bands, component, data, pitch, stride);

  return self;
}

void
babl_image_each (BablEachFunction  each_fun,
                    void             *user_data)
{
  int i;
  return;

  while (db[i])
    {
      if (each_fun ((Babl *) (db[i]), user_data))
        {
          return;
        }
      else
        {
          i++;
        }
    }
}


void
babl_image_destroy (void)
{
}

void
babl_image_init (void)
{
}
