#include <stdio.h>
#include <stdlib.h>
#include "../config.h"
#include "babl/babl-internal.h"

static int
file_get_contents (const char  *path,
                   char       **contents,
                   long        *length,
                   void        *error);


#define ICC_HEADER_LEN 128
#define TAG_COUNT_OFF  ICC_HEADER_LEN

static int load_byte (const char *icc, int length, int offset)
{
/* all reading functions take both the char *pointer and the length of the
 * buffer, and all reads thus gets protected by this condition.
 */
  if (offset < 0 || offset > length)
    return 0;

  return *(uint8_t*) (&icc[offset]);
}

static int load_sbyte (const char *icc, int length, int offset)
{
  if (offset < 0 || offset > length)
    return 0;

  return *(int8_t*) (&icc[offset]);
}

static int16_t load_u1f15 (const char *icc, int length, int offset)
{
  return load_byte (icc, length, offset + 1) +
         (load_sbyte (icc, length, offset + 0) << 8);
}

static uint16_t load_u16 (const char *icc, int length, int offset)
{
  return load_byte (icc, length, offset + 1) +
         (load_byte (icc, length, offset + 0) << 8);
}

static double load_s15f16 (const char *icc, int length, int offset)
{
  return load_u1f15 (icc, length, offset) +
         load_u16 (icc, length, offset + 2) / 65535.0f;
}

static uint32_t load_u32 (const char *icc, int length, int offset)
{
  return load_byte (icc, length, offset + 3) +
         (load_byte (icc, length, offset + 2) << 8) +
         (load_byte (icc, length, offset + 1) << 16) +
         (load_byte (icc, length, offset + 0) << 24);
}

static void load_sign (const char *icc, int length,
                       int offset, char *sign)
{
  sign[0]=load_byte(icc, length, offset);
  sign[1]=load_byte(icc, length, offset + 1);
  sign[2]=load_byte(icc, length, offset + 2);
  sign[3]=load_byte(icc, length, offset + 3);
  sign[4]=0;
}

/* looks up offset and length for a specific icc tag
 */
static int icc_tag (const char *icc, int length,
                    const char *tag, int *offset, int *el_length)
{
  int tag_count = load_u32 (icc, length, TAG_COUNT_OFF);
  int t;

  for (t =  0; t < tag_count; t++)
  {
     char tag_signature[5];
     load_sign (icc, length, TAG_COUNT_OFF + 4 + 12 * t, tag_signature);
     if (!strcmp (tag_signature, tag))
     {
        if (offset)
          *offset = load_u32 (icc, length, TAG_COUNT_OFF + 4 + 12* t + 4);
        if (el_length)
          *el_length = load_u32 (icc, length, TAG_COUNT_OFF + 4 + 12* t + 4*2);
        return 1;
     }
  }
  return 0;
}

#if 0

#define ICC_HEADER_LEN 128
#define TAG_COUNT_OFF  ICC_HEADER_LEN

static int load_byte (const char *icc, int offset)
{
  return *(uint8_t*) (&icc[offset]);
}

static int16_t load_u1Fixed15 (const char *icc, int offset)
{
  return load_byte (icc, offset + 1) +
         (load_byte (icc, offset + 0) << 8);
}

static uint16_t load_u16 (const char *icc, int offset)
{
  return load_byte (icc, offset + 1) +
         (load_byte (icc, offset + 0) << 8);
}

static double load_s15f16 (const char *icc, int offset)
{
  return load_u1Fixed15 (icc, offset) + load_u16 (icc, offset + 2) / 65535.0f;
}

static double load_u16f16 (const char *icc, int offset)
{
  return load_u16 (icc, offset) + load_u16 (icc, offset + 2) / 65535.0;
}

static uint32_t load_u32 (const char *icc, int offset)
{
  return load_byte (icc, offset + 3) +
         (load_byte (icc, offset + 2) << 8) +
         (load_byte (icc, offset + 1) << 16) +
         (load_byte (icc, offset + 0) << 24);
}

static float load_float32 (const char *icc, int offset)
{
  char buf[4]={load_byte (icc, offset + 3),
         load_byte (icc, offset + 2),
         load_byte (icc, offset + 1),
         load_byte (icc, offset + 0)};
  float *val = (float*)(&buf[0]);
  return *val;
}

static uint64_t load_uint64 (const char *icc, int offset)
{
  return ((uint64_t)load_byte (icc, offset + 7) << (8*0)) +
         ((uint64_t)load_byte (icc, offset + 6) << (8*1)) +
         ((uint64_t)load_byte (icc, offset + 5) << (8*2)) +
         ((uint64_t)load_byte (icc, offset + 4) << (8*3)) +
         ((uint64_t)load_byte (icc, offset + 3) << (8*4)) +
         ((uint64_t)load_byte (icc, offset + 2) << (8*5)) +
         ((uint64_t)load_byte (icc, offset + 1) << (8*6)) +
         ((uint64_t)load_byte (icc, offset + 0) << (8*7));
}

