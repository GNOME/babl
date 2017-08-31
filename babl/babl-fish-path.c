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
 * <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include <math.h>
#include "babl-internal.h"
#include "babl-ref-pixels.h"

#define BABL_TOLERANCE             0.000005
#define BABL_MAX_COST_VALUE        2000000
#define BABL_HARD_MAX_PATH_LENGTH  8
#define BABL_MAX_NAME_LEN          1024

#ifndef MIN
#define MIN(a, b) (((a) > (b)) ? (b) : (a))
#endif

#define NUM_TEST_PIXELS            (babl_get_num_path_test_pixels ())
#define MAX_BUFFER_SIZE            2048 /* XXX: reasonable size for this should be profiled */


int   babl_in_fish_path = 0;

typedef struct _FishPathInstrumentation
{
  const Babl   *fmt_rgba_double;
  int     num_test_pixels;
  void   *source;
  void   *destination;
  void   *ref_destination;
  double *destination_rgba_double;
  double *ref_destination_rgba_double;
  const Babl   *fish_rgba_to_source;
  const Babl   *fish_reference;
  const Babl   *fish_destination_to_rgba;
  double  reference_cost;
  int     init_instrumentation_done;
} FishPathInstrumentation;

typedef struct PathContext {
  Babl     *fish_path;
  Babl     *to_format;
  BablList *current_path;
} PathContext;

static void
init_path_instrumentation (FishPathInstrumentation *fpi,
                           Babl                    *fmt_source,
                           Babl                    *fmt_destination);

static void
destroy_path_instrumentation (FishPathInstrumentation *fpi);

static void
get_path_instrumentation (FishPathInstrumentation *fpi,
                          BablList                *path,
                          double                  *path_cost,
                          double                  *ref_cost,
                          double                  *path_error);


static long
process_conversion_path (BablList   *path,
                         const void *source_buffer,
                         int         source_bpp,
                         void       *destination_buffer,
                         int         dest_bpp,
                         long        n);

static void
get_conversion_path (PathContext *pc,
                     Babl *current_format,
                     int current_length,
                     int max_length);

char *
_babl_fish_create_name (char       *buf,
                        const Babl *source,
                        const Babl *destination,
                        int         is_reference);


static int max_path_length (void);

static int debug_conversions = 0;

double _babl_legal_error (void)
{
  static double error = 0.0;
  const char   *env;

  if (error != 0.0)
    return error;

  env = getenv ("BABL_TOLERANCE");
  if (env && env[0] != '\0')
    error = babl_parse_double (env);
  else
    error = BABL_TOLERANCE;

  env = getenv ("BABL_DEBUG_CONVERSIONS");
  if (env && env[0] != '\0')
    debug_conversions = 1;
  else
    debug_conversions = 0;

  return error;
}

static int max_path_length (void)
{
  static int  max_length = 0;
  const char *env;

  if (max_length != 0)
    return max_length;

  env = getenv ("BABL_PATH_LENGTH");
  if (env)
    max_length = atoi (env);
  else
    max_length = 3; /* reducing this number makes finding short fishes much
                       faster - even if we lose out on some of the fast
                       bigger fish
                     */
  if (max_length > BABL_HARD_MAX_PATH_LENGTH)
    max_length = BABL_HARD_MAX_PATH_LENGTH;
  else if (max_length <= 0)
    max_length = 1;
  return max_length;
}

int _babl_max_path_len (void)
{
  return max_path_length ();
}

/* The task of BablFishPath construction is to compute
 * the shortest path in a graph where formats are the vertices
 * and conversions are the edges. However, there is an additional
 * constraint to the shortest path, that limits conversion error
 * introduced by such a path to be less than BABL_TOLERANCE. This
 * prohibits usage of any reasonable shortest path construction
 * algorithm such as Dijkstra's algorithm. The shortest path is
 * constructed by enumerating all available paths that are less
 * than BABL_PATH_LENGTH long, computing their costs and
 * conversion errors and backtracking. The backtracking is
 * implemented by recursive function get_conversion_path ().
 */


