#include <stdlib.h>
#include <stdint.h>
#include "config.h"
#include <math.h>
#include "babl-internal.h"

#define PIXELS 127*256 //less than threshold for generating

#ifndef HAVE_SRANDOM
#define random rand
#endif

static double
test_u8_premul (void)
{
  uint8_t *src = malloc (PIXELS*4);
  uint8_t *dst = malloc (PIXELS*4);
  uint8_t *dst2 = malloc (PIXELS*4);
  double error = 0.0;

  for (int i = 0; i < PIXELS; i++)
    for (int c = 0; c < 4; c++)
      src[i*4+c] = random();

  babl_process (
      babl_fish (
          babl_format_with_space ("R'aG'aB'aA u8", babl_space("Apple")),
          babl_format_with_space ("R'aG'aB'aA u8", babl_space("ProPhoto"))),
      src, dst, PIXELS);
  babl_process (
      babl_fish (
          babl_format_with_space ("R'aG'aB'aA u8", babl_space("Apple")),
          babl_format_with_space ("R'aG'aB'aA u8", babl_space("ProPhoto"))),
      src, dst2, PIXELS);
  babl_process (
      babl_fish (
          babl_format_with_space ("R'aG'aB'aA u8", babl_space("Apple")),
          babl_format_with_space ("R'aG'aB'aA u8", babl_space("ProPhoto"))),
      src, dst2, PIXELS);
  babl_process (
      babl_fish (
          babl_format_with_space ("R'aG'aB'aA u8", babl_space("Apple")),
          babl_format_with_space ("R'aG'aB'aA u8", babl_space("ProPhoto"))),
      src, dst2, PIXELS);

  for (int i = 0; i < PIXELS; i++)
  {
    error += sqrt ((dst[i*4+0] - dst2[i*4+0])*
                   (dst[i*4+0] - dst2[i*4+0])+
                   (dst[i*4+1] - dst2[i*4+1])*
                   (dst[i*4+1] - dst2[i*4+1])+
                   (dst[i*4+2] - dst2[i*4+2])*
                   (dst[i*4+2] - dst2[i*4+2]));
  }

  free (src);
  free (dst);
  free (dst2);

  return error;
}


static double
test_rgb (void)
{
  uint8_t *src = malloc (PIXELS*4);
  uint8_t *dst = malloc (PIXELS*4);
  uint8_t *dst2 = malloc (PIXELS*4);
  double error = 0.0;

  for (int i = 0; i < PIXELS; i++)
    for (int c = 0; c < 4; c++)
      src[i*4+c] = random();

  babl_process (
      babl_fish (
          babl_format_with_space ("R'G'B' u8", babl_space("Apple")),
          babl_format_with_space ("R'G'B' u8", babl_space("ProPhoto"))),
      src, dst, PIXELS);
  babl_process (
      babl_fish (
          babl_format_with_space ("R'G'B' u8", babl_space("Apple")),
          babl_format_with_space ("R'G'B' u8", babl_space("ProPhoto"))),
      src, dst2, PIXELS);
  babl_process (
      babl_fish (
          babl_format_with_space ("R'G'B' u8", babl_space("Apple")),
          babl_format_with_space ("R'G'B' u8", babl_space("ProPhoto"))),
      src, dst2, PIXELS);
  babl_process (
      babl_fish (
          babl_format_with_space ("R'G'B' u8", babl_space("Apple")),
          babl_format_with_space ("R'G'B' u8", babl_space("ProPhoto"))),
      src, dst2, PIXELS);

  for (int i = 0; i < PIXELS; i++)
  {
    error += sqrt ((dst[i*3+0] - dst2[i*3+0])*
                   (dst[i*3+0] - dst2[i*3+0])+
                   (dst[i*3+1] - dst2[i*3+1])*
                   (dst[i*3+1] - dst2[i*3+1])+
                   (dst[i*3+2] - dst2[i*3+2])*
                   (dst[i*3+2] - dst2[i*3+2]));
  }

  free (src);
  free (dst);
  free (dst2);

  return error;
}


