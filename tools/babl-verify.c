#include <stdio.h>
#include <stdlib.h>
#include "../config.h"
#include "babl/babl-internal.h"

int main (int argc, char **argv)
{
  int final = 0;
  const Babl *fish;
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

  fish = babl_fish (babl_format(argv[1]), babl_format (argv[2]));
  if (!fish)
    return -1;

  if (final)
  switch (fish->class_type)
  {
    case BABL_FISH:
      fprintf (stderr, "%s\n", babl_get_name (fish));
      break;
    case BABL_FISH_PATH:
      fprintf (stderr, "chosen %s to %s: steps: %i error: %f cost: %f\n", argv[1], argv[2], fish->fish_path.conversion_list->count, fish->fish.error, fish->fish_path.cost);
        for (int i = 0; i < fish->fish_path.conversion_list->count; i++)
          {
            fprintf (stderr, "\t%s\n",
                      babl_get_name(fish->fish_path.conversion_list->items[i]  ));
          }
      break;
  }


  babl_exit ();

  return 0;
}