static void
get_conversion_path (PathContext *pc,
                     Babl        *current_format,
                     int          current_length,
                     int          max_length)
{
  if (current_length > max_length)
    {
      /* We have reached the maximum recursion
       * depth, let's bail out */
      return;
    }
  else if ((current_length > 0) && (current_format == pc->to_format))
    {
       /* We have found a candidate path, let's
        * see about it's properties */
      double path_cost  = 0.0;
      double ref_cost   = 0.0;
      double path_error = 1.0;
#if 1
      int    i;
      for (i = 0; i < babl_list_size (pc->current_path); i++)
        {
          path_error *= (1.0 + babl_conversion_error ((BablConversion *) pc->current_path->items[i]));
        }

      if (path_error - 1.0 <= _babl_legal_error ())
                /* check this before the more accurate measurement of error -
                   to bail earlier */
#endif
        {
          FishPathInstrumentation fpi;
          memset (&fpi, 0, sizeof (fpi));

          fpi.source = (Babl*) babl_list_get_first (pc->current_path)->conversion.source;
          fpi.destination = pc->to_format;

          get_path_instrumentation (&fpi, pc->current_path, &path_cost, &ref_cost, &path_error);
          //path_cost += pc->current_path->count * 1000; // punish long chains
          if(debug_conversions && current_length == 1)
            fprintf (stderr, "%s  error:%f cost:%f  \n",
                 babl_get_name (pc->current_path->items[0]),
                 /*babl_get_name (pc->fish_path->fish.source),
                 babl_get_name (pc->fish_path->fish.destination),*/
                 path_error,
                 path_cost /*, current_length*/);

          if ((path_cost < ref_cost) && /* do not use paths that took longer to compute than reference */
              (path_cost < pc->fish_path->fish_path.cost) &&
              (path_error <= _babl_legal_error ())
              )
            {
              /* We have found the best path so far,
               * let's copy it into our new fish */
              pc->fish_path->fish_path.cost = path_cost;
              pc->fish_path->fish.error  = path_error;
              babl_list_copy (pc->current_path,
                              pc->fish_path->fish_path.conversion_list);
            }

          destroy_path_instrumentation (&fpi);
        }
    }
  else
    {
      /*
       * we have to search deeper...
       */
      BablList *list;
      int i;

      list = current_format->format.from_list;
      if (list)
        {
          /* Mark the current format in conversion path as visited */
          current_format->format.visited = 1;

          /* Iterate through unvisited formats from the current format ...*/
          for (i = 0; i < babl_list_size (list); i++)
            {
              Babl *next_conversion = BABL (list->items[i]);
              Babl *next_format = BABL (next_conversion->conversion.destination);
              if (!next_format->format.visited)
                {
                  /* next_format is not in the current path, we can pay a visit */
                  babl_list_insert_last (pc->current_path, next_conversion);
                  get_conversion_path (pc, next_format, current_length + 1, max_length);
                  babl_list_remove_last (pc->current_path);
                }
            }

          /* Remove the current format from current path */
          current_format->format.visited = 0;
        }
   }
}

char *
_babl_fish_create_name (char       *buf,
                        const Babl *source,
                        const Babl *destination,
                        int         is_reference)
{
  /* fish names are intentionally kept short */
  snprintf (buf, BABL_MAX_NAME_LEN, "%s %p %p %i", "",
            source, destination, is_reference);
  return buf;
}

int
_babl_fish_path_destroy (void *data);

int
_babl_fish_path_destroy (void *data)
{
  Babl *babl=data;
  if (babl->fish_path.conversion_list)
    babl_free (babl->fish_path.conversion_list);
  babl->fish_path.conversion_list = NULL;
  return 0;
}

static int
show_item (Babl *babl,
           void *user_data)
{
  BablConversion *conv = (void *)babl;

  if (conv->destination->class_type == BABL_FORMAT)
  {
    fprintf (stderr, "%s : %.12f\n", babl_get_name (babl), babl_conversion_error(conv));
  }

  return 0;
}

static inline int
show_fmt (Babl *babl,
          void *user_data)
{
  fprintf (stderr, "[[%s\n", babl_get_name (babl));
  return 0;
}


