/* babl - dynamically extendable universal pixel conversion tool.
 * Copyright (C) 2022 Jehan
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

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "babl-shared-util.h"

#include <babl/babl.h>


static const Babl * babl_cli_get_space     (const char    *path,
                                            BablIccIntent  intent);
static void         babl_cli_print_version (FILE          *stream);
static void         babl_cli_print_usage   (FILE          *stream);


int
main (int    argc,
      char **argv)
{
  const Babl    *from_format;
  const Babl    *from_space       = NULL;
  const Babl    *to_format;
  const Babl    *to_space         = NULL;
  const Babl    *fish;
  const char    *from             = NULL;
  const char    *to               = NULL;
  const char    *from_profile     = NULL;
  const char    *to_profile       = NULL;
  BablIccIntent  intent           = BABL_ICC_INTENT_RELATIVE_COLORIMETRIC;
  char          *source;
  char          *dest;
  int            set_from         = 0;
  int            set_to           = 0;
  int            set_from_profile = 0;
  int            set_to_profile   = 0;
  int            set_intent       = 0;
  int            brief_output     = 0;
  int            options_ended    = 0;
  int            n_components;
  int            data_index;
  int            c;
  int            i;

  babl_init ();

  if (argc == 1)
    {
      babl_cli_print_usage (stderr);
      return 2;
    }

  /* Looping through arguments to get source and destination formats. */
  for (i = 1; i < argc; i++)
    {
      if (set_from)
        {
          from = argv[i];
          set_from = 0;
          if (! babl_format_exists (from))
            {
              fprintf (stderr, "babl: unknown format: %s\n", from);
              return 1;
            }
        }
      else if (set_to)
        {
          to = argv[i];
          set_to = 0;
          if (! babl_format_exists (to))
            {
              fprintf (stderr, "babl: unknown format: %s\n", to);
              return 1;
            }
        }
      else if (set_from_profile)
        {
          set_from_profile = 0;
          from_profile = argv[i];
        }
      else if (set_to_profile)
        {
          set_to_profile = 0;
          to_profile = argv[i];
        }
      else if (set_intent)
        {
          set_intent = 0;

          if (strcmp (argv[i], "perceptual") == 0)
            {
              intent = BABL_ICC_INTENT_PERCEPTUAL;
            }
          else if (strcmp (argv[i], "relative") == 0)
            {
              intent = BABL_ICC_INTENT_RELATIVE_COLORIMETRIC;
            }
          else if (strcmp (argv[i], "saturation") == 0)
            {
              intent = BABL_ICC_INTENT_SATURATION;
            }
          else if (strcmp (argv[i], "absolute") == 0)
            {
              intent = BABL_ICC_INTENT_ABSOLUTE_COLORIMETRIC;
            }
          else
            {
              fprintf (stderr, "babl: unknown intent: %s\n", argv[i]);
              fprintf (stderr, "valid intents: perceptual, relative, saturation, absolute.\n");
              return 2;
            }
        }
      else if (strcmp (argv[i], "--") == 0)
        {
          break;
        }
      else if (strcmp (argv[i], "--help") == 0 ||
               strcmp (argv[i], "-h") == 0)
        {
          babl_cli_print_usage (stdout);

          return 0;
        }
      else if (strcmp (argv[i], "--version") == 0 ||
               strcmp (argv[i], "-v") == 0)
        {
          babl_cli_print_version (stdout);

          return 0;
        }
      else if (strcmp (argv[i], "--from") == 0 ||
               strcmp (argv[i], "-f") == 0)
        {
          set_from = 1;
        }
      else if (strcmp (argv[i], "--to") == 0 ||
               strcmp (argv[i], "-t") == 0)
        {
          set_to = 1;
        }
      else if (strcmp (argv[i], "--input-space") == 0   ||
               /* Deprecated option name, but keep compatibility. */
               strcmp (argv[i], "--input-profile") == 0 ||
               strcmp (argv[i], "-i") == 0)
        {
          if (strcmp (argv[i], "--input-profile") == 0)
            fprintf (stderr, "babl: warning: --input-profile option renamed --input-space\n");
          set_from_profile = 1;
        }
      else if (strcmp (argv[i], "--output-space") == 0   ||
               /* Deprecated option name, but keep compatibility. */
               strcmp (argv[i], "--output-profile") == 0 ||
               strcmp (argv[i], "-o") == 0)
        {
          if (strcmp (argv[i], "--output-profile") == 0)
            fprintf (stderr, "babl: warning: --output-profile option renamed --output-space\n");
          set_to_profile = 1;
        }
      else if (strcmp (argv[i], "--intent") == 0 ||
               strcmp (argv[i], "-r") == 0)
        {
          set_intent = 1;
        }
      else if (strcmp (argv[i], "--brief") == 0 ||
               strcmp (argv[i], "-b") == 0)
        {
          brief_output = 1;
        }
    }

  if (from_profile != NULL)
    {
      if (strlen (from_profile) > 2 && *from_profile == '*')
        {
          if (from_profile[1] != '*')
            from_space = babl_space (from_profile + 1);
          else
            from_space = babl_cli_get_space (from_profile + 1, intent);
        }
      else
        {
          from_space = babl_cli_get_space (from_profile, intent);
        }

      if (! from_space)
        {
          if (strlen (from_profile) > 2 && *from_profile == '*' && from_profile[1] != '*')
            {
              fprintf (stderr, "babl: unknown named space '%s'\n",
                       from_profile + 1);
              fprintf (stderr, "      Known spaces: sRGB, scRGB (sRGB with linear TRCs), Rec2020, "
                       "Adobish, Apple, ProPhoto, ACEScg and ACES2065-1.\n");
            }
          return 6;
        }
    }

  if (to_profile != NULL)
    {
      if (strlen (to_profile) > 2 && *to_profile == '*')
        {
          if (to_profile[1] != '*')
            to_space = babl_space (to_profile + 1);
          else
            to_space = babl_cli_get_space (to_profile + 1, intent);
        }
      else
        {
          to_space = babl_cli_get_space (to_profile, intent);
        }

      if (! to_space)
        {
          if (strlen (to_profile) > 2 && *to_profile == '*' && to_profile[1] != '*')
            {
              fprintf (stderr, "babl: unknown named space '%s'\n",
                       to_profile + 1);
              fprintf (stderr, "      Known spaces: sRGB, scRGB (sRGB with linear TRCs), Rec2020, "
                       "Adobish, Apple, ProPhoto, ACEScg and ACES2065-1.\n");
            }
          return 6;
        }
    }

  if (from == NULL)
    {
      if (babl_space_is_cmyk (from_space))
        from = "CMYK float";
      else if (babl_space_is_gray (from_space))
        from = "Y' float";
      else
        from = "R'G'B' float";
    }
  if (to == NULL)
    {
      if (babl_space_is_cmyk (to_space))
        to = "CMYK float";
      else if (babl_space_is_gray (to_space))
        to = "Y' float";
      else
        to = "R'G'B' float";
    }

  from_format  = babl_format_with_space (from, from_space);
  n_components = babl_format_get_n_components (from_format);
  source       = malloc (babl_format_get_bytes_per_pixel (from_format));
  data_index   = 0;

  to_format    = babl_format_with_space (to, to_space);
  dest         = malloc (babl_format_get_bytes_per_pixel (to_format));

  /* Re-looping through arguments, to be more flexible with argument orders.
   * In this second loop, we get the source components' values.
   */
  set_from = set_to = set_to_profile = set_from_profile = 0;
  for (i = 1, c = 0; i < argc; i++)
    {
      if (set_from)
        {
          set_from = 0;
          /* Pass. */
        }
      else if (set_to)
        {
          set_to = 0;
          /* Pass. */
        }
      else if (set_from_profile)
        {
          set_from_profile = 0;
          /* Pass. */
        }
      else if (set_to_profile)
        {
          set_to_profile = 0;
          /* Pass. */
        }
      else if (set_intent)
        {
          set_intent = 0;
          /* Pass. */
        }
      else if (! options_ended && strncmp (argv[i], "-", 1) == 0)
        {
          if (strcmp (argv[i], "--") == 0)
            {
              options_ended = 1;
            }
          else if (strcmp (argv[i], "--help") == 0 ||
                   strcmp (argv[i], "-h") == 0)
             {
               /* Pass. */
             }
          else if (strcmp (argv[i], "--version") == 0 ||
                   strcmp (argv[i], "-v") == 0)
             {
               /* Pass. */
             }
          else if (strcmp (argv[i], "--from") == 0 ||
                   strcmp (argv[i], "-f") == 0)
            {
              set_from = 1;
            }
          else if (strcmp (argv[i], "--to") == 0 ||
                   strcmp (argv[i], "-t") == 0)
            {
              set_to = 1;
            }
          else if (strcmp (argv[i], "--input-profile") == 0 ||
                   strcmp (argv[i], "--input-space") == 0   ||
                   strcmp (argv[i], "-i") == 0)
            {
              set_from_profile = 1;
            }
          else if (strcmp (argv[i], "--output-profile") == 0 ||
                   strcmp (argv[i], "--output-space") == 0   ||
                   strcmp (argv[i], "-o") == 0)
            {
              set_to_profile = 1;
            }
          else if (strcmp (argv[i], "--intent") == 0 ||
                   strcmp (argv[i], "-r") == 0)
            {
              set_intent = 1;
            }
          else if (strcmp (argv[i], "--brief") == 0 ||
                   strcmp (argv[i], "-b") == 0)
            {
              /* Pass. */
            }
          else
            {
              fprintf (stderr, "babl: unknown option: %s\n", argv[i]);
              babl_cli_print_usage (stderr);
              return 2;
            }
        }
      else
        {
          const Babl *arg_type;
          char       *endptr = NULL;

          if (c >= n_components)
            {
              fprintf (stderr, "babl: unexpected component: %s\n", argv[i]);
              babl_cli_print_usage (stderr);
              return 2;
            }

          arg_type = babl_format_get_type (from_format, c);

          if (strcmp (babl_get_name (arg_type), "double") == 0)
            {
              double  value = strtod (argv[i], &endptr);
              double *dsrc = (double *) (source + data_index);

              if (value == 0.0 && endptr == argv[i])
                {
                  fprintf (stderr, "babl: expected type of component %d is '%s', invalid value: %s\n",
                           c, babl_get_name (arg_type), argv[i]);
                  return 3;
                }

              *dsrc = value;
              data_index += 8;
            }
          else if (strcmp (babl_get_name (arg_type), "float") == 0)
            {
              float  value = strtof (argv[i], &endptr);
              float *fsrc = (float *) (source + data_index);

              if (value == 0.0f && endptr == argv[i])
                {
                  fprintf (stderr, "babl: expected type of component %d is '%s', invalid value: %s\n",
                           c, babl_get_name (arg_type), argv[i]);
                  return 3;
                }

              *fsrc = value;
              data_index += 4;
            }
          else if (strcmp (babl_get_name (arg_type), "half") == 0)
            {
              float  value = strtof (argv[i], &endptr);
              void  *fsrc  = (void *) (source + data_index);

              if (value == 0.0f && endptr == argv[i])
                {
                  fprintf (stderr, "babl: expected type of component %d is '%s', invalid value: %s\n",
                           c, babl_get_name (arg_type), argv[i]);
                  return 3;
                }

              _babl_float_to_half (fsrc, &value, 1);
              data_index += 2;
            }
          else if (strncmp (babl_get_name (arg_type), "u", 1) == 0)
            {
              long int value = strtol (argv[i], &endptr, 10);

              if (value == 0 && endptr == argv[i])
                {
                  fprintf (stderr, "babl: expected type of component %d is '%s', invalid value: %s\n",
                           c, babl_get_name (arg_type), argv[i]);
                  return 3;
                }

              if (strcmp (babl_get_name (arg_type), "u8") == 0)
                {
                  uint8_t *usrc = (uint8_t *) (source + data_index);

                  *usrc = value;
                  data_index += 1;
                }
              else if (strcmp (babl_get_name (arg_type), "u16") == 0)
                {
                  uint16_t *usrc = (uint16_t *) (source + data_index);

                  *usrc = value;
                  data_index += 2;
                }
              else if (strcmp (babl_get_name (arg_type), "u32") == 0)
                {
                  uint32_t *usrc = (uint32_t *) (source + data_index);

                  *usrc = value;
                  data_index += 4;
                }
              else
                {
                  fprintf (stderr, "babl: unsupported unsigned type '%s' of component %d: %s\n",
                           babl_get_name (arg_type), c, argv[i]);
                  return 4;
                }
            }
          else
            {
              fprintf (stderr, "babl: unsupported type '%s' of component %d: %s\n",
                       babl_get_name (arg_type), c, argv[i]);
              return 4;
            }

          c++;
        }
    }

  if (c != n_components)
    {
      fprintf (stderr, "babl: %d components expected, %d components were passed\n",
               n_components, c);
      babl_cli_print_usage (stderr);
      return 2;
    }

  /* Actual processing. */
  fish = babl_fish (from_format, to_format);
  babl_process (fish, source, dest, 1);

  /* Now displaying the result. */
  n_components = babl_format_get_n_components (to_format);
  data_index   = 0;

  if (! brief_output)
    printf ("Converting from \"%s\" to \"%s\":\n",
                    babl_get_name (from_format),
                    babl_get_name (to_format));

  for (c = 0; c < n_components; c++)
    {
      const Babl *arg_type = NULL;

      arg_type = babl_format_get_type (to_format, c);

      if (strcmp (babl_get_name (arg_type), "double") == 0)
        {
          double value = *((double *) (dest + data_index));

          data_index += 8;

          if (brief_output)
            printf ("%s%f", c > 0 ? " ":"", value);
          else
            printf ("- %f\n", value);
        }
      else if (strcmp (babl_get_name (arg_type), "float") == 0)
        {
          float value = *((float *) (dest + data_index));

          data_index += 4;

          if (brief_output)
            printf ("%s%f", c > 0 ? " ":"", value);
          else
            printf ("- %f\n", value);
        }
      else if (strcmp (babl_get_name (arg_type), "half") == 0)
        {
          void  *value = (void *) (dest + data_index);
          float  fvalue;

          _babl_half_to_float (&fvalue, value, 1);
          data_index += 2;

          if (brief_output)
            printf ("%s%f", c > 0 ? " ":"", fvalue);
          else
            printf ("- %f\n", fvalue);
        }
      else if (strcmp (babl_get_name (arg_type), "u8") == 0)
        {
          uint8_t value = *((uint8_t *) (dest + data_index));

          data_index += 1;

          if (brief_output)
            printf ("%s%d", c > 0 ? " ":"", value);
          else
            printf ("- %d\n", value);
        }
      else if (strcmp (babl_get_name (arg_type), "u16") == 0)
        {
          uint16_t value = *((uint16_t *) (dest + data_index));

          data_index += 2;

          if (brief_output)
            printf ("%s%d", c > 0 ? " ":"", value);
          else
            printf ("- %d\n", value);
        }
      else if (strcmp (babl_get_name (arg_type), "u32") == 0)
        {
          uint32_t value = *((uint32_t *) (dest + data_index));

          data_index += 4;

          if (brief_output)
            printf ("%s%d", c > 0 ? " ":"", value);
          else
            printf ("- %d\n", value);
        }
      else
        {
          fprintf (stderr, "babl: unsupported type '%s' of returned component %d.\n",
                   babl_get_name (arg_type), c);
          return 5;
        }
    }

  babl_exit ();

  free (source);
  free (dest);

  return 0;
}

