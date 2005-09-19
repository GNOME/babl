/* perform a symmetricality of conversion test on a set of randomized
 * RGBA data */

#include <stdlib.h>
#include <math.h>
#include "babl-internal.h"

#define pixels  1024
int total_length=0;
int total_cost=0;
int total = 0;
int ok = 0;

static double test[pixels * 4];

static void
test_init (void)
{
  int    i;

  for (i = 0; i < pixels * 4; i++)
     test [i] = (double)random () / RAND_MAX;
}

static int qux=0;

static char *utf8_bar[]={" ","·","▁","▂","▃","▄","▅","▆","▇","█"};
//static char *utf8_bar[]=  {"!","▁","▃","▅","▇","█","!","!","!"};
//static char *utf8_bar[]={"·", "█", "▇", "▆", "▅", "▄", "▃", "▂", "▁", };
//static char *utf8_bar[]={" ","1","2","3","4","5","6","7","8"};

static int
table_destination_each (Babl *babl,
                        void *userdata)
{
  Babl *source = userdata;
  Babl *destination = babl;

  if ((qux++) % babl_formats_count () == qux/ babl_formats_count ())
     printf ("<td class='cell'>&nbsp;</td>");
  else
    {
      Babl *temp = babl_fish_path (source, destination);

      if (temp)
        {
          printf ("<td class='cell'><a href='javascript:o()'>%s",
          utf8_bar[temp->fish_path.conversions]);
          total_length += temp->fish_path.conversions;
          total_cost   += temp->fish_path.cost;

          {
            int i;
            printf ("<div class='tooltip'>");
            printf ("<h3><span class='g'>path</span> %s <span class='g'>to</span> %s</h3>", source->instance.name, destination->instance.name);
            printf ("<table>\n");

                printf ("<tr>");
                printf ("<td><em>conversion</em></td>");
                printf ("<td style='text-align:right'><em>cost</em></td>");
                printf ("</tr>");

             for (i=0; i< temp->fish_path.conversions; i++)
               {
                printf ("<tr>");
                printf ("<td>%s</td>", BABL(temp->fish_path.conversion[i])->instance.name);
                printf ("<td class='r'>%i</td>", BABL(temp->fish_path.conversion[i])->conversion.cost);
                printf ("</tr>");
               }

                printf ("<tr>");
                printf ("<td><em>total</em></td>");
                printf ("<td class='r'><em>%3.0f</em></td>", temp->fish_path.cost);
                printf ("</tr>");
            printf ("</table>\n");
            printf ("</div>\n");
          }
          printf ("</a></td>");
          ok ++;
          total ++;
        }
      else
        {
          printf ("<td class='cell'><a href='javascript:o()'>%s", "&nbsp");
          {
            printf ("<div class='tooltip'>");
            printf ("<h3><span class='g'>Reference</span> %s <span class='g'>to</span> %s</h3>", source->instance.name, destination->instance.name);
            printf ("</div>\n");
          }
          printf ("</a></td>");
        }
    }
  return 0;
}

static int source_no=0;

static int
table_source_each (Babl *babl,
                   void *userdata)
{
  char expanded_name[512];
  const char *s;
  char *d;

  s=babl->instance.name;
  d=&expanded_name[0];

  while (*s)
    {
      switch (*s)
        {
          case ' ':
            *(d++)='&';
            *(d++)='n';
            *(d++)='b';
            *(d++)='s';
            *(d++)='p';
            *(d++)=';';
            *(d)  ='\0';
            s++;
            break;
          default:
            *(d++)=*(s++);
            *(d)  ='\0';
            break;
        }
    }
  
  printf ("<tr>");
  printf ("<td class='format_name'><a href='javascript:o();'>%s", expanded_name);
  {
    int i;

    printf ("<div class='tooltip' id='format_%p'>", babl);
    printf ("<h3>%s</h3>", babl->instance.name);

    printf ("<dl>");
    printf ("<dt>bytes/pixel</dt><dd>%i</dd>", babl->format.bytes_per_pixel);
    printf ("<dt>model</dt><dd>%s</dd>", BABL(babl->format.model)->instance.name  );
    printf ("<dt>loss</dt><dd>%f</dd>", babl->format.loss );
    printf ("<dt>components</dt><dd><table class='nopad'>");

    for (i=0; i< babl->format.components; i++)
      {
        printf ("<tr><td class='type'>%s</td><td class='component'>%s</td></tr>",
         BABL(babl->format.type[i])->instance.name,
         BABL(babl->format.component[i])->instance.name  );
      }
    printf ("</table></dd></dl>");

    printf ("</div>\n");
  }

  printf ("</a></td>");
  babl_format_each (table_destination_each, babl);
  printf ("</tr>\n");
  source_no++;
  return 0;
}

