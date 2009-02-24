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

#include "config.h"
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include "babl-internal.h"
#include "babl-db.h"


static int
each_babl_model_destroy (Babl *babl,
                         void *data)
{
  if (babl->model.from_list)
    babl_list_destroy (babl->model.from_list);
  babl_free (babl);
  return 0;  /* continue iterating */
}

static char buf[512] = "";

static const char *
create_name (const char     *name,
             int             components,
             BablComponent **component)
{
  char *p = buf;

  if (name)
    return name;
  while (components--)
    {
      sprintf (p, (*component)->instance.name);
      p += strlen ((*component)->instance.name);
      component++;
    }
  return buf;
}

static Babl *
model_new (const char     *name,
           int             id,
           int             components,
           BablComponent **component)
{
  Babl *babl;

  babl = babl_malloc (sizeof (BablModel) +
                      sizeof (BablComponent *) * (components) +
                      strlen (name) + 1);
  babl->model.component = (void *) (((char *) babl) + sizeof (BablModel));
  babl->instance.name   = (void *) (((char *) babl->model.component) + sizeof (BablComponent *) * (components));

  babl->class_type       = BABL_MODEL;
  babl->instance.id      = id;
  babl->model.components = components;
  strcpy (babl->instance.name, name);
  memcpy (babl->model.component, component, sizeof (BablComponent *) * components);

  babl->model.from_list  = NULL;
  return babl;
}

Babl *
babl_model_new (void *first_argument,
                ...)
{
  va_list        varg;
  Babl          *babl;
  int            id         = 0;
  int            components = 0;
  const char    *arg        = first_argument;
  const char    *name       = NULL;
  BablComponent *component [BABL_MAX_COMPONENTS];

  va_start (varg, first_argument);

  while (1)
    {
      if (BABL_IS_BABL (arg))
        {
          Babl *babl = (Babl *) arg;

          switch (babl->class_type)
            {
              case BABL_COMPONENT:
                component [components] = (BablComponent *) babl;
                components++;

                if (components >= BABL_MAX_COMPONENTS)
                  {
                    babl_log ("maximum number of components (%i) exceeded for %s",
                              BABL_MAX_COMPONENTS, name);
                  }
                break;

              case BABL_MODEL:
                babl_log ("submodels not handled yet");
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
                babl_log ("%s unexpected", babl_class_name (babl->class_type));
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

      else if (!strcmp (arg, "name"))
        {
          name = va_arg (varg, char *);
        }

      else
        {
          babl_fatal ("unhandled argument '%s' for babl_model '%s'", arg, name);
        }

      arg = va_arg (varg, char *);
      if (!arg)
        break;
    }

  va_end (varg);

  name = create_name (name, components, component);

  babl = babl_db_exist (db, id, name);
  if (babl)
    {
      /* There is an instance already registered by the required id/name,
       * returning the preexistent one instead.
       */
      return babl;
    }

  babl = model_new (name, id, components, component);

  /* Since there is not an already registered instance by the required
   * id/name, inserting newly created class into database.
   */
  babl_db_insert (db, babl);
  return babl;
}


#define TOLERANCE      0.001

#define test_pixels    512

static double *
test_create (void)
{
  double *test;
  int     i;

  srandom (20050728);

  test = babl_malloc (sizeof (double) * test_pixels * 4);

  for (i = 0; i < test_pixels * 4; i++)
    test [i] = ((double) random () / RAND_MAX) * 1.4 - 0.2;

  return test;
}

static Babl *reference_format (void)
{
  static Babl *self = NULL;

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

static Babl *construct_double_format (Babl *model)
{
  void *argument[42 + 1];
  int   args = 0;
  int   i;

  argument[args++] = model;
  argument[args++] = babl_type ("double");

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
babl_model_is_symmetric (Babl *babl)
{
  double *test;
  void   *original;
  double *clipped;
  void   *destination;
  double *transformed;
  int     symmetric = 1;

  Babl   *ref_fmt;
  Babl   *fmt;
  Babl   *fish_to;
  Babl   *fish_from;

  test      = test_create ();
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

  fish_to->fish.processings   -= 2;
  fish_from->fish.processings -= 2;
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
  babl_free (test);
  return symmetric;
}

BABL_CLASS_IMPLEMENT (model)
