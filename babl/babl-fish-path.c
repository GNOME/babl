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

static double
chain_error (Babl            *fmt_source,
             Babl            *fmt_destination,
             BablConversion **chain,
             int              conversions);

//#define BABL_LEGAL_ERROR 0.000001
//#define BABL_LEGAL_ERROR 0.01

static double legal_error (void)
{
  static double error = 0.0;
  const char   *env;

  if (error != 0.0)
    return error;

  env = getenv ("BABL_ERROR");
  if (env)
    error = atof (env);
  else
    error = 0.000001;
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

typedef struct BablChainContext
{
  Babl *from;
  Babl *to;

  double *best_cost;
  double *best_loss;
  double *best_error;

  BablConversion **chain;
  int             *conversions;

  BablConversion **temp_chain;
  int              temp_conversions;

  int max_conversions;
} BablChainContext;

static int
chain_gen_each (Babl *babl,
                void *userdata);

static int
get_conversion_chain (Babl            *from,
                      Babl            *to,
                      double          *best_cost,
                      double          *best_loss,
                      double          *best_error,
                      BablConversion **chain,
                      int             *conversions,
                      BablConversion **temp_chain,
                      int              temp_conversions,
                      int              max_conversions)
{
  BablChainContext context;

  if (temp_conversions >= max_conversions)
    return 0;

  if (temp_conversions == 0)
    {
      /* chain initialization */
      *conversions  = 0;
      *best_cost    = 200000.0;
      *best_loss    = 200000.0;
      *best_error   = 200000.0;
      chain[0]      = NULL;
      temp_chain[0] = NULL;

      /* Bail out if requesting something stupid (to and from same format, an
       * optimized memcpy should be used instead (assuming linear buffers).
       */

      if (from == to)
        return 0;
    }

  /* copy parameters to stack */
  context.from = from;
  context.to   = to;

  context.best_cost   = best_cost;
  context.best_loss   = best_loss;
  context.best_error  = best_error;
  context.chain       = chain;
  context.conversions = conversions;

  context.temp_chain       = temp_chain;
  context.temp_conversions = temp_conversions;

  context.max_conversions = max_conversions;

  if (temp_conversions == 0)
    {
      temp_chain[temp_conversions] = NULL;
      babl_assert (from);
      babl_assert (from->class_type == BABL_FORMAT);
      if (!from->format.from)
        return 0;

      babl_list_each ((void **) from->format.from,
                      chain_gen_each,
                      &context);
    }
  else
    {
      if (BABL (temp_chain[temp_conversions - 1]) &&
          BABL (temp_chain[temp_conversions - 1]->destination)->
          format.from)

        babl_list_each (
          (void **)
          BABL (temp_chain[temp_conversions - 1]->destination)->
          format.from,
          chain_gen_each,
          &context);
    }

  return 0;
}

static int
chain_contains_fmt (BablConversion **chain,
                    int              conversions,
                    Babl            *fmt)
{
  int i;

  for (i = 0; i < conversions; i++)
    if (BABL (chain[i]->destination) == fmt ||
        BABL (chain[i]->source) == fmt)
      {
        return 1;
      }
  return 0;
}

static int
chain_gen_each (Babl *babl,
                void *userdata)
{
  BablChainContext *c = userdata;

  /* fill in the conversion for the chain index we are at */
  c->temp_chain[c->temp_conversions] = (BablConversion *) babl;

  {
    if ((BABL (babl->conversion.destination) == c->to))
      {
        /* a candidate path has been found */

        double temp_cost  = 0.0;
        double temp_error = 1.0;
        int    i;

        for (i = 0; i < c->temp_conversions + 1; i++)
          {
            temp_error *= (1.0 + babl_conversion_error (c->temp_chain[i]));
            temp_cost  += babl_conversion_cost (c->temp_chain[i]);
          }

        if (temp_cost < *c->best_cost &&
            temp_error - 1.0 <= legal_error () &&     /* this check before the next; which does a more accurate
                                                         measurement of the error */
            (temp_error = chain_error (c->from, c->to, c->temp_chain, c->temp_conversions + 1)) <= legal_error ()
        )
          {
            int i;

            *c->best_cost   = temp_cost;
            *c->best_error  = temp_error;
            *c->conversions = c->temp_conversions + 1;

            /* copy from temp chain to best chain */
            for (i = 0; i < *c->conversions; i++)
              c->chain[i] = c->temp_chain[i];
          }
      }
    else if (babl->conversion.source != babl->conversion.destination &&
             !chain_contains_fmt (c->temp_chain,
                                  c->temp_conversions,
                                  BABL (babl->conversion.destination)))
      {
        /* try to add another conversion level in chain,.. */
        get_conversion_chain (c->from,      /* irrelevant when recalled */
                              c->to,

                              c->best_cost,
                              c->best_loss,
                              c->best_error,
                              c->chain,
                              c->conversions,

                              c->temp_chain,
                              c->temp_conversions + 1,

                              c->max_conversions);
      }
  }
  return 0;
}

static inline Babl *
assert_conversion_find (void *source,
                        void *destination)
{
  int    i = 0;
  Babl **conversion;

  conversion = (void *) BABL (source)->type.from;
  while (conversion && conversion[i])
    {
      if (conversion[i]->conversion.destination == destination)
        return (Babl *) conversion[i];
      i++;
    }
  babl_fatal ("failed, aborting");
  return NULL;
}

static char buf[1024];
static char *
create_name (Babl *source,
             Babl *destination,
             int   is_reference)
{
  /* fish names are intentionally kept short */
  snprintf (buf, 1024, "%s %p %p", "",
            source, destination);
  return buf;
}

Babl *
babl_fish_path (Babl *source,
                Babl *destination)
{
  Babl           *babl = NULL;
  char           *name = create_name (source, destination, 1);
  BablConversion *temp_chain[BABL_HARD_MAX_PATH_LENGTH];

  babl_assert (BABL_IS_BABL (source));
  babl_assert (BABL_IS_BABL (destination));

  babl_assert (source->class_type == BABL_FORMAT);
  babl_assert (destination->class_type == BABL_FORMAT);

  babl = babl_calloc (1, sizeof (BablFishPath) +
                      strlen (name) + 1);
  babl->class_type    = BABL_FISH_PATH;
  babl->instance.id   = 0;
  babl->instance.name = ((char *) babl) + sizeof (BablFishPath);
  strcpy (babl->instance.name, name);
  babl->fish.source      = source;
  babl->fish.destination = destination;

  babl->fish.processings = 0;
  babl->fish.pixels      = 0;
  babl->fish.usecs       = 0;
  babl->fish.error       = 200000;

  babl->fish_path.cost          = 200000;
  babl->fish_path.loss          = 200000;
  babl->fish_path.conversions   = 0;
  babl->fish_path.conversion[0] = NULL;

  babl_assert (source->class_type == BABL_FORMAT);
  babl_assert (destination->class_type == BABL_FORMAT);

  get_conversion_chain (source,
                        destination,
                        &babl->fish_path.cost,
                        &babl->fish_path.loss,
                        &babl->fish.error,
                        (BablConversion **) (babl->fish_path.conversion),
                        &babl->fish_path.conversions,
                        temp_chain,
                        0,
                        max_path_length ());

  if (babl->fish_path.conversions == 0)
    {
      babl_free (babl);
      return NULL;
    }

  {
    Babl *ret = babl_db_insert (babl_fish_db (), babl);
    if (ret != babl)
      babl_free (babl);
    return ret;
  }
}

static long
chain_process (BablConversion *chain[],
               int             conversions,
               void           *source,
               void           *destination,
               long            n)
{
  void *bufA = NULL;
  void *bufB = NULL;
  int   i;

  babl_assert (source);
  babl_assert (destination);

  if (conversions > 1)
    bufA = babl_malloc (n * sizeof (double) * 5);
  if (conversions > 2)
    bufB = babl_malloc (n * sizeof (double) * 5);

  for (i = 0; i < conversions; i++)
    {
      if (i == 0 && conversions == 1)
        {
          babl_conversion_process (BABL (chain[i]),
                                   source, destination, n);
        }
      else if (i == 0)
        {
          babl_conversion_process (BABL (chain[i]),
                                   source, bufA, n);
        }
      else if (i % 2 == 0)
        {
          if (i + 1 == conversions)
            {
              babl_conversion_process (BABL (chain[i]),
                                       bufB, destination, n);
            }
          else
            {
              babl_conversion_process (BABL (chain[i]),
                                       bufB, bufA, n);
            }
        }
      else if (i % 2 == 1)
        {
          if (i + 1 == conversions)
            {
              babl_conversion_process (BABL (chain[i]),
                                       bufA, destination, n);
            }
          else
            {
              babl_conversion_process (BABL (chain[i]),
                                       bufA, bufB, n);
            }
        }
    }
  if (bufA)
    babl_free (bufA);
  if (bufB)
    babl_free (bufB);

  return n;
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

  ret = chain_process (babl->fish_path.conversion,
                       babl->fish_path.conversions,
                       source,
                       destination,
                       n);

  return ret;
}


#define test_pixels    128

static double *
test_create (void)
{
  double *test;
  int     i;

  srandom (20050728);

  test = babl_malloc (sizeof (double) * test_pixels * 4);

  for (i = 0; i < test_pixels * 4; i++)
    test [i] = (double) random () / RAND_MAX;

  return test;
}

static double
chain_error (Babl            *fmt_source,
             Babl            *fmt_destination,
             BablConversion **chain,
             int              conversions)
{
  Babl *fmt_rgba_double = babl_format_new (
    babl_model ("RGBA"),
    babl_type ("double"),
    babl_component ("R"),
    babl_component ("G"),
    babl_component ("B"),
    babl_component ("A"),
    NULL);

  double  error = 0.0;

  double *test;
  void   *source;
  void   *destination;
  double *destination_rgba_double;
  void   *ref_destination;
  double *ref_destination_rgba_double;

  Babl   *fish_rgba_to_source      = babl_fish_reference (fmt_rgba_double, fmt_source);
  Babl   *fish_reference           = babl_fish_reference (fmt_source, fmt_destination);
  Babl   *fish_destination_to_rgba = babl_fish_reference (fmt_destination, fmt_rgba_double);

  test = test_create ();


  source                      = babl_calloc (test_pixels, fmt_source->format.bytes_per_pixel);
  destination                 = babl_calloc (test_pixels, fmt_destination->format.bytes_per_pixel);
  ref_destination             = babl_calloc (test_pixels, fmt_destination->format.bytes_per_pixel);
  destination_rgba_double     = babl_calloc (test_pixels, fmt_rgba_double->format.bytes_per_pixel);
  ref_destination_rgba_double = babl_calloc (test_pixels, fmt_rgba_double->format.bytes_per_pixel);

  /* create sourcebuffer from testbuffer in the correct format */
  babl_process (fish_rgba_to_source,
                test, source, test_pixels);

  /* calculate the reference buffer of how it should be */
  babl_process (fish_reference,
                source, ref_destination, test_pixels);

  /* calculate this chains view of what the result should be */
  chain_process (chain, conversions, source, destination, test_pixels);

  /* transform the reference and the actual destination buffers to RGBA
   * for comparison with each other
   */
  babl_process (fish_destination_to_rgba,
                ref_destination, ref_destination_rgba_double, test_pixels);
  babl_process (fish_destination_to_rgba,
                destination, destination_rgba_double, test_pixels);

  error = babl_rel_avg_error (destination_rgba_double,
                              ref_destination_rgba_double,
                              test_pixels * 4);

  fish_rgba_to_source->fish.processings--;
  fish_reference->fish.processings--;
  fish_destination_to_rgba->fish.processings -= 2;

  fish_rgba_to_source->fish.pixels      -= test_pixels;
  fish_reference->fish.pixels           -= test_pixels;
  fish_destination_to_rgba->fish.pixels -= 2 * test_pixels;

  babl_free (source);
  babl_free (destination);
  babl_free (destination_rgba_double);
  babl_free (ref_destination);
  babl_free (ref_destination_rgba_double);
  babl_free (test);

  return error;
}
