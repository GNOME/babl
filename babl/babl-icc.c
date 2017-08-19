#include "config.h"
#include "babl-internal.h"
#include <stdio.h>
#include <stdlib.h>

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

static uint16_t load_uint16 (const char *icc, int offset)
{
  return load_byte (icc, offset + 1) +
         (load_byte (icc, offset + 0) << 8);
}

static double load_s15Fixed16 (const char *icc, int offset)
{
  return load_u1Fixed15 (icc, offset) + load_uint16 (icc, offset + 2) / 65535.0f;
}

static double load_u16Fixed16 (const char *icc, int offset)
{
  return load_uint16 (icc, offset) + load_uint16 (icc, offset + 2) / 65535.0;
}

static uint32_t load_uint32 (const char *icc, int offset)
{
  return load_byte (icc, offset + 3) +
         (load_byte (icc, offset + 2) << 8) +
         (load_byte (icc, offset + 1) << 16) +
         (load_byte (icc, offset + 0) << 24);
}

static void load_sign (const char *icc, int offset, char *sign)
{
  sign[0]=load_byte(icc, offset);
  sign[1]=load_byte(icc, offset + 1);
  sign[2]=load_byte(icc, offset + 2);
  sign[3]=load_byte(icc, offset + 3);
  sign[4]=0;
}

/* looks up offset and length for a specifi icc tag
 */
static int icc_tag (const char *icc, const char *tag, int *offset, int *length)
{
  int tag_count = load_uint32 (icc, TAG_COUNT_OFF);
  int profile_size = load_uint32 (icc, 0);
  int t;

  for (t =  0; t < tag_count; t++)
  {
     char tag_signature[5];
     load_sign (icc, TAG_COUNT_OFF + 4 + 12 * t, tag_signature);
     if (!strcmp (tag_signature, tag))
     {
        if (!offset)
          return 1;
        *offset = load_uint32 (icc, TAG_COUNT_OFF + 4 + 12* t + 4);
        /* avert some potential for maliciousnes.. */
        if (*offset >= profile_size)
          {
            *offset = profile_size - 1;
          }
        if (!length)
          return 1;
        *length = load_uint32 (icc, TAG_COUNT_OFF + 4 + 12* t + 4 * 2);
        /* avert some potential for maliciousnes.. */
        if (*offset + *length >= profile_size)
          {
            *length = profile_size - *offset - 1;
          }
        return 1;
     }
  }
  return 0;
}

static const Babl *babl_trc_from_icc (const char *icc,
                                      int         length,
                                      char      **error)
{
  int offset = 0;
  {
    int count = load_uint32 (icc, offset + 8);
    int i;
    {
      if (count == 0)
      {
        return babl_trc_gamma (1.0);
      }
      else if (count == 1)
      {
        return babl_trc_gamma (load_byte (icc, offset + 12) +
                               load_byte (icc, offset + 12 + 1) / 255.0);
      }
      else
      {
        return babl_trc_gamma (2.2);

        // XXX: todo implement a curve trc babl type
        //      as well as detect sRGB curve from LUTs

        for (i = 0; i < count && i < 10; i ++)
        {
          fprintf (stdout, "%i=%i ", i, load_uint16 (icc, offset + 12 + i * 2));
          if (i % 7 == 0)
           fprintf (stdout, "\n");
        }
      }
    }
  }
  return NULL;
}