static int
alias_conversion (Babl *babl,
                  void *user_data)
{
  BablConversion *conv = (void *)babl;
  BablSpace *space = user_data;

  if ((conv->source->class_type == BABL_FORMAT) &&
      (conv->destination->class_type == BABL_FORMAT))
  {
    if ((conv->source->format.space == (void*)babl_space ("sRGB")) &&
        (conv->destination->format.space == babl_space ("sRGB")))
  {
    switch (conv->instance.class_type)
    {
      case BABL_CONVERSION_LINEAR:
       babl_conversion_new (
              babl_format_with_space (
                    (void*)conv->source->instance.name, (void*)space),
              babl_format_with_space (
                    (void*)conv->destination->instance.name, (void*)space),
              "linear", conv->function.linear,
              "data", conv->data,
              NULL);
        break;
      case BABL_CONVERSION_PLANAR:
       babl_conversion_new (
              babl_format_with_space (
                    (void*)conv->source->instance.name, (void*)space),
              babl_format_with_space (
                    (void*)conv->destination->instance.name, (void*)space),
              "planar", conv->function.planar,
              "data", conv->data,
              NULL);
        break;
      case BABL_CONVERSION_PLANE:
        babl_conversion_new (
              babl_format_with_space (
                    (void*)conv->source->instance.name, (void*)space),
              babl_format_with_space (
                    (void*)conv->destination->instance.name, (void*)space),
              "plane", conv->function.plane,
              "data", conv->data,
              NULL);
        break;
      default:
        break;
    }
  }
  }
  else
  if ((conv->source->class_type == BABL_MODEL) &&
      (conv->destination->class_type == BABL_MODEL))
  {
    if ((conv->source->model.space == (void*)babl_space ("sRGB")) &&
        (conv->destination->model.space == babl_space ("sRGB")))
  {
    switch (conv->instance.class_type)
    {
      case BABL_CONVERSION_LINEAR:
        babl_conversion_new (
              babl_remodel_with_space (
                    (void*)conv->source, (void*)space),
              babl_remodel_with_space (
                    (void*)conv->destination, (void*)space),
              "linear", conv->function,
              NULL);
        break;
      case BABL_CONVERSION_PLANAR:
        babl_conversion_new (
              babl_remodel_with_space (
                    (void*)conv->source, (void*)space),
              babl_remodel_with_space (
                    (void*)conv->destination, (void*)space),
              "planar", conv->function,
              NULL);
        break;
      case BABL_CONVERSION_PLANE:
        babl_conversion_new (
              babl_remodel_with_space (
                    (void*)conv->source, (void*)space),
              babl_remodel_with_space (
                    (void*)conv->destination, (void*)space),
              "plane", conv->function,
              NULL);
        break;
      default:
        break;
    }
  }
  }
  else
  if ((conv->source->class_type == BABL_TYPE) &&
      (conv->destination->class_type == BABL_TYPE))
  {
  }
  return 0;
}

static void prep_conversion (const Babl *babl)
{
  Babl *conversion = (void*) babl;
  const Babl *source_space = babl_conversion_get_source_space (conversion);
  float *matrixf;
  int i;
  float *lut;

  double matrix[9];
  babl_matrix_mul_matrix (
     (conversion->conversion.destination)->format.space->space.XYZtoRGB,
     (conversion->conversion.source)->format.space->space.RGBtoXYZ,
     matrix);

  matrixf = babl_calloc (sizeof (float), 9 + 256); // we leak this matrix , which is a singleton
  babl_matrix_to_float (matrix, matrixf);
  conversion->conversion.data = matrixf;

  lut = matrixf + 9;
  for (i = 0; i < 256; i++)
  {
    lut[i] = babl_trc_to_linear (source_space->space.trc[0], i/255.0);
    // XXX: should have green and blue luts as well
  }
}

static inline long
universal_nonlinear_rgb_converter (const Babl *conversion,unsigned char *src_char, unsigned char *dst_char, long samples)
{
  const Babl *source_space = babl_conversion_get_source_space (conversion);
  const Babl *destination_space = babl_conversion_get_destination_space (conversion);

  void   *to_trc_red;
  void   *to_trc_green;
  void   *to_trc_blue;
  float (*to_linear_red) (void *trc, float value);
  float (*to_linear_green) (void *trc, float value);
  float (*to_linear_blue) (void *trc, float value);
  void   *from_trc_red;
  void   *from_trc_green;
  void   *from_trc_blue;
  float (*from_linear_red) (void *trc, float value);
  float (*from_linear_green) (void *trc, float value);
  float (*from_linear_blue) (void *trc, float value);

  float * matrixf = conversion->conversion.data;
  int i;
  float *rgba_in = (void*)src_char;
  float *rgba_out = (void*)dst_char;

  to_linear_red   = (void*)source_space->space.trc[0]->trc.fun_to_linear;
  to_trc_red      = (void*)source_space->space.trc[0];
  from_linear_red = (void*)destination_space->space.trc[0]->trc.fun_from_linear;
  from_trc_red    = (void*)destination_space->space.trc[0];

  to_linear_green = (void*)source_space->space.trc[1]->trc.fun_to_linear;
  to_trc_green    = (void*)source_space->space.trc[1];
  from_linear_green= (void*)destination_space->space.trc[1]->trc.fun_from_linear;
  from_trc_green  = (void*)destination_space->space.trc[1];

  to_linear_blue  = (void*)source_space->space.trc[2]->trc.fun_to_linear;
  to_trc_blue     = (void*)source_space->space.trc[2];
  from_linear_blue= (void*)destination_space->space.trc[2]->trc.fun_from_linear;
  from_trc_blue   = (void*)destination_space->space.trc[2];

  for (i = 0; i < samples; i++)
  {
    rgba_out[i*4]  =to_linear_red(to_trc_red, rgba_in[i*4]);
    rgba_out[i*4+1]=to_linear_green(to_trc_green, rgba_in[i*4+1]);
    rgba_out[i*4+2]=to_linear_blue(to_trc_blue, rgba_in[i*4+1]);
    rgba_out[i*4+3]=rgba_in[3];
  }

  babl_matrix_mul_vectorff_buf4 (matrixf, rgba_out, rgba_out, samples);

  for (i = 0; i < samples; i++)
  {
    rgba_out[0] = from_linear_red(from_trc_red, rgba_out[0]);
    rgba_out[1] = from_linear_green(from_trc_green, rgba_out[1]);
    rgba_out[2] = from_linear_blue(from_trc_blue, rgba_out[2]);
    rgba_in  += 4;
    rgba_out += 4;
  }

  return samples;
}

