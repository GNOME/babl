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
#include <math.h>
#include "babl-internal.h"
#include "babl-ref-pixels.h"

#define BABL_TOLERANCE             0.0000047
#define BABL_MAX_COST_VALUE        2000000
#define BABL_HARD_MAX_PATH_LENGTH  8
#define BABL_MAX_NAME_LEN          1024

#define BABL_TEST_ITER             4

#ifndef MIN
#define MIN(a, b) (((a) > (b)) ? (b) : (a))
#endif

#define NUM_TEST_PIXELS            (babl_get_num_path_test_pixels ())
#define MAX_BUFFER_SIZE            512


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


static inline void
process_conversion_path (BablList   *path,
                         const void *source_buffer,
                         int         source_bpp,
                         void       *destination_buffer,
                         int         dest_bpp,
                         long        n);

static void
get_conversion_path (PathContext *pc,
                     Babl        *current_format,
                     int          current_length,
                     int          max_length,
                     double       legal_error);

char *
_babl_fish_create_name (char       *buf,
                        const Babl *source,
                        const Babl *destination,
                        int         is_reference);


static int max_path_length (void);

static int debug_conversions = 0;
int _babl_instrument = 0;

double 
_babl_legal_error (void)
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

  env = getenv ("BABL_INSTRUMENT");
  if (env && env[0] != '\0')
    _babl_instrument = 1;
  else
    _babl_instrument = 0;

  return error;
}

static int 
max_path_length (void)
{
  static int  max_length = 0;
  const char *env;

  if (max_length != 0)
    return max_length;

  env = getenv ("BABL_PATH_LENGTH");
  if (env)
    max_length = atoi (env);
  else
    max_length = 4; /* reducing this number makes finding short fishes much
                       faster - even if we lose out on some of the fast
                       bigger fish
                     */
  if (max_length > BABL_HARD_MAX_PATH_LENGTH)
    max_length = BABL_HARD_MAX_PATH_LENGTH;
  else if (max_length <= 0)
    max_length = 1;
  return max_length;
}

int 
_babl_max_path_len (void)
{
  return max_path_length ();
}

static int
bad_idea (const Babl *from, const Babl *to, const Babl *format)
{
  if (babl_format_has_alpha (from) &&
      babl_format_has_alpha (to) &&
      !babl_format_has_alpha (format))
  {
    return 1;
  }
  if (from->format.components > format->format.components &&
      to->format.components > format->format.components)
  {
    return 1;
  }
  if (from->format.type[0]->bits > format->format.type[0]->bits &&
      to->format.type[0]->bits > format->format.type[0]->bits)
  {
    /* XXX: perhaps we especially avoid going to half-float, when
     * going between u16 formats as well? */
    return 1;
  }

  return 0;
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
                     int          max_length,
                     double       legal_error)
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

      if (path_error - 1.0 <= legal_error )
                /* check this before the more accurate measurement of error -
                   to bail earlier, this also leads to a stricter
                   discarding of bad fast paths  */
