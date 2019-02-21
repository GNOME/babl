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
 * <https://www.gnu.org/licenses/>.
 */

#include "config.h"
#include <string.h>
#include <stdarg.h>
#include <math.h>
#define NEEDS_BABL_DB
#include "babl-internal.h"
#include "babl-db.h"
#include "babl-ref-pixels.h"

static const Babl *construct_double_format (const Babl *model);

static int
babl_model_destroy (void *data)
{
  Babl *babl = data;
  if (babl->model.from_list)
    babl_free (babl->model.from_list);
  return 0;
}

static char *
babl_model_create_name (int             components,
                        BablComponent **component)
{
  char *p = NULL;

  while (components--)
    {
      p = babl_strcat(p, (*component)->instance.name);
      component++;
    }

  return p;
}

static Babl *
model_new (const char     *name,
           const Babl     *space,
           int             id,
           int             components,
           BablComponent **component,
           BablModelFlag   flags)
{
  Babl *babl;

  babl = babl_malloc (sizeof (BablModel) +
                      sizeof (BablComponent *) * (components) +
                      strlen (name) + 1);
  babl_set_destructor (babl, babl_model_destroy);
  babl->model.component = (void *) (((char *) babl) + sizeof (BablModel));
  babl->instance.name   = (void *) (((char *) babl->model.component) + sizeof (BablComponent *) * (components));

  babl->class_type       = BABL_MODEL;
  babl->instance.id      = id;
  babl->model.components = components;
  babl->model.space      = space;
  babl->model.data       = NULL;
  babl->model.model      = NULL;
  babl->model.flags      = flags;
  strcpy (babl->instance.name, name);
  memcpy (babl->model.component, component, sizeof (BablComponent *) * components);

  babl->model.from_list  = NULL;
  return babl;
}

static int
is_model_duplicate (Babl           *babl, 
                    const Babl     *space, 
                    int             components, 
                    BablComponent **component)
{
  int   i;

  if (babl->model.space != space)
    return 0;

  if (babl->model.components != components)
    return 0;

  for (i = 0; i < components; i++)
    {
      if (babl->model.component[i] != component[i])
        return 0;
    }

  return 1;
}


