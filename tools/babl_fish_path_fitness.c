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

//#define UTF8

#ifdef UTF8

static char *utf8_bar[] = { " %s%s", "·%s%s", "▁%s%s", "▂%s%s", "▃%s%s", "▄%s%s", "▅%s%s", "▆%s%s", "▇%s%s", "█%s%s" };
#define      DIRECT " "
#define      SELF   " "
#define      EMPTY  " %s%s"
#define      SL     ""
#define      NL     "\n"

#else


static char *utf8_bar[] = { " ",
                            "<span class='direct' title=\"%s\" style='%s'>▄</span>",
                            "<span class='path2'  title=\"%s\" style='%s'>▄</span>",
                            "<span class='path3'  title=\"%s\" style='%s'>▄</span>",
                            "<span class='path4'  title=\"%s\" style='%s'>▄</span>",
                            "<span class='path5'  title=\"%s\" style='%s'>▄</span>",
                            "<span class='path6'  title=\"%s\" style='%s'>▄</span>",
                            "<span class='path7'  title=\"%s\" style='%s'>▄</span>",
                            "<span class='path8'  title=\"%s\" style='%s'>▄</span>"};
#define      SL     "<div>"
#define      SELF   "<span class='self'>&nbsp;</span>"
#define      EMPTY  "<span class='empty' title=\"%s\" style='%s'>▄</span>"
#define      NL     "</div>\n"

#endif



/*
static char *utf8_bar[]=  {"!","▁","▃","▅","▇","█","!","!","!"};
static char *utf8_bar[]={"·", "█", "▇", "▆", "▅", "▄", "▃", "▂", "▁", };
static char *utf8_bar[]={" ","1","2","3","4","5","6","7","8"};
*/

#define NUM_TEST_PIXELS  (1024*1024)
static float test_pixels_float[NUM_TEST_PIXELS*4];
static char  test_pixels_in[NUM_TEST_PIXELS * 6 * 8];
static char  test_pixels_out[NUM_TEST_PIXELS * 6 * 8];


static double rand_double (void)
{
  return (double) random () / RAND_MAX;
}

static double rand_range_double (double min, double max)
{
  return rand_double () * (max - min) + min;
}

static void init_test_pixels (void)
{
  static int done = 0;
  int i = 0;
  int pix_no = 0;
  srandom (111);

  if (done)
    return;
  done = 1;


  for (i = 0; i < 256 && pix_no < NUM_TEST_PIXELS; i++)
  {
    test_pixels_float[pix_no*4+0] = rand_double();
    test_pixels_float[pix_no*4+1] = rand_double();
    test_pixels_float[pix_no*4+2] = rand_double();
    test_pixels_float[pix_no*4+3] = rand_double();
    pix_no ++;
  }
  for (i = 0; i < 256 && pix_no < NUM_TEST_PIXELS; i++)
  {
    test_pixels_float[pix_no*4+0] = rand_range_double(1.0, 2.0);
    test_pixels_float[pix_no*4+1] = rand_range_double(1.0, 2.0);
    test_pixels_float[pix_no*4+2] = rand_range_double(1.0, 2.0);
    test_pixels_float[pix_no*4+3] = rand_range_double(1.0, 2.0);
    pix_no ++;
  }
  for (i = 0; i < 256 && pix_no < NUM_TEST_PIXELS; i++)
  {
    test_pixels_float[pix_no*4+0] = rand_range_double(-1.0, 1.0);
    test_pixels_float[pix_no*4+1] = rand_range_double(-1.0, 1.0);
    test_pixels_float[pix_no*4+2] = rand_range_double(-1.0, 1.0);
    test_pixels_float[pix_no*4+3] = rand_range_double(-1.0, 1.0);
    pix_no ++;
  }
  for (i = 0; i < 16 && pix_no < NUM_TEST_PIXELS; i++)
  {
    test_pixels_float[pix_no*4+0] = rand_range_double(0.0, 1.0);
    test_pixels_float[pix_no*4+1] = rand_range_double(0.0, 1.0);
    test_pixels_float[pix_no*4+2] = rand_range_double(0.0, 1.0);
    test_pixels_float[pix_no*4+3] = 0;
    pix_no ++;
  }
  for (i = 0; i < 16 && pix_no < NUM_TEST_PIXELS; i++)
  {
    test_pixels_float[pix_no*4+0] = rand_range_double(0.0, 16.0);
    test_pixels_float[pix_no*4+1] = rand_range_double(0.0, 16.0);
    test_pixels_float[pix_no*4+2] = rand_range_double(0.0, 16.0);
    test_pixels_float[pix_no*4+2] = rand_range_double(-1.0, 1.0);
    pix_no ++;
  }
  for (i = 0;  pix_no < NUM_TEST_PIXELS; i++)
  {
    test_pixels_float[pix_no*4+0] = rand_range_double(-1.0, 61.0);
    test_pixels_float[pix_no*4+1] = rand_range_double(-1.0, 61.0);
    test_pixels_float[pix_no*4+2] = rand_range_double(-1.0, 66.0);
    test_pixels_float[pix_no*4+3] = rand_range_double(-1.0, 3.0);
    pix_no ++;
  }
  for (i = 0; i < sizeof(test_pixels_in); i++)
     test_pixels_in[i]=(rand()/10000)&0xff;
}

