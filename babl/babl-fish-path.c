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

#include <math.h>
#include "babl-internal.h"

#define BABL_LEGAL_ERROR    0.000001
#define BABL_MAX_COST_VALUE 2000000

static void
init_path_instrumentation (Babl *fmt_source,
                           Babl *fmt_destination);

static void
destroy_path_instrumentation (void);

static void
get_path_instrumentation (BablList *path,
                          double   *path_cost,
                          double   *ref_cost,
                          double   *path_error);

static long
process_conversion_path (BablList *path,
                         void     *source_buffer,
                         void     *destination_buffer,
                         long     n);

static void
get_conversion_path (Babl *current_format,
                     int current_length,
                     int max_length);

static double *
test_create (void);

static char *
create_name (const Babl *source,
             const Babl *destination,
             int         is_reference);

static double legal_error (void);

static int max_path_length (void);


static double legal_error (void)
{
  static double error = 0.0;
  const char   *env;

  if (error != 0.0)
    return error;

  env = getenv ("BABL_ACCURACY");
  if (env)
    error = atof (env);
  else
    error = BABL_LEGAL_ERROR;
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
    max_length = 4;
  if (max_length > BABL_HARD_MAX_PATH_LENGTH)
    max_length = BABL_HARD_MAX_PATH_LENGTH;
  else if (max_length <= 0)
    max_length = 1;
  return max_length;
}


/* The task of BablFishPath construction is to compute
 * the shortest path in a graph where formats are the vertices
 * and conversions are the edges. However, there is an additional
 * constraint to the shortest path, that limits conversion error
 * introduced by such a path to be less than BABL_ACCURACY. This
 * prohibits usage of any reasonable shortest path construction
 * algorithm such as Dijkstra's algorithm. The shortest path is
 * constructed by enumerating all available paths that are less
 * than BABL_PATH_LENGTH long, computing their costs and
 * conversion errors and backtracking. The backtracking is
 * implemented by recursive function get_conversion_path ().
 */

static Babl *fish_path;
static Babl *to_format;
static BablList *current_path;

static void
get_conversion_path (Babl *current_format,
                     int current_length,
                     int max_length)
{
  if (current_length > max_length)
    {
      /* We have reached the maximum recursion
       * depth, let's bail out */
      return;
    }
  else if ((current_length > 0) && (current_format == to_format))
    {
       /* We have found a candidate path, let's
        * see about it's properties */
      double path_cost  = 0.0;
      double ref_cost   = 0.0;
      double path_error = 1.0;
      int    i;

      for (i = 0; i < babl_list_size (current_path); i++)
        {
          path_error *= (1.0 + babl_conversion_error ((BablConversion *) current_path->items[i]));
        }

      if (path_error - 1.0 <= legal_error ()) /* check this before the next;
                                                 which does a more accurate
                                                 measurement of the error */
        {
          get_path_instrumentation (current_path, &path_cost, &ref_cost, &path_error);

          if ((path_cost < ref_cost) && /* do not use paths that took longer to compute than reference */
              (path_cost < fish_path->fish_path.cost) &&
              (path_error <= legal_error ()))
            {
              /* We have found the best path so far,
               * let's copy it into our new fish */
              fish_path->fish_path.cost = path_cost;
              fish_path->fish.error  = path_error;
              babl_list_copy (current_path,
                              fish_path->fish_path.conversion_list);
            }
        }
    }
  else
    {
      /*
       * Bummer, we have to search deeper... */
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
                  babl_list_insert_last (current_path, next_conversion);
                  get_conversion_path (next_format, current_length + 1, max_length);
                  babl_list_remove_last (current_path);
                }
            }

          /* Remove the current format from current path */
          current_format->format.visited = 0;
        }
      }
}

static char buf[1024];

static char *
create_name (const Babl *source,
             const Babl *destination,
             int         is_reference)
{
  /* fish names are intentionally kept short */
  snprintf (buf, 1024, "%s %p %p", "",
            source, destination);
  return buf;
}

