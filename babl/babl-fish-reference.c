/* babl - dynamically extendable universal pixel fish library.
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
#include "babl-internal.h"

static Babl *
assert_conversion_find (const void *source,
                        const void *destination)
{
  Babl *ret = babl_conversion_find (source, destination);

  if (!ret)
    babl_fatal ("failed finding conversion between %s and %s aborting",
                babl_get_name (source), babl_get_name (destination));
  return ret;
}

static int
create_name_internal (char *buf,
                      size_t maxlen,
                      const Babl *source,
                      const Babl *destination,
                      int   is_reference)
{
  return snprintf (buf, maxlen, "%s %p %p",
                   is_reference ? "ref "
                   : "",
                   source, destination);
}

#ifdef HAVE_TLS

static __thread char buf[1024];

static char *
create_name (const Babl *source,
             const Babl *destination,
             int   is_reference)
{
  int size = 0;

  size = create_name_internal (buf, sizeof(buf), source, destination, is_reference);

  if (size < 0)
    return NULL;

  return buf;
}


#else

static char *
create_name (const Babl *source,
             const Babl *destination,
             int   is_reference)
{
  int size = 0;
  char *buf = NULL;

  size = create_name_internal (buf, size, source, destination, is_reference);

  if (size < 0)
    return NULL;

  size++;             /* For '\0' */
  buf = malloc (size);
  if (buf == NULL)
    return NULL;

  size = create_name_internal (buf, size, source, destination, is_reference);

  if (size < 0)
    {
      free (buf);
      return NULL;
    }

  return buf;
}

#endif

/* need an internal version that only ever does double,
 * for use in path evaluation? and perhaps even self evaluation of float code path?
 */
Babl *
babl_fish_reference (const Babl *source,
                     const Babl *destination)
{
  Babl *babl = NULL;
  char *name = create_name (source, destination, 1);

  babl_assert (name);

  babl = babl_db_exist_by_name (babl_fish_db (), name);
  if (babl)
    {
      /* There is an instance already registered by the required name,
       * returning the preexistent one instead.
       */
#ifndef HAVE_TLS
      free (name);
#endif
      _babl_fish_rig_dispatch (babl);
      return babl;
    }

  babl_assert (BABL_IS_BABL (source));
  babl_assert (BABL_IS_BABL (destination));

  babl_assert (source->class_type == BABL_FORMAT);
  babl_assert (destination->class_type == BABL_FORMAT);

  babl = babl_calloc (1, sizeof (BablFishReference) +
                      strlen (name) + 1);
  babl->class_type    = BABL_FISH_REFERENCE;
  babl->instance.id   = babl_fish_get_id (source, destination);
  babl->instance.name = ((char *) babl) + sizeof (BablFishReference);
  strcpy (babl->instance.name, name);
  babl->fish.source      = source;
  babl->fish.destination = destination;

  babl->fish.pixels      = 0;
  babl->fish.error       = 0.0;  /* assuming the provided reference conversions for types
                                    and models are as exact as possible
                                  */
  _babl_fish_rig_dispatch (babl);

  /* Since there is not an already registered instance by the required
   * name, inserting newly created class into database.
   */
  babl_db_insert (babl_fish_db (), babl);
#ifndef HAVE_TLS
  free (name);
#endif
  return babl;
}


