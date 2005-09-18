/* babl - dynamically extendable universal pixel fish library.
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

#include "babl-internal.h"

#define BABL_LEGAL_ERROR 0.000001

typedef struct BablChainContext {
  Babl  *from;
  Babl  *to;

  double *best_cost;
  double *best_loss;

  Babl **chain;
  int   *conversions;

  Babl **temp_chain;
  int    temp_conversions;

  int    max_conversions;
} BablChainContext;

static int
format_has_alpha (Babl *babl)
{
  int i;
  for (i=0; i<babl->format.components; i++)
    if (babl->format.component[i]->instance.id == BABL_ALPHA)
      return 1;
  return 0;
}

static int
format_analytic_loss (Babl *source,
                      Babl *destination)
{
  int loss = 0;

  if (source->format.components <
      destination->format.components)
    {
      loss |= 8;
    }

  if ( format_has_alpha (source) &&
      !format_has_alpha (destination))
    {
      loss |= 4;
    }

  if ( BABL(source->format.type[0])->type.bits >
       BABL(destination->format.type[0])->type.bits)
    {
      loss |= 2;
    }

  if ( source->format.bytes_per_pixel >
       destination->format.bytes_per_pixel)
    {
      loss |= 1;
    }




  return loss;
}


static int
chain_gen_each (Babl *babl,
                void *userdata);

static int
get_conversion_chain (Babl   *from,
                      Babl   *to,

                      double *best_cost,
                      double *best_loss,
                      Babl  **chain,
                      int    *conversions,

                      Babl  **temp_chain,
                      int     temp_conversions,

                      int     max_conversions)
{
  BablChainContext context;

  if (temp_conversions>=max_conversions)
    return 0;

  if (temp_conversions == 0)
    {
      /* chain initialization */
      *conversions = 0;
      *best_cost   = 200000.0;
      *best_loss   = 200000.0;
      chain[0] = NULL;
      temp_chain[0] = NULL;

      /* Bail out if requesting something stupid (to and from same format, an
       * optimized memcpy should be used instead (assuming linear buffers).
       */

      if (from == to)
         return 0;
    }

  /* copy parameters to stack */
  context.from             = from;
  context.to               = to;

  context.best_cost        = best_cost;
  context.best_loss        = best_loss;
  context.chain            = chain;
  context.conversions      = conversions;

  context.temp_chain       = temp_chain;
  context.temp_conversions = temp_conversions;

  context.max_conversions  = max_conversions;

  if (temp_conversions == 0)
    {
      temp_chain[temp_conversions]=NULL;
      babl_assert (from);
      babl_assert (from->class_type == BABL_FORMAT);
      if (!from->format.from)
        return 0;

      babl_list_each ((void**) from->format.from,
                       chain_gen_each,
                       &context);
    }
  else
    {
      if (BABL(temp_chain[temp_conversions-1]) &&
          BABL(temp_chain[temp_conversions-1]->conversion.destination)->
          format.from)

         babl_list_each (
           (void **) 
           BABL(temp_chain[temp_conversions-1]->conversion.destination)->
           format.from,
           chain_gen_each,
           &context);
    }

  return 0;
}