static const Babl *
babl_cli_get_space (const char    *path,
                    BablIccIntent  intent)
{
  const Babl *space;
  FILE       *f;
  char       *icc_data;
  long        icc_length;
  size_t      icc_read;
  const char *error = NULL;

  f = fopen (path, "r");

  if (f == NULL)
    {
      fprintf (stderr, "babl: failed to open '%s': %s\n",
               path, strerror (errno));
      return NULL;
    }

  fseek (f, 0, SEEK_END);
  icc_length = ftell (f);
  fseek (f, 0, SEEK_SET);

  icc_data = malloc (icc_length);
  icc_read = fread(icc_data, icc_length, 1, f);
  if (icc_read != 1)
    {
      fprintf(stderr, "babl: failed to read '%s': %s\n",
              path, strerror(errno));
      fclose(f);
      free(icc_data);
      return NULL;
    }

  fclose (f);

  space = babl_space_from_icc (icc_data, icc_length, intent, &error);

  free(icc_data);

  if (space == NULL)
    {
      fprintf (stderr, "babl: failed to load space from '%s': %s\n",
               path, error);
      return NULL;
    }

  return space;
}

static void
babl_cli_print_version (FILE *stream)
{
  fprintf (stream,
           BABL_VERSION
           "\n");
}

static void
babl_cli_print_usage (FILE *stream)
{
  fprintf (stream,
           "Usage: babl [options] [c1 ..]\n"
           "Convert color data from a specific Babl format and space to another.\n"
           "\n"
           "  Options:\n"
           "     -h, --help            this help information\n"
           "\n"
           "     -v, --version         Babl version\n"
           "\n"
           "     -f, --from            input Babl format\n"
           "\n"
           "     -t, --to              output Babl format\n"
           "\n"
           "     -i, --input-space     input profile or named space\n"
           "                           named spaced are asterisk-prefixed, i.e. '*Rec2020'\n"
           "                           as special-case, double the first asterisk if your profile path starts with '*'\n"
           "\n"
           "     -o, --output-space    output profile or named space\n"
           "                           named spaced are asterisk-prefixed, i.e. '*Rec2020'\n"
           "                           as special-case, double the first asterisk if your profile path starts with '*'\n"
           "\n"
           "     -r, --intent          rendering intent\n"
           "                           it only works with an output profile\n"
           "\n"
           "     -b, --brief           brief output\n"
           "                           it can be re-entered as input for chain conversions\n"
           "\n"
           "All parameters following -- are considered components values. "
           "This is useful to input negative components.\n\n"
           "The tool expects exactly the number of components of your input format.\n\n"
           "The default input and output formats are \"R'G'B' float\" (respectively \"CMYK float\" "
           "or \"Y' float\" if you specified a CMYK or grayscale profile).\n\n"
           "The default space is sRGB for RGB formats or a naive CMYK space for CMYK formats.\n"
           "Other spaces can be specified through an ICC profile or a named space prefixed by "
           "an asterisk.\n"
           "Known spaces: sRGB, scRGB (sRGB with linear TRCs), Rec2020, Adobish, Apple, ProPhoto, "
           "ACEScg and ACES2065-1.\n");
}