static void
convert_to_double (BablFormat      *source_fmt,
                   const char      *source_buf,
                   char            *double_buf,
                   int              n)
{
  int        i;

  BablImage *src_img;
  BablImage *dst_img;

  src_img = (BablImage *) babl_image_new (
    babl_component_from_id (BABL_GRAY_LINEAR), NULL, 1, 0, NULL);
  dst_img = (BablImage *) babl_image_new (
    babl_component_from_id (BABL_GRAY_LINEAR), NULL, 1, 0, NULL);

  dst_img->type[0]  = (BablType *) babl_type_from_id (BABL_DOUBLE);
  dst_img->pitch[0] =
    (dst_img->type[0]->bits / 8) * source_fmt->model->components;
  dst_img->stride[0] = 0;

  src_img->type[0]   = (BablType *) babl_type_from_id (BABL_DOUBLE);
  src_img->pitch[0]  = source_fmt->bytes_per_pixel;
  src_img->stride[0] = 0;

  {
  /* i is dest position */
  for (i = 0; i < source_fmt->model->components; i++)
    {
      int j;
      int found = 0;

      dst_img->data[0] =
        double_buf + (dst_img->type[0]->bits / 8) * i;

      src_img->data[0] = (char *)source_buf;

      /* j is source position */
      for (j = 0; j < source_fmt->components; j++)
        {
          src_img->type[0] = source_fmt->type[j];

          if (source_fmt->component[j] ==
              source_fmt->model->component[i])
            {
              babl_conversion_process (assert_conversion_find (src_img->type[0], dst_img->type[0]),
                                       (void*)src_img, (void*)dst_img, n);
              found = 1;
              break;
            }

          src_img->data[0] += src_img->type[0]->bits / 8;
        }

      if (!found)
        {
          char *dst_ptr = dst_img->data[0];
          double value;

          value = source_fmt->model->component[i]->instance.id == BABL_ALPHA ? 1.0 : 0.0;

          for (j = 0; j < n; j++)
            {
              double *dst_component = (double *) dst_ptr;

              *dst_component = value;
              dst_ptr += dst_img->pitch[0];
            }
        }
    }
  }
  babl_free (src_img);
  babl_free (dst_img);
}


static void
convert_from_double (BablFormat *destination_fmt,
                     char       *destination_double_buf,
                     char       *destination_buf,
                     int         n)
{
  int        i;

  BablImage *src_img;
  BablImage *dst_img;

  src_img = (BablImage *) babl_image_new (
    babl_component_from_id (BABL_GRAY_LINEAR), NULL, 1, 0, NULL);
  dst_img = (BablImage *) babl_image_new (
    babl_component_from_id (BABL_GRAY_LINEAR), NULL, 1, 0, NULL);

  src_img->type[0]   = (BablType *) babl_type_from_id (BABL_DOUBLE);
  src_img->pitch[0]  = (src_img->type[0]->bits / 8) * destination_fmt->model->components;
  src_img->stride[0] = 0;

  dst_img->data[0]  = destination_buf;
  dst_img->type[0]  = (BablType *) babl_type_from_id (BABL_DOUBLE);
  dst_img->pitch[0] = destination_fmt->bytes_per_pixel;
  dst_img->stride[0] = 0;

  for (i = 0; i < destination_fmt->components; i++)
    {
      int j;

      dst_img->type[0] = destination_fmt->type[i];

      for (j = 0; j < destination_fmt->model->components; j++)
        {
          if (destination_fmt->component[i] ==
              destination_fmt->model->component[j])
            {
              src_img->data[0] =
                destination_double_buf + (src_img->type[0]->bits / 8) * j;

              babl_conversion_process (assert_conversion_find (src_img->type[0],
                                       dst_img->type[0]),
                                       (void*)src_img, (void*)dst_img, n);
              break;
            }
        }

      dst_img->data[0] += dst_img->type[0]->bits / 8;
    }
  babl_free (src_img);
  babl_free (dst_img);
}


static void
ncomponent_convert_to_double (BablFormat       *source_fmt,
                              char             *source_buf,
                              char             *source_double_buf,
                              int               n)
{
  BablImage *src_img;
  BablImage *dst_img;

  src_img = (BablImage *) babl_image_new (
    babl_component_from_id (BABL_GRAY_LINEAR), NULL, 1, 0, NULL);
  dst_img = (BablImage *) babl_image_new (
    babl_component_from_id (BABL_GRAY_LINEAR), NULL, 1, 0, NULL);

  dst_img->type[0]  = (BablType *) babl_type_from_id (BABL_DOUBLE);
  dst_img->pitch[0] = (dst_img->type[0]->bits / 8);
  dst_img->stride[0] = 0;

  src_img->data[0] = source_buf;
  src_img->type[0] = source_fmt->type[0];
  src_img->pitch[0] = source_fmt->type[0]->bits / 8;
  src_img->stride[0] = 0;

  dst_img->data[0] = source_double_buf;

  babl_conversion_process (
    assert_conversion_find (src_img->type[0], dst_img->type[0]),
    (void*)src_img, (void*)dst_img,
    n * source_fmt->components);
  babl_free (src_img);
  babl_free (dst_img);
}