static double
test_u8 (void)
{
  uint8_t *src = malloc (PIXELS*4);
  uint8_t *dst = malloc (PIXELS*4);
  uint8_t *dst2 = malloc (PIXELS*4);
  double error = 0.0;

  for (int i = 0; i < PIXELS; i++)
    for (int c = 0; c < 4; c++)
      src[i*4+c] = random();

  babl_process (
      babl_fish (
          babl_format_with_space ("R'G'B'A u8", babl_space("Apple")),
          babl_format_with_space ("R'G'B'A u8", babl_space("ProPhoto"))),
      src, dst, PIXELS);
  babl_process (
      babl_fish (
          babl_format_with_space ("R'G'B'A u8", babl_space("Apple")),
          babl_format_with_space ("R'G'B'A u8", babl_space("ProPhoto"))),
      src, dst2, PIXELS);
  babl_process (
      babl_fish (
          babl_format_with_space ("R'G'B'A u8", babl_space("Apple")),
          babl_format_with_space ("R'G'B'A u8", babl_space("ProPhoto"))),
      src, dst2, PIXELS);
  babl_process (
      babl_fish (
          babl_format_with_space ("R'G'B'A u8", babl_space("Apple")),
          babl_format_with_space ("R'G'B'A u8", babl_space("ProPhoto"))),
      src, dst2, PIXELS);

  for (int i = 0; i < PIXELS; i++)
  {
    error += sqrt ((dst[i*4+0] - dst2[i*4+0])*
                   (dst[i*4+0] - dst2[i*4+0])+
                   (dst[i*4+1] - dst2[i*4+1])*
                   (dst[i*4+1] - dst2[i*4+1])+
                   (dst[i*4+2] - dst2[i*4+2])*
                   (dst[i*4+2] - dst2[i*4+2]));
  }

  free (src);
  free (dst);
  free (dst2);

  return error;
}

static double
test_ya_half (void)
{
  uint8_t *src = malloc (PIXELS*4*2);
  uint8_t *dst = malloc (PIXELS*4*2);
  uint8_t *dst2 = malloc (PIXELS*4*2);
  double error = 0.0;

  for (int i = 0; i < PIXELS; i++)
  {
    for (int c = 0; c < 8; c++)
      src[i*8+c] = random();
  }

  babl_process (
      babl_fish (
          babl_format_with_space ("YA half", babl_space("Apple")),
          babl_format_with_space ("R'G'B'A u8", babl_space("ProPhoto"))),
      src, dst, PIXELS);
  for (int i =0 ; i < 10; i++)
  babl_process (
      babl_fish (
          babl_format_with_space ("YA half", babl_space("Apple")),
          babl_format_with_space ("R'G'B'A u8", babl_space("ProPhoto"))),
      src, dst2, PIXELS);

  for (int i = 0; i < PIXELS; i++)
  {
    error += sqrt ((dst[i*4+0] - dst2[i*4+0])*
                   (dst[i*4+0] - dst2[i*4+0])+
                   (dst[i*4+1] - dst2[i*4+1])*
                   (dst[i*4+1] - dst2[i*4+1])+
                   (dst[i*4+2] - dst2[i*4+2])*
                   (dst[i*4+2] - dst2[i*4+2]));
  }

  free (src);
  free (dst);
  free (dst2);

  return error;
}

static double
test_Ya_half (void)
{
  uint8_t *src = malloc (PIXELS*4*2);
  uint8_t *dst = malloc (PIXELS*4*2);
  uint8_t *dst2 = malloc (PIXELS*4*2);
  double error = 0.0;

  for (int i = 0; i < PIXELS; i++)
  {
    for (int c = 0; c < 8; c++)
      src[i*8+c] = random();
  }

  babl_process (
      babl_fish (
          babl_format_with_space ("Y'A half", babl_space("Apple")),
          babl_format_with_space ("R'G'B'A u8", babl_space("ProPhoto"))),
      src, dst, PIXELS);
  for (int i =0 ; i < 10; i++)
  babl_process (
      babl_fish (
          babl_format_with_space ("Y'A half", babl_space("Apple")),
          babl_format_with_space ("R'G'B'A u8", babl_space("ProPhoto"))),
      src, dst2, PIXELS);

  for (int i = 0; i < PIXELS; i++)
  {
    error += sqrt ((dst[i*4+0] - dst2[i*4+0])*
                   (dst[i*4+0] - dst2[i*4+0])+
                   (dst[i*4+1] - dst2[i*4+1])*
                   (dst[i*4+1] - dst2[i*4+1])+
                   (dst[i*4+2] - dst2[i*4+2])*
                   (dst[i*4+2] - dst2[i*4+2]));
  }

  free (src);
  free (dst);
  free (dst2);

  return error;
}