static int destination_each (Babl *babl,
                             void *userdata)
{
  Babl *source      = userdata;
  Babl *destination = babl;
  init_test_pixels ();

  if (qux % babl_formats_count () == qux / babl_formats_count ())
    printf (SELF);
  else
    {
      Babl *temp = babl_fish_path (source, destination);
      const Babl *fish = babl_fish (source, destination);

      char style[128] = "";
      char title[1024] = "";
#ifdef UTF8
#else
     {
        float pixels_per_second;
        long ticks_start, ticks_end;

        ticks_start = babl_ticks ();
        babl_process (fish, &test_pixels_in[0],
                            &test_pixels_out[0],
                            NUM_TEST_PIXELS);
        ticks_end = babl_ticks ();

        pixels_per_second = (NUM_TEST_PIXELS) / ((ticks_end - ticks_start)/1000.0);

        {
          float colval = pixels_per_second/1000/1000.0/0.7;
          if (colval > 1)
            colval = 1;

          if (colval < 0.2)
          {
            sprintf (style, "color:rgb(%i, 0, 0);", (int)(colval * 5 * 255));
          }
          else if (colval < 0.4)
          {
            sprintf (style, "color:rgb(255, %i, 0);", (int)((colval-0.2) * 5 * 255));
          }
          else
          {
            sprintf (style, "color:rgb(255, 255, %i);", (int)((colval-0.4) * 1.666 * 255));
          }

        }
        {
          int steps = 0;
          if (temp)
            steps = babl_list_size (temp->fish_path.conversion_list);
          sprintf (title, "%s to %s %i steps %.3f mpix/s ", babl_get_name (source), babl_get_name (destination), steps, pixels_per_second/1000.0);
        }
     }
#endif

      if (temp)
        {
          printf (utf8_bar[babl_list_size (temp->fish_path.conversion_list)], title, style);
          total_length += babl_list_size (temp->fish_path.conversion_list);
          total_cost   += temp->fish_path.cost;
          ok++;
          total++;
        }
      else
        {
          printf (EMPTY, title, style);
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
"body { background: black; color:white; padding: 2em; line-height: 1em; font-size: 9px;}"
".empty  { background: black; border-left: 1px solid transparent;}"
".self   { background: yellow;border-left: 1px solid transparent; }"
".direct { background: blue; border-left: 1px solid blue;}"
/*
".path1  { background: green; }"
".path2  { background: #8ff; }"
".path3  { background: #8f8; }"
".path4  { background: #2f2; }"
".path5  { background: #0a0; }"
".path6  { background: #082; }"
".path7  { background: #050; }"
".path8  { background: #020; }"
*/

".path1, .path2, .path3, .path4, .path5, .path6, .path7 "
"{ border-left: 1px solid blue; }"


".empty, .self, .direct, .path1, .path2, .path3, .path4, .path5, .path6, .path7 "
"{ cursor: crosshair; }"

"</style>\n");
  printf ("</head><body><p>babl profiling matrix, rows are source formats columns are destinations, blue background means direct conversion and blue left border means multi-step conversion, no blue means reference conversion, gradient from black, through red, yellow and white indicates memory-bandwidth sum bytes read + bytes written / time, hover items to see further details on an individual conversion.</p><div style='position:absolute; width:220em;'>\n");

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
