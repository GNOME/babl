/* babl - dynamically extendable universal pixel conversion library.
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

#include "babl.h"
#include "babl-internal.h"    /* for babl_log */

static void model_html              (Babl *babl);
static void type_html               (Babl *babl);
static void pixel_format_html       (Babl *babl);

static int  each_item               (Babl *babl,
                                     void *user_data);

int
main (void)
{
  babl_init ();


  printf ("<a name='Data-types'></a>\n");
  printf ("<table><tr><th>Data type</th><th>bits</th><th>bytes</th></tr>\n");
  babl_type_each (each_item, NULL);
  printf ("<tr><td><a name='Color-models'></a>&nbsp;</td></tr>\n");
  printf ("<tr><th>Color model</th><th>components</th></tr>\n");
  babl_model_each (each_item, NULL);
  printf ("<tr><td><a name='Pixel-formats'></a>&nbsp;</td></tr>\n");
  printf ("<tr><th>Pixel format</th><th>bytes/pixel</th><th>color model</th><th>bands</th> </tr>\n");
  babl_pixel_format_each (each_item, NULL);
  printf ("</table>\n");

  babl_destroy ();

  return 0;
}

static int
each_item (Babl *babl,
              void *user_data)
{
  printf ("\t<tr><td valign='top'><span class='name'>%s</span></td>", babl->instance.name);

  switch (babl->class_type)
    {
      case BABL_TYPE:
        type_html (babl);
        break;
      case BABL_MODEL:
        model_html (babl);
        break;
      case BABL_PIXEL_FORMAT:
        pixel_format_html (babl);
        break;
      default:
        break;
    }

  printf ("</tr>\n");
  return 0;
}

static void
model_html (Babl *babl)
{
  int i;

  printf ("<td>");
  for (i=0; i< babl->model.components; i++)
    {
      printf ("<span class='component'>%s</span><span class='spacer'>&nbsp;</span><br/>", BABL(babl->model.component[i])->instance.name  );
    }
  printf ("</td>");
}

static void
type_html (Babl *babl)
{
  printf ("<td>%i</td>", babl->type.bits);
  printf ("<td>%i</td>", babl->type.bits / 8);
}

static void
pixel_format_html (Babl *babl)
{
  int i;

  printf ("<td valign='top'>");
    {
      int bytes=0;
      for (i=0; i< babl->pixel_format.bands; i++)
      {
        bytes += BABL(babl->pixel_format.type[i])->type.bits/8;
      }
      printf ("<span class='name'>%i</span>", bytes);
    }
  printf ("</td>");
  printf ("<td valign='top'>");
  printf ("<span class='name'>%s</span>", BABL(babl->pixel_format.model)->instance.name  );
  printf ("</td>");
  printf ("<td>");
  for (i=0; i< babl->pixel_format.bands; i++)
    {
      printf ("<span class='type'>%s </span><span class='component'>%s</span><span class='spacer'>&nbsp;</span><br/>",
       BABL(babl->pixel_format.type[i])->instance.name,
       BABL(babl->pixel_format.component[i])->instance.name  );
    }
  printf ("</td>");
#if 0
  int i;
  babl_log ("\t\tplanar=%i", babl->pixel_format.planar);
  babl_log ("\t\tbands=%i",  babl->pixel_format.bands);

  for (i=0; i< babl->pixel_format.bands; i++)
    {
      babl_log ("\t\tband[%i] type='%s' component='%s' sampling='%s'",
                i,   babl->pixel_format.type[i]->instance.name,
                     babl->pixel_format.component[i]->instance.name,
                     babl->pixel_format.sampling[i]->instance.name);
    }
#endif
}

