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

  printf ("<div>");
  printf ("<h3>Types</h3>");
  babl_type_class_for_each (each_item, NULL);
  printf ("</div>\n");

  printf ("<div>");
  printf ("<h3>Models</h3>");
  babl_model_class_for_each (each_item, NULL);
  printf ("</div>\n");


  printf ("<div>");
  printf ("<h3>Pixelformats</h3>");
  babl_format_class_for_each (each_item, NULL);
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
  const char *fun_pre = "babl_type";
  const char *fun_post = ")";

  switch (babl->class_type)
  {
    case BABL_MODEL: fun_pre = "babl_model"; break;
    case BABL_FORMAT: fun_pre = "babl_format_with_space";
                      fun_post = ", space|NULL)";
    break;
  }

  printf ("<div><div class='item_title'><a href='#%s', name='%s'><tt>%s (\"%s\"%s</tt></a></div>\n",
          normalize (babl->instance.name),
          normalize (babl->instance.name),
          fun_pre,
          babl->instance.name,
          fun_post);
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
      BablModelFlag flags = babl_get_model_flags (babl);
      if (flags & BABL_MODEL_FLAG_RGB)
      {
        printf ("RGB");

        if (flags & BABL_MODEL_FLAG_LINEAR)
          printf (" linear");
        if (flags & BABL_MODEL_FLAG_NONLINEAR)
          printf (" with TRC from space");
        if (flags & BABL_MODEL_FLAG_PERCEPTUAL)
          printf (" with perceptual (sRGB) TRC");
      }
      else if (flags & BABL_MODEL_FLAG_GRAY)
      {
        if (flags & BABL_MODEL_FLAG_LINEAR)
          printf (" Luminance");
        if (flags & BABL_MODEL_FLAG_NONLINEAR)
          printf (" Grayscale with TRC from space");
        if (flags & BABL_MODEL_FLAG_PERCEPTUAL)
          printf (" Grayscale with perceptual (sRGB) TRC");
      }
      else if (flags & BABL_MODEL_FLAG_CMYK)
      {
        if (flags & BABL_MODEL_FLAG_INVERTED)
          printf ("CMYK with inverted color components (0.0=full coverage), for additive compositing");
        else
          printf ("CMYK");
      }

      if (flags & BABL_MODEL_FLAG_ALPHA)
      {
        if (flags & BABL_MODEL_FLAG_ASSOCIATED)
        {
          printf (", associated alpha");
        }
        else
        {
          printf (", separate alpha");
        }
      }

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
      //const Babl *type = BABL (babl->format.type[0]);
      model_doc (model);
      //if (type->instance.doc)
      //  printf (" %s", type->instance.doc);
      //else
      //  printf (" %s", type->instance.name);
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