#endif
        {
          FishPathInstrumentation fpi;
          memset (&fpi, 0, sizeof (fpi));

          fpi.source = (Babl*) babl_list_get_first (pc->current_path)->conversion.source;
          fpi.destination = pc->to_format;

          get_path_instrumentation (&fpi, pc->current_path, &path_cost, &ref_cost, &path_error);
          if(debug_conversions && current_length == 1)
            fprintf (stderr, "%s  error:%f cost:%f  \n",
                 babl_get_name (pc->current_path->items[0]), path_error, path_cost);

          if ((path_cost < ref_cost) && /* do not use paths that took longer to compute than reference */
              (path_cost < pc->fish_path->fish_path.cost) && // best thus far
              (path_error <= legal_error )               // within tolerance
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
              if (!next_format->format.visited && !bad_idea (current_format, pc->to_format, next_format))
                {
                  /* next_format is not in the current path, we can pay a visit */
                  babl_list_insert_last (pc->current_path, next_conversion);
                  get_conversion_path (pc, next_format, current_length + 1, max_length, legal_error);
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

static int
alias_conversion (Babl *babl,
                  void *user_data)
{
  BablConversion *conv = (void *)babl;
  BablSpace *space = user_data;

  if ((conv->source->class_type == BABL_FORMAT) &&
      (conv->destination->class_type == BABL_FORMAT) &&
      (!babl_format_is_palette (conv->source)) &&
      (!babl_format_is_palette (conv->destination)))
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

void
_babl_fish_prepare_bpp (Babl *babl)
{
   const Babl *babl_source = babl->fish.source;
   const Babl *babl_dest = babl->fish.destination;

   switch (babl_source->instance.class_type)
     {
       case BABL_FORMAT:
         babl->fish_path.source_bpp = babl_source->format.bytes_per_pixel;
         break;
       case BABL_TYPE:
         babl->fish_path.source_bpp = babl_source->type.bits / 8;
         break;
       default:
         babl_log ("=eeek{%i}\n", babl_source->instance.class_type - BABL_MAGIC);
     }

   switch (babl_dest->instance.class_type)
     {
       case BABL_FORMAT:
         babl->fish_path.dest_bpp = babl_dest->format.bytes_per_pixel;
         break;
       case BABL_TYPE:
         babl->fish_path.dest_bpp = babl_dest->type.bits / 8;
         break;
       default:
         babl_log ("-eeek{%i}\n", babl_dest->instance.class_type - BABL_MAGIC);
     }
}

void
_babl_fish_missing_fast_path_warning (const Babl *source,
                                      const Babl *destination)
{
#ifndef BABL_UNSTABLE
  if (debug_conversions)
#endif
  {
    static int warnings = 0;

    if (_babl_legal_error() <= 0.0000000001)
      return;

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
}


static Babl *
babl_fish_path2 (const Babl *source,
                 const Babl *destination,
                 double      tolerance)
{
  Babl *babl = NULL;
  const Babl *sRGB = babl_space ("sRGB");
  char name[BABL_MAX_NAME_LEN];
  int is_fast = 0;

  _babl_fish_create_name (name, source, destination, 1);
  babl_mutex_lock (babl_format_mutex);
  babl = babl_db_exist_by_name (babl_fish_db (), name);

  if (tolerance <= 0.0)
  {
    is_fast = 0;
    tolerance = _babl_legal_error ();
  }
  else
    is_fast = 1;

  if (!is_fast)
  {
  if (babl)
    {
      /* There is an instance already registered by the required name,
       * returning the preexistent one instead.
       */
      babl_mutex_unlock (babl_format_mutex);
      return babl;
    }
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

      _babl_space_add_universal_rgb (source->format.space);
    }
    if ((done & 2) == 0 && (destination->format.space != source->format.space) && (destination->format.space != sRGB))
    {
      run_once[i++] = destination->format.space;
      babl_conversion_class_for_each (alias_conversion, (void*)destination->format.space);

      _babl_space_add_universal_rgb (destination->format.space);
    }

    if (!done && 0)
    {
      babl_conversion_class_for_each (show_item, (void*)source->format.space);
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

    get_conversion_path (&pc, (Babl *) source, 0, max_path_length (), tolerance);

    /* second attempt,. at path length + 1*/
    if (babl->fish_path.conversion_list->count == 0 &&
        max_path_length () + 1 <= BABL_HARD_MAX_PATH_LENGTH)
    {
      get_conversion_path (&pc, (Babl *) source, 0, max_path_length () + 1, tolerance);

#if 0
      if (babl->fish_path.conversion_list->count)
      {
        fprintf (stderr, "babl is using a rather long chain, room exists for optimization here\n");
        babl_list_each (babl->fish_path.conversion_list, show_item, NULL);
      }
#endif
    }

    /* third attempt,. at path length + 2 */
    if (babl->fish_path.conversion_list->count == 0 &&
        max_path_length () + 2 <= BABL_HARD_MAX_PATH_LENGTH)
    {
      get_conversion_path (&pc, (Babl *) source, 0, max_path_length () + 2, tolerance);
#if 0
      if (babl->fish_path.conversion_list->count)
      {
        fprintf (stderr, "babl is using very long chain, should be optimized\n");
        babl_list_each (babl->fish_path.conversion_list, show_item, NULL);
      }
#endif
    }

    babl_in_fish_path--;
    babl_free (pc.current_path);
  }

  if (babl_list_size (babl->fish_path.conversion_list) == 0)
    {
      babl_free (babl);
      babl_mutex_unlock (babl_format_mutex);

      /* it is legitimate for reference paths to be faster than long chains
         of paths, thus it is time to retire this warning. XXX: remove it fully 

          _babl_fish_missing_fast_path_warning (source, destination); */

      return NULL;
    }

  _babl_fish_prepare_bpp (babl);
  _babl_fish_rig_dispatch (babl);
  /* Since there is not an already registered instance by the required
   * name, inserting newly created class into database.
   */
  if (!is_fast)
  {
    babl_db_insert (babl_fish_db (), babl);
  }
  babl_mutex_unlock (babl_format_mutex);
  return babl;
}

const Babl * 
babl_fast_fish (const void *source_format,
                const void *destination_format,
                const char *performance)
{
  double tolerance = 0.0;

  if (!performance || !strcmp (performance, "default"))
    tolerance = 0.0; // note: not _babl_legal_error() to trigger,
                      // right code paths in babl_fish_path2
  else if (!strcmp (performance, "exact"))
    tolerance=0.0000000001;
  else if (!strcmp (performance, "precise"))
    tolerance=0.00001;
  if (!strcmp (performance, "fast"))
    tolerance=0.001;
  else if (!strcmp (performance, "glitch"))
    tolerance=0.01;
  else {
    tolerance = babl_parse_double (performance);
  }

  return babl_fish_path2 (source_format, destination_format, tolerance);
}

Babl *
babl_fish_path (const Babl *source,
                const Babl *destination)
{
  return babl_fish_path2 (source, destination, 0.0);
}


static void
babl_fish_path_process (const Babl *babl,
                        const char *source,
                        char       *destination,
                        long        n,
                        void       *data)
{
  process_conversion_path (babl->fish_path.conversion_list,
                           source,
                           babl->fish_path.source_bpp,
                           destination,
                           babl->fish_path.dest_bpp,
                           n);
}

static void
babl_fish_memcpy_process (const Babl *babl,
                          const char *source,
                          char       *destination,
                          long        n,
                          void       *data)
{
  memcpy (destination, source, n * babl->fish.source->format.bytes_per_pixel);
}

void
_babl_fish_rig_dispatch (Babl *babl)
{
  babl->fish.data     = (void*)&(babl->fish.data);

  if (babl->fish.source == babl->fish.destination)
    {
      babl->fish.dispatch = babl_fish_memcpy_process;
      return;
    }

  switch (babl->class_type)
    {
      case BABL_FISH_REFERENCE:
        babl->fish.dispatch = babl_fish_reference_process;
        break;

      case BABL_FISH_SIMPLE:
        if (BABL (babl->fish_simple.conversion)->class_type == BABL_CONVERSION_LINEAR)
          {
            /* lift out conversion from single step conversion and make it be the dispatch function
             * itself
             */
            babl->fish.data     = &(babl->fish_simple.conversion->data);
            babl->fish.dispatch = babl->fish_simple.conversion->dispatch;
          }
        else
          {
            babl_fatal ("Cannot use a simple fish to process without a linear conversion");
          }
        break;

      case BABL_FISH_PATH:
        if (babl_list_size(babl->fish_path.conversion_list) == 1)
        {
          BablConversion *conversion = (void*)babl_list_get_first(babl->fish_path.conversion_list);

          /* do same short-circuit optimization as for simple fishes */
          babl->fish.dispatch = conversion->dispatch;
          babl->fish.data     = &conversion->data;
        }
        else
        {
          babl->fish.dispatch = babl_fish_path_process;
        }
        break;

      case BABL_CONVERSION:
      case BABL_CONVERSION_LINEAR:
      case BABL_CONVERSION_PLANE:
      case BABL_CONVERSION_PLANAR:
        babl_assert (0);
        break;

      default:
        babl_log ("NYI");
        break;
    }
}

static long
_babl_process (const Babl *cbabl,
               const void *source,
               void       *destination,
               long        n)
{
  Babl *babl = (void*)cbabl;
  babl->fish.dispatch (babl, source, destination, n, *babl->fish.data);
  if (_babl_instrument)
    babl->fish.pixels += n;
  return n;
}

long
babl_process (const Babl *babl,
              const void *source,
              void       *destination,
              long        n)
{
  return _babl_process ((void*)babl, source, destination, n);
}

long
babl_process_rows (const Babl *fish,
                   const void *source,
                   int         source_stride,
                   void       *dest,
                   int         dest_stride,
                   long        n,
                   int         rows)
{
  Babl          *babl = (Babl*)fish;
  const uint8_t *src  = source;
  uint8_t       *dst  = dest;
  int            row;

  babl_assert (babl && BABL_IS_BABL (babl) && source && dest);

  if (n <= 0)
    return 0;

  if (_babl_instrument)
    babl->fish.pixels += n * rows;
  for (row = 0; row < rows; row++)
    {
      babl->fish.dispatch (babl, (void*)src, (void*)dst, n, *babl->fish.data);

      src += source_stride;
      dst += dest_stride;
    }
  return n * rows;
}

#include <stdint.h>

#define BABL_ALIGN 16
static void inline *align_16 (unsigned char *ret)
{
  int offset = BABL_ALIGN - ((uintptr_t) ret) % BABL_ALIGN;
  ret = ret + offset;
  return ret;
}

static inline void
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

      void *temp_buffer = align_16 (alloca (MIN(n, MAX_BUFFER_SIZE) *
                                    sizeof (double) * 5 + 16));
      void *temp_buffer2 = NULL;

      if (conversions > 2)
        {
          /* We'll need one more auxiliary buffer */
          temp_buffer2 = align_16 (alloca (MIN(n, MAX_BUFFER_SIZE) *
                                   sizeof (double) * 5 + 16));
        }

      for (j = 0; j < n; j+= MAX_BUFFER_SIZE)
        {
          long c = MIN (n - j, MAX_BUFFER_SIZE);
          int i;

          void *aux1_buffer = temp_buffer;
          void *aux2_buffer = temp_buffer2;

          /* The first conversion goes from source_buffer to aux1_buffer */
          babl_conversion_process (babl_list_get_first (path),
                                   (void*)(((unsigned char*)source_buffer) +
                                                          (j * source_bpp)),
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
              {
                /* Swap the auxiliary buffers */
                void *swap_buffer = aux1_buffer;
                aux1_buffer = aux2_buffer;
                aux2_buffer = swap_buffer;
              }
            }

          /* The last conversion goes from aux1_buffer to destination_buffer */
          babl_conversion_process (babl_list_get_last (path),
                                   aux1_buffer,
                                   (void*)((unsigned char*)destination_buffer +
                                                           (j * dest_bpp)),
                                   c);
        }
  }
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
      fpi->fmt_rgba_double =
          babl_format_with_space ("RGBA double",
                                  fmt_destination->format.space);
    }

  fpi->num_test_pixels = babl_get_num_path_test_pixels ();

  fpi->fish_rgba_to_source =
      babl_fish_reference (fpi->fmt_rgba_double, fmt_source);

  fpi->fish_reference =
      babl_fish_reference (fmt_source, fmt_destination);

  fpi->fish_destination_to_rgba =
      babl_fish_reference (fmt_destination, fpi->fmt_rgba_double);

  fpi->source =
      babl_calloc (fpi->num_test_pixels,
                   fmt_source->format.bytes_per_pixel);

  fpi->destination =
      babl_calloc (fpi->num_test_pixels,
                   fmt_destination->format.bytes_per_pixel);

  fpi->ref_destination =
      babl_calloc (fpi->num_test_pixels,
                   fmt_destination->format.bytes_per_pixel);

  fpi->destination_rgba_double =
      babl_calloc (fpi->num_test_pixels,
                   fpi->fmt_rgba_double->format.bytes_per_pixel);

  fpi->ref_destination_rgba_double =
      babl_calloc (fpi->num_test_pixels,
                   fpi->fmt_rgba_double->format.bytes_per_pixel);

  /* create sourcebuffer from testbuffer in the correct format */
  _babl_process (fpi->fish_rgba_to_source,
                 test_pixels, fpi->source,fpi->num_test_pixels);

  /* calculate the reference buffer of how it should be */
  ticks_start = babl_ticks ();
  _babl_process (fpi->fish_reference,
                 fpi->source, fpi->ref_destination,
                 fpi->num_test_pixels);
  ticks_end = babl_ticks ();
  fpi->reference_cost = (ticks_end - ticks_start) * BABL_TEST_ITER;

  /* transform the reference destination buffer to RGBA */
  _babl_process (fpi->fish_destination_to_rgba,
                 fpi->ref_destination, fpi->ref_destination_rgba_double,
                 fpi->num_test_pixels);
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
        babl_log ("-eeek{%i}\n",
                  babl_destination->instance.class_type - BABL_MAGIC);
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
  for (int i = 0; i < BABL_TEST_ITER; i ++)
  process_conversion_path (path, fpi->source, source_bpp, fpi->destination,
                           dest_bpp, fpi->num_test_pixels);
  ticks_end = babl_ticks ();
  *path_cost = (ticks_end - ticks_start);

  /* transform the reference and the actual destination buffers to RGBA
   * for comparison with each other
   */
  _babl_process (fpi->fish_destination_to_rgba,
                 fpi->destination, fpi->destination_rgba_double,
                 fpi->num_test_pixels);

  *path_error = babl_rel_avg_error (fpi->destination_rgba_double,
                                    fpi->ref_destination_rgba_double,
                                    fpi->num_test_pixels * 4);

  *ref_cost = fpi->reference_cost;
}