static void
ncomponent_convert_from_double (BablFormat *destination_fmt,
                                char       *destination_double_buf,
                                char       *destination_buf,
                                int         n)
{
  BablImage *src_img;
  BablImage *dst_img;

  src_img = (BablImage *) babl_image_new (
    babl_component_from_id (BABL_GRAY_LINEAR), NULL, 1, 0, NULL);
  dst_img = (BablImage *) babl_image_new (
    babl_component_from_id (BABL_GRAY_LINEAR), NULL, 1, 0, NULL);

  src_img->type[0]   = (BablType *) babl_type_from_id (BABL_DOUBLE);
  src_img->pitch[0]  = (src_img->type[0]->bits / 8);
  src_img->stride[0] = 0;

  dst_img->data[0]  = destination_buf;
  dst_img->type[0]  = (BablType *) babl_type_from_id (BABL_DOUBLE);
  dst_img->pitch[0] = destination_fmt->type[0]->bits/8;
  dst_img->stride[0] = 0;

  dst_img->type[0] = destination_fmt->type[0];
  src_img->data[0] = destination_double_buf;

  babl_conversion_process (
    assert_conversion_find (src_img->type[0], dst_img->type[0]),
    (void*)src_img, (void*)dst_img,
    n * destination_fmt->components);

  dst_img->data[0] += dst_img->type[0]->bits / 8;
  babl_free (src_img);
  babl_free (dst_img);
}


static int
process_to_n_component (const Babl  *babl,
                        const char *source,
                        char       *destination,
                        long        n)
{
  void *double_buf;
#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif
  int components = MAX(BABL (babl->fish.source)->format.model->components,
                       BABL (babl->fish.source)->format.components);
  components = MAX(components, BABL (babl->fish.destination)->format.components);
  components = MAX(components, BABL (babl->fish.destination)->model.components);

  double_buf = babl_malloc (sizeof (double) * n * components);
      memset (double_buf, 0,sizeof (double) * n * components);

 /* a single precision path could be added here*/
    {
      ncomponent_convert_to_double (
        (BablFormat *) BABL (babl->fish.source),
        (char *) source,
        double_buf,
        n
      );

      ncomponent_convert_from_double (
        (BablFormat *) BABL (babl->fish.destination),
        double_buf,
        (char *) destination,
        n
      );
    }

  babl_free (double_buf);
  return 0;
}

static int compatible_components (const BablFormat *a,
                                  const BablFormat *b)
{
  int i;
  if (a->components != b->components)
    return 0;
  for (i = 0; i < a->components; i++)
   if (a->component[i] != b->component[i])
     return 0;
  return 1;
}

static void
ncomponent_convert_to_float (BablFormat       *source_fmt,
                             char             *source_buf,
                             char             *source_float_buf,
                             int               n)
{
  BablImage *src_img;
  BablImage *dst_img;

  src_img = (BablImage *) babl_image_new (
    babl_component_from_id (BABL_GRAY_LINEAR), NULL, 1, 0, NULL);
  dst_img = (BablImage *) babl_image_new (
    babl_component_from_id (BABL_GRAY_LINEAR), NULL, 1, 0, NULL);

  dst_img->type[0]  = (BablType *) babl_type_from_id (BABL_FLOAT);
  dst_img->pitch[0] = (dst_img->type[0]->bits / 8);
  dst_img->stride[0] = 0;

  src_img->data[0] = source_buf;
  src_img->type[0] = source_fmt->type[0];
  src_img->pitch[0] = source_fmt->type[0]->bits / 8;
  src_img->stride[0] = 0;

  dst_img->data[0] = source_float_buf;

  babl_conversion_process (
    assert_conversion_find (src_img->type[0], dst_img->type[0]),
    (void*)src_img, (void*)dst_img,
    n * source_fmt->components);
  babl_free (src_img);
  babl_free (dst_img);
}

