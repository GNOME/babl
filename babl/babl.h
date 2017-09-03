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
 * babl_trc:
 *
 * Look up a TRC by name, "sRGB" "1.0" "linear" and "2.2" are recognized
 * strings in a stock babl configuration.
 */
const Babl * babl_trc       (const char *name);

/**
 * babl_trc_gamma:
 *
 * Creates a Babl TRC for a specific gamma value, it will be given
 * a name that is a short string representation of the value.
 */
const Babl * babl_trc_gamma (double gamma);

/**
 * babl_space:
 *
 * Returns the babl object representing the specific RGB matrix color
 * working space referred to by name. Babl knows of:
 *    sRGB, Adobe, Apple and ProPhoto
 */
const Babl * babl_space (const char *name);

/**
 * babl_space_from_chromaticities:
 *
 * Creates a new babl-space/ RGB matrix color space definition with the
 * specified CIE xy(Y) values for white point: wx, wy and primary
 * chromaticities: rx,ry,gx,gy,bx,by and TRCs to be used. After registering a
 * new babl-space it can be used with babl_space() passing its name;
 *
 * Internally this does the math to derive the RGBXYZ matrix as used in an ICC
 * profile.
 */
const Babl * babl_space_from_chromaticities (const char *name,
                                             double wx, double wy,
                                             double rx, double ry,
                                             double gx, double gy,
                                             double bx, double by,
                                             const Babl *trc_red,
                                             const Babl *trc_green,
                                             const Babl *trc_blue);

/**
 * babl_space_from_rgbxyz_matrix:
 *
 * Creates a new RGB matrix color space definition using a precomputed D50
 * adapted 3x3 matrix and associated CIE XYZ whitepoint, as possibly read from
 * an ICC profile.
 */
const Babl *
babl_space_from_rgbxyz_matrix (const char *name,
                               double wx, double wy, double wz,
                               double rx, double gx, double bx,
                               double ry, double gy, double by,
                               double rz, double gz, double bz,
                               const Babl *trc_red,
                               const Babl *trc_green,
                               const Babl *trc_blue);
/**
 * babl_space_from_icc:
 *
 * @icc_data: pointer to icc profile in memory
 * @icc_length: length of icc profile in bytes
 " @error: pointer to a string where decoding errors can be stored,
 *         if an error occurs, NULL is returned and an error message
 *         is provided in error.
 *
 * Create a babl space from an in memory ICC profile, the
 * profile does no longer need to be loaded for the space to work,
 * multiple calls with the same icc profile will result in the
 * same space.
 *
 * If a BablSpace cannot be created from the profile NULL is returned and
 * a static string is set on the provided error location.
 */
const Babl *babl_space_from_icc (const char  *icc_data,
                                 int          icc_length,
                                 const char **error);


/* babl_space_to_icc:
 *
 * Creates an ICCv2 RGB matrix profile for a babl space. The profiles strive to
 * be as small and compact as possible, TRCs are stored as 1024 entry LUT(s).
 *
 * you should make a copy of the profile before making another call to this
 * function.
 */
const char *babl_space_to_icc (const Babl *space, int *icc_length);

/* babl_space_get_rgbtoxyz:

   Returns the internal, double-precision 3x3 matrix used to convert linear
   RGB data to CIE XYZ.
 */
const double * babl_space_get_rgbtoxyz (const Babl *space);

/* babl_space_to_xyz:
 *
 * converts a double triplet from linear RGB to CIE XYZ.
 */
void babl_space_to_xyz   (const Babl *space, const double *rgb, double *xyz);

/* babl_space_from_xyz:
 *
 * converts double triplet from CIE XYZ to linear RGB
 */
void babl_space_from_xyz (const Babl *space, const double *xyz, double *rgb);

/**
 * babl_format:
 *
 * Returns the babl object representing the color format given by
 * @name such as for example "RGB u8", "CMYK float" or "CIE Lab u16".
 */
const Babl * babl_format            (const char *name);

/**
 * babl_format_with_space:
 *
 * Returns the babl object representing the color format given by
 * @name such as for example "RGB u8", "R'G'B'A float", "Y float" with
 * a specific RGB working space used as the space, the resulting format
 * has -space suffixed to it, unless the space requested is sRGB then
 * the unsuffixed version is used. If a format is passed in as space
 * the space of the format is used.
 */
const Babl * babl_format_with_space (const char *name, const Babl *space);

/*
 * babl_format_get_space:
 *
 * Retrieve the RGB color space used for a pixel format.
 */
const Babl * babl_format_get_space  (const Babl *format);


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
 * Returns a string describing a Babl object.
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
 * babl_conversion_get_source_space:
 *
 * Returns the RGB space defined for the source of conversion.
 */
const Babl *babl_conversion_get_source_space      (const Babl *conversion);

/**
 * babl_conversion_get_destination_space:
 *
 * Returns the RGB space defined for the destination of conversion.
 */
const Babl *babl_conversion_get_destination_space (const Babl *conversion);

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
