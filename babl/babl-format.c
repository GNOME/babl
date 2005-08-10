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
#include "babl-sampling.h"
#include "babl-type.h"
#include "babl-component.h"
#include "babl-db.h"

#define BABL_MAX_BANDS 32

static int 
each_babl_pixel_format_destroy (Babl *babl,
                                void *data)
{
  babl_free (babl->pixel_format.from);
  babl_free (babl->pixel_format.to);
  babl_free (babl->pixel_format.component);
  babl_free (babl->pixel_format.type);
  babl_free (babl->pixel_format.sampling);
  babl_free (babl->instance.name);
  babl_free (babl);

  return 0;  /* continue iterating */
}

static BablPixelFormat *
pixel_format_new (const char     *name,
                  int             id,
                  int             planar,
                  int             bands,
                  BablComponent **band_component,
                  BablSampling  **band_sampling,
                  BablType      **band_type)
{
  Babl *self;
  int              band;

  self                     = babl_calloc (sizeof (BablPixelFormat), 1);

  self->class_type    = BABL_PIXEL_FORMAT;
  self->instance.id   = id;
  self->instance.name = babl_strdup (name);

  self->pixel_format.bands    = bands;
  self->pixel_format.planar   = planar;

  self->pixel_format.component = babl_malloc (sizeof (BablComponent*) * (bands+1));
  self->pixel_format.type     = babl_malloc (sizeof (BablType*)      * (bands+1));
  self->pixel_format.sampling = babl_malloc (sizeof (BablSampling*)  * (bands+1));

  for (band=0; band < bands; band++)
    {
      self->pixel_format.component[band] = band_component[band];
      self->pixel_format.type[band] = band_type[band];
      self->pixel_format.sampling[band] = band_sampling[band];
    }
  self->pixel_format.component[band] = NULL;
  self->pixel_format.type[band]      = NULL;
  self->pixel_format.sampling[band]  = NULL;

  return (BablPixelFormat*)self;
}

BablPixelFormat *
babl_pixel_format_new (const char *name,
                       ...)
{
  va_list varg;
  BablPixelFormat *self;
  int              id     = 0;
  int              planar = 0;
  int              bands  = 0;
  BablComponent   *band_component [BABL_MAX_BANDS];
  BablSampling    *band_sampling  [BABL_MAX_BANDS];
  BablType        *band_type      [BABL_MAX_BANDS];

  BablSampling    *current_sampling = babl_sampling (1,1);
  BablType        *current_type     = babl_type_id (BABL_U8);
  const char      *arg=name;

  va_start (varg, name);

  
  while (1)
    {
      arg = va_arg (varg, char *);
      if (!arg)
        break;


      if (BABL_IS_BABL (arg))
        {
          Babl *babl = (Babl*)arg;

          switch (babl->class_type)
            {
              case BABL_TYPE:
                current_type = (BablType*) babl;
                break;
              case BABL_COMPONENT:
                band_component [bands] = (BablComponent*) babl;
                band_type      [bands] = current_type;
                band_sampling  [bands] = current_sampling;
                bands++;

                if (bands>=BABL_MAX_BANDS)
                  {
                    babl_log ("maximum number of bands (%i) exceeded for %s",
                              BABL_MAX_BANDS, name);
                  }
                break;
              case BABL_SAMPLING:
                  current_sampling = (BablSampling*)arg;
                  break;
              case BABL_INSTANCE:
              case BABL_MODEL:
                babl_log ("%s(): %s not handled in pixel format yet",
                          __FUNCTION__, babl_class_name (babl->class_type));
                  break;
              case BABL_PIXEL_FORMAT:

              case BABL_CONVERSION:
              case BABL_CONVERSION_TYPE:
              case BABL_CONVERSION_TYPE_PLANAR:
              case BABL_CONVERSION_MODEL_PLANAR:
              case BABL_CONVERSION_PIXEL_FORMAT:
              case BABL_CONVERSION_PIXEL_FORMAT_PLANAR:
              case BABL_FISH:
              case BABL_FISH_REFERENCE:
              case BABL_IMAGE:
                babl_log ("%s(): %s unexpected",
                          __FUNCTION__, babl_class_name (babl->class_type));
                break;
              case BABL_SKY: /* shut up compiler */
                break;
            }
        }
      /* if we didn't point to a babl, we assume arguments to be strings */
      else if (!strcmp (arg, "id"))
        {
          id = va_arg (varg, int);
        }
      
      else if (!strcmp (arg, "packed"))
        {
          planar = 0;
        }
      
      else if (!strcmp (arg, "planar"))
        {
          planar = 1;
        }
      
      else
        {
          babl_log ("%s: unhandled parameter '%s' for pixel_format '%s'",
                    __FUNCTION__, arg, name);
          exit (-1);
        }
    }
    
  va_end   (varg);


  self =pixel_format_new (name, id,
                          planar,
                          bands, band_component, band_sampling, band_type);

  
  if ((BablPixelFormat*) db_insert ((Babl*)self) == self)
    {
      return self;
    }
  else
    {
      each_babl_pixel_format_destroy ((Babl*)self, NULL);
      return NULL;
    }
}

BABL_CLASS_TEMPLATE (BablPixelFormat, babl_pixel_format, "BablPixelFormat")