static inline long
universal_nonlinear_rgb_linear_converter (const Babl *conversion,unsigned char *src_char, unsigned char *dst_char, long samples)
{
  const Babl *source_space = babl_conversion_get_source_space (conversion);

  void   *to_trc_red;
  void   *to_trc_green;
  void   *to_trc_blue;
  float (*to_linear_red) (void *trc, float value);
  float (*to_linear_green) (void *trc, float value);
  float (*to_linear_blue) (void *trc, float value);

  float * matrixf = conversion->conversion.data;
  int i;
  float *rgba_in = (void*)src_char;
  float *rgba_out = (void*)dst_char;

  to_linear_red   = (void*)source_space->space.trc[0]->trc.fun_to_linear;
  to_trc_red      = (void*)source_space->space.trc[0];
  to_linear_green = (void*)source_space->space.trc[1]->trc.fun_to_linear;
  to_trc_green    = (void*)source_space->space.trc[1];
  to_linear_blue  = (void*)source_space->space.trc[2]->trc.fun_to_linear;
  to_trc_blue     = (void*)source_space->space.trc[2];

  for (i = 0; i < samples; i++)
  {
    rgba_out[i*4]=to_linear_red(to_trc_red, rgba_in[0]);
    rgba_out[i*4+1]=to_linear_green(to_trc_green, rgba_in[1]);
    rgba_out[i*4+2]=to_linear_blue(to_trc_blue, rgba_in[2]);
    rgba_out[i*4+3]=rgba_in[3];
    rgba_in  += 4;
  }

  babl_matrix_mul_vectorff_buf4 (matrixf, rgba_out, rgba_out, samples);

  return samples;
}


static inline long
universal_nonlinear_rgba_u8_converter (const Babl *conversion,unsigned char *src_char, unsigned char *dst_char, long samples)
{
  const Babl *destination_space = conversion->conversion.destination->format.space;

  float * matrixf = conversion->conversion.data;
  float * in_trc_lut = matrixf + 9;
  int i;
  uint8_t *rgba_in_u8 = (void*)src_char;
  uint8_t *rgba_out_u8 = (void*)dst_char;

  float *rgb = alloca (sizeof(float) * 3 * samples);

  for (i = 0; i < samples; i++)
  {
    rgb[i*3+0]=in_trc_lut[rgba_in_u8[i*4+0]];
    rgb[i*3+1]=in_trc_lut[rgba_in_u8[i*4+1]];
    rgb[i*3+2]=in_trc_lut[rgba_in_u8[i*4+2]];
  }

  babl_matrix_mul_vectorff_buf3 (matrixf, rgb, rgb, samples);

  {
  const Babl *from_trc_red   = (void*)destination_space->space.trc[0];
  const Babl *from_trc_green = (void*)destination_space->space.trc[1];
  const Babl *from_trc_blue  = (void*)destination_space->space.trc[2];
  float (*from_linear_red)   (const Babl *trc, float value) = from_trc_red->trc.fun_from_linear;
  float (*from_linear_green) (const Babl *trc, float value) = from_trc_green->trc.fun_from_linear;
  float (*from_linear_blue)  (const Babl *trc, float value) = from_trc_blue->trc.fun_from_linear;
  for (i = 0; i < samples; i++)
  {
    rgba_out_u8[0] = from_linear_red (from_trc_red,     rgb[i*3+0]) * 255.5f;
    rgba_out_u8[1] = from_linear_green (from_trc_green, rgb[i*3+1]) * 255.5f;
    rgba_out_u8[2] = from_linear_blue (from_trc_blue ,  rgb[i*3+2]) * 255.5f;
    rgba_out_u8[3] = rgba_in_u8[3];
    rgba_in_u8  += 4;
    rgba_out_u8 += 4;
  }
  }

  return samples;
}


static inline long
universal_nonlinear_rgba_u8_float_converter (const Babl *conversion,unsigned char *src_char, unsigned char *dst_char, long samples)
{
  float * matrixf = conversion->conversion.data;
  float * in_trc_lut = matrixf + 9;
  int i;
  uint8_t *rgba_in_u8 = (void*)src_char;
  float   *rgba_out   = (void*)dst_char;

  for (i = 0; i < samples; i++)
  {
    rgba_out[i*3+0]=in_trc_lut[rgba_in_u8[i*4+0]];
    rgba_out[i*3+1]=in_trc_lut[rgba_in_u8[i*4+1]];
    rgba_out[i*3+2]=in_trc_lut[rgba_in_u8[i*4+2]];
    rgba_out[i*3+2]=rgba_in_u8[i*4+3] / 255.0;
  }
  babl_matrix_mul_vectorff_buf4 (matrixf, rgba_out, rgba_out, samples);

  return samples;
}