static void
ncomponent_convert_from_float (BablFormat *destination_fmt,
                               char       *destination_float_buf,
                               char       *destination_buf,
                               int         n)
{
  BablImage *src_img;
  BablImage *dst_img;

  src_img = (BablImage *) babl_image_new (
    babl_component_from_id (BABL_GRAY_LINEAR), NULL, 1, 0, NULL);
  dst_img = (BablImage *) babl_image_new (
    babl_component_from_id (BABL_GRAY_LINEAR), NULL, 1, 0, NULL);

  src_img->type[0]   = (BablType *) babl_type_from_id (BABL_FLOAT);
  src_img->pitch[0]  = (src_img->type[0]->bits / 8);
  src_img->stride[0] = 0;

  dst_img->data[0]  = destination_buf;
  dst_img->type[0]  = (BablType *) babl_type_from_id (BABL_FLOAT);
  dst_img->pitch[0] = destination_fmt->type[0]->bits/8;
  dst_img->stride[0] = 0;

  dst_img->type[0] = destination_fmt->type[0];
  src_img->data[0] = destination_float_buf;

  babl_conversion_process (
    assert_conversion_find (src_img->type[0], dst_img->type[0]),
    (void*)src_img, (void*)dst_img,
    n * destination_fmt->components);

  dst_img->data[0] += dst_img->type[0]->bits / 8;
  babl_free (src_img);
  babl_free (dst_img);
}

static void
convert_to_float (BablFormat *source_fmt,
                  const char *source_buf,
                  char       *float_buf,
                  int         n)
{
  int        i;

  BablImage *src_img;
  BablImage *dst_img;

  src_img = (BablImage *) babl_image_new (
    babl_component_from_id (BABL_GRAY_LINEAR), NULL, 1, 0, NULL);
  dst_img = (BablImage *) babl_image_new (
    babl_component_from_id (BABL_GRAY_LINEAR), NULL, 1, 0, NULL);

  dst_img->type[0]  = (BablType *) babl_type_from_id (BABL_FLOAT);
  dst_img->pitch[0] =
    (dst_img->type[0]->bits / 8) * source_fmt->model->components;
  dst_img->stride[0] = 0;

  src_img->type[0]   = (BablType *) babl_type_from_id (BABL_FLOAT);
  src_img->pitch[0]  = source_fmt->bytes_per_pixel;
  src_img->stride[0] = 0;

  {
  /* i is dest position */
  for (i = 0; i < source_fmt->model->components; i++)
    {
      int j;
      int found = 0;

      dst_img->data[0] =
        float_buf + (dst_img->type[0]->bits / 8) * i;

      src_img->data[0] = (char *)source_buf;

      /* j is source position */
      for (j = 0; j < source_fmt->components; j++)
        {
          src_img->type[0] = source_fmt->type[j];

          if (source_fmt->component[j] ==
              source_fmt->model->component[i])
            {
              babl_conversion_process (assert_conversion_find (src_img->type[0], dst_img->type[0]),
                                       (void*)src_img, (void*)dst_img, n);
              found = 1;
              break;
            }

          src_img->data[0] += src_img->type[0]->bits / 8;
        }

      if (!found)
        {
          char *dst_ptr = dst_img->data[0];
          float value;

          value = source_fmt->model->component[i]->instance.id == BABL_ALPHA ? 1.0 : 0.0;

          for (j = 0; j < n; j++)
            {
              float *dst_component = (float *) dst_ptr;

              *dst_component = value;
              dst_ptr += dst_img->pitch[0];
            }
        }
    }
  }
  babl_free (src_img);
  babl_free (dst_img);
}


