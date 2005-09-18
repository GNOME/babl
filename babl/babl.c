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

#include "babl-internal.h"

static int ref_count=0;

void
babl_init (void)
{
  if (ref_count++==0)
    {
      babl_internal_init ();
      babl_type_init ();
      babl_sampling_init ();
      babl_component_init ();
      babl_model_init ();
      babl_format_init ();
      babl_conversion_init ();
      babl_core_init ();
      babl_sanity ();
      babl_extension_base ();
      babl_sanity ();
      babl_extension_init ();
      babl_sanity ();
      babl_fish_init ();
      babl_sanity ();
      babl_extension_post_load ();  /* measure timings and errors */
      babl_sanity ();

    }
}

void
babl_destroy (void)
{
  if (!--ref_count)
    {
      babl_extension_destroy ();
      babl_fish_destroy ();
      babl_conversion_destroy ();
      babl_format_destroy ();
      babl_model_destroy ();
      babl_component_destroy ();
      babl_sampling_destroy ();
      babl_type_destroy ();
      babl_internal_destroy ();
      babl_memory_sanity ();
    }
}