static inline long
universal_rgba_converter (const Babl *conversion,unsigned char *src_char, unsigned char *dst_char, long samples)
{
  float *matrixf = conversion->conversion.data;
  float *rgba_in = (void*)src_char;
  float *rgba_out = (void*)dst_char;

  babl_matrix_mul_vectorff_buf4 (matrixf, rgba_in, rgba_out, samples);

  return samples;
}


static inline long
universal_rgb_converter (const Babl *conversion,unsigned char *src_char, unsigned char *dst_char, long samples)
{
  float *matrixf = conversion->conversion.data;
  float *rgb_in = (void*)src_char;
  float *rgb_out = (void*)dst_char;

  babl_matrix_mul_vectorff_buf3 (matrixf, rgb_in, rgb_out, samples);

  return samples;
}

static int
add_rgb_adapter (Babl *babl,
                 void *space)
{
  if (babl != space)
  {
    prep_conversion(babl_conversion_new(babl_format_with_space("RGBA float", space),
                        babl_format_with_space("RGBA float", babl),
                        "linear", universal_rgba_converter,
                        NULL));
    prep_conversion(babl_conversion_new(babl_format_with_space("RGBA float", babl),
                        babl_format_with_space("RGBA float", space),
                        "linear", universal_rgba_converter,
                        NULL));
    prep_conversion(babl_conversion_new(babl_format_with_space("RGB float", space),
                        babl_format_with_space("RGB float", babl),
                        "linear", universal_rgb_converter,
                        NULL));
    prep_conversion(babl_conversion_new(babl_format_with_space("RGB float", babl),
                        babl_format_with_space("RGB float", space),
                        "linear", universal_rgb_converter,
                        NULL));
    prep_conversion(babl_conversion_new(babl_format_with_space("R'G'B'A float", space),
                        babl_format_with_space("R'G'B'A float", babl),
                        "linear", universal_nonlinear_rgb_converter,
                        NULL));
    prep_conversion(babl_conversion_new(babl_format_with_space("R'G'B'A float", babl),
                        babl_format_with_space("R'G'B'A float", space),
                        "linear", universal_nonlinear_rgb_converter,
                        NULL));
    prep_conversion(babl_conversion_new(babl_format_with_space("R'G'B'A float", space),
                        babl_format_with_space("RGBA float", babl),
                        "linear", universal_nonlinear_rgb_linear_converter,
                        NULL));
    prep_conversion(babl_conversion_new(babl_format_with_space("R'G'B'A float", babl),
                        babl_format_with_space("RGBA float", space),
                        "linear", universal_nonlinear_rgb_linear_converter,
                        NULL));
    prep_conversion(babl_conversion_new(babl_format_with_space("R'G'B'A u8", space),
                        babl_format_with_space("R'G'B'A u8", babl),
                        "linear", universal_nonlinear_rgba_u8_converter,
                        NULL));
    prep_conversion(babl_conversion_new(babl_format_with_space("R'G'B'A u8", babl),
                        babl_format_with_space("R'G'B'A u8", space),
                        "linear", universal_nonlinear_rgba_u8_converter,
                        NULL));
    prep_conversion(babl_conversion_new(babl_format_with_space("R'G'B'A u8", space),
                        babl_format_with_space("RGBA float", babl),
                        "linear", universal_nonlinear_rgba_u8_float_converter,
                        NULL));
    prep_conversion(babl_conversion_new(babl_format_with_space("R'G'B'A u8", babl),
                        babl_format_with_space("RGBA float", space),
                        "linear", universal_nonlinear_rgba_u8_float_converter,
                        NULL));
  }
  return 0;
}

static void add_universal_rgb (const Babl *space)
{
  babl_space_class_for_each (add_rgb_adapter, (void*)space);
}