const Babl *
babl_model_new (void *first_argument,
                ...)
{
  va_list        varg;
  Babl          *babl          = NULL;
  int            id            = 0;
  int            components    = 0;
  const char    *arg           = first_argument;
  const char    *assigned_name = NULL;
  char          *name          = NULL;
  const Babl    *space         = babl_space ("sRGB");
  BablComponent *component [BABL_MAX_COMPONENTS];
  BablModelFlag  flags         = 0;

  va_start (varg, first_argument);

  while (1)
    {
      /* first, we assume arguments to be strings */
      if (!strcmp (arg, "id"))
        {
          id = va_arg (varg, int);
        }

      else if (!strcmp (arg, "name"))
        {
          assigned_name = va_arg (varg, char *);
        }
      else if (!strcmp (arg, "gray"))
        {
          flags |= BABL_MODEL_FLAG_GRAY;
        }
      else if (!strcmp (arg, "CIE"))
        {
          flags |= BABL_MODEL_FLAG_CIE;
        }
      else if (!strcmp (arg, "rgb"))
        {
          flags |= BABL_MODEL_FLAG_RGB;
        }
      else if (!strcmp (arg, "cmyk"))
        {
          flags |= BABL_MODEL_FLAG_CMYK;
        }
      else if (!strcmp (arg, "inverted"))
        {
          flags |= BABL_MODEL_FLAG_INVERTED;
        }
      else if (!strcmp (arg, "premultiplied"))
        {
          flags |= BABL_MODEL_FLAG_PREMULTIPLIED;
        }
      else if (!strcmp (arg, "alpha"))
        {
          flags |= BABL_MODEL_FLAG_ALPHA;
        }
      else if (!strcmp (arg, "linear"))
        {
          flags |= BABL_MODEL_FLAG_LINEAR;
        }
      else if (!strcmp (arg, "nonlinear"))
        {
          flags |= BABL_MODEL_FLAG_NONLINEAR;
        }
      else if (!strcmp (arg, "perceptual"))
        {
          flags |= BABL_MODEL_FLAG_PERCEPTUAL;
        }

      /* if we didn't point to a known string, we assume argument to be babl */
      else if (BABL_IS_BABL (arg))
        {
          Babl *bablc = (Babl *) arg;

          switch (bablc->class_type)
            {
              case BABL_COMPONENT:
                if (components >= BABL_MAX_COMPONENTS)
                  {
                    babl_log ("maximum number of components (%i) exceeded for %s",
                              BABL_MAX_COMPONENTS,
                              assigned_name ? assigned_name : "(unnamed)");
                  }
                component [components++] = (BablComponent *) bablc;
                break;

              case BABL_MODEL:
                babl_log ("submodels not handled yet");
                break;

              case BABL_SPACE:
                space = bablc;
                break;

              case BABL_TYPE:
              case BABL_TYPE_INTEGER:
              case BABL_TYPE_FLOAT:
              case BABL_SAMPLING:
              case BABL_INSTANCE:
              case BABL_FORMAT:


              case BABL_CONVERSION:
              case BABL_CONVERSION_LINEAR:
              case BABL_CONVERSION_PLANE:
              case BABL_CONVERSION_PLANAR:
              case BABL_FISH:
              case BABL_FISH_SIMPLE:
              case BABL_FISH_REFERENCE:
              case BABL_FISH_PATH:
              case BABL_IMAGE:
              case BABL_EXTENSION:
                babl_log ("%s unexpected", babl_class_name (bablc->class_type));
                break;

              case BABL_SKY: /* shut up compiler */
                break;
            }
        }

      else
        {
          babl_fatal ("unhandled argument '%s' for babl_model '%s'",
                      arg, assigned_name ? assigned_name : "(unnamed)");
        }

      arg = va_arg (varg, char *);
      if (!arg)
        break;
    }

  va_end (varg);

  if (assigned_name)
    name = babl_strdup(assigned_name);
  else
    name = babl_model_create_name (components, component);

  if (!components)
    {
      babl_log("no components specified for model '%s'", name);
      goto out;
    }

  babl = babl_db_exist (db, id, name);
  if (id && !babl && babl_db_exist (db, 0, name))
    babl_fatal ("Trying to reregister BablModel '%s' with different id!", name);

  if (! babl)
    {
      babl = model_new (name, space, id, components, component, flags);
      babl_db_insert (db, babl);
      construct_double_format (babl);
    }
  else
    {
      if (!is_model_duplicate (babl, space, components, component))
              babl_fatal ("BablModel '%s' already registered "
                          "with different components!", name);
    }

 out:
  babl_free (name);

  return babl;
}


#define TOLERANCE      0.001

static const Babl *
reference_format (void)
{
  static const Babl *self = NULL;

  if (!self)
    self = babl_format_new (
      babl_model ("RGBA"),
      babl_type ("double"),
      babl_component ("R"),
      babl_component ("G"),
      babl_component ("B"),
      babl_component ("A"),
      NULL);
  return self;
}

static const Babl *
construct_double_format (const Babl *model)
{
  const void *argument[44 + 1];
  int   args = 0;
  int   i;

  if (model == babl_model_from_id (BABL_RGBA))
    {
      argument[args++] = "id";
      argument[args++] = (void*) BABL_RGBA_DOUBLE;
    }
  argument[args++] = model;
  argument[args++] = babl_type_from_id (BABL_DOUBLE);

  for (i = 0; i < model->model.components; i++)
    {
      argument[args++] = model->model.component[i];
    }
  argument[args++] = NULL;

#define o(argno)    argument[argno],
  return babl_format_new (o (0)  o (1)  o (2)  o (3)
                          o (4)  o (5)  o (6)  o (7)
                          o (8)  o (9) o (10) o (11)
                          o (12) o (13) o (14) o (15)
                          o (16) o (17) o (18) o (19)
                          o (20) o (21) o (22) o (23)
                          o (24) o (25) o (26) o (27)
                          o (28) o (29) o (30) o (31)
                          o (32) o (33) o (34) o (35)
                          o (36) o (37) o (38) o (39)
                          o (40) o (41) o (42) NULL);
#undef o
}

