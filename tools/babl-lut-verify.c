#include <stdlib.h>
#include <stdint.h>
#include "config.h"
#include <math.h>
#include "babl-internal.h"

#define PIXELS 127*256 //less than threshold for generating

#ifndef HAVE_SRANDOM
#define random rand
#endif

#ifdef _WIN32
/* On Windows setenv() does not exist, using _putenv_s() instead. The overwrite
 * arg is ignored (i.e. same as always 1).
 */
#define setenv(name,value,overwrite) _putenv_s(name, value)
#endif

static double
test_generic (const Babl *source, const Babl *dest)
{
  uint8_t *src = malloc (PIXELS*16);
  uint8_t *dst = malloc (PIXELS*16);
  uint8_t *dst2 = malloc (PIXELS*16);
  uint8_t *dstb = malloc (PIXELS*16);
  uint8_t *dst2b = malloc (PIXELS*16);
  double error = 0.0;

  for (int i = 0; i < PIXELS * 16; i++)
      src[i] = random();

  babl_process ( babl_fish (source, dest), src, dst, PIXELS);
  babl_process ( babl_fish (source, dest), src, dst2, PIXELS);
  babl_process ( babl_fish (source, dest), src, dst2, PIXELS);
  babl_process ( babl_fish (source, dest), src, dst2, PIXELS);
  babl_process ( babl_fish (dest, babl_format_with_space ("R'G'B'A u8", dest)), dst2, dst2b, PIXELS);
  babl_process ( babl_fish (dest, babl_format_with_space ("R'G'B'A u8", dest)), dst, dstb, PIXELS);

  for (int i = 0; i < PIXELS; i++)
  {
    error += sqrt ((dst[i*4+0] - dst2b[i*4+0])*
                   (dstb[i*4+0] - dst2b[i*4+0])+
                   (dstb[i*4+1] - dst2b[i*4+1])*
                   (dstb[i*4+1] - dst2b[i*4+1])+
                   (dstb[i*4+2] - dst2b[i*4+2])*
                   (dstb[i*4+2] - dst2b[i*4+2]));
  }

  free (src);
  free (dst);
  free (dst2);
  free (dstb);
  free (dst2b);

  return error;
}

