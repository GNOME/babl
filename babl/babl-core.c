#include <string.h>
#include "util.h"
#include "babl.h"

static void
convert_double_double (void *src,
                       void *dst,
                       int   src_pitch,
                       int   dst_pitch,
                       int   n)
{
  if (src_pitch == 64 &&
      dst_pitch == 64)
    {
      memcpy (dst, src, n/8);
      return;
    }
  while (n--)
    {
      (*(double *) dst) = (*(double *) src);
      dst += dst_pitch;
      src += src_pitch;
    }
}


static void
copy_strip_1 (int    src_bands,
              void **src,
              int   *src_pitch,
              int    dst_bands,
              void **dst,
              int   *dst_pitch,
              int    n)
{
  BABL_PLANAR_SANITY
  while (n--)
    {
      int i;

      for (i=0;i<dst_bands;i++)
        {
          double foo;
          if (i<src_bands)
            foo = *(double *) src[i];
          else
            foo = 1.0;
          *(double*)dst[i] = foo;
        }

      BABL_PLANAR_STEP
    }
}

void
babl_core_init (void)
{
  babl_type_new (
    "double",
    "id",          BABL_DOUBLE,
    "bits",        64,
    NULL);

  babl_conversion_new (
    "babl-base: double to double",
    "source",      babl_type_id (BABL_DOUBLE),
    "destination", babl_type_id (BABL_DOUBLE),
    "linear", convert_double_double,
    NULL
  );

  babl_component_new (
    "R",
    "id", BABL_RED,
    "luma",
    "chroma",
    NULL);

  babl_component_new (
   "G",
    "id",   BABL_GREEN,
   "luma", 
   "chroma",
   NULL);
  
  babl_component_new (
   "B",
    "id",   BABL_BLUE,
   "luma",
   "chroma",
   NULL);

  babl_component_new (
   "A",
   "id",    BABL_ALPHA,
   "alpha",
   NULL);

  babl_model_new (
    "rgba",
    "id", BABL_RGBA,
    babl_component_id (BABL_RED),
    babl_component_id (BABL_GREEN),
    babl_component_id (BABL_BLUE),
    babl_component_id (BABL_ALPHA),
    NULL);

  babl_conversion_new (
    "babl-base: rgba to rgba",
    "source",      babl_model_id (BABL_RGBA),
    "destination", babl_model_id (BABL_RGBA),
    "planar",      copy_strip_1,
    NULL
  );
}