static int
chain_gen_each (Babl *babl,
                void *userdata)
{
  BablChainContext *c = userdata;

  /* fill in the conversion for the chain index we are at */
  c->temp_chain[c->temp_conversions] = babl;

    {
      if (BABL(babl->conversion.destination) == c->to)
        {
          /* a candidate path has been found */
        
          double    temp_cost = 0.0;
          double    temp_loss = 0.0;
          double    error     = 1.0;
          int       analytic_loss = 0;
          int       i;

          for (i=0; i < c->temp_conversions+1; i++)
            {
              error     *= (1.0+c->temp_chain[i]->conversion.error);
              temp_cost += c->temp_chain[i]->conversion.cost;
              analytic_loss |= format_analytic_loss (
                BABL(c->temp_chain[i]->conversion.source),
                BABL(c->temp_chain[i]->conversion.destination));
            }
          temp_loss = analytic_loss;
          
          if (error <= (1.0 + BABL_LEGAL_ERROR)  /* we're legal */ &&  
             
              /* better than the existing best candidate */ 
              ( temp_loss <  *c->best_loss ||
               (temp_loss == *c->best_loss &&
                temp_cost <  *c->best_cost)))
            {
              int i;

              *c->best_cost   = temp_cost;
              *c->best_loss   = temp_loss;
              *c->conversions = c->temp_conversions + 1;

              /* copy from temp chain to best chain */
              for (i = 0.0; i < *c->conversions; i++)
                 c->chain[i] = c->temp_chain[i];
            }
        }
      else
        {
            /* try to add another conversion level in chain,.. */
            get_conversion_chain (c->from,  /* irrelevant when recalled */
                                  c->to,

                                  c->best_cost,
                                  c->best_loss,
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
  int i=0;
  Babl **conversion;

  conversion = (void*)BABL(source)->type.from;
  while (conversion && conversion[i])
    {
      if (conversion[i]->conversion.destination == destination)
        return (Babl*)conversion[i];
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
babl_fish_path (Babl   *source,
                Babl   *destination)
{
  Babl *babl = NULL;
  char *name = create_name (source, destination, 1);
  Babl *temp_chain[BABL_MAX_PATH_LENGTH];

  babl_assert (BABL_IS_BABL (source));
  babl_assert (BABL_IS_BABL (destination));

  babl_assert (source->class_type == BABL_FORMAT);
  babl_assert (destination->class_type == BABL_FORMAT);

  babl                   = babl_calloc (1, sizeof (BablFishPath) +
                                        strlen (name) + 1);
  babl->class_type       = BABL_FISH_PATH;
  babl->instance.id      = 0;
  babl->instance.name    = ((void *)babl) + sizeof(BablFishPath);
  strcpy (babl->instance.name, name);
  babl->fish.source      = (union Babl*)source;
  babl->fish.destination = (union Babl*)destination;

  babl->fish.processings = 0;
  babl->fish.pixels      = 0;

  babl->fish_path.cost        = 200000;
  babl->fish_path.loss        = 200000;
  babl->fish_path.conversions = 0;
  babl->fish_path.conversion[0] = NULL;

  babl_assert (source->class_type == BABL_FORMAT);
  babl_assert (destination->class_type == BABL_FORMAT);
  
  get_conversion_chain (source,
                        destination,
                        &babl->fish_path.cost,
                        &babl->fish_path.loss,
                        (Babl**)(babl->fish_path.conversion),
                        &babl->fish_path.conversions,
                        temp_chain,
                        0, 
                        BABL_MAX_PATH_LENGTH);
  if (babl->fish_path.conversions==0)
    {
      babl_free (babl);
      return NULL;
    }
  
  { 
    Babl *ret = babl_db_insert (babl_fish_db (), babl);
    if (ret!=babl)
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
  
  for (i=0; i<conversions; i++)
    {
      if (i==0 && conversions == i+1)
        {
          babl_conversion_process ( BABL(chain[i]),
              source, destination, n);
        }
      else if (i == 0)
        {
          babl_conversion_process ( BABL(chain[i]),
              source, bufA, n);
        }
      else if (i % 2 == 1)
        {
          if (i + 1 == conversions)
            {
              babl_conversion_process ( BABL(chain[i]),
                  bufA, destination, n);
            }
          else
            {
              babl_conversion_process ( BABL(chain[i]),
                  bufA, bufB, n);
            }
        }
      else if (i % 2 == 0)
        {
          if (i + 1 == conversions)
            {
              babl_conversion_process ( BABL(chain[i]),
                  bufB, destination, n);
            }
          else
            {
              babl_conversion_process ( BABL(chain[i]),
                  bufB, bufA, n);
            }
        }
      i ++;
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
                        long n)
{
  /*
  int i;
  */

  babl_assert (source);
  babl_assert (destination);
/*  
  babl_log ("path processing from %s to %s",
            BABL(babl->fish.source)->instance.name,
            BABL(babl->fish.destination)->instance.name);

  for (i=0; i< babl->fish_path.conversions; i++)
    babl_log ("\t%s\n",
             BABL(babl->fish_path.conversion[i])->instance.name);
*/
  
  return chain_process (babl->fish_path.conversion,
                        babl->fish_path.conversions,
                        source,
                        destination,
                        n);
}