static void
convert_from_float (BablFormat *destination_fmt,
                     char      *destination_float_buf,
                     char      *destination_buf,
                     int        n)
{
  int        i;

  BablImage *src_img;
  BablImage *dst_img;

  src_img = (BablImage *) babl_image_new (
    babl_component_from_id (BABL_GRAY_LINEAR), NULL, 1, 0, NULL);
  dst_img = (BablImage *) babl_image_new (
    babl_component_from_id (BABL_GRAY_LINEAR), NULL, 1, 0, NULL);

  src_img->type[0]   = (BablType *) babl_type_from_id (BABL_FLOAT);
  src_img->pitch[0]  = (src_img->type[0]->bits / 8) * destination_fmt->model->components;
  src_img->stride[0] = 0;

  dst_img->data[0]  = destination_buf;
  dst_img->type[0]  = (BablType *) babl_type_from_id (BABL_FLOAT);
  dst_img->pitch[0] = destination_fmt->bytes_per_pixel;
  dst_img->stride[0] = 0;

  for (i = 0; i < destination_fmt->components; i++)
    {
      int j;

      dst_img->type[0] = destination_fmt->type[i];

      for (j = 0; j < destination_fmt->model->components; j++)
        {
          if (destination_fmt->component[i] ==
              destination_fmt->model->component[j])
            {
              src_img->data[0] =
                destination_float_buf + (src_img->type[0]->bits / 8) * j;

              babl_conversion_process (assert_conversion_find (src_img->type[0],
                                       dst_img->type[0]),
                                       (void*)src_img, (void*)dst_img, n);
              break;
            }
        }

      dst_img->data[0] += dst_img->type[0]->bits / 8;
    }
  babl_free (src_img);
  babl_free (dst_img);
}



static void
process_same_model (const Babl  *babl,
                    const char *source,
                    char       *destination,
                    long        n)
{
  const void *type_float = babl_type_from_id (BABL_FLOAT);
#define MAX(a, b) ((a) > (b) ? (a) : (b))



  if ((babl->fish.source->format.type[0]->bits < 32 ||
       babl->fish.source->format.type[0] == type_float) &&
      (babl->fish.destination->format.type[0]->bits < 32 ||
       babl->fish.destination->format.type[0] == type_float))
  {
     void *float_buf = babl_malloc (sizeof (float) * n *
                            MAX (BABL (babl->fish.source)->format.model->components,
                                 BABL (babl->fish.source)->format.components));
    if (compatible_components ((void*)babl->fish.source,
                               (void*)babl->fish.destination))
    {
        ncomponent_convert_to_float (
          (BablFormat *) BABL (babl->fish.source),
          (char *) source,
          float_buf,
          n);
        ncomponent_convert_from_float (
          (BablFormat *) BABL (babl->fish.destination),
          float_buf,
          (char *) destination,
          n);
    }
    else
    {
        convert_to_float (
          (BablFormat *) BABL (babl->fish.source),
          (char *) source,
          float_buf,
          n);

        convert_from_float (
          (BablFormat *) BABL (babl->fish.destination),
          float_buf,
          (char *) destination,
          n);
    }
    babl_free (float_buf);
  }
  else
  {
     void *double_buf = babl_malloc (sizeof (double) * n *
                            MAX (BABL (babl->fish.source)->format.model->components,
                                 BABL (babl->fish.source)->format.components));
#undef MAX
    if (compatible_components ((void*)babl->fish.source,
                               (void*)babl->fish.destination))
    {
        ncomponent_convert_to_double (
          (BablFormat *) BABL (babl->fish.source),
          (char *) source,
          double_buf,
          n);
        ncomponent_convert_from_double (
          (BablFormat *) BABL (babl->fish.destination),
          double_buf,
          (char *) destination,
          n);
    }
    else
    {
        convert_to_double (
          (BablFormat *) BABL (babl->fish.source),
          (char *) source,
          double_buf,
          n);

        convert_from_double (
          (BablFormat *) BABL (babl->fish.destination),
          double_buf,
          (char *) destination,
          n);
    }
    babl_free (double_buf);
  }
}