static double
test_ya_u16 (void)
{
  uint8_t *src = malloc (PIXELS*4*2);
  uint8_t *dst = malloc (PIXELS*4*2);
  uint8_t *dst2 = malloc (PIXELS*4*2);
  double error = 0.0;

  for (int i = 0; i < PIXELS; i++)
  {
    for (int c = 0; c < 8; c++)
      src[i*8+c] = random();
  }

  babl_process (
      babl_fish (
          babl_format_with_space ("YA u16", babl_space("Apple")),
          babl_format_with_space ("R'G'B'A u8", babl_space("ProPhoto"))),
      src, dst, PIXELS);
  for (int i =0 ; i < 10; i++)
  babl_process (
      babl_fish (
          babl_format_with_space ("YA u16", babl_space("Apple")),
          babl_format_with_space ("R'G'B'A u8", babl_space("ProPhoto"))),
      src, dst2, PIXELS);

  for (int i = 0; i < PIXELS; i++)
  {
    error += sqrt ((dst[i*4+0] - dst2[i*4+0])*
                   (dst[i*4+0] - dst2[i*4+0])+
                   (dst[i*4+1] - dst2[i*4+1])*
                   (dst[i*4+1] - dst2[i*4+1])+
                   (dst[i*4+2] - dst2[i*4+2])*
                   (dst[i*4+2] - dst2[i*4+2]));
  }

  free (src);
  free (dst);
  free (dst2);

  return error;
}



static double
test_u16 (void)
{
  uint8_t *src = malloc (PIXELS*4*2);
  uint8_t *dst = malloc (PIXELS*4*2);
  uint8_t *dst2 = malloc (PIXELS*4*2);
  double error = 0.0;

  for (int i = 0; i < PIXELS; i++)
  {
    for (int c = 0; c < 6; c++)
      src[i*8+c] = random();
    src[i*8+6] = 255;
    src[i*8+7] = 255;
  }

  babl_process (
      babl_fish (
          babl_format_with_space ("R'G'B'A u16", babl_space("Apple")),
          babl_format_with_space ("R'G'B'A u8", babl_space("ProPhoto"))),
      src, dst, PIXELS);
  for (int i =0 ; i < 10; i++)
  babl_process (
      babl_fish (
          babl_format_with_space ("R'G'B'A u16", babl_space("Apple")),
          babl_format_with_space ("R'G'B'A u8", babl_space("ProPhoto"))),
      src, dst2, PIXELS);

  for (int i = 0; i < PIXELS; i++)
  {
    error += sqrt ((dst[i*4+0] - dst2[i*4+0])*
                   (dst[i*4+0] - dst2[i*4+0])+
                   (dst[i*4+1] - dst2[i*4+1])*
                   (dst[i*4+1] - dst2[i*4+1])+
                   (dst[i*4+2] - dst2[i*4+2])*
                   (dst[i*4+2] - dst2[i*4+2]));
  }

  free (src);
  free (dst);
  free (dst2);

  return error;
}


static double
test_u16_linear (void)
{
  uint8_t *src = malloc (PIXELS*4*2);
  uint8_t *dst = malloc (PIXELS*4*2);
  uint8_t *dst2 = malloc (PIXELS*4*2);
  double error = 0.0;

  for (int i = 0; i < PIXELS; i++)
  {
    for (int c = 0; c < 6; c++)
      src[i*8+c] = random();
    src[i*8+6] = 255;
    src[i*8+7] = 255;
  }

  babl_process (
      babl_fish (
          babl_format_with_space ("RGBA u16", babl_space("Apple")),
          babl_format_with_space ("R'G'B'A u8", babl_space("ProPhoto"))),
      src, dst, PIXELS);
  for (int i =0 ; i < 10; i++)
  babl_process (
      babl_fish (
          babl_format_with_space ("RGBA u16", babl_space("Apple")),
          babl_format_with_space ("R'G'B'A u8", babl_space("ProPhoto"))),
      src, dst2, PIXELS);

  for (int i = 0; i < PIXELS; i++)
  {
    error += sqrt ((dst[i*4+0] - dst2[i*4+0])*
                   (dst[i*4+0] - dst2[i*4+0])+
                   (dst[i*4+1] - dst2[i*4+1])*
                   (dst[i*4+1] - dst2[i*4+1])+
                   (dst[i*4+2] - dst2[i*4+2])*
                   (dst[i*4+2] - dst2[i*4+2]));
  }

  free (src);
  free (dst);
  free (dst2);

  return error;
}