double
babl_model_is_symmetric (const Babl *cbabl)
{
  Babl *babl = (Babl*)cbabl;
  void   *original;
  double *clipped;
  void   *destination;
  double *transformed;
  int     symmetric = 1;

  const Babl *ref_fmt;
  const Babl *fmt;
  Babl *fish_to;
  Babl *fish_from;

  const int test_pixels = babl_get_num_model_test_pixels ();
  const double *test = babl_get_model_test_pixels ();

  ref_fmt   = reference_format ();
  fmt       = construct_double_format (babl);
  fish_to   = babl_fish_reference (ref_fmt, fmt);
  fish_from = babl_fish_reference (fmt, ref_fmt);

  original    = babl_calloc (1, 64 / 8 * babl->model.components * test_pixels);
  clipped     = babl_calloc (1, 64 / 8 * 4 * test_pixels);
  destination = babl_calloc (1, 64 / 8 * babl->model.components * test_pixels);
  transformed = babl_calloc (1, 64 / 8 * 4 * test_pixels);

  babl_process (fish_to, test, original, test_pixels);
  babl_process (fish_from, original, clipped, test_pixels);
  babl_process (fish_to, clipped, destination, test_pixels);
  babl_process (fish_from, destination, transformed, test_pixels);

  fish_to->fish.pixels        -= test_pixels * 2;
  fish_from->fish.pixels      -= test_pixels * 2;

  {
    int i;
    int log = 0;

    for (i = 0; i < test_pixels; i++)
      {
        int j;
        for (j = 0; j < 4; j++)
          if (fabs (clipped[i *4 + j] - transformed[i * 4 + j]) > TOLERANCE)
            {
              if (!log)
                log = 1;
              symmetric = 0;
            }
        if (log && log < 5)
          {
            babl_log ("%s", babl->instance.name);
            babl_log ("\ttest:     %2.3f %2.3f %2.3f %2.3f", test [i *4 + 0],
                      test [i * 4 + 1],
                      test [i * 4 + 2],
                      test [i * 4 + 3]);
            babl_log ("\tclipped:  %2.3f %2.3f %2.3f %2.3f", clipped [i *4 + 0],
                      clipped [i * 4 + 1],
                      clipped [i * 4 + 2],
                      clipped [i * 4 + 3]);
            babl_log ("\ttrnsfrmd: %2.3f %2.3f %2.3f %2.3f", transformed [i *4 + 0],
                      transformed [i * 4 + 1],
                      transformed [i * 4 + 2],
                      transformed [i * 4 + 3]);
            log++;
          }
      }
  }

  babl_free (original);
  babl_free (clipped);
  babl_free (destination);
  babl_free (transformed);
  return symmetric;
}

BABL_CLASS_IMPLEMENT (model)

/* XXX: probably better to do like with babl_format, add a -suffix and
 *      insert in normal database than to have this static cache list
 */
static const Babl *babl_remodels[512]={NULL,};
int          babl_n_remodels = 0;

const Babl *
babl_remodel_with_space (const Babl *model, 
                         const Babl *space)
{
  Babl *ret;
  int i;
  assert (BABL_IS_BABL (model));

  if (!space) space = babl_space ("sRGB");
  if (space->class_type == BABL_FORMAT)
  {
    space = space->format.space;
  }
  else if (space->class_type == BABL_MODEL)
  {
    space = space->model.space;
  }
  else if (space->class_type != BABL_SPACE)
  {
    return NULL;
  }

  if (model->model.space == space)
    return (void*)model;

  assert (BABL_IS_BABL (model));

  /* get back to the sRGB model if we are in a COW clone of it  */
  if (model->model.model)
    model = (void*)model->model.model;

  assert (BABL_IS_BABL (model));

  for (i = 0; i < babl_n_remodels; i++)
  {
    if (babl_remodels[i]->model.model == model &&
        babl_remodels[i]->model.space == space)
          return babl_remodels[i];
  }

  ret = babl_calloc (sizeof (BablModel), 1);
  memcpy (ret, model, sizeof (BablModel));
  ret->model.space = space;
  ret->model.model = (void*)model; /* use the data as a backpointer to original model */
  return babl_remodels[babl_n_remodels++] = ret;
  return (Babl*)ret;
}

const Babl *
babl_model_with_space (const char *name, 
                       const Babl *space)
{
  return babl_remodel_with_space (babl_model (name), space);
}

BablModelFlag 
babl_get_model_flags (const Babl *babl)
{
  if (!babl) return 0;
  switch (babl->class_type)
  {
    case BABL_MODEL:
      return babl->model.flags;
    case BABL_FORMAT:
      return babl->format.model->flags;
  }
  return 0;
}