Babl *
babl_fish_path (const Babl *source,
                const Babl *destination)
{
  Babl *babl = NULL;
  char *name;

  name = create_name (source, destination, 1);
  babl = babl_db_exist_by_name (babl_fish_db (), name);
  if (babl)
    {
      /* There is an instance already registered by the required name,
       * returning the preexistent one instead.
       */
      return babl;
    }

  babl = babl_calloc (1, sizeof (BablFishPath) +
                      strlen (name) + 1);

  babl->class_type                = BABL_FISH_PATH;
  babl->instance.id               = babl_fish_get_id (source, destination);
  babl->instance.name             = ((char *) babl) + sizeof (BablFishPath);
  strcpy (babl->instance.name, name);
  babl->fish.source               = source;
  babl->fish.destination          = destination;
  babl->fish.processings          = 0;
  babl->fish.pixels               = 0;
  babl->fish.usecs                = 0;
  babl->fish.error                = BABL_MAX_COST_VALUE;
  babl->fish_path.cost            = BABL_MAX_COST_VALUE;
  babl->fish_path.loss            = BABL_MAX_COST_VALUE;
  babl->fish_path.conversion_list = babl_list_init_with_size (BABL_HARD_MAX_PATH_LENGTH);

  current_path = babl_list_init_with_size (BABL_HARD_MAX_PATH_LENGTH);
  fish_path = babl;
  to_format = (Babl *) destination;

  get_conversion_path ((Babl *) source, 0, max_path_length ());
  destroy_path_instrumentation ();

  babl_list_destroy (current_path);

  if (babl_list_size (babl->fish_path.conversion_list) == 0)
    {
      babl_list_destroy (babl->fish_path.conversion_list);
      babl_free (babl);
      return NULL;
    }

  /* Since there is not an already registered instance by the required
   * name, inserting newly created class into database.
   */
  babl_db_insert (babl_fish_db (), babl);
  return babl;
}

long
babl_fish_path_process (Babl *babl,
                        void *source,
                        void *destination,
                        long  n)
{
  long ret;

  babl_assert (source);
  babl_assert (destination);

  ret = process_conversion_path (babl->fish_path.conversion_list,
                                 source,
                                 destination,
                                 n);

  return ret;
}

static long
process_conversion_path (BablList *path,
                         void     *source_buffer,
                         void     *destination_buffer,
                         long     n)
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
      void *aux1_buffer = babl_malloc (n * sizeof (double) * 5);
      void *aux2_buffer = NULL;
      void *swap_buffer = NULL;
      int   i;

      if (conversions > 2)
        {
          /* We'll need one more auxiliary buffer */
          aux2_buffer = babl_malloc (n * sizeof (double) * 5);
        }

      /* The first conversion goes from source_buffer to aux1_buffer */
      babl_conversion_process (babl_list_get_first (path),
                               source_buffer,
                               aux1_buffer,
                               n);

      /* Process, if any, conversions between the first and the last
       * conversion in the path, in a loop */
      for (i = 1; i < conversions - 1; i++)
        {
          babl_conversion_process (path->items[i],
                                   aux1_buffer,
                                   aux2_buffer,
                                   n);
          /* Swap the auxiliary buffers */
          swap_buffer = aux1_buffer;
          aux1_buffer = aux2_buffer;
          aux2_buffer = swap_buffer;
        }

      /* The last conversion goes from aux1_buffer to destination_buffer */
      babl_conversion_process (babl_list_get_last (path),
                               aux1_buffer,
                               destination_buffer,
                               n);

      /* Free auxiliary buffers */
      if (aux1_buffer)
        babl_free (aux1_buffer);
      if (aux2_buffer)
        babl_free (aux2_buffer);
  }

  return n;
}

#define NUM_TEST_PIXELS  (256 + 16 + 16)

static double *
test_create (void)
{
  static double test[sizeof (double) * NUM_TEST_PIXELS * 4];
  int           i, j;

  /* There is no need to generate the test
   * more times ... */

  srandom (20050728);

  /*  add 128 pixels in the valid range between 0.0 and 1.0  */
  for (i = 0; i < 256 * 4; i++)
    test [i] = (double) random () / RAND_MAX;

  /*  add 16 pixels between -1.0 and 0.0  */
  for (j = 0; j < 16 * 4; i++, j++)
    test [i] = 0.0 - (double) random () / RAND_MAX;

  /*  add 16 pixels between 1.0 and 2.0  */
  for (j = 0; j < 16 * 4; i++, j++)
    test [i] = 1.0 + (double) random () / RAND_MAX;

  return test;
}

// FishPath instrumentation

static Babl   *fmt_rgba_double = NULL;
static double *test = NULL;
static void   *source;
static void   *destination;
static void   *ref_destination;
static double *destination_rgba_double;
static double *ref_destination_rgba_double;
static Babl   *fish_rgba_to_source;
static Babl   *fish_reference;
static Babl   *fish_destination_to_rgba;
static double  reference_cost;
static int     init_instrumentation_done = 0;

