#include <stdio.h>
#include <stdlib.h>
#include "../config.h"
#include "babl/babl-internal.h"

//#define SPACE1 babl_space("ProPhoto")
//#define SPACE1 babl_space("Apple")
#define SPACE1 babl_space("sRGB")
//#define SPACE2 babl_space("Apple")

#ifdef _WIN32
/* On Windows setenv() does not exist, using _putenv_s() instead. The overwrite
 * arg is ignored (i.e. same as always 1).
 */
#define setenv(name,value,overwrite) _putenv_s(name, value)
#define putenv _putenv
#endif

int
file_get_contents (const char  *path,
                         char       **contents,
                         long        *length,
                         void        *error);
int
file_get_contents (const char  *path,
                         char       **contents,
                         long        *length,
                         void        *error)
{
  FILE *file;
  long  size;
  char *buffer;

  file = fopen (path,"rb");

  if (!file)
    return -1;

  if (fseek (file, 0, SEEK_END) == -1 || (size = ftell (file)) == -1)
    {
      fclose (file);
      return -1;
    }
  if (length) *length = size;
  rewind (file);
  if ((size_t) size > SIZE_MAX - 8)
    {
      fclose (file);
      return -1;
    }
  buffer = calloc(size + 8, 1);

  if (!buffer)
    {
      fclose(file);
      return -1;
    }

  size -= fread (buffer, 1, size, file);
  if (size)
    {
      fclose (file);
      free (buffer);
      return -1;
    }
  fclose (file);
  *contents = buffer;
  return 0;
}

int 
main (int    argc, 
      char **argv)
{
  int final = 0;
  const Babl *fish;
  const Babl *SPACE2 = NULL;
  setenv ("BABL_INHIBIT_CACHE", "1", 1);

  if (argc < 3)
  {
    fprintf (stderr, "need two args, from and to babl-formats\n");
    return -1;
  }
  if (argc == 4)
    final = 1;

  if (!final)
  {
    putenv ("BABL_DEBUG_CONVERSIONS" "=" "1");
    putenv ("BABL_TOLERANCE"         "=" "0.2");
  }

  babl_init ();

//#define ICC_PATH "/tmp/my.icc"
//#define ICC_PATH "/usr/share/color/icc/colord/AppleRGB.icc"
//#define ICC_PATH "/tmp/ACEScg-elle-V2-labl.icc"
//#define ICC_PATH "/tmp/ACEScg-elle-V2-g10.icc"
//#define ICC_PATH "/tmp/ACEScg-elle-V4-g10.icc"
//#define ICC_PATH "/tmp/ACEScg-elle-V4-g22.icc"


  {
    //char *icc_data = NULL;
    //long     length = 0;
    //file_get_contents (ICC_PATH, &icc_data, &length, NULL);
    //SPACE2 = babl_space_from_icc (icc_data, length, BABL_ICC_INTENT_RELATIVE_COLORIMETRIC, NULL);
    SPACE2 = babl_space ("sRGB");
  }

  fish = babl_fish (babl_format_with_space(argv[1], SPACE1), babl_format_with_space (argv[2], SPACE2));
  if (!fish)
  {
    fprintf (stderr, "!!!! %s %s\n", argv[1], argv[2]);
    return -1;
  }

  if (final)
  switch (fish->class_type)
  {
    case BABL_FISH:
      fprintf (stderr, ">%s\n", babl_get_name (fish));
      break;
    case BABL_FISH_PATH:
      fprintf (stderr, "chosen %s to %s: steps: %i error: %.12f cost: %f\n", argv[1], argv[2], fish->fish_path.conversion_list->count, fish->fish.error, fish->fish_path.cost);
        for (int i = 0; i < fish->fish_path.conversion_list->count; i++)
          {
            fprintf (stderr, "\t%s (cost: %li)\n",
                      babl_get_name(fish->fish_path.conversion_list->items[i]  ), 
    fish->fish_path.conversion_list->items[i]->conversion.cost);
          }
      break;
  }


  babl_exit ();

  return 0;
}