int main (void)
{
  double error = 0;
  setenv ("BABL_INHIBIT_CACHE", "1", 1);
  babl_init ();
  {
          
  const Babl *format_sets[][2]={
          {
           babl_format_with_space ("R'G'B' u8", babl_space("ProPhoto")),
           babl_format_with_space ("R'G'B' u8", babl_space("Rec2020"))
          },
          {
           babl_format_with_space ("R'G'B' u8", babl_space("ProPhoto")),
           babl_format_with_space ("R'G'B'A u8", babl_space("Rec2020"))
          },
          {
           babl_format_with_space ("R'G'B' u8", babl_space("ProPhoto")),
           babl_format_with_space ("R'G'B'A half", babl_space("Rec2020"))
          },
          {
           babl_format_with_space ("R'G'B' u8", babl_space("ProPhoto")),
           babl_format_with_space ("R'G'B'A u16", babl_space("Rec2020"))
          },
          {
           babl_format_with_space ("R'G'B' u8", babl_space("ProPhoto")),
           babl_format_with_space ("R'G'B'A float", babl_space("Rec2020"))
          },
          {
           babl_format_with_space ("R'G'B' u8", babl_space("ProPhoto")),
           babl_format_with_space ("Y'A float", babl_space("Rec2020"))
          },
          {
           babl_format_with_space ("R'G'B' u8", babl_space("ProPhoto")),
           babl_format_with_space ("Y'A u16", babl_space("Rec2020"))
          },
          {
           babl_format_with_space ("R'G'B' u8", babl_space("ProPhoto")),
           babl_format_with_space ("Y'A u8", babl_space("Rec2020"))
          },
          {
           babl_format_with_space ("R'G'B' u8", babl_space("ProPhoto")),
           babl_format_with_space ("Y' float", babl_space("Rec2020"))
          },
          {
           babl_format_with_space ("R'G'B' u8", babl_space("ProPhoto")),
           babl_format_with_space ("Y' u16", babl_space("Rec2020"))
          },
          {
           babl_format_with_space ("R'G'B' u8", babl_space("ProPhoto")),
           babl_format_with_space ("Y' u8", babl_space("Rec2020"))
          },

          {
           babl_format_with_space ("R'G'B'A u8", babl_space("ProPhoto")),
           babl_format_with_space ("R'G'B'A u8", babl_space("Rec2020"))
          },
          {
           babl_format_with_space ("R'G'B'A u8", babl_space("ProPhoto")),
           babl_format_with_space ("R'G'B'A half", babl_space("Rec2020"))
          },
          {
           babl_format_with_space ("R'G'B'A u8", babl_space("ProPhoto")),
           babl_format_with_space ("R'G'B'A u16", babl_space("Rec2020"))
          },
          {
           babl_format_with_space ("R'G'B'A u8", babl_space("ProPhoto")),
           babl_format_with_space ("R'G'B'A float", babl_space("Rec2020"))
          },
          {
           babl_format_with_space ("R'G'B'A u8", babl_space("ProPhoto")),
           babl_format_with_space ("Y'A float", babl_space("Rec2020"))
          },
          {
           babl_format_with_space ("R'G'B'A u8", babl_space("ProPhoto")),
           babl_format_with_space ("Y'A u16", babl_space("Rec2020"))
          },
          {
           babl_format_with_space ("R'G'B'A u8", babl_space("ProPhoto")),
           babl_format_with_space ("Y'A half", babl_space("Rec2020"))
          },
          {
           babl_format_with_space ("R'G'B'A u8", babl_space("ProPhoto")),
           babl_format_with_space ("Y'A u8", babl_space("Rec2020"))
          },
          {
           babl_format_with_space ("R'G'B'A u8", babl_space("ProPhoto")),
           babl_format_with_space ("Y' float", babl_space("Rec2020"))
          },
          {
           babl_format_with_space ("R'G'B'A u8", babl_space("ProPhoto")),
           babl_format_with_space ("Y' u16", babl_space("Rec2020"))
          },
          {
           babl_format_with_space ("R'G'B'A u8", babl_space("ProPhoto")),
           babl_format_with_space ("Y' u8", babl_space("Rec2020"))
          },

          {
           babl_format_with_space ("YaA half", babl_space("ProPhoto")), 
           babl_format_with_space ("R'G'B'A u8", babl_space("Rec2020"))
          },
          {
           babl_format_with_space ("YaA half", babl_space("ProPhoto")), 
           babl_format_with_space ("R'G'B'A half", babl_space("Rec2020"))
          },
          {
           babl_format_with_space ("YaA half", babl_space("ProPhoto")), 
           babl_format_with_space ("R'G'B'A u16", babl_space("Rec2020"))
          },
          {
           babl_format_with_space ("YaA half", babl_space("ProPhoto")), 
           babl_format_with_space ("R'G'B'A float", babl_space("Rec2020"))
          },
          {
           babl_format_with_space ("YaA half", babl_space("ProPhoto")), 
           babl_format_with_space ("Y'A float", babl_space("Rec2020"))
          },
          {
           babl_format_with_space ("YaA half", babl_space("ProPhoto")), 
           babl_format_with_space ("Y'A u16", babl_space("Rec2020"))
          },
          {
           babl_format_with_space ("YaA half", babl_space("ProPhoto")), 
           babl_format_with_space ("Y'A u8", babl_space("Rec2020"))
          },

          {
           babl_format_with_space ("YaA half", babl_space("ProPhoto")), 
           babl_format_with_space ("R'G'B'A u8", babl_space("Rec2020"))
          },
          {
           babl_format_with_space ("YaA half", babl_space("ProPhoto")), 
           babl_format_with_space ("R'G'B'A half", babl_space("Rec2020"))
          },
          {
           babl_format_with_space ("YA half", babl_space("ProPhoto")), 
           babl_format_with_space ("R'G'B'A u16", babl_space("Rec2020"))
          },
          {
           babl_format_with_space ("YA half", babl_space("ProPhoto")), 
           babl_format_with_space ("R'G'B'A float", babl_space("Rec2020"))
          },
          {
           babl_format_with_space ("YA half", babl_space("ProPhoto")), 
           babl_format_with_space ("Y'A float", babl_space("Rec2020"))
          },
          {
           babl_format_with_space ("YA half", babl_space("ProPhoto")), 
           babl_format_with_space ("Y'A u16", babl_space("Rec2020"))
          },
          {
           babl_format_with_space ("YA half", babl_space("ProPhoto")), 
           babl_format_with_space ("Y'A u8", babl_space("Rec2020"))
          },
          {
           babl_format_with_space ("YA half", babl_space("ProPhoto")), 
           babl_format_with_space ("Y' float", babl_space("Rec2020"))
          },
          {
           babl_format_with_space ("YA half", babl_space("ProPhoto")), 
           babl_format_with_space ("Y' u16", babl_space("Rec2020"))
          },
          {
           babl_format_with_space ("YA half", babl_space("ProPhoto")), 
           babl_format_with_space ("Y' u8", babl_space("Rec2020"))
          },


          {
           babl_format_with_space ("Y half", babl_space("ProPhoto")), 
           babl_format_with_space ("R'G'B'A u8", babl_space("Rec2020"))
          },
          {
           babl_format_with_space ("Y half", babl_space("ProPhoto")), 
           babl_format_with_space ("R'G'B'A half", babl_space("Rec2020"))
          },
          {
           babl_format_with_space ("Y half", babl_space("ProPhoto")), 
           babl_format_with_space ("R'G'B'A u16", babl_space("Rec2020"))
          },
          {
           babl_format_with_space ("Y half", babl_space("ProPhoto")), 
           babl_format_with_space ("R'G'B'A float", babl_space("Rec2020"))
          },
          {
           babl_format_with_space ("Y half", babl_space("ProPhoto")), 
           babl_format_with_space ("Y'A float", babl_space("Rec2020"))
          },
          {
           babl_format_with_space ("Y half", babl_space("ProPhoto")), 
           babl_format_with_space ("Y'A u16", babl_space("Rec2020"))
          },
          {
           babl_format_with_space ("Y half", babl_space("ProPhoto")), 
           babl_format_with_space ("Y'A u8", babl_space("Rec2020"))
          },
          {
           babl_format_with_space ("Y half", babl_space("ProPhoto")), 
           babl_format_with_space ("Y' float", babl_space("Rec2020"))
          },
          {
           babl_format_with_space ("Y half", babl_space("ProPhoto")), 
           babl_format_with_space ("Y' u16", babl_space("Rec2020"))
          },
          {
           babl_format_with_space ("Y half", babl_space("ProPhoto")), 
           babl_format_with_space ("Y' u8", babl_space("Rec2020"))
          }

  };


  for (size_t i = 0; i < sizeof (format_sets) / sizeof(format_sets[0]); i++)
  {
    fprintf (stdout, "%s to %s: ", babl_get_name (format_sets[i][0]),
                                  babl_get_name (format_sets[i][1])),
    error = test_generic (format_sets[i][0], format_sets[i][1]);
    if (error != 0.0)
      fprintf (stdout, "%.20f\n", error/(PIXELS*4));
    else
      fprintf (stdout, "OK\n");
  }
  }

  babl_exit ();
  return 0;
}