static void
babl_fish_reference_process_double (const Babl *babl,
                                    const char *source,
                                    char       *destination,
                                    long        n,
                                    void       *data)
{
    Babl *source_image = NULL;
    Babl *rgba_image = NULL;
    Babl *destination_image = NULL;
    void *source_double_buf_alloc = NULL;
    void *source_double_buf;
    void *rgba_double_buf_alloc = NULL;
    void *rgba_double_buf;
    void *destination_double_buf_alloc = NULL;
    void *destination_double_buf;
    const void *type_double  = babl_type_from_id (BABL_DOUBLE);

    if (babl->fish.source->format.type[0] == type_double &&
        BABL(babl->fish.source)->format.components ==
        BABL(babl->fish.source)->format.model->components && 0)
    {
      source_double_buf = (void*)source;
      source_image = babl_image_from_linear (
         source_double_buf, BABL (BABL ((babl->fish.source))->format.model));
    }
    else
    {
      source_double_buf_alloc = babl_malloc (sizeof (double) * n *
                                  BABL (babl->fish.source)->format.model->components);

      source_double_buf = source_double_buf_alloc;
      source_image = babl_image_from_linear (
        source_double_buf, BABL (BABL ((babl->fish.source))->format.model));
      convert_to_double (
        (BablFormat *) BABL (babl->fish.source),
        source,
        source_double_buf,
        n
      );
    }

    if (babl_model_is ((void*)babl->fish.source->format.model, "RGBA"))
    {
      rgba_double_buf = source_double_buf;
      rgba_image = babl_image_from_linear (
          rgba_double_buf,
          (void*)babl->fish.source->format.model);
    }
    else
    {
      Babl *conv =
        assert_conversion_find (
        BABL (babl->fish.source)->format.model,

      babl_remodel_with_space (babl_model_from_id (BABL_RGBA),
                           BABL (BABL ((babl->fish.source))->format.space)));

      rgba_double_buf_alloc  = babl_malloc (sizeof (double) * n * 4);
      rgba_double_buf        = rgba_double_buf_alloc;

      rgba_image = babl_image_from_linear (
          rgba_double_buf, babl_remodel_with_space (babl_model_from_id (BABL_RGBA),
          BABL (BABL ((babl->fish.source))->format.space)) );

      if (conv->class_type == BABL_CONVERSION_PLANAR)
      {
        babl_conversion_process (
          conv,
          (void*)source_image, (void*)rgba_image,
          n);
      }
      else if (conv->class_type == BABL_CONVERSION_LINEAR)
      {
        babl_conversion_process (
          conv,
          source_double_buf, rgba_double_buf,
          n);
      }
      else babl_fatal ("oops");
    }

    if (((babl->fish.source)->format.space !=
        ((babl->fish.destination)->format.space)))
    {
      double matrix[9];
      double *rgba = rgba_double_buf;
      babl_matrix_mul_matrix (
        (babl->fish.destination)->format.space->space.XYZtoRGB,
        (babl->fish.source)->format.space->space.RGBtoXYZ,
        matrix);

      babl_matrix_mul_vector_buf4 (matrix, rgba, rgba, n);
    }

    {
      const Babl *destination_rgba_format =
        babl_remodel_with_space (babl_model_from_id (BABL_RGBA),
             BABL (BABL ((babl->fish.destination))->format.space));

      if(BABL (babl->fish.destination)->format.model == (void*)destination_rgba_format)
      {
         destination_double_buf = rgba_double_buf;
      }
      else
      {
      Babl *conv =
        assert_conversion_find (destination_rgba_format,
           BABL (babl->fish.destination)->format.model);

         destination_double_buf_alloc = babl_malloc (sizeof (double) * n *
                                          BABL (babl->fish.destination)->format.model->components);
         destination_double_buf = destination_double_buf_alloc;

      if (conv->class_type == BABL_CONVERSION_PLANAR)
        {
          destination_image = babl_image_from_linear (
            destination_double_buf, BABL (BABL ((babl->fish.destination))->format.model));


          babl_conversion_process (
            conv,
            (void*)rgba_image, (void*)destination_image,
            n);
        }
      else if (conv->class_type == BABL_CONVERSION_LINEAR)
        {
          babl_conversion_process (
            conv,
            rgba_double_buf, destination_double_buf,
            n);
        }
      else babl_fatal ("oops");
      }
   }

    convert_from_double (
      (BablFormat *) BABL (babl->fish.destination),
      destination_double_buf,
      destination,
      n
    );

    if (destination_double_buf_alloc)
      babl_free (destination_double_buf_alloc);
    if (rgba_double_buf_alloc)
      babl_free (rgba_double_buf_alloc);
    if (source_double_buf_alloc)
      babl_free (source_double_buf_alloc);
    if (source_image)
      babl_free (source_image);
    if (rgba_image)
      babl_free (rgba_image);
    if (destination_image)
      babl_free (destination_image);
}

