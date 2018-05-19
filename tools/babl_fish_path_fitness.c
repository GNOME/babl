/* perform a symmetricality of conversion test on a set of randomized
 * RGBA data */

#include "config.h"
#include <stdlib.h>
#include <math.h>
#include "babl-internal.h"

#ifndef HAVE_SRANDOM
#define srandom srand
#define random  rand
#endif

int           total_length = 0;
int           total_cost   = 0;
int           total        = 0;
int           ok           = 0;

static int   qux = 0;

#define UTF8

#ifdef UTF8

static char *utf8_bar[] = { " ", "·", "▁", "▂", "▃", "▄", "▅", "▆", "▇", "█" };
#define      DIRECT " "
#define      SELF   " "
#define      EMPTY  " "
#define      SL     ""
#define      NL     "\n"

#else


static char *utf8_bar[] = { " ",
                            "<span class='direct'> </span>",
                            "<span class='path2'> </span>",
                            "<span class='path3'> </span>",
                            "<span class='path4'> </span>",
                            "<span class='path5'> </span>",
                            "<span class='path6'> </span>",
                            "<span class='path7'> </span>",
                            "<span class='path8'> </span>"};
#define      SL     "<div>"
#define      SELF   "<span class='self'> </span>"
#define      EMPTY  "<span class='empty'> </span>"
#define      NL     "</div>\n"

#endif



/*
static char *utf8_bar[]=  {"!","▁","▃","▅","▇","█","!","!","!"};
static char *utf8_bar[]={"·", "█", "▇", "▆", "▅", "▄", "▃", "▂", "▁", };
static char *utf8_bar[]={" ","1","2","3","4","5","6","7","8"};
*/

static int destination_each (Babl *babl,
                             void *userdata)
{
  Babl *source      = userdata;
  Babl *destination = babl;

  if (qux % babl_formats_count () == qux / babl_formats_count ())
    printf (SELF);
  else
    {
      Babl *temp = babl_fish_path (source, destination);

      if (temp)
        {
          printf ("%s", utf8_bar[babl_list_size (temp->fish_path.conversion_list)]);
          total_length += babl_list_size (temp->fish_path.conversion_list);
          total_cost   += temp->fish_path.cost;
          ok++;
          total++;
        }
      else
        {
          printf (EMPTY);
          total++;
        }
    }
  qux++;
  return 0;
}

static int source_no = 0;

static int source_each (Babl *babl,
                        void *userdata)
{
  printf ("%s", SL);
  babl_format_class_for_each (destination_each, babl);
#ifdef UTF8
  printf ("──%2i %s%s", source_no++, babl->instance.name, NL);
#else
  printf ("<span>%2i:%s</span>%s", source_no++, babl->instance.name, NL);
#endif
  return 0;
}

int main (void)
{
  babl_init ();

  babl_set_extender (babl_extension_quiet_log ());

#ifndef UTF8
  printf ("<html><head>\n");
   printf ("<meta http-equiv='Content-Type' content='text/html; charset=UTF-8'/>\n");

  printf ("<style>\n"

".empty, .direct, .self, .path1, .path2, .path3, .path4, .path5 { float:left; width: 1em; height: 1em; }"
"body { background: black; padding: 2em; line-height: 1em; }"
".empty  { background: black; }"
".self   { background: yellow; }"
".direct { background: blue; }"
".path1  { background: green; }"
".path2  { background: #8ff; }"
".path3  { background: #8f8; }"
".path4  { background: #2f2; }"
".path5  { background: #0a0; }"
".path6  { background: #082; }"
".path7  { background: #050; }"
".path8  { background: #020; }"


"</style>\n");
  printf ("</head><body>\n");

#endif

  babl_format_class_for_each (source_each, NULL);

#ifdef UTF8
  {
    int i;

    for (i = 0; i < babl_formats_count (); i++)
      printf ("|");

    printf ("\n");

    for (i = 0; i < babl_formats_count (); i++)
      {
        if (i / 100 == 0)
          printf ("|");
        else
          printf ("%i", (i / 100) % 10);
      }

    printf ("\n");

    for (i = 0; i < babl_formats_count (); i++)
      {
        if (i / 10 == 0)
          printf ("|");
        else
          printf ("%i", (i / 10) % 10);
      }

    printf ("\n");
    /*
    for (i = 0; i < babl_formats_count (); i++)
      printf ("│");

    printf ("\n");

    for (i = 0; i < babl_formats_count (); i++)
      {
        if (i / 10 == 0)
          printf ("│");
        else
          printf ("%i", (i / 10) % 10);
      }

    printf ("\n");
    */

    for (i = 0; i < babl_formats_count (); i++)
      printf ("%i", (i) % 10);

    printf ("\n");
  }

  printf ("total length: %i\n", total_length);
  printf ("total cost  : %i\n", total_cost);
  /*printf ("ok / total : %i %i %f\n", ok, total, (1.0*ok) / total);
   */
#else
  printf ("</body></html>\n");
#endif

  babl_exit ();

  return 0;
}