static void
init_path_instrumentation (Babl *fmt_source,
                           Babl *fmt_destination)
{
  long   ticks_start = 0;
  long   ticks_end   = 0;

  if (!fmt_rgba_double)
    {
      fmt_rgba_double = babl_format_new (
        babl_model ("RGBA"),
        babl_type ("double"),
        babl_component ("R"),
        babl_component ("G"),
        babl_component ("B"),
        babl_component ("A"),
        NULL);
    }

  if (!test)
    test = test_create ();

  fish_rgba_to_source      = babl_fish_reference (fmt_rgba_double,
                                                  fmt_source);
  fish_reference           = babl_fish_reference (fmt_source,
                                                  fmt_destination);
  fish_destination_to_rgba = babl_fish_reference (fmt_destination,
                                                  fmt_rgba_double);

  source                      = babl_calloc (NUM_TEST_PIXELS,
                                             fmt_source->format.bytes_per_pixel);
  destination                 = babl_calloc (NUM_TEST_PIXELS,
                                             fmt_destination->format.bytes_per_pixel);
  ref_destination             = babl_calloc (NUM_TEST_PIXELS,
                                             fmt_destination->format.bytes_per_pixel);
  destination_rgba_double     = babl_calloc (NUM_TEST_PIXELS,
                                             fmt_rgba_double->format.bytes_per_pixel);
  ref_destination_rgba_double = babl_calloc (NUM_TEST_PIXELS,
                                             fmt_rgba_double->format.bytes_per_pixel);

  /* create sourcebuffer from testbuffer in the correct format */
  babl_process (fish_rgba_to_source,
                test, source, NUM_TEST_PIXELS);

  /* calculate the reference buffer of how it should be */
  ticks_start = babl_ticks ();
  babl_process (fish_reference,
                source, ref_destination, NUM_TEST_PIXELS);
  ticks_end = babl_ticks ();
  reference_cost = babl_process_cost (ticks_start, ticks_end);

  /* transform the reference destination buffer to RGBA */
  babl_process (fish_destination_to_rgba,
                ref_destination, ref_destination_rgba_double, NUM_TEST_PIXELS);
}

static void
destroy_path_instrumentation (void)
{
  if (init_instrumentation_done)
    {
      babl_free (source);
      babl_free (destination);
      babl_free (destination_rgba_double);
      babl_free (ref_destination);
      babl_free (ref_destination_rgba_double);

      /* nulify the flag for potential new search */
      init_instrumentation_done = 0;
  }
}

static void
get_path_instrumentation (BablList *path,
                          double   *path_cost,
                          double   *ref_cost,
                          double   *path_error)
{
  long   ticks_start = 0;
  long   ticks_end   = 0;

  if (!init_instrumentation_done)
    {
      /* this initialization can be done only once since the
       * source and destination formats do not change during
       * the search */
      Babl *fmt_source = (Babl *) BABL (babl_list_get_first (path))->conversion.source;
      Babl *fmt_destination = (Babl *) BABL (babl_list_get_last (path))->conversion.destination;
      init_path_instrumentation (fmt_source, fmt_destination);
      init_instrumentation_done = 1;
    }

  /* calculate this path's view of what the result should be */
  ticks_start = babl_ticks ();
  process_conversion_path (path, source, destination, NUM_TEST_PIXELS);
  ticks_end = babl_ticks ();
  *path_cost = babl_process_cost (ticks_start, ticks_end);

  /* transform the reference and the actual destination buffers to RGBA
   * for comparison with each other
   */
  babl_process (fish_destination_to_rgba,
                destination, destination_rgba_double, NUM_TEST_PIXELS);

  *path_error = babl_rel_avg_error (destination_rgba_double,
                                    ref_destination_rgba_double,
                                    NUM_TEST_PIXELS * 4);

  fish_rgba_to_source->fish.processings--;
  fish_reference->fish.processings--;
  fish_destination_to_rgba->fish.processings -= 2;

  fish_rgba_to_source->fish.pixels      -= NUM_TEST_PIXELS;
  fish_reference->fish.pixels           -= NUM_TEST_PIXELS;
  fish_destination_to_rgba->fish.pixels -= 2 * NUM_TEST_PIXELS;

  *ref_cost = reference_cost;
}