Babl *
babl_fish_path (const Babl *source,
                const Babl *destination)
{
  Babl *babl = NULL;
  const Babl *sRGB = babl_space ("sRGB");
  char name[BABL_MAX_NAME_LEN];

  _babl_fish_create_name (name, source, destination, 1);
  babl_mutex_lock (babl_format_mutex);
  babl = babl_db_exist_by_name (babl_fish_db (), name);
  if (babl)
    {
      /* There is an instance already registered by the required name,
       * returning the preexistent one instead.
       */
      babl_mutex_unlock (babl_format_mutex);
      return babl;
    }

  if ((source->format.space != sRGB) ||
      (destination->format.space != sRGB))
  {
    static const Babl *run_once[512]={NULL};
    int i;
    int done = 0;
    for (i = 0; run_once[i]; i++)
    {
      if (run_once[i] == source->format.space)
        done |= 1;
      else if (run_once[i] == destination->format.space)
        done |= 2;
    }

    if ((done & 1) == 0 && (source->format.space != sRGB))
    {
      run_once[i++] = source->format.space;
      babl_conversion_class_for_each (alias_conversion, (void*)source->format.space);

      add_universal_rgb (source->format.space);
    }
    if ((done & 2) == 0 && (destination->format.space != source->format.space) && (destination->format.space != sRGB))
    {
      run_once[i++] = destination->format.space;
      babl_conversion_class_for_each (alias_conversion, (void*)destination->format.space);

      add_universal_rgb (destination->format.space);
    }

    if (!done)
    {
      if(0)babl_conversion_class_for_each (show_item, (void*)source->format.space);
      //babl_format_class_for_each (show_fmt, NULL);
      //babl_model_class_for_each (show_fmt, NULL);
    }

  }

  babl = babl_calloc (1, sizeof (BablFishPath) +
                      strlen (name) + 1);
  babl_set_destructor (babl, _babl_fish_path_destroy);

  babl->class_type                = BABL_FISH_PATH;
  babl->instance.id               = babl_fish_get_id (source, destination);
  babl->instance.name             = ((char *) babl) + sizeof (BablFishPath);
  strcpy (babl->instance.name, name);
  babl->fish.source               = source;
  babl->fish.destination          = destination;
  babl->fish.processings          = 0;
  babl->fish.pixels               = 0;
  babl->fish.error                = BABL_MAX_COST_VALUE;
  babl->fish_path.cost            = BABL_MAX_COST_VALUE;
  babl->fish_path.conversion_list = babl_list_init_with_size (BABL_HARD_MAX_PATH_LENGTH);

  {
    PathContext pc;
    pc.current_path = babl_list_init_with_size (BABL_HARD_MAX_PATH_LENGTH);
    pc.fish_path = babl;
    pc.to_format = (Babl *) destination;

    /* we hold a global lock whilerunning get_conversion_path since
     * it depends on keeping the various format.visited members in
     * a consistent state, this code path is not performance critical
     * since created fishes are cached.
     */
    babl_in_fish_path++;

    get_conversion_path (&pc, (Babl *) source, 0, max_path_length ());

    /* second attempt,. at path length + 1*/
    if (babl->fish_path.conversion_list->count == 0 &&
        max_path_length () + 1 <= BABL_HARD_MAX_PATH_LENGTH)
      get_conversion_path (&pc, (Babl *) source, 0, max_path_length () + 1);

    babl_in_fish_path--;
    babl_free (pc.current_path);
  }

  if (babl_list_size (babl->fish_path.conversion_list) == 0)
    {
      babl_free (babl);
      babl_mutex_unlock (babl_format_mutex);

#ifndef BABL_UNSTABLE
      if (debug_conversions)
#endif
      {
        static int warnings = 0;

        if (_babl_legal_error() <= 0.0000000001)
            return NULL;

        if (warnings++ == 0)
          fprintf (stderr,
"Missing fast-path babl conversion detected, Implementing missing babl fast paths\n"
"accelerates GEGL, GIMP and other software using babl, warnings are printed on\n"
"first occurance of formats used where a conversion has to be synthesized\n"
"programmatically by babl based on format description\n"
"\n");

        fprintf (stderr, "*WARNING* missing babl fast path(s): \"%s\" to \"%s\"\n",
           babl_get_name (source),
           babl_get_name (destination));

      }
      return NULL;
    }

  /* Since there is not an already registered instance by the required
   * name, inserting newly created class into database.
   */
  babl_db_insert (babl_fish_db (), babl);
  babl_mutex_unlock (babl_format_mutex);
  return babl;
}

static long
babl_fish_path_process (Babl       *babl,
                        const void *source,
                        void       *destination,
                        long        n)
{
   const Babl *babl_source = babl->fish.source;
   const Babl *babl_dest = babl->fish.destination;
   int source_bpp = 0;
   int dest_bpp = 0;

   switch (babl_source->instance.class_type)
     {
       case BABL_FORMAT:
         source_bpp = babl_source->format.bytes_per_pixel;
         break;
       case BABL_TYPE:
         source_bpp = babl_source->type.bits / 8;
         break;
       default:
         babl_log ("=eeek{%i}\n", babl_source->instance.class_type - BABL_MAGIC);
     }

   switch (babl_dest->instance.class_type)
     {
       case BABL_FORMAT:
         dest_bpp = babl_dest->format.bytes_per_pixel;
         break;
       case BABL_TYPE:
         dest_bpp = babl_dest->type.bits / 8;
         break;
       default:
         babl_log ("-eeek{%i}\n", babl_dest->instance.class_type - BABL_MAGIC);
     }

  return process_conversion_path (babl->fish_path.conversion_list,
                                  source,
                                  source_bpp,
                                  destination,
                                  dest_bpp,
                                  n);
}

