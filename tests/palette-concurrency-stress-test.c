/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2009 Martin Nordholts
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

#include <stdlib.h>
#include <pthread.h>

#include "babl.h"


#define N_THREADS 10
#define N_PIXELS  1000000 /* (per thread) */


/* should be the same as HASH_TABLE_SIZE in babl/babl-palette.c */
#define BABL_PALETTE_HASH_TABLE_SIZE 1111


typedef struct
{
  const Babl    *fish;
  unsigned char  src[4 * N_PIXELS];
  unsigned char  dest[N_PIXELS];
} ThreadContext;


static void *
thread_proc (void *data)
{
  ThreadContext *ctx = data;

  babl_process (ctx->fish, ctx->src, ctx->dest, N_PIXELS);

  return NULL;
}

int
main (int    argc,
      char **argv)
{
  const Babl    *pal;
  const Babl    *pal_format;
  unsigned char  colors[4 * N_THREADS];
  pthread_t      threads[N_THREADS];
  ThreadContext *ctx[N_THREADS];
  int            i, j;
  int            OK = 1;

  babl_init ();

  /* create a palette of N_THREADS different colors, all of which have the same
   * hash
   */
  pal = babl_new_palette (NULL, &pal_format, NULL);

  for (i = 0; i < N_THREADS; i++)
    {
      unsigned char *p = &colors[4 * i];
      unsigned int   v;

      v = i * BABL_PALETTE_HASH_TABLE_SIZE;

      p[0] = (v >>  0) & 0xff;
      p[1] = (v >>  8) & 0xff;
      p[2] = (v >> 16) & 0xff;
      p[3] = 0xff;
    }

  babl_palette_set_palette (pal, babl_format ("R'G'B'A u8"), colors, N_THREADS);

  /* initialize the thread contexts such that each thread processes a buffer
   * containing a single, distinct color
   */
  for (i = 0; i < N_THREADS; i++)
    {
      ctx[i] = malloc (sizeof (ThreadContext));

      ctx[i]->fish = babl_fish (babl_format ("R'G'B'A u8"), pal_format);

      for (j = 0; j < 4 * N_PIXELS; j++)
        {
          ctx[i]->src[j] = colors[4 * i + j % 4];
        }
    }

  /* run all threads at the same time */
  for (i = 0; i < N_THREADS; i++)
    {
      pthread_create (&threads[i],
                      NULL, /* attr */
                      thread_proc,
                      ctx[i]);
    }

  /* wait for them to finish */
  for (i = 0; i < N_THREADS; i++)
    {
      pthread_join (threads[i],
                    NULL /* thread_return */);
    }

  /* verify the results */
  for (i = 0; i < N_THREADS; i++)
    {
      for (j = 0; OK && j < N_PIXELS; j++)
        {
          OK = (ctx[i]->dest[j] == i);
        }

      free (ctx[i]);
    }

  babl_exit ();

  return ! OK;
}
