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
 * <http://www.gnu.org/licenses/>.
 */

#include "babl-internal.h"

static int ref_count = 0;

void
babl_init (void)
{
  babl_cpu_accel_set_use (1);

  if (ref_count++ == 0)
    {
      babl_internal_init ();
      babl_type_class_init ();
      babl_sampling_class_init ();
      babl_component_class_init ();
      babl_model_class_init ();
      babl_format_class_init ();
      babl_conversion_class_init ();
      babl_core_init ();
      babl_sanity ();
      babl_extension_base ();
      babl_sanity ();
      babl_extension_class_init ();
      babl_sanity ();
      babl_fish_class_init ();
      babl_sanity ();
    }
}

void
babl_exit (void)
{
  if (!-- ref_count)
    {
      if (getenv ("BABL_STATS"))
        {
          char  logfile_name[] = "/tmp/babl-stats.html";
          FILE *logfile;
          logfile = fopen (logfile_name, "w");
          if (logfile)
            {
              babl_fish_stats (logfile);
              fclose (logfile);
            }
        }

      babl_extension_class_destroy ();
      babl_fish_class_destroy ();
      babl_conversion_class_destroy ();
      babl_format_class_destroy ();
      babl_model_class_destroy ();
      babl_component_class_destroy ();
      babl_sampling_class_destroy ();
      babl_type_class_destroy ();
      babl_internal_destroy ();
      babl_memory_sanity ();
    }
}
