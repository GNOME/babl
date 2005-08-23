#ifndef _BABL_INSTANCE_H
#define _BABL_INSTANCE_H

#include "babl-classes.h"
typedef int  (*BablEachFunction) (Babl *entry,
                                  void *data);


/* these defines are kept here to keep the typing needed in class
 * headers to a minimum, only the ones overriding the basic api with
 * custom ways of construction.
 */
#define BABL_DEFINE_CLASS(type_name)                             \
                                                                 \
void   type_name##_init    (void);                        \
void   type_name##_destroy (void);                        \
void   type_name##_each    (BablEachFunction  each_fun,   \
                            void             *user_data); \
Babl * type_name           (const char       *name);      \
Babl * type_name##_id      (int               id);        \
Babl * type_name##_new     (const char       *name,       \
                            ...);
#define BABL_DEFINE_CLASS_NO_NEW_NO_ID(type_name)                \
                                                                 \
void   type_name##_init    (void);                        \
void   type_name##_destroy (void);                        \
void   type_name##_each    (BablEachFunction  each_fun,   \
                            void             *user_data);

#endif