static void
babl_fish_reference_process_float (const Babl *babl,
                                   const char *source,
                                   char       *destination,
                                   long        n,
                                   void       *data)
{
  const void *type_float = babl_type_from_id (BABL_FLOAT);
    Babl *source_image = NULL;
    Babl *rgba_image = NULL;
    Babl *destination_image = NULL;
    void *source_float_buf_alloc = NULL;
    void *source_float_buf;
    void *rgba_float_buf_alloc = NULL;
    void *rgba_float_buf;
    void *destination_float_buf_alloc = NULL;
    void *destination_float_buf;
    const Babl *destination_float_format;
    Babl *conv_to_rgba;
    Babl *conv_from_rgba;
    char dst_name[256];

    {
    char src_name[256];
    sprintf (src_name, "%s float", babl_get_name((void*)babl->fish.source->format.model));
    conv_to_rgba =
        babl_conversion_find (
        babl_format_with_space (src_name,
                   BABL (BABL ((babl->fish.source))->format.space)),
        babl_format_with_space ("RGBA float",
                   BABL (BABL ((babl->fish.source))->format.space)));
    }
    {
      sprintf (dst_name, "%s float", babl_get_name((void*)babl->fish.destination->format.model));
      conv_from_rgba  =
        babl_conversion_find (
        babl_format_with_space ("RGBA float",
                   BABL (BABL ((babl->fish.destination))->format.space)),
        babl_format_with_space (dst_name,
                   BABL (BABL ((babl->fish.destination))->format.space)));
      destination_float_format =
        babl_format_with_space (dst_name,
                   BABL (BABL ((babl->fish.destination))->format.space));
    }

    if (!conv_to_rgba || !conv_from_rgba)
    {
      /* needed float conversions not found, using double code path instead */
      babl_fish_reference_process_double (babl, source, destination, n, data);
      return;
    }

    babl_mutex_lock (babl_reference_mutex);
    if (babl->fish.source->format.type[0] == type_float &&
        BABL(babl->fish.source)->format.components ==
        BABL(babl->fish.source)->format.model->components && 0)
    {
      source_float_buf = (void*)source;
      source_image = babl_image_from_linear (
         source_float_buf,
         babl_format_with_model_as_type ((void*)((babl->fish.source))->format.model,
         type_float));
    }
    else
    {
      source_float_buf_alloc = babl_malloc (sizeof (float) * n *
                                  (BABL (babl->fish.source)->format.model->components));

      source_float_buf = source_float_buf_alloc;
      source_image = babl_image_from_linear (
        source_float_buf,
      babl_format_with_model_as_type (BABL(BABL ((babl->fish.source))->format.model),
         type_float));
      convert_to_float (
        (BablFormat *) BABL (babl->fish.source),
        source,
        source_float_buf,
        n
      );
    }

    if (babl_model_is ((void*)babl->fish.source->format.model, "RGBA") && 0)
    {
      rgba_float_buf = source_float_buf;
      rgba_image = babl_image_from_linear (
          rgba_float_buf,
          (void*)babl->fish.source->format.model);
    }
    else
    {
      rgba_float_buf_alloc  = babl_malloc (sizeof (float) * n * 4);
      rgba_float_buf        = rgba_float_buf_alloc;

      rgba_image = babl_image_from_linear (
          rgba_float_buf,
        babl_format_with_space ("RGBA float",
                   BABL (BABL ((babl->fish.source))->format.space)));


    if (conv_to_rgba->class_type == BABL_CONVERSION_PLANAR)
      {
        babl_conversion_process (
          conv_to_rgba,
          (void*)source_image, (void*)rgba_image,
          n);
      }
    else if (conv_to_rgba->class_type == BABL_CONVERSION_LINEAR)
      {
        babl_conversion_process (
          conv_to_rgba,
          source_float_buf, rgba_float_buf,
          n);
      }
    }
    babl_mutex_unlock (babl_reference_mutex);

    if (((babl->fish.source)->format.space !=
        ((babl->fish.destination)->format.space)))
    {
      float matrix[9];
      float *rgba = rgba_float_buf;
      babl_matrix_mul_matrixf (
        (babl->fish.destination)->format.space->space.XYZtoRGBf,
        (babl->fish.source)->format.space->space.RGBtoXYZf,
        matrix);

      babl_matrix_mul_vectorff_buf4 (matrix, rgba, rgba, n);
    }

    {

      if(babl_format_with_space ("RGBA float",
                   BABL (BABL ((babl->fish.destination))->format.space)) ==
         babl_format_with_space (dst_name,
                   BABL (BABL ((babl->fish.destination))->format.space)))
      {
        destination_float_buf = rgba_float_buf;
      }
      else
      {
        destination_float_buf_alloc = babl_malloc (sizeof (float) * n *
                                        BABL (babl->fish.destination)->format.model->components);
        destination_float_buf = destination_float_buf_alloc;

        if (conv_from_rgba->class_type == BABL_CONVERSION_PLANAR)
        {
           destination_image = babl_image_from_linear (
             destination_float_buf, destination_float_format);
             babl_conversion_process (
               conv_from_rgba,
               (void*)rgba_image, (void*)destination_image,
             n);
        }
        else if (conv_from_rgba->class_type == BABL_CONVERSION_LINEAR)
        {
          babl_conversion_process (conv_from_rgba, rgba_float_buf,
                                   destination_float_buf, n);
        }
      }
    }

    convert_from_float (
      (BablFormat *) BABL (babl->fish.destination),
      destination_float_buf,
      destination,
      n
    );

    if (destination_float_buf_alloc)
      babl_free (destination_float_buf_alloc);
    if (rgba_float_buf_alloc)
      babl_free (rgba_float_buf_alloc);
    if (source_float_buf_alloc)
      babl_free (source_float_buf_alloc);
    if (source_image)
      babl_free (source_image);
    if (rgba_image)
      babl_free (rgba_image);
    if (destination_image)
      babl_free (destination_image);
}