static void load_sign (const char *icc, int offset, char *sign)
{
  sign[0]=load_byte(icc, offset);
  sign[1]=load_byte(icc, offset + 1);
  sign[2]=load_byte(icc, offset + 2);
  sign[3]=load_byte(icc, offset + 3);
  sign[4]=0;
}

static int icc_tag (const char *icc, const char *tag, int *offset, int *length)
{
  int tag_count = load_u32 (icc, TAG_COUNT_OFF);
  int profile_size = load_u32 (icc, 0);
  int t;

  for (t =  0; t < tag_count; t++)
  {
     char tag_signature[5];
     load_sign (icc, TAG_COUNT_OFF + 4 + 12 * t, tag_signature);
     if (!strcmp (tag_signature, tag))
     {
        *offset = load_u32 (icc, TAG_COUNT_OFF + 4 + 12* t + 4);
        *length = load_u32 (icc, TAG_COUNT_OFF + 4 + 12* t + 4 * 2);
        /* avert potential for maliciousnes.. */
        if (*offset >= profile_size)
          {
            *offset = profile_size - 1;
          }
        if (*offset + *length >= profile_size)
          {
            *length = profile_size - *offset - 1;
          }
        return 1;
     }
  }
  return 0;
}
#endif

static int load_icc_from_memory (const char *icc, long length, char **error)
{
  int  tag_count         = load_u32 (icc, length, TAG_COUNT_OFF);
  int  profile_size      = load_u32 (icc, length, 0);
  int  profile_version_major = load_byte (icc, length, 8);
  int  profile_version_minor = load_byte (icc, length, 9) >> 4;
  int  profile_version_micro = load_byte (icc, length, 9) & 0xf;
  char profile_class[5];
  char color_space[5];
  char pcs_space[5];
  int rendering_intent = load_u32 (icc, length, 64);
  int t;
  // 64..67 rendering intent
  // 68..79 XYZ of D50

  load_sign (icc, length, 16, color_space);
  load_sign (icc, length, 20, pcs_space);
  load_sign (icc, length, 12, profile_class);

  if (strcmp (profile_class, "mntr"))
  {
    *error = "not a monitor-class profile";
    return -1;
  }
  if (strcmp (color_space, "RGB "))
  {
    *error = "not defining an RGB space";
    return -1;
  }
#if 0
  if (profile_version_major > 2)
  {
    *error = "only ICC v2 profiles supported";
    return -1;
  }
#endif
  {
     int offset, element_size;
     icc_tag (icc, length, "desc", &offset, &element_size);
     fprintf (stdout, "desc: %s\n", icc + offset + 12);
  }
  {
     int offset, element_size;
     icc_tag (icc, length, "cprt", &offset, &element_size);
     fprintf (stdout, "copyright: %s\n", icc + offset + 8);
  }

#if 1
  fprintf (stdout, "icc version: %i.%i.%i\n", profile_version_major, profile_version_minor, profile_version_micro);
  fprintf (stdout, "profile-size: %i\n", profile_size);
  fprintf (stdout, "profile-class: %s\n", profile_class);
  fprintf (stdout, "color-space: %s\n", color_space);
  fprintf (stdout, "rendering-intent: %i\n", rendering_intent);
  fprintf (stdout, "pcs-space: %s\n", pcs_space);
  fprintf (stdout, "byte length: %li\n", length);
  fprintf (stdout, "tag-count: %i\n", tag_count);

  for (t =  0; t < tag_count; t++)
  {
     char tag_signature[5];
     int offset, element_size;
     load_sign (icc, length, TAG_COUNT_OFF + 4 + 12 * t, tag_signature);
     icc_tag (icc, length, tag_signature, &offset, &element_size);
     fprintf (stdout, "tag %i %s %i %i\n", t, tag_signature, offset, element_size);
  }
#endif
  fprintf (stdout, "tags: ");
  for (t =  0; t < tag_count; t++)
  {
     char tag_signature[5];
     int offset, element_size;
     load_sign (icc, length, TAG_COUNT_OFF + 4 + 12 * t, tag_signature);
     icc_tag (icc, length, tag_signature, &offset, &element_size);
     fprintf (stdout, "%s ", tag_signature);
  }
  fprintf (stdout, "\n");

  {
     int offset, element_size;
     if (icc_tag (icc, length, "chrm", &offset, &element_size))
     {
     int channels = load_u16 (icc, length, offset + 8);
     int phosporant = load_u16 (icc, length, offset + 10);
     double redX   = load_s15f16 (icc, length, offset + 12);
     double redY   = load_s15f16 (icc, length, offset + 12 + 4);
     double greenX = load_s15f16 (icc, length, offset + 20);
     double greenY = load_s15f16 (icc, length, offset + 20 + 4);
     double blueX  = load_s15f16 (icc, length, offset + 28);
     double blueY  = load_s15f16 (icc, length, offset + 28 + 4);
     fprintf (stdout, "chromaticity:\n");
     fprintf (stdout, "  channels: %i\n", channels);
     fprintf (stdout, "  phosphorant: %i\n", phosporant);
     fprintf (stdout, "  CIE xy red: %f %f\n", redX, redY);
     fprintf (stdout, "  CIE xy green: %f %f\n", greenX, greenY);
     fprintf (stdout, "  CIE xy blue: %f %f\n", blueX, blueY);
     }
  }

  {
     int offset, element_size;
     if (icc_tag (icc, length, "wtpt", &offset, &element_size))
     {
       double wX = load_s15f16 (icc, length, offset + 8);
       double wY = load_s15f16 (icc, length, offset + 8 + 4);
       double wZ = load_s15f16 (icc, length, offset + 8 + 4 * 2);
       fprintf (stdout, "whitepoint CIE xyz: %f %f %f\n", wX, wY, wZ);
     }
  }

  {
     int offset, element_size;
     if (icc_tag (icc, length, "rXYZ", &offset, &element_size))
     {
       double wX = load_s15f16 (icc, length, offset + 8);
       double wY = load_s15f16 (icc, length, offset + 8 + 4);
       double wZ = load_s15f16 (icc, length, offset + 8 + 4 * 2);
       fprintf (stdout, "Red        CIE xyz: %f %f %f\n", wX, wY, wZ);
     }
  }
  {
     int offset, element_size;
     if (icc_tag (icc, length, "gXYZ", &offset, &element_size))
     {
       double wX = load_s15f16 (icc, length, offset + 8);
       double wY = load_s15f16 (icc, length, offset + 8 + 4);
       double wZ = load_s15f16 (icc, length, offset + 8 + 4 * 2);
       fprintf (stdout, "Green      CIE xyz: %f %f %f\n", wX, wY, wZ);
     }
  }
  {
     int offset, element_size;
     if (icc_tag (icc, length, "bXYZ", &offset, &element_size))
     {
       double wX = load_s15f16 (icc, length, offset + 8);
       double wY = load_s15f16 (icc, length, offset + 8 + 4);
       double wZ = load_s15f16 (icc, length, offset + 8 + 4 * 2);
       fprintf (stdout, "Blue       CIE xyz: %f %f %f\n", wX, wY, wZ);
     }
  }

  if (1) {
     int offset, element_size;
     if (icc_tag (icc, length, "rTRC", &offset, &element_size))
     {
       int count = load_u32 (icc, length, offset + 8);
       int i;
       if (!strcmp (icc + offset, "para"))
       {
         int function_type = load_u16 (icc, length, offset + 8);
         float g,a,b,c,d,e,f;
         switch (function_type)
         {
            case 0:
              g = load_s15f16 (icc, length, offset + 12 + 2 * 0);
              fprintf (stdout, "parametric TRC gamma type %f\n", g);
              break;

            case 3:
              g = load_s15f16 (icc, length, offset + 12 + 2 * 0);
              a = load_s15f16 (icc, length, offset + 12 + 2 * 1);
              b = load_s15f16 (icc, length, offset + 12 + 2 * 2);
              c = load_s15f16 (icc, length, offset + 12 + 2 * 3);
              d = load_s15f16 (icc, length, offset + 12 + 2 * 4);
              e = load_s15f16 (icc, length, offset + 12 + 2 * 5);
              f = load_s15f16 (icc, length, offset + 12 + 2 * 5);
              fprintf (stdout, "parametric TRC sRGB type %f %f %f %f %f %f %f\n", g, a, b, c, d, e, f);
              break;
            default:
            fprintf (stdout, "unhandled parametric TRC type %i\n", function_type);
            break;
         }
       }
       else
       {
       fprintf (stdout, "rTRC count: %i  %s\n", count,  icc + offset);
       if (count == 0)
       {
         fprintf (stdout, "linear TRC\n");
       }
       else if (count == 1)
       {
         fprintf (stdout, "gamma TRC of: %f\n", load_byte (icc, length, offset + 12) +
                                                load_byte (icc, length, offset + 12 + 1) / 255.0);
       }
       else for (i = 0; i < count && i < 10; i ++)
       {
         fprintf (stdout, "%i=%i ", i, load_u16 (icc, length, offset + 12 + i * 2));
         if (i % 7 == 0)
            fprintf (stdout, "\n");
       }
       }
     }
  }
  return 0;
}

static int load_icc (const char *path, char **error)
{
  char *icc = NULL;
  long length = 0;
  int ret = 0;
  file_get_contents (path, &icc, &length, NULL);
  if (icc)
  {
    ret = load_icc_from_memory (icc, length, error);
    free (icc);
  }
  return ret;
}

static int
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

  fseek (file, 0, SEEK_END);
  *length = size = ftell (file);
  rewind (file);
  buffer = malloc(size + 8);

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

int main (int argc, char **argv)
{
  char *error = NULL;
  if (argc < 2)
  {
    fprintf (stdout, "need one arg, an ICC profile file\n");
    return -1;
  }

  load_icc (argv[1], &error);
  if (error)
  {
    fprintf (stdout, "icc-parse-problem: %s\n", error);
  }

  return 0;
}