static long
babl_fish_process (Babl       *babl,
                   const void *source,
                   void       *destination,
                   long        n)
{
  long ret = 0;

  switch (babl->class_type)
    {
      case BABL_FISH_REFERENCE:
        if (babl->fish.source == babl->fish.destination)
          { /* XXX: we're assuming linear buffers */
            memcpy (destination, source, n * babl->fish.source->format.bytes_per_pixel);
            ret = n;
          }
        else
          {
            ret = babl_fish_reference_process (babl, source, destination, n);
          }
        break;

      case BABL_FISH_SIMPLE:
        if (BABL (babl->fish_simple.conversion)->class_type == BABL_CONVERSION_LINEAR)
          {
            ret = babl_conversion_process (BABL (babl->fish_simple.conversion),
                                           source, destination, n);
          }
        else
          {
            babl_fatal ("Cannot use a simple fish to process without a linear conversion");
          }
        break;

      case BABL_FISH_PATH:
        ret = babl_fish_path_process (babl, source, destination, n);
        break;

      default:
        babl_log ("NYI");
        ret = -1;
        break;
    }
  return ret;
}

long
babl_process (const Babl *cbabl,
              const void *source,
              void       *destination,
              long        n)
{
  Babl *babl = (Babl*)cbabl;
  babl_assert (babl);
  babl_assert (source);
  babl_assert (destination);
  babl_assert (BABL_IS_BABL (babl));
  if (n == 0)
    return 0;
  babl_assert (n > 0);

  /* first check if it is a fish since that is our fast path */
  if (babl->class_type >= BABL_FISH &&
      babl->class_type <= BABL_FISH_PATH)
    {
      babl->fish.processings++;
      babl->fish.pixels +=
             babl_fish_process (babl, source, destination, n);
      return n;
    }

  /* matches all conversion classes */
  if (babl->class_type >= BABL_CONVERSION &&
      babl->class_type <= BABL_CONVERSION_PLANAR)
    return babl_conversion_process (babl, source, destination, n);

  babl_fatal ("eek");
  return -1;
}

#include <stdint.h>

#define BABL_ALIGN 16
static void inline *align_16 (unsigned char *ret)
{
  int offset = BABL_ALIGN - ((uintptr_t) ret) % BABL_ALIGN;
  ret = ret + offset;
  return ret;
}

static long
process_conversion_path (BablList   *path,
                         const void *source_buffer,
                         int         source_bpp,
                         void       *destination_buffer,
                         int         dest_bpp,
                         long        n)
{
  int conversions = babl_list_size (path);

  if (conversions == 1)
    {
      babl_conversion_process (BABL (babl_list_get_first (path)),
                               source_buffer,
                               destination_buffer,
                               n);
    }
  else
    {
      long j;

      void *temp_buffer = align_16 (alloca (MIN(n, MAX_BUFFER_SIZE) * sizeof (double) * 5 + 16));
      void *temp_buffer2 = NULL;

      if (conversions > 2)
        {
          /* We'll need one more auxiliary buffer */
          temp_buffer2 = align_16 (alloca (MIN(n, MAX_BUFFER_SIZE) * sizeof (double) * 5 + 16));
        }

      for (j = 0; j < n; j+= MAX_BUFFER_SIZE)
        {
          long c = MIN (n - j, MAX_BUFFER_SIZE);
          int i;

          /* this is where the loop unrolling should happen */
          void *aux1_buffer = temp_buffer;
          void *aux2_buffer = NULL;
          void *swap_buffer = NULL;
          aux2_buffer = temp_buffer2;

          /* The first conversion goes from source_buffer to aux1_buffer */
          babl_conversion_process (babl_list_get_first (path),
                                   (void*)(((unsigned char*)source_buffer) + (j * source_bpp)),
                                   aux1_buffer,
                                   c);

          /* Process, if any, conversions between the first and the last
           * conversion in the path, in a loop */
          for (i = 1; i < conversions - 1; i++)
            {
              babl_conversion_process (path->items[i],
                                       aux1_buffer,
                                       aux2_buffer,
                                       c);
              /* Swap the auxiliary buffers */
              swap_buffer = aux1_buffer;
              aux1_buffer = aux2_buffer;
              aux2_buffer = swap_buffer;
            }

          /* The last conversion goes from aux1_buffer to destination_buffer */
          babl_conversion_process (babl_list_get_last (path),
                                   aux1_buffer,
                                   (void*)((unsigned char*)destination_buffer + (j * dest_bpp)),
                                   c);
        }
  }

  return n;
}