const Babl *
babl_space_rgb_icc (const char *icc,
                    int         length,
                    char      **error)
{
  int  profile_size          = load_uint32 (icc, 0);
  int  icc_ver_major         = load_byte (icc, 8);
  const Babl *trc_red = NULL;
  const Babl *trc_green = NULL;
  const Babl *trc_blue = NULL;
  char profile_class[5];
  char color_space[5];

  if (profile_size != length)
  {
    *error = "icc profile length inconsistency";
    return NULL;
  }
  if (icc_ver_major > 2)
  {
    *error = "only ICC v2 profiles supported";
    return NULL;
  }
  load_sign (icc, 12, profile_class);
  if (strcmp (profile_class, "mntr"))
  {
    *error = "not a monitor-class profile";
    return NULL;
  }
  load_sign (icc, 16, color_space);
  if (strcmp (color_space, "RGB "))
  {
    *error = "not defining an RGB space";
    return NULL;
  }
  {
     int offset, element_size;
     if (icc_tag (icc, "rTRC", &offset, &element_size))
     {
       trc_red = babl_trc_from_icc (icc + offset, element_size, error);
       if (*error)
         return NULL;
     }
     if (icc_tag (icc, "gTRC", &offset, &element_size))
     {
       trc_green = babl_trc_from_icc (icc + offset, element_size, error);
       if (*error)
         return NULL;
     }
     if (icc_tag (icc, "bTRC", &offset, &element_size))
     {
       trc_blue = babl_trc_from_icc (icc + offset, element_size, error);
       if (*error)
         return NULL;
     }
  }

  if (!trc_red || !trc_green || !trc_blue)
  {
     *error = "missing TRC";
     return NULL;
  }

  if (icc_tag (icc, "chrm", NULL, NULL) &&
      icc_tag (icc, "wtpt", NULL, NULL))
  {
     int offset, element_size;
     double redX, redY, greenX, greenY, blueX, blueY;

     icc_tag (icc, "chrm", &offset, &element_size);
     {
     int channels   = load_uint16 (icc, offset + 8);
     int phosporant = load_uint16 (icc, offset + 10);

     redX    = load_s15Fixed16 (icc, offset + 12);
     redY    = load_s15Fixed16 (icc, offset + 12 + 4);
     greenX  = load_s15Fixed16 (icc, offset + 20);
     greenY  = load_s15Fixed16 (icc, offset + 20 + 4);
     blueX   = load_s15Fixed16 (icc, offset + 28);
     blueY   = load_s15Fixed16 (icc, offset + 28 + 4);

     fprintf (stdout, "chromaticity:\n");
     fprintf (stdout, "  channels: %i\n", channels);
     fprintf (stdout, "  phosphorant: %i\n", phosporant);
     fprintf (stdout, "  CIE xy red: %f %f\n", redX, redY);
     fprintf (stdout, "  CIE xy green: %f %f\n", greenX, greenY);
     fprintf (stdout, "  CIE xy blue: %f %f\n", blueX, blueY);
     }

     icc_tag (icc, "wtpt", &offset, &element_size);
     {
       double wX = load_u16Fixed16 (icc, offset + 8);
       double wY = load_u16Fixed16 (icc, offset + 8 + 4);
       double wZ = load_u16Fixed16 (icc, offset + 8 + 4 * 2);

       return babl_space_rgb_chromaticities (NULL,
                       wX / (wX + wY + wZ),
                       wY / (wX + wY + wZ),
                       redX, redY,
                       greenX, greenY,
                       blueX, blueY,
                       trc_red, trc_green, trc_blue);

     }
  }
  else if (icc_tag (icc, "rXYZ", NULL, NULL) &&
           icc_tag (icc, "gXYZ", NULL, NULL) &&
           icc_tag (icc, "bXYZ", NULL, NULL))
  {
     int offset, element_size;
     double rx, gx, bx;
     double ry, gy, by;
     double rz, gz, bz;

     icc_tag (icc, "rXYZ", &offset, &element_size);
     rx = load_u16Fixed16 (icc, offset + 8 + 4 * 0);
     ry = load_u16Fixed16 (icc, offset + 8 + 4 * 1);
     rz = load_u16Fixed16 (icc, offset + 8 + 4 * 2);
     icc_tag (icc, "gXYZ", &offset, &element_size);
     gx = load_u16Fixed16 (icc, offset + 8 + 4 * 0);
     gy = load_u16Fixed16 (icc, offset + 8 + 4 * 1);
     gz = load_u16Fixed16 (icc, offset + 8 + 4 * 2);
     icc_tag (icc, "bXYZ", &offset, &element_size);
     bx = load_u16Fixed16 (icc, offset + 8 + 4 * 0);
     by = load_u16Fixed16 (icc, offset + 8 + 4 * 1);
     bz = load_u16Fixed16 (icc, offset + 8 + 4 * 2);

     return babl_space_rgb_matrix (NULL,
                rx, gx, bx,
                ry, gy, by,
                rz, gz, bz,
                trc_red, trc_green, trc_blue);
  }

  *error = "didnt find RGB primaries";
  return NULL;
}