static double
test_u16_half (void)
{
  uint8_t *src = malloc (PIXELS*4*2);
  uint8_t *dst = malloc (PIXELS*4*2);
  uint8_t *dst2 = malloc (PIXELS*4*2);
  double error = 0.0;

  for (int i = 0; i < PIXELS; i++)
  {
    for (int c = 0; c < 8; c++)
      src[i*8+c] = random();
  }

  babl_process (
      babl_fish (
          babl_format_with_space ("RGBA half", babl_space("Apple")),
          babl_format_with_space ("R'G'B'A u8", babl_space("ProPhoto"))),
      src, dst, PIXELS);
  for (int i =0 ; i < 10; i++)
  babl_process (
      babl_fish (
          babl_format_with_space ("RGBA half", babl_space("Apple")),
          babl_format_with_space ("R'G'B'A u8", babl_space("ProPhoto"))),
      src, dst2, PIXELS);

  for (int i = 0; i < PIXELS; i++)
  {
    error += sqrt ((dst[i*4+0] - dst2[i*4+0])*
                   (dst[i*4+0] - dst2[i*4+0])+
                   (dst[i*4+1] - dst2[i*4+1])*
                   (dst[i*4+1] - dst2[i*4+1])+
                   (dst[i*4+2] - dst2[i*4+2])*
                   (dst[i*4+2] - dst2[i*4+2]));
  }

  free (src);
  free (dst);
  free (dst2);

  return error;
}



int main (int argc, char **argv)
{
  double error = 0;
  babl_init ();

  fprintf (stdout, "u8 ");
  error = test_u8 ();
  if (error != 0.0)
    fprintf (stdout, "%.20f\n", error/(PIXELS*4));
  else
    fprintf (stdout, "OK\n");

  fprintf (stdout, "R'G'B u8 ");
  error = test_rgb ();
  if (error != 0.0)
    fprintf (stdout, "%.20f\n", error/(PIXELS*4));
  else
    fprintf (stdout, "OK\n");


  fprintf (stdout, "u8 premul ");
  error = test_u8_premul ();
  if (error != 0.0)
    fprintf (stdout, "%.20f\n", error/(PIXELS*4));
  else
    fprintf (stdout, "OK\n");

  fprintf (stdout, "u16 ");
  error = test_u16 ();
  if (error != 0.0)
    fprintf (stdout, "%.20f\n", error/(PIXELS*4));
  else
    fprintf (stdout, "OK\n");

  fprintf (stdout, "u16 linear ");
  error = test_u16_linear ();
  if (error != 0.0)
    fprintf (stdout, "%.20f\n", error/(PIXELS*4));
  else
    fprintf (stdout, "OK\n");


  fprintf (stdout, "u16 half ");
  error = test_u16_half ();
  if (error != 0.0)
    fprintf (stdout, "%.20f\n", error/(PIXELS*4));
  else
    fprintf (stdout, "OK\n");

  fprintf (stdout, "YA half ");
  error = test_ya_half ();
  if (error != 0.0)
    fprintf (stdout, "%.20f\n", error/(PIXELS*4));
  else
    fprintf (stdout, "OK\n");

  fprintf (stdout, "Y'A half ");
  error = test_Ya_half ();
  if (error != 0.0)
    fprintf (stdout, "%.20f\n", error/(PIXELS*4));
  else
    fprintf (stdout, "OK\n");

  fprintf (stdout, "YA u16 ");
  error = test_ya_u16 ();
  if (error != 0.0)
    fprintf (stdout, "%.20f\n", error/(PIXELS*4));
  else
    fprintf (stdout, "OK\n");

  babl_exit ();
  return 0;
}