int main (void)
{
  babl_init ();
  test_init ();

  babl_set_extender (babl_extension_quiet_log ());

  printf (
"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n"
"<html>\n"
"<head>\n"
"<title>BablFishPath introspection</title>\n"

   "<style type='text/css'>"
   " body {"
   "   font-family: sans;"
   "   margin-left: 1em;"
   " }"
   " .cell {"
   "  overflow : none;"
   "  height: 1em;"
   "  font-family: monospace;"
   "  border: 1px solid #eee;"
   "  padding: 0;"
   "  margin : 0;"
   "}"
   ".cell>a {"
   "    text-decoration: none;"
   "    color: black;"
   "    cursor: help;"
   "}"
   "div.tooltip {"
   "   border: 0.2em solid black;"
   "   padding-top: 1em;"
   "   padding-right: 2em;"
   "   display: none;"
   "   padding-left: 2em;"
   "   padding-bottom: 3em;"
   "   background-color: white;"
   "   background-repeat: no-repeat;"
   "   background-image: url(graphics/babl-48x48.png);"
   "   background-position: bottom right;"
   "   color: black;"
   "}"
   " .cell>a:hover {"
   "  background-color: black;"
   "  color: white;"
   "}"
   " .format_name {"
   "  height: 1em;"
   "  background-color: #eee;"
   "  padding-right: 0.5em;"
   "  padding-left:  0.5em;"
   "  border-bottom: 1px solid #fff;"
   "}"
   " .format_name>a {"
   "  text-decoration: none;"
   "  color: blue;"
   "    cursor: help;"
   " }"
   " .format_name>a:hover {"
   "  background-color: blue;"
   "  color: white;"
   " }"

   "a:hover>div.tooltip {"
   "   display: block;"
   "   position: fixed;"
   "   bottom: 0;"
   "   right: 0;"
   "}"

   "td.component {"
   "  background-color: #060;"
   "  padding-left: 0.5em;"
   "  padding-top: 0.1em;"
   "  padding-bottom: 0.1em;"
   "  overflow: hidden;"
   "  width: 4em;"
   "  color: white;"
   "  border: 1px solid white;"
   "}"
   "td.type {"
   "  background-color: #006;"
   "  padding-left: 0.5em;"
   "  padding-top: 0.1em;"
   "  padding-bottom: 0.1em;"
   "  overflow: hidden;"
   "  width: 4em;"
   "  color: white;"
   "  border: 1px solid white;"
   "}"
   ".g {"
   "  color: gray;"
   "}"
   ".r {"
   "  text-align: right;"
   "}"

   "</style>"

"<script type='text/javascript'>"
"var tick_count=0;"
"function o ()"
"{"
"   tick_count++;"
"   if (tick_count == 11)"
"        alert(\"«The mind is it's own place,\\nand in itself can make a heaven of hell;\\na hell of heaven.»\\n--Milton\");"
"   else if (tick_count == 42)"
"        alert(\"«So long and thanks for all the fish.»\\n--Adams\");"
"}"
"</script>"


"</head>\n");

  printf ( "<body>\n");

  printf ("<h1>BablFishPath introspection</h1>");
  printf ("<p>The table below represents many of the possible conversions available through babl, (the selection of formats includes all formats that shortcut conversions have been registered for.) </p>");
  printf ("<p>Hover your mouse over a formats name, or a non blank cell on the horizontal line to see further information, both rows and colums represent pixel formats, but only the vertical axis has labels.</p>");

  printf ( "<table cellspacing='0'>\n");
  babl_format_each (table_source_each, NULL);
  printf ("</table>");

  printf ("<div style='height:20em'></div>\n");

  printf ("</body></html>\n");

  babl_destroy ();

  return 0;
}
