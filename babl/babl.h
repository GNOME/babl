/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2005-2008, Øyvind Kolås.
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

#ifndef _BABL_H
#define _BABL_H

#ifdef __cplusplus
extern "C" {
#endif

#define BABL_INSIDE_BABL_H
#include <babl/babl-macros.h>
#include <babl/babl-types.h>
#include <babl/babl-version.h>
#undef BABL_INSIDE_BABL_H


/**
 * babl_init:
 *
 * Initializes the babl library.
 */
void         babl_init      (void);

/**
 * babl_exit:
 *
 * Deinitializes the babl library and frees any resources used when
 * matched with the number of calls to babl_init().
 */
void         babl_exit      (void);

/**
 * babl_type:
 *
 * Returns the babl object representing the data type given by @name
 * such as for example "u8", "u16" or "float".
 */
const Babl * babl_type      (const char *name);

/**
 * babl_sampling:
 *
 * Returns the babl object representing the @horizontal and @vertical
 * sampling such as for example 2, 2 for the chroma components in
 * YCbCr.
 */
const Babl * babl_sampling  (int horizontal,
                             int vertical);

/**
 * babl_component:
 *
 * Returns the babl object representing the color component given by
 * @name such as for example "R", "cyan" or "CIE L".
 */
const Babl * babl_component (const char *name);

/**
 * babl_model:
 *
 * Returns the babl object representing the color model given by @name
 * such as for example "RGB", "CMYK" or "CIE Lab".
 */
const Babl * babl_model     (const char *name);

/**
 * babl_format:
 *
 * Returns the babl object representing the color format given by
 * @name such as for example "RGB u8", "CMYK float" or "CIE Lab u16".
 */
const Babl * babl_format    (const char *name);

/**
 * babl_fish:
 *
 *  Create a babl fish capable of converting from source_format to
 *  destination_format, source and destination can be either strings
 *  with the names of the formats or Babl-format objects.
 */
const Babl * babl_fish      (const void *source_format,
                             const void *destination_format);

/**
 * babl_process:
 *
 *  Process n pixels from source to destination using babl_fish,
 *  returns number of pixels converted.
 */
long         babl_process   (const Babl *babl_fish,
                             const void *source,
                             void *destination,
                             long  n);


/**
 * babl_get_name:
 *
 * Returns a string decsribing a Babl object.
 */
const char * babl_get_name                     (const Babl *babl);

/**
 * babl_format_has_alpha:
 *
 * Returns whether the @format has an alpha channel.
 */
int          babl_format_has_alpha             (const Babl *format);

/**
 * babl_format_get_bytes_per_pixel:
 *
 * Returns the bytes per pixel for a babl color format.
 */
int          babl_format_get_bytes_per_pixel   (const Babl *format);

/**
 * babl_format_get_model:
 *
 * Return the model used for constructing the format.
 */
const Babl * babl_format_get_model             (const Babl *format);

/**
 * babl_format_get_n_components:
 *
 * Returns the number of components for the given @format.
 */
int          babl_format_get_n_components      (const Babl *format);

/**
 * babl_format_get_type:
 *
 * Returns the type in the given @format for the given
 * @component_index.
 */
const Babl * babl_format_get_type              (const Babl *format,
                                                int         component_index);


/**
 * babl_type_new:
 *
 * Defines a new data type in babl. A data type that babl can have in
 * its buffers requires conversions to and from "double" to be
 * registered before passing sanity.
 *
 *     babl_type_new       (const char *name,
 *                          "bits",     int bits,
 *                          ["min_val", double min_val,]
 *                          ["max_val", double max_val,]
 *                          NULL);
 */
const Babl * babl_type_new (void *first_arg,
                            ...) BABL_ARG_NULL_TERMINATED;

/**
 * babl_component_new:
 *
 * Defines a new color component with babl.
 *
 *     babl_component_new  (const char *name,
 *                          NULL);
 */
const Babl * babl_component_new  (void *first_arg,
                                  ...) BABL_ARG_NULL_TERMINATED;

/**
 * babl_model_new:
 *
 * Defines a new color model in babl. If no name is provided a name is
 * generated by concatenating the name of all the involved components.
 *
 *     babl_model_new      (["name", const char *name,]
 *                          BablComponent *component1,
 *                          [BablComponent *componentN, ...]
 *                          NULL);
 */
const Babl * babl_model_new (void *first_arg,
                            ...) BABL_ARG_NULL_TERMINATED;

/**
 * babl_format_new:
 *
 * Defines a new pixel format in babl. Provided BablType and|or
 * BablSampling is valid for the following components as well. If no
 * name is provided a (long) descriptive name is used.
 *
 *     babl_format_new     (["name", const char *name,]
 *                          BablModel          *model,
 *                          [BablType           *type,]
 *                          [BablSampling,      *sampling,]
 *                          BablComponent      *component1,
 *                          [[BablType           *type,]
 *                           [BablSampling       *sampling,]
 *                           BablComponent      *componentN,
 *                           ...]
 *                          ["planar",]
 *                          NULL);
 */
const Babl * babl_format_new (const void *first_arg,
                              ...) BABL_ARG_NULL_TERMINATED;

/*
 * babl_format_n:
 *
 * Defines a new pixel format in babl. With the specified data storage
 * type and the given number of components. At the moment behavior of
 * conversions are only well defined to other babl_format_n derived formats
 * with the same number of components.
 */
const Babl *
babl_format_n (const Babl *type,
               int   components);

/**
 * babl_format_is_format_n:
 *
 * Returns whether the @format is a format_n type.
 */
int babl_format_is_format_n (const Babl *format);

/**
 * babl_conversion_new:
 *
 * Defines a new conversion between either two formats, two models or
 * two types in babl.
 *
 *     babl_conversion_new (<BablFormat *source, BablFormat *destination|
 *                          BablModel  *source, BablModel  *destination|
 *                          BablType   *source, BablType   *destination>,
 *                          <"linear"|"planar">, <BablFuncLinear | BablFuncPlanar> conv_func,
 *                          NULL);
 */
const Babl * babl_conversion_new (const void *first_arg,
                                  ...) BABL_ARG_NULL_TERMINATED;


/**
 * babl_new_palette:
 *
 * create a new palette based format, name is optional pass in NULL to get
 * an anonymous format. If you pass in with_alpha the format also gets
 * an 8bit alpha channel. Returns the BablModel of the color model. If
 * you pass in the same name the previous formats will be provided
 * again.
 */
const Babl *babl_new_palette (const char  *name,
                              const Babl **format_u8,
                              const Babl **format_u8_with_alpha);

/**
 * babl_format_is_palette:
 *
 * check whether a format is a palette backed format.
 */
int   babl_format_is_palette   (const Babl *format);

/**
 * babl_palette_set_palette:
 *
 * Assign a palette to a palette format, the data is a single span of pixels
 * representing the colors of the palette.
 */
void  babl_palette_set_palette (const Babl        *babl,
                                const Babl        *format,
                                void              *data,
                                int                count);

/**
 * babl_palette_reset:
 *
 * reset a palette to initial state, frees up some caches that optimize
 * conversions.
 */
void  babl_palette_reset       (const Babl        *babl);



/**
 * babl_set_user_data: (skip)
 *
 * associate a data pointer with a format/model, this data can be accessed and
 * used from the conversion functions, encoding color profiles, palettes or
 * similar with the data, perhaps this should be made internal API, not
 * accesible at all from
 */
void   babl_set_user_data     (const Babl *babl, void *data);

/**
 * babl_get_user_data: (skip)
 *
 * Get data set with babl_set_user_data
 */
void * babl_get_user_data     (const Babl *babl);



/*
 * Backwards compatibility stuff
 *
 * NOTE: will most likely be removed in the first stable release!
 */
#ifndef BABL_DISABLE_DEPRECATED
#define babl_destroy babl_exit
#endif


#ifdef __cplusplus
}
#endif

#endif
