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

/* NOTE! the babl API is polymorphic, it is probably easier to learn
 * it's usage by studying examples than this header file. The public
 * functions are attempted explained anyways.
 */

#ifndef _BABL_H
#define _BABL_H

#include "babl-classes.h"
#ifdef _BABL_INTERNAL_H
#error babl.h included after babl-internal.h
#endif
#ifndef _BABL_CLASSES_H
#define Babl  void
#endif

/** Initialize the babl library */
void   babl_init       (void);

/** Deinitialize the babl library (frees any resources used, if the number
 *  of calls to babl_destroy() is is equal to the number of calls to
 *  babl_init()
 */
void   babl_destroy    (void);

#if     __GNUC__ >= 4
#define BABL_ARG_NULL_TERMINATED __attribute__((__sentinel__))
#else
#define BABL_ARG_NULL_TERMINATED
#endif

#define BABL_CLASS_NO_NAME(type_name)                            \
                                                                 \
void   babl_##type_name##_init    (void);                        \
void   babl_##type_name##_destroy (void);                        \
Babl * babl_##type_name##_id      (int id);                      \
void   babl_##type_name##_each    (BablEachFunction  each_fun,   \
                                   void             *user_data);

#define BABL_CLASS(type_name)                                    \
                                                                 \
BABL_CLASS_NO_NAME (type_name)                                   \
Babl * babl_##type_name           (const char       *name);      \
Babl * babl_##type_name##_new     (void             *first_arg,  \
                                   ...) BABL_ARG_NULL_TERMINATED;

typedef int  (*BablEachFunction) (Babl *entry,
                                  void *data);


/****************************************************************/
/* BablFish */                           BABL_CLASS_NO_NAME (fish)
/*  Create a babl fish capable of converting from source_format to
 *  destination_format, source and destination can be
 *  either strings with the names of the formats or BablFormat objects.
 */
Babl * babl_fish       (void *source_format,
                        void *destination_format,
                        ...);

/** Process n pixels from source to destination using babl_fish,
 *  returns number of pixels converted. 
 */
long   babl_process    (Babl *babl_fish,
                        void *source,
                        void *destination,
                        long  n);

/****************************************************************/
/* BablImage */                         BABL_CLASS_NO_NAME (image)
/*
 * Babl images can be used for planar buffers instead of linear buffers for
 * babl_process(), BablImages are still experimental, for now BablImages can be
 * passed to babl_process, two different babl_process() functions will be
 * needed for this since the polymorphism cannot be trusted to work on linear
 * buffers.
 * 
 * Babl * babl_image (BablComponent *component1,
 *                    void          *data,
 *                    int            pitch,
 *                    int            stride,
 *                   [BablComponent *component1,
 *                    void          *data,
 *                    int            pitch,
 *                    int            stride,
 *                    ...]
 *                    NULL);
 */
Babl * babl_image      (void *first_component,
                        ...) BABL_ARG_NULL_TERMINATED;



/****************************************************************/
/* BablType */                                   BABL_CLASS (type)
/*
 * A data type that babl can have in it's buffers, requires
 * conversions to and from "double" to be registered before
 * passing sanity.
 * 
 * Babl * babl_type_new (  const char *name,
 *                         "bits",     int bits,
 *                         ["min_val", double min_val,] 
 *                         ["max_val", double max_val,] 
 *                         NULL);
 */  


/****************************************************************/
/* BablComponent */                         BABL_CLASS (component)
/*
 * Babl * babl_component_new (const char *name,
 *                            NULL);
 */  


/****************************************************************/
/* BablModel */                                 BABL_CLASS (model)
/*
 * Babl * babl_model_new (["name", const char *name,]
 *                        BablComponent *component1,
 *                       [BablComponent *componentN, ...]
 *                        NULL);
 *
 * If no name is provided a name is generated by concatenating the
 * name of all the involved components.
 *
 */  

/****************************************************************/
/* BablSampling */                   BABL_CLASS_NO_NAME (sampling)
/**/
Babl * babl_sampling   (int horizontal,
                        int vertical);

/****************************************************************/
/* BablFormat */                               BABL_CLASS (format)
/*
 * Babl * babl_format_new (["name", const char *name,]
 *                          BablModel          *model,
 *                         [BablType           *type,]
 *                         [BablSampling,      *sampling,]
 *                          BablComponent      *component1,
 *                        [[BablType           *type,]
 *                         [BablSampling       *sampling,]
 *                          BablComponent      *componentN,
 *                        ...]
 *                          ["planar",]
 *                          NULL);
 *
 * Provided BablType and|or BablSampling is valid for the following
 * components as well. If no name is provided a (long) descriptive
 * name is used.
 */  


/****************************************************************/
/* BablExtension */                         BABL_CLASS (extension)
/*
 * BablExtension objects are only used internally in babl.
 */

/****************************************************************/
/* BablConversion */                       BABL_CLASS (conversion)
/*
 * Babl * babl_conversion_new (<BablFormat *source, BablFormat *destination|
 *                              BablModel  *source, BablModel  *destination|
 *                              BablType   *source, BablType   *destination>,
 *                             <"linear"|"planar">, BablConversionFunc conv_func,
 *                              NULL);
 */  


const char *babl_name (Babl *babl);  /* returns the name of a babl object */
void   babl_introspect (Babl *babl); /* introspect a given BablObject     */

#undef BABL_CLASS
#include <stdlib.h>

#endif
