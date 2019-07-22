/* babl - dynamically extendable universal pixel conversion library.
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
 * <https://www.gnu.org/licenses/>.
 *
 */
 
#include "config.h"
#include "babl-internal.h"    /* needed for babl_log */

static void 
model_html (Babl *babl);

static void 
type_html (Babl *babl);

static void 
format_html (Babl *babl);

static void 
space_html (Babl *babl);

static void 
conversion_html (Babl *babl);

static int  
each_item (Babl *babl,
           void *user_data);
           
int
main (void)
{
  babl_init ();

  printf ("<div class='expander'>");
  printf ("<div class='expander_title'><a style='font-size:110%%' name='Data-types' href='javascript:toggle_visible(\"x_types\")'>Data types</a></div><div class='expander_content' id='x_types'>\n");
  babl_type_class_for_each (each_item, NULL);
  printf ("</div>\n");
  printf ("</div>\n");

  printf ("<div class='expander'>");
  printf ("<div class='expander_title'><a style='font-size:110%%' name='Color-models' href='javascript:toggle_visible(\"x_models\")'>Color models</a></div><div class='expander_content' id='x_models'>\n");
  babl_model_class_for_each (each_item, NULL);
  printf ("</div>\n");
  printf ("</div>\n");


  printf ("<div class='expander'>");
  printf ("<div class='expander_title'><a style='font-size:110%%' name='Pixel-formats' href='javascript:toggle_visible(\"x_formats\")'>Pixel formats</a></div><div class='expander_content' id='x_formats'>\n");
  babl_format_class_for_each (each_item, NULL);
  printf ("</div>\n");
  printf ("</div>\n");

/*
   printf ("<div class='expander'>");
   printf ("<div class='expander_title'><a style='font-size:110%%' name='Conversions' href='javascript:toggle_visible(\"x_conversions\")'>Conversions</a></div><div class='expander_content' id='x_conversions'>\n");
   babl_conversion_each (each_item, NULL);
   printf ("</div>\n");
   printf ("</div>\n");
 */
  babl_exit ();

  return 0;
}


static char normalized_buf[512];

static const char *
normalize (const char *str)
{
  char *s = normalized_buf;

  strcpy (normalized_buf, str);

  while (*s)
    {
      if ((*s >= 'a' && *s <= 'z') ||
          (*s >= 'A' && *s <= 'Z') ||
          (*s >= '0' && *s <= '9'))
        {
        }
      else if (*s == '~')
        {
        }
      else
        {
          *s = '_';
        }
      s++;
    }
  return normalized_buf;
}


static int
each_item (Babl *babl,
           void *user_data)
{
  printf ("<div><div class='item_title'><a href='#%s', name='%s'>%s</a></div>\n",
          normalize (babl->instance.name),
          normalize (babl->instance.name),
          babl->instance.name);
  printf ("<div class='item_body'>");


  switch (babl->class_type)
    {
      case BABL_TYPE:
        type_html (babl);
        break;

      case BABL_MODEL:
        model_html (babl);
        break;

      case BABL_FORMAT:
        format_html (babl);
        break;

      case BABL_SPACE:
        space_html (babl);
        break;

      case BABL_CONVERSION:
      case BABL_CONVERSION_LINEAR:
      case BABL_CONVERSION_PLANE:
      case BABL_CONVERSION_PLANAR:
        conversion_html (babl);
        break;

      default:
        break;
    }

  printf ("</div>\n");
  printf ("</div>\n");
  return 0;
}

static void
model_doc (const Babl *babl)
{
  if (babl->instance.doc)
    printf ("%s", babl->instance.doc);
  else
    {
    }
}


static void
model_html (Babl *babl)
{
  int i;
  printf ("<p>");
  model_doc (babl);
  printf ("</p>");

  printf ("<dl>");
  printf ("<dt>components</dt><dd><table class='nopad'>");

  for (i = 0; i < babl->model.components; i++)
    {
      printf ("<tr><td class='type'>%s</td></tr>",
              BABL (babl->model.component[i])->instance.name);
    }
  printf ("</table></dd></dl>");
}

static void
type_html (Babl *babl)
{
  if (babl->instance.doc)
    printf ("<p>%s</p>", babl->instance.doc);

  printf ("<dl><dt>bits</dt><dd>%i</dd>", babl->type.bits);
  printf ("<dt>bytes</dt><dd>%i</dd></dl>", babl->type.bits / 8);
}


static void
conversion_html (Babl *babl)
{
  printf ("%s<br/>\n", babl->instance.name);
}

static void
space_html (Babl *babl)
{
  printf ("%s<br/>\n", babl->instance.name);
}

static void
format_html (Babl *babl)
{
  int i;
  const Babl *model = BABL (babl->format.model);

  printf ("<p>");
  if (babl->instance.doc)
    printf ("%s", babl->instance.doc);
  else
    {
      const Babl *type = BABL (babl->format.type[0]);
      model_doc (model);
      if (type->instance.doc)
        printf (" %s", type->instance.doc);
      else
        printf (" %s", type->instance.name);
    }
  printf ("</p>");

  printf ("<dl>");
  printf ("<dt>bytes/pixel</dt><dd>%i</dd>", babl->format.bytes_per_pixel);
  printf ("<dt>model</dt><dd><a href='#%s'>%s</a></dd>",
    normalize( BABL (babl->format.model)->instance.name),
    BABL (babl->format.model)->instance.name);
  printf ("<dt>components</dt><dd><table class='nopad'>");

  for (i = 0; i < babl->format.components; i++)
    {
      printf ("<tr><td class='type'><a href='#%s' style='color:white''>%s</a></td><td class='component'>%s</td></tr>",
              normalize(BABL (babl->format.type[i])->instance.name),
              BABL (babl->format.type[i])->instance.name,
              BABL (babl->format.component[i])->instance.name);
    }
  printf ("</table></dd></dl>");
}