void
babl_fish_reference_process (const Babl *babl,
                             const char *source,
                             char       *destination,
                             long        n,
                             void       *data)
{
  static const void *type_float = NULL;
  static int allow_float_reference = -1;

  if (!type_float) type_float = babl_type_from_id (BABL_FLOAT);

  /* same format in source/destination */
  if (BABL (babl->fish.source) == BABL (babl->fish.destination))
  {
    if (source == destination)
    {
      memcpy (destination, source, n * babl->fish.source->format.bytes_per_pixel);
    }
    return;
  }

  /* same model and space, only convert type */
  if ((BABL (babl->fish.source)->format.model ==
       BABL (babl->fish.destination)->format.model) &&
      (BABL (babl->fish.source)->format.space ==
       BABL (babl->fish.destination)->format.space)
      )
  {
    process_same_model (babl, source, destination, n);
    return;
  }

  /* we're converting to a n_component format - special case   */
  if (babl_format_is_format_n (BABL (babl->fish.destination)))
  {
    process_to_n_component (babl, source, destination, n);
    return;
  }

  if (allow_float_reference == -1)
    allow_float_reference = getenv ("BABL_REFERENCE_NOFLOAT") ? 0 : 1;

  /* both source and destination are either single precision float or <32bit component,
     we then do a similar to the double reference - using the first registered
     float conversions - note that this makes the first registered float conversion of
     a given type a reference, and we thus rely on the *first* float conversion regsitered
     to be correct, this will be the case if the code paths are duplicated and we register
     either a planar or linear conversion to and from "RGBA float" at the same time as
     registering conversions for double. When needed conversions do not exist, we defer
     to the double code paths
   */
  if (allow_float_reference &&
      (babl->fish.source->format.type[0]->bits < 32 ||
      babl->fish.source->format.type[0] == type_float) &&
      (babl->fish.destination->format.type[0]->bits < 32 ||
      babl->fish.destination->format.type[0] == type_float) &&
      !babl_format_is_palette (babl->fish.source) &&
      !babl_format_is_palette (babl->fish.destination))
  {
    babl_fish_reference_process_float (babl, source, destination, n, data);
  }
  else /*   double  */
  {
    babl_fish_reference_process_double (babl, source, destination, n, data);
  }

}