static void
init_path_instrumentation (FishPathInstrumentation *fpi,
                           Babl                    *fmt_source,
                           Babl                    *fmt_destination)
{
  long   ticks_start = 0;
  long   ticks_end   = 0;

  const double *test_pixels = babl_get_path_test_pixels ();

  if (!fpi->fmt_rgba_double)
    {
      fpi->fmt_rgba_double = babl_format_with_space ("RGBA double",
                                                     fmt_destination->format.space);
    }

  fpi->num_test_pixels = babl_get_num_path_test_pixels ();

  fpi->fish_rgba_to_source      = babl_fish_reference (fpi->fmt_rgba_double,
                                                  fmt_source);
  fpi->fish_reference           = babl_fish_reference (fmt_source,
                                                  fmt_destination);
  fpi->fish_destination_to_rgba = babl_fish_reference (fmt_destination,
                                                  fpi->fmt_rgba_double);

  fpi->source                      = babl_calloc (fpi->num_test_pixels,
                                             fmt_source->format.bytes_per_pixel);
  fpi->destination                 = babl_calloc (fpi->num_test_pixels,
                                             fmt_destination->format.bytes_per_pixel);
  fpi->ref_destination             = babl_calloc (fpi->num_test_pixels,
                                             fmt_destination->format.bytes_per_pixel);
  fpi->destination_rgba_double     = babl_calloc (fpi->num_test_pixels,
                                             fpi->fmt_rgba_double->format.bytes_per_pixel);
  fpi->ref_destination_rgba_double = babl_calloc (fpi->num_test_pixels,
                                             fpi->fmt_rgba_double->format.bytes_per_pixel);

  /* create sourcebuffer from testbuffer in the correct format */
  babl_process (fpi->fish_rgba_to_source,
                test_pixels, fpi->source, fpi->num_test_pixels);

  /* calculate the reference buffer of how it should be */
  ticks_start = babl_ticks ();
  babl_process (fpi->fish_reference,
                fpi->source, fpi->ref_destination, fpi->num_test_pixels);
  ticks_end = babl_ticks ();
  fpi->reference_cost = ticks_end - ticks_start;

  /* transform the reference destination buffer to RGBA */
  babl_process (fpi->fish_destination_to_rgba,
                fpi->ref_destination, fpi->ref_destination_rgba_double, fpi->num_test_pixels);
}

static void
destroy_path_instrumentation (FishPathInstrumentation *fpi)
{
  if (fpi->init_instrumentation_done)
    {
      babl_free (fpi->source);
      babl_free (fpi->destination);
      babl_free (fpi->destination_rgba_double);
      babl_free (fpi->ref_destination);
      babl_free (fpi->ref_destination_rgba_double);

      /* nulify the flag for potential new search */
      fpi->init_instrumentation_done = 0;
  }
}

static void
get_path_instrumentation (FishPathInstrumentation *fpi,
                          BablList                *path,
                          double                  *path_cost,
                          double                  *ref_cost,
                          double                  *path_error)
{
  long   ticks_start = 0;
  long   ticks_end   = 0;

  Babl *babl_source = fpi->source;
  Babl *babl_destination = fpi->destination;

  int source_bpp = 0;
  int dest_bpp = 0;

  switch (babl_source->instance.class_type)
    {
      case BABL_FORMAT:
        source_bpp = babl_source->format.bytes_per_pixel;
        break;
      case BABL_TYPE:
        source_bpp = babl_source->type.bits / 8;
        break;
      default:
        babl_log ("=eeek{%i}\n", babl_source->instance.class_type - BABL_MAGIC);
    }

  switch (babl_destination->instance.class_type)
    {
      case BABL_FORMAT:
        dest_bpp = babl_destination->format.bytes_per_pixel;
        break;
      case BABL_TYPE:
        dest_bpp = babl_destination->type.bits / 8;
        break;
      default:
        babl_log ("-eeek{%i}\n", babl_destination->instance.class_type - BABL_MAGIC);
     }

  if (!fpi->init_instrumentation_done)
    {
      /* this initialization can be done only once since the
       * source and destination formats do not change during
       * the search */
      init_path_instrumentation (fpi, babl_source, babl_destination);
      fpi->init_instrumentation_done = 1;
    }

  /* calculate this path's view of what the result should be */
  ticks_start = babl_ticks ();
  process_conversion_path (path, fpi->source, source_bpp, fpi->destination, dest_bpp, fpi->num_test_pixels);
  ticks_end = babl_ticks ();
  *path_cost = ticks_end - ticks_start;

  /* transform the reference and the actual destination buffers to RGBA
   * for comparison with each other
   */
  babl_process (fpi->fish_destination_to_rgba,
                fpi->destination, fpi->destination_rgba_double, fpi->num_test_pixels);

  *path_error = babl_rel_avg_error (fpi->destination_rgba_double,
                                    fpi->ref_destination_rgba_double,
                                     fpi->num_test_pixels * 4);

  *ref_cost = fpi->reference_cost;
}
