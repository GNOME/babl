/* babl - dynamically extendable universal pixel fish library.
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
 * <https://www.gnu.org/licenses/>.
 */

#include "config.h"
#include <math.h>
#include "babl-internal.h"
#include "babl-ref-pixels.h"

#define BABL_TOLERANCE             0.0000047
#define BABL_MAX_COST_VALUE        2000000
#define BABL_HARD_MAX_PATH_LENGTH  8
#define BABL_MAX_NAME_LEN          1024

#define BABL_TEST_ITER             16

#ifndef MIN
#define MIN(a, b) (((a) > (b)) ? (b) : (a))
#endif

static int enable_lut = 0;

typedef struct GcContext {
   long time;
} GcContext;

static float lut_unused_minutes_limit = 5.0;

static int lut_info_level = 0;

#define _LUT_LOG(level, ...) do{\
     if (level <= lut_info_level)\
       fprintf (stdout, __VA_ARGS__);\
     fflush(NULL);\
     }while(0)
#define LUT_LOG(...) _LUT_LOG(1, __VA_ARGS__)
#define LUT_INFO(...) _LUT_LOG(2, __VA_ARGS__)
#define LUT_DETAIL(...) _LUT_LOG(3, __VA_ARGS__)

static int gc_fishes (Babl *babl, void *userdata)
{
  GcContext *context = userdata;
  if (babl->class_type == BABL_FISH_PATH)
  {
    if (babl->fish_path.u8_lut)
    {
      if (context->time - babl->fish_path.last_lut_use >
          1000 * 1000 * 60 * lut_unused_minutes_limit)
      {
        void *lut =babl->fish_path.u8_lut;
        BABL(babl)->fish_path.u8_lut = NULL;
        free (lut);
        BABL(babl)->fish.pixels = 0;
        LUT_LOG("freeing LUT %s to %s unused for >%.1f minutes\n",
                babl_get_name (babl->conversion.source),
                babl_get_name (babl->conversion.destination),
                lut_unused_minutes_limit);
      }
      else if (lut_info_level >=4)
      {
        LUT_DETAIL("active LUT %s to %s  %8li pixels last used %.1f minutes ago\n",
                babl_get_name (babl->conversion.source),
                babl_get_name (babl->conversion.destination),
                babl->fish.pixels,
         (context->time - babl->fish_path.last_lut_use)/1000.0/1000.0/60.0);
      }
    }
    else if (lut_info_level >= 4 && babl->fish.pixels)
    {
        if (babl->fish_path.is_u8_color_conv)
        LUT_DETAIL("potential LUT %s to %s  %8li pixels\n",
                babl_get_name (babl->conversion.source),
                babl_get_name (babl->conversion.destination),
                babl->fish.pixels);
        else if (lut_info_level >=5)
        LUT_DETAIL("%i step path %s to %s  %8li pixels\n",
                babl->fish_path.conversion_list->count,
                babl_get_name (babl->conversion.source),
                babl_get_name (babl->conversion.destination),
                babl->fish.pixels);
    }
    babl->fish.pixels /= 2; // decay pixel count// this is enough that we *will* reach 0
  }
  return 0;
}
                           
static void
babl_gc_fishes (void)
{
  GcContext context;
  context.time = babl_ticks ();
  if (lut_info_level >= 5)
  {
     fprintf (stdout, "\e[H\e[2J");
  }
  babl_fish_class_for_each (gc_fishes, &context);
}

static long babl_conv_counter = 0;

void
babl_gc (void)
{
  if (babl_conv_counter > 1000 * 1000 * 10) // run gc every 10 megapixels
  {
    babl_conv_counter = 0;
    babl_gc_fishes ();
    //malloc_trim (0); 
    //  is responsibility of higher layers
  }
}

#define BABL_LIKELY(x)      __builtin_expect(!!(x), 1)
#define BABL_UNLIKELY(x)    __builtin_expect(!!(x), 0)

static float timings[256] = {0,};

#define BPP_4ASSOCIATED   14

static inline int _do_lut (uint32_t *lut,
                           int   source_bpp,
                           int   dest_bpp,
                           const void *__restrict__ source,
                           void *__restrict__ destination,
                           long n)
{
        if (source_bpp == BPP_4ASSOCIATED  && dest_bpp == 4)
        {
          uint32_t *src = (uint32_t*)source;
          uint32_t *dst = (uint32_t*)destination;
          while (n--)
          {
             uint32_t col = *src++;
             uint8_t *rgba=(uint8_t*)&col;
             uint8_t oalpha = rgba[3];
             if (oalpha==0)
             {
               *dst++ = 0;
             }
             else
             {
               uint32_t col_opaque = col;
               uint8_t *rgbaB=(uint8_t*)&col_opaque;
               uint32_t ralpha = 0;
               ralpha = (256*255)/oalpha;
               rgbaB[0] = (rgba[0]*ralpha)>>8;
               rgbaB[1] = (rgba[1]*ralpha)>>8;
               rgbaB[2] = (rgba[2]*ralpha)>>8;
               rgbaB[3] = 0;
               *dst++ = lut[col_opaque] | (oalpha<<24);
             }
          }
        }
        else if (source_bpp == 4 && dest_bpp == 16)
        {
          uint32_t *src = (uint32_t*)source;
          uint32_t *dst = (uint32_t*)destination;
          while (n--)
          {
             uint32_t col = *src++;
             uint32_t lut_offset = col & 0xffffff;
             float alpha = (col>>24)/255.0f;

             *dst++ = lut[lut_offset*4+0];
             *dst++ = lut[lut_offset*4+1];
             *dst++ = lut[lut_offset*4+2];
             ((float*)(dst))[0] = alpha;
             dst++;
          }
        }
        else if (source_bpp == 4 && dest_bpp == 8)
        {
          uint32_t *src = (uint32_t*)source;
          uint16_t *dst = (uint16_t*)destination;
          uint16_t *lut16 = (uint16_t*)lut;
          while (n--)
          {
             uint32_t col = *src++;
             uint32_t lut_offset = col & 0xffffff;
             uint16_t alpha = (col>>24) << 8; 

             dst[0] = lut16[lut_offset*2+0];
             dst[1] = lut16[lut_offset*2+1];
             dst[2] = lut16[lut_offset*2+2];
             dst[3] = alpha;
             dst+=4;
          }
        }
        else if (source_bpp == 2 && dest_bpp == 16)
        {
          uint16_t *src = (uint16_t*)source;
          uint32_t *dst = (uint32_t*)destination;
          while (n--)
          {
             uint32_t col = *src++;
             *dst++ = lut[col*4+0];
             *dst++ = lut[col*4+1];
             *dst++ = lut[col*4+2];
             *dst++ = lut[col*4+3];
          }
        }
        else if (source_bpp == 4 && dest_bpp == 4)
        {
          uint32_t *src = (uint32_t*)source;
          uint32_t *dst = (uint32_t*)destination;
          while (n--)
          {
             uint32_t col = *src++;
             *dst = (col & 0xff000000) | lut[col & 0xffffff];
             dst++;
          }
        }
        else if (source_bpp == 2 && dest_bpp == 4)
        {
          uint16_t *src = (uint16_t*)source;
          uint32_t *dst = (uint32_t*)destination;
          while (n--)
          {
            *dst = lut[*src++];
            dst++;
          }
        }
        else if (source_bpp == 2 && dest_bpp == 2)
        {
          uint16_t *src = (uint16_t*)source;
          uint16_t *dst = (uint16_t*)destination;
          uint16_t *lut16 = (uint16_t*)lut;
          while (n--)
          {
             *dst = lut16[*src++];
             dst++;
          }
        }
        else if (source_bpp == 1 && dest_bpp == 4)
        {
          uint8_t *src = (uint8_t*)source;
          uint32_t *dst = (uint32_t*)destination;
          while (n--)
          {
             *dst = lut[*src++];
             dst++;
          }
        }
        else if (source_bpp == 3 && dest_bpp == 3)
        {
          uint8_t *src = (uint8_t*)source;
          uint8_t *dst = (uint8_t*)destination;
          while (n--)
          {
             uint32_t col = src[0]*256*256+src[1]*256+src[2];
             uint32_t val = lut[col];
             dst[2]=(val >> 16) & 0xff;
             dst[1]=(val >> 8) & 0xff;
             dst[0]=val & 0xff;
             dst+=3;
             src+=3;
          }
        }
        else if (source_bpp == 3 && dest_bpp == 4)
        {
          uint8_t *src = (uint8_t*)source;
          uint32_t *dst = (uint32_t*)destination;
          while (n--)
          {
             *dst = lut[src[0]*256*256+src[1]*256+src[2]];
             dst++;
             src+=3;
          }
        }
        else
        {
          return 0;
        }
        return 1;
}

void babl_test_lut (uint32_t *lut,
             int   source_bpp,
             int   dest_bpp,
             void *__restrict__ source,
             void *__restrict__ dest,
             long count);
void babl_test_lut (uint32_t *lut,
             int   source_bpp,
             int   dest_bpp,
             void *__restrict__ source,
             void *__restrict__ dest,
             long count)
{
   _do_lut (lut, source_bpp, dest_bpp, source, dest, count);
}

static inline float lut_timing_for (int source_bpp, int dest_bpp)
{
  return timings[source_bpp * 16 + dest_bpp];
}

static void measure_timings(void)
{
   int num_pixels = babl_get_num_path_test_pixels () * 1000;
   int pairs[][2]={{4,4},{BPP_4ASSOCIATED,4},{4,8},{3,4},{3,3},{2,4},{2,2},{1,4},{2,16},{4,16}};
   uint32_t *lut = malloc (256 * 256 * 256 * 16);
   uint32_t *src = malloc (num_pixels * 16);
   uint32_t *dst = malloc (num_pixels * 16);

   memset (lut, 11, 256 * 256 * 256 *16);
   memset (src, 12, num_pixels * 16);

   if (getenv ("BABL_LUT_INFO"))
   {
      lut_info_level = atoi (getenv ("BABL_LUT_INFO"));
   }
   if (getenv ("BABL_LUT_UNUSED_LIMIT"))
   {
      lut_unused_minutes_limit = atof (getenv ("BABL_LUT_UNUSED_LIMIT"));
   }

   LUT_LOG("BABL_LUT_UNUSED_LIMIT=%.1f\n", lut_unused_minutes_limit);

   LUT_LOG("measuring lut timings          \n");
   for (size_t p = 0; p < sizeof (pairs)/sizeof(pairs[0]);p++)
   {
     int source_bpp = pairs[p][0];
     int dest_bpp = pairs[p][1];
     long start,end;
     start = babl_ticks ();
     babl_test_lut (lut, source_bpp, dest_bpp, src, dst, num_pixels);

     end = babl_ticks ();
     timings[source_bpp * 16 + dest_bpp] = (end-start)/1000.0;
       LUT_LOG ("   %ibpp to %ibpp: %.2f\n", source_bpp, dest_bpp,
          timings[source_bpp * 16 + dest_bpp]
                     );
   }
   free (lut);
   free (src);
   free (dst);
}

static inline void
process_conversion_path (BablList   *path,
                         const void *source_buffer,
                         int         source_bpp,
                         void       *destination_buffer,
                         int         dest_bpp,
                         long        n);

static inline int babl_fish_lut_process_maybe (const Babl *babl,
                                               const char *source,
                                               char *destination,
                                               long        n,
                                               void       *data)
{
     int source_bpp = babl->fish_path.source_bpp;
     int dest_bpp = babl->fish_path.dest_bpp;
     uint32_t *lut = (uint32_t*)babl->fish_path.u8_lut;
 

     if (BABL_UNLIKELY(!lut && babl->fish.pixels >= 128 * 256))
     {
       LUT_LOG("generating LUT for %s to %s\n",
               babl_get_name (babl->conversion.source),
               babl_get_name (babl->conversion.destination));
       if (source_bpp ==4 && dest_bpp == 4)
       {
         lut = malloc (256 * 256 * 256 * 4);
         for (int o = 0; o < 256 * 256 * 256; o++)
           lut[o] = o | 0xff000000;
         process_conversion_path (babl->fish_path.conversion_list,
                                  lut, 4,
                                  lut, 4,
                                  256*256*256);
         for (int o = 0; o < 256 * 256 * 256; o++)
           lut[o] = lut[o] & 0x00ffffff;

       }
       else if (source_bpp == 4 && dest_bpp == 16)
       {
         uint32_t *temp_lut = malloc (256 * 256 * 256 * 4);
         lut = malloc (256 * 256 * 256 * 16);
         for (int o = 0; o < 256 * 256 * 256; o++)
           temp_lut[o] = o | 0xff000000;
         process_conversion_path (babl->fish_path.conversion_list,
                                  temp_lut, 4,
                                  lut, 16,
                                  256*256*256);
         free (temp_lut);
       }
       else if (source_bpp == 4 && dest_bpp == 8)
       {
         uint32_t *temp_lut = malloc (256 * 256 * 256 * 4);
         lut = malloc (256 * 256 * 256 * 8);
         for (int o = 0; o < 256 * 256 * 256; o++)
           temp_lut[o] = o | 0xff000000;
         process_conversion_path (babl->fish_path.conversion_list,
                                  temp_lut, 4,
                                  lut, 8,
                                  256*256*256);
         free (temp_lut);
       }
       else if (source_bpp == 3 && dest_bpp == 3)
       {
         uint8_t *temp_lut = malloc (256 * 256 * 256 * 3);
         uint8_t *temp_lut2 = malloc (256 * 256 * 256 * 3);
         int o = 0;
         lut = malloc (256 * 256 * 256 * 4);
         for (int r = 0; r < 256; r++)
         for (int g = 0; g < 256; g++)
         for (int b = 0; b < 256; b++, o++)
         {
           temp_lut[o*3+0]=r;
           temp_lut[o*3+1]=g;
           temp_lut[o*3+2]=b;
         }
         process_conversion_path (babl->fish_path.conversion_list,
                                  temp_lut, 3,
                                  temp_lut2, 3,
                                  256*256*256);
         babl_process (babl_fish (babl_format ("R'G'B' u8"), babl_format ("R'G'B'A u8")),
                       temp_lut2, lut, 256*256*256);
         for (int o = 0; o < 256 * 256 * 256; o++)
           lut[o] = lut[o] & 0x00ffffff;
         free (temp_lut);
         free (temp_lut2);
       }
       else if (source_bpp == 3 && dest_bpp == 4)
       {
         uint8_t *temp_lut = malloc (256 * 256 * 256 * 3);
         int o = 0;
         lut = malloc (256 * 256 * 256 * 4);
         for (int r = 0; r < 256; r++)
         for (int g = 0; g < 256; g++)
         for (int b = 0; b < 256; b++, o++)
         {
           temp_lut[o*3+0]=r;
           temp_lut[o*3+1]=g;
           temp_lut[o*3+2]=b;
         }
         process_conversion_path (babl->fish_path.conversion_list,
                                  temp_lut, 3,
                                  lut, 4,
                                  256*256*256);
         free (temp_lut);
       }
       else if (source_bpp == 2 && dest_bpp == 2)
       {
         uint16_t *temp_lut = malloc (256 * 256 * 2);
         lut = malloc (256 * 256 * 4);
         for (int o = 0; o < 256*256; o++)
         {
           temp_lut[o]=o;
         }
         process_conversion_path (babl->fish_path.conversion_list,
                                  temp_lut, 2,
                                  lut, 2,
                                  256*256);
         free (temp_lut);
       }
       else if (source_bpp == 2 && dest_bpp == 4)
       {
         uint16_t *temp_lut = malloc (256 * 256 * 2);
         lut = malloc (256 * 256 * 4);
         for (int o = 0; o < 256*256; o++)
         {
           temp_lut[o]=o;
         }
         process_conversion_path (babl->fish_path.conversion_list,
                                  temp_lut, 2,
                                  lut, 4,
                                  256*256);
         free (temp_lut);
       }
       else if (source_bpp == 2 && dest_bpp == 16)
       {
         uint16_t *temp_lut = malloc (256 * 256 * 2);
         lut = malloc (256 * 256 * 16);
         for (int o = 0; o < 256*256; o++)
         {
           temp_lut[o]=o;
         }
         process_conversion_path (babl->fish_path.conversion_list,
                                  temp_lut, 2,
                                  lut, 16,
                                  256*256);
         free (temp_lut);
       }
       else if (source_bpp == 1 && dest_bpp == 4)
       {
         uint8_t *temp_lut = malloc (256);
         lut = malloc (256 * 4);
         for (int o = 0; o < 256; o++)
         {
           temp_lut[o]=o;
         }
         process_conversion_path (babl->fish_path.conversion_list,
                                  temp_lut, 1,
                                  lut, 4,
                                  256);
         free (temp_lut);
       }

       if (babl->fish_path.u8_lut == NULL)
       {
         (BABL(babl)->fish_path.u8_lut) = lut;
         // XXX need memory barrier?
         if ((BABL(babl)->fish_path.u8_lut) != lut)
         {
           free (lut);
           lut = babl->fish_path.u8_lut;
         }
       }
       else
       {
         free (lut);
         lut = babl->fish_path.u8_lut;
       }
     }

     if (lut)
     {
       if (source_bpp == 4 && 
           ((babl->conversion.source->format.model->flags &
           BABL_MODEL_FLAG_ASSOCIATED)!=0))
         source_bpp = BPP_4ASSOCIATED;

       if (_do_lut (lut, source_bpp, dest_bpp, source, destination, n))
       {
         BABL(babl)->fish_path.last_lut_use = babl_ticks ();
         return 1;
       }
     }
     return 0;
}



#define MAX_BUFFER_SIZE            512

int   babl_in_fish_path = 0;

typedef struct _FishPathInstrumentation
{
  const Babl   *fmt_rgba_double;
  int     num_test_pixels;
  void   *source;
  void   *destination;
  void   *ref_destination;
  double *destination_rgba_double;
  double *ref_destination_rgba_double;
  const Babl   *fish_rgba_to_source;
  const Babl   *fish_reference;
  const Babl   *fish_destination_to_rgba;
  double  reference_cost;
  int     init_instrumentation_done;
} FishPathInstrumentation;

typedef struct PathContext {
  Babl     *fish_path;
  Babl     *to_format;
  BablList *current_path;
} PathContext;

static void
init_path_instrumentation (FishPathInstrumentation *fpi,
                           Babl                    *fmt_source,
                           Babl                    *fmt_destination);

static void
destroy_path_instrumentation (FishPathInstrumentation *fpi);

static void
get_path_instrumentation (FishPathInstrumentation *fpi,
                          BablList                *path,
                          double                  *path_cost,
                          double                  *ref_cost,
                          double                  *path_error);


static inline void
process_conversion_path (BablList   *path,
                         const void *source_buffer,
                         int         source_bpp,
                         void       *destination_buffer,
                         int         dest_bpp,
                         long        n);

static void
get_conversion_path (PathContext *pc,
                     Babl        *current_format,
                     int          current_length,
                     int          max_length,
                     double       legal_error);

char *
_babl_fish_create_name (char       *buf,
                        const Babl *source,
                        const Babl *destination,
                        int         is_reference);


static int max_path_length (void);

static int debug_conversions = 0;

double
_babl_legal_error (void)
{
  static double error = 0.0;
  const char   *env;

  if (error != 0.0)
    return error;

  env = getenv ("BABL_TOLERANCE");
  if (env && env[0] != '\0')
    error = babl_parse_double (env);
  else
    error = BABL_TOLERANCE;

  env = getenv ("BABL_DEBUG_CONVERSIONS");
  if (env && env[0] != '\0')
    debug_conversions = 1;
  else
    debug_conversions = 0;

  env = getenv ("BABL_LUT");
  if (env && env[0] != '\0')
    enable_lut = atoi(getenv("BABL_LUT"));
  else
    enable_lut = 1;

  { 
    const uint32_t u32 = 1;
    if ( *((char*)&u32) == 0)
    {  /* disable use of LUTs if we are running on big endian */
       enable_lut = 0;
    }
  }

  return error;
}

static int
max_path_length (void)
{
  static int  max_length = 0;
  const char *env;

  if (max_length != 0)
    return max_length;

  env = getenv ("BABL_PATH_LENGTH");
  if (env)
    max_length = atoi (env);
  else
    max_length = 3; /* reducing this number makes finding short fishes much
                       faster - even if we lose out on some of the fast
                       bigger fish, the fishes we can get with a max_length of 3
                       is actually 5, since we deepen the search to that
                       depth if none are found within two steps in the
                       initial search.
                     */
  if (max_length > BABL_HARD_MAX_PATH_LENGTH)
    max_length = BABL_HARD_MAX_PATH_LENGTH;
  else if (max_length <= 0)
    max_length = 1;
  return max_length;
}

int
_babl_max_path_len (void)
{
  return max_path_length ();
}

static int
bad_idea (const Babl *from, const Babl *to, const Babl *format)
{
  if (babl_format_has_alpha (from) &&
      babl_format_has_alpha (to) &&
      !babl_format_has_alpha (format))
  {
    return 1;
  }
  if (from->format.components > format->format.components &&
      to->format.components > format->format.components)
  {
    return 1;
  }
  if (from->format.type[0]->bits > format->format.type[0]->bits &&
      to->format.type[0]->bits > format->format.type[0]->bits)
  {
    /* XXX: perhaps we especially avoid going to half-float, when
     * going between u16 formats as well? */
    return 1;
  }

  return 0;
}


/* The task of BablFishPath construction is to compute
 * the shortest path in a graph where formats are the vertices
 * and conversions are the edges. However, there is an additional
 * constraint to the shortest path, that limits conversion error
 * introduced by such a path to be less than BABL_TOLERANCE. This
 * prohibits usage of any reasonable shortest path construction
 * algorithm such as Dijkstra's algorithm. The shortest path is
 * constructed by enumerating all available paths that are less
 * than BABL_PATH_LENGTH long, computing their costs and
 * conversion errors and backtracking. The backtracking is
 * implemented by recursive function get_conversion_path ().
 */

static void
get_conversion_path (PathContext *pc,
                     Babl        *current_format,
                     int          current_length,
                     int          max_length,
                     double       legal_error)
{
  if (current_length > max_length)
    {
      /* We have reached the maximum recursion
       * depth, let's bail out */
      return;
    }
  else if ((current_length > 0) && (current_format == pc->to_format))
    {
       /* We have found a candidate path, let's
        * see about it's properties */
      double path_cost  = 0.0;
      double ref_cost   = 0.0;
      double path_error = 1.0;
#if 1
      int    i;
      for (i = 0; i < babl_list_size (pc->current_path); i++)
        {
          path_error *= (1.0 + babl_conversion_error ((BablConversion *) pc->current_path->items[i]));
        }

      if (path_error - 1.0 <= legal_error )
                /* check this before the more accurate measurement of error -
                   to bail earlier, this also leads to a stricter
                   discarding of bad fast paths  */
#endif
        {
          FishPathInstrumentation fpi;
          memset (&fpi, 0, sizeof (fpi));

          fpi.source = (Babl*) babl_list_get_first (pc->current_path)->conversion.source;
          fpi.destination = pc->to_format;

          get_path_instrumentation (&fpi, pc->current_path, &path_cost, &ref_cost, &path_error);
          if(debug_conversions && current_length == 1)
            fprintf (stderr, "%s  error:%f cost:%f  \n",
                 babl_get_name (pc->current_path->items[0]), path_error, path_cost);

          if ((path_cost < ref_cost) && /* do not use paths that took longer to compute than reference */
              (path_cost < pc->fish_path->fish_path.cost) && // best thus far
              (path_error <= legal_error )               // within tolerance
              )
            {
              /* We have found the best path so far,
               * let's copy it into our new fish */
              pc->fish_path->fish_path.cost = path_cost;
              pc->fish_path->fish.error  = path_error;
              babl_list_copy (pc->current_path,
                              pc->fish_path->fish_path.conversion_list);
            }

          destroy_path_instrumentation (&fpi);
        }
    }
  else
    {
      /*
       * we have to search deeper...
       */
      BablList *list;
      int i;

      list = current_format->format.from_list;
      if (list)
        {
          /* Mark the current format in conversion path as visited */
          current_format->format.visited = 1;

          /* Iterate through unvisited formats from the current format ...*/
          for (i = 0; i < babl_list_size (list); i++)
            {
              Babl *next_conversion = BABL (list->items[i]);
              Babl *next_format = BABL (next_conversion->conversion.destination);
              if (!next_format->format.visited && !bad_idea (current_format, pc->to_format, next_format))
                {
                  /* next_format is not in the current path, we can pay a visit */
                  babl_list_insert_last (pc->current_path, next_conversion);
                  get_conversion_path (pc, next_format, current_length + 1, max_length, legal_error);
                  babl_list_remove_last (pc->current_path);
                }
            }

          /* Remove the current format from current path */
          current_format->format.visited = 0;
        }
   }
}

char *
_babl_fish_create_name (char       *buf,
                        const Babl *source,
                        const Babl *destination,
                        int         is_reference)
{
  /* fish names are intentionally kept short */
  snprintf (buf, BABL_MAX_NAME_LEN, "%s %p %p %i", "",
            source, destination, is_reference);
  return buf;
}

int
_babl_fish_path_destroy (void *data);

int
_babl_fish_path_destroy (void *data)
{
  Babl *babl=data;
  if (babl->fish_path.u8_lut)
    free (babl->fish_path.u8_lut);
  babl->fish_path.u8_lut = NULL;
  if (babl->fish_path.conversion_list)
    babl_free (babl->fish_path.conversion_list);
  babl->fish_path.conversion_list = NULL;
  return 0;
}

static int
show_item (Babl *babl,
           void *user_data)
{
  BablConversion *conv = (void *)babl;

  if (conv->destination->class_type == BABL_FORMAT)
  {
    fprintf (stderr, "%s : %.12f\n", babl_get_name (babl), babl_conversion_error(conv));
  }

  return 0;
}

static int
alias_conversion (Babl *babl,
                  void *user_data)
{
  const Babl *sRGB = babl_space ("sRGB");
  BablConversion *conv = (void *)babl;
  BablSpace *space = user_data;

  if ((conv->source->class_type == BABL_FORMAT) &&
      (conv->destination->class_type == BABL_FORMAT) &&
      (!babl_format_is_palette (conv->source)) &&
      (!babl_format_is_palette (conv->destination)))
  {
    if ((conv->source->format.space == sRGB) &&
        (conv->destination->format.space == sRGB))
    {
      switch (conv->instance.class_type)
      {
        case BABL_CONVERSION_LINEAR:
         babl_conversion_new (
                babl_format_with_space (
                      (void*)conv->source->instance.name, (void*)space),
                babl_format_with_space (
                      (void*)conv->destination->instance.name, (void*)space),
                "linear", conv->function.linear,
                "data", conv->data,
                NULL);
          break;
        case BABL_CONVERSION_PLANAR:
         babl_conversion_new (
                babl_format_with_space (
                      (void*)conv->source->instance.name, (void*)space),
                babl_format_with_space (
                      (void*)conv->destination->instance.name, (void*)space),
                "planar", conv->function.planar,
                "data", conv->data,
                NULL);
          break;
        case BABL_CONVERSION_PLANE:
          babl_conversion_new (
                babl_format_with_space (
                      (void*)conv->source->instance.name, (void*)space),
                babl_format_with_space (
                      (void*)conv->destination->instance.name, (void*)space),
                "plane", conv->function.plane,
                "data", conv->data,
                NULL);
          break;
        default:
          break;
      }
    }
  }
  else
  if ((conv->source->class_type == BABL_MODEL) &&
      (conv->destination->class_type == BABL_MODEL))
  {
    if ((conv->source->model.space == sRGB) &&
        (conv->destination->model.space == sRGB))
    {
      switch (conv->instance.class_type)
      {
        case BABL_CONVERSION_LINEAR:
          babl_conversion_new (
                babl_remodel_with_space (
                      (void*)conv->source, (void*)space),
                babl_remodel_with_space (
                      (void*)conv->destination, (void*)space),
                "linear", conv->function.linear,
                "data",   conv->data,
                NULL);
          break;
        case BABL_CONVERSION_PLANAR:
          babl_conversion_new (
                babl_remodel_with_space (
                      (void*)conv->source, (void*)space),
                babl_remodel_with_space (
                      (void*)conv->destination, (void*)space),
                "planar", conv->function.planar,
                "data",   conv->data,
                NULL);
          break;
        case BABL_CONVERSION_PLANE:
          babl_conversion_new (
                babl_remodel_with_space (
                      (void*)conv->source, (void*)space),
                babl_remodel_with_space (
                      (void*)conv->destination, (void*)space),
                "plane", conv->function.plane,
                "data",  conv->data,
                NULL);
          break;
        default:
          break;
      }
    }
  }
  else
  if ((conv->source->class_type == BABL_TYPE) &&
      (conv->destination->class_type == BABL_TYPE))
  {
  }
  return 0;
}

void
_babl_fish_prepare_bpp (Babl *babl)
{
   const Babl *babl_source = babl->fish.source;
   const Babl *babl_dest = babl->fish.destination;

   switch (babl_source->instance.class_type)
     {
       case BABL_FORMAT:
         babl->fish_path.source_bpp = babl_source->format.bytes_per_pixel;
         break;
       case BABL_TYPE:
         babl->fish_path.source_bpp = babl_source->type.bits / 8;
         break;
       default:
         babl_log ("=eeek{%i}\n",
			  babl_source->instance.class_type - BABL_MAGIC);
     }

   switch (babl_dest->instance.class_type)
     {
       case BABL_FORMAT:
         babl->fish_path.dest_bpp = babl_dest->format.bytes_per_pixel;
         break;
       case BABL_TYPE:
         babl->fish_path.dest_bpp = babl_dest->type.bits / 8;
         break;
       default:
         babl_log ("-eeek{%i}\n", babl_dest->instance.class_type - BABL_MAGIC);
     }

  if (enable_lut)
  {
  int         source_bpp  = babl->fish_path.source_bpp;
  int         dest_bpp    = babl->fish_path.dest_bpp;
  const Babl *source_type = babl_format_get_type (babl_source,
                                                  babl_format_get_n_components (babl_source) - 1);
  const Babl *dest_type   = babl_format_get_type (babl_dest,
                                                  babl_format_get_n_components (babl_dest) - 1);

  int src_not_associated = ((babl->conversion.source->format.model->flags &
          BABL_MODEL_FLAG_ASSOCIATED)==0);
  int dest_not_associated = ((babl->conversion.destination->format.model->flags &
          BABL_MODEL_FLAG_ASSOCIATED)==0);
  if (
      (babl->conversion.source->format.type[0]->bits < 32)       

      && (  (   source_bpp == 2
             && dest_bpp   == 16)

          ||(   source_bpp  == 4
             && dest_bpp    == 16
             && source_type == babl_type_from_id (BABL_U8)
             && dest_type   == babl_type_from_id (BABL_FLOAT)
             && src_not_associated
             && dest_not_associated)

          ||(   source_bpp == 4
             && dest_bpp   == 4
             && dest_type  == source_type
             && dest_not_associated)

          ||(   source_bpp  == 4
             && dest_bpp    == 8
             && source_type == babl_type_from_id (BABL_U8)
             && dest_type   == babl_type_from_id (BABL_U16)
             && src_not_associated
             && dest_not_associated)

          ||(   source_bpp == 3
             && dest_bpp   == 4)

          ||(   source_bpp == 2
             && dest_bpp   == 4)

          ||(   source_bpp == 2
             && dest_bpp   == 2)

          ||(   source_bpp == 1
             && dest_bpp   == 4)

          ||(   source_bpp == 3
             && dest_bpp   == 3)
      )
     )
  {
     // as long as the highest 8bit of the 32bit of a 4 byte input is ignored
     // (alpha) - and it is not an associated color model. A 24 bit LUT provides
     // exact data. 
     // Note that we can only copy alpha from source to complete when
     // types are matching expectations - the source_bpp/dest_bpp pairs have
     // currently have built-in expectation for what type alpha is filled in
     {
       static int measured_timings = 0;
       float scaling = 10.0;
       if (!measured_timings) measure_timings ();
       measured_timings = 1;
       LUT_LOG ("%sLUT for %s to %s   %.2f%s%.2f\n",

       ((lut_timing_for (source_bpp, dest_bpp) * scaling) <
                           babl->fish_path.cost)?"possible ":"no ",

                        babl_get_name (babl->conversion.source),
                        babl_get_name (babl->conversion.destination),
                        (lut_timing_for (source_bpp, dest_bpp) * scaling),
       ((lut_timing_for (source_bpp, dest_bpp) * scaling) <
                           babl->fish_path.cost)?" < ":" > ",
                        babl->fish_path.cost);
       if ((lut_timing_for (source_bpp, dest_bpp) * scaling) <
                           babl->fish_path.cost)
       {
         babl->fish_path.is_u8_color_conv = 1;
       }
     }
  }
  }
}

void
_babl_fish_missing_fast_path_warning (const Babl *source,
                                      const Babl *destination)
{
#ifndef BABL_UNSTABLE
  if (debug_conversions)
#endif
  {
    static int warnings = 0;

    if (_babl_legal_error() <= 0.0000000001)
      return;

    if (warnings++ == 0)
      fprintf (stderr,
"Missing fast-path babl conversion detected, Implementing missing babl fast paths\n"
"accelerates GEGL, GIMP and other software using babl, warnings are printed on\n"
"first occurance of formats used where a conversion has to be synthesized\n"
"programmatically by babl based on format description\n"
"\n");

    fprintf (stderr, "*WARNING* missing babl fast path(s): \"%s\" to \"%s\"\n",
       babl_get_name (source),
       babl_get_name (destination));

  }
}


static Babl *
babl_fish_path2 (const Babl *source,
                 const Babl *destination,
                 double      tolerance)
{
  Babl *babl = NULL;
  const Babl *sRGB = babl_space ("sRGB");
  char name[BABL_MAX_NAME_LEN];
  int is_fast = 0;
  static int debug_missing = -1;
  if (debug_missing < 0)
  {
     const char *val = getenv ("BABL_DEBUG_MISSING");
     if (val && strcmp (val, "0"))
       debug_missing = 1;
     else
       debug_missing = 0;
  }

  _babl_fish_create_name (name, source, destination, 1);
  babl_mutex_lock (babl_format_mutex);
  babl = babl_db_exist_by_name (babl_fish_db (), name);

  if (tolerance <= 0.0)
  {
    is_fast = 0;
    tolerance = _babl_legal_error ();
  }
  else
    is_fast = 1;

  if (!is_fast)
  {
  if (babl)
    {
      /* There is an instance already registered by the required name,
       * returning the preexistent one instead.
       */
      babl_mutex_unlock (babl_format_mutex);
      return babl;
    }
  }

  if ((source->format.space != sRGB) ||
      (destination->format.space != sRGB))
  {
    static const Babl *run_once[512]={NULL};
    int i;
    int done = 0;

    for (i = 0; run_once[i]; i++)
    {
      if (run_once[i] == source->format.space)
        done |= 1;
      else if (run_once[i] == destination->format.space)
        done |= 2;
    }

    /* source space not in initialization array */
    if ((done & 1) == 0 && (source->format.space != sRGB))
    {
      run_once[i++] = source->format.space;
      babl_conversion_class_for_each (alias_conversion, (void*)source->format.space);

      _babl_space_add_universal_rgb (source->format.space);
    }

    /* destination space not in initialization array */
    if ((done & 2) == 0 && (destination->format.space != source->format.space) && (destination->format.space != sRGB))
    {
      run_once[i++] = destination->format.space;
      babl_conversion_class_for_each (alias_conversion, (void*)destination->format.space);

      _babl_space_add_universal_rgb (destination->format.space);
    }

    if (!done && 0)
    {
      babl_conversion_class_for_each (show_item, (void*)source->format.space);
    }
  }

  babl = babl_calloc (1, sizeof (BablFishPath) +
                      strlen (name) + 1);
  babl_set_destructor (babl, _babl_fish_path_destroy);

  babl->class_type                = BABL_FISH_PATH;
  babl->instance.id               = babl_fish_get_id (source, destination);
  babl->instance.name             = ((char *) babl) + sizeof (BablFishPath);
  strcpy (babl->instance.name, name);
  babl->fish.source               = source;
  babl->fish.destination          = destination;
  babl->fish.pixels               = 0;
  babl->fish.error                = BABL_MAX_COST_VALUE;
  babl->fish_path.cost            = BABL_MAX_COST_VALUE;
  babl->fish_path.conversion_list = babl_list_init_with_size (BABL_HARD_MAX_PATH_LENGTH);



  {
    PathContext pc;
    int start_depth = max_path_length ();
    int end_depth = start_depth + 1 + ((destination->format.space != sRGB)?1:0);
    end_depth = MIN(end_depth, BABL_HARD_MAX_PATH_LENGTH);

    pc.current_path = babl_list_init_with_size (BABL_HARD_MAX_PATH_LENGTH);
    pc.fish_path = babl;
    pc.to_format = (Babl *) destination;

    /* we hold a global lock whilerunning get_conversion_path since
     * it depends on keeping the various format.visited members in
     * a consistent state, this code path is not performance critical
     * since created fishes are cached.
     */
    babl_in_fish_path++;

    for (int max_depth = start_depth;
         babl->fish_path.conversion_list->count == 0 && max_depth <= end_depth;
         max_depth++)
    {
      get_conversion_path (&pc, (Babl *) source, 0, max_depth, tolerance);
    }

    if (debug_missing)
    {
      if (babl->fish_path.conversion_list->count == 0)
        fprintf (stderr, "babl: WARNING lacking conversion path for %s to %s\n",
          babl_get_name (source), babl_get_name (destination));
      else if (babl->fish_path.conversion_list->count == end_depth)
        fprintf (stderr, "babl: WARNING need %i step conversion for %s to %s\n", end_depth,
          babl_get_name (source), babl_get_name (destination));
      else
        fprintf (stderr, "babl: found %i step conversion for %s to %s\n",
          babl->fish_path.conversion_list->count,
          babl_get_name (source), babl_get_name (destination));
    }

    babl_in_fish_path--;
    babl_free (pc.current_path);
  }

  if (babl_list_size (babl->fish_path.conversion_list) == 0)
    {
      babl_free (babl);
      babl_mutex_unlock (babl_format_mutex);

      return NULL;
    }

  _babl_fish_prepare_bpp (babl);

  _babl_fish_rig_dispatch (babl);
  /* Since there is not an already registered instance by the required
   * name, inserting newly created class into database.
   */
  if (!is_fast)
  {
    babl_db_insert (babl_fish_db (), babl);
  }
  babl_mutex_unlock (babl_format_mutex);
  return babl;
}

const Babl * 
babl_fast_fish (const void *source_format,
                const void *destination_format,
                const char *performance)
{
  double tolerance = 0.0;

  if (!performance || !strcmp (performance, "default"))
    tolerance = 0.0; // note: not _babl_legal_error() to trigger,
                      // right code paths in babl_fish_path2
  else if (!strcmp (performance, "exact"))
    tolerance=0.0000000001;
  else if (!strcmp (performance, "precise"))
    tolerance=0.00001;
  if (!strcmp (performance, "fast"))
    tolerance=0.001;
  else if (!strcmp (performance, "glitch"))
    tolerance=0.01;
  else {
    tolerance = babl_parse_double (performance);
  }

  return babl_fish_path2 (source_format, destination_format, tolerance);
}

Babl *
babl_fish_path (const Babl *source,
                const Babl *destination)
{
  return babl_fish_path2 (source, destination, 0.0);
}


static void
babl_fish_path_process (const Babl *babl,
                        const char *source,
                        char       *destination,
                        long        n,
                        void       *data)
{
  BABL(babl)->fish.pixels += n;
  if (babl->fish_path.is_u8_color_conv)
  {
     if (babl_fish_lut_process_maybe (babl,
                                      source, destination, n,
                                      data))
       return;
  }
  else
  {
    babl_conv_counter+=n;
  }
  process_conversion_path (babl->fish_path.conversion_list,
                           source,
                           babl->fish_path.source_bpp,
                           destination,
                           babl->fish_path.dest_bpp,
                           n);
}

static void
babl_fish_memcpy_process (const Babl *babl,
                          const char *source,
                          char       *destination,
                          long        n,
                          void       *data)
{
  memcpy (destination, source, n * babl->fish.source->format.bytes_per_pixel);
}

void
_babl_fish_rig_dispatch (Babl *babl)
{
  babl->fish.data     = (void*)&(babl->fish.data);

  if (babl->fish.source == babl->fish.destination)
    {
      babl->fish.dispatch = babl_fish_memcpy_process;
      return;
    }

  switch (babl->class_type)
    {
      case BABL_FISH_REFERENCE:
        babl->fish.dispatch = babl_fish_reference_process;
        break;

      case BABL_FISH_SIMPLE:
        if (BABL (babl->fish_simple.conversion)->class_type == BABL_CONVERSION_LINEAR)
          {
            /* lift out conversion from single step conversion and make it be the dispatch function
             * itself
             */
            babl->fish.data     = &(babl->fish_simple.conversion->data);
            babl->fish.dispatch = babl->fish_simple.conversion->dispatch;
          }
        else
          {
            babl_fatal ("Cannot use a simple fish to process without a linear conversion");
          }
        break;

      case BABL_FISH_PATH:
        if (babl_list_size(babl->fish_path.conversion_list) == 1)
        {
          BablConversion *conversion = (void*)babl_list_get_first(babl->fish_path.conversion_list);

          /* do same short-circuit optimization as for simple fishes */
          babl->fish.dispatch = conversion->dispatch;
          babl->fish.data     = &conversion->data;
        }
        else
        {
          babl->fish.dispatch = babl_fish_path_process;
        }
        break;

      case BABL_CONVERSION:
      case BABL_CONVERSION_LINEAR:
      case BABL_CONVERSION_PLANE:
      case BABL_CONVERSION_PLANAR:
        babl_assert (0);
        break;

      default:
        babl_log ("NYI");
        break;
    }
}

static inline long
_babl_process (const Babl *cbabl,
               const void *source,
               void       *destination,
               long        n)
{
  Babl *babl = (void*)cbabl;
  babl->fish.dispatch (babl, source, destination, n, *babl->fish.data);
  return n;
}

long
babl_process (const Babl *babl,
              const void *source,
              void       *destination,
              long        n)
{
  return _babl_process ((void*)babl, source, destination, n);
}

long
babl_process_rows (const Babl *fish,
                   const void *source,
                   int         source_stride,
                   void       *dest,
                   int         dest_stride,
                   long        n,
                   int         rows)
{
  Babl          *babl = (Babl*)fish;
  const uint8_t *src  = source;
  uint8_t       *dst  = dest;
  int            row;

  babl_assert (babl && BABL_IS_BABL (babl) && source && dest);

  if (n <= 0)
    return 0;

  for (row = 0; row < rows; row++)
    {
      babl->fish.dispatch (babl, (void*)src, (void*)dst, n, *babl->fish.data);

      src += source_stride;
      dst += dest_stride;
    }
  return n * rows;
}

#include <stdint.h>

#define BABL_ALIGN 16
static inline void *align_16 (unsigned char *ret)
{
  int offset = BABL_ALIGN - ((uintptr_t) ret) % BABL_ALIGN;
  ret = ret + offset;
  return ret;
}

static inline void
process_conversion_path (BablList   *path,
                         const void *source_buffer,
                         int         source_bpp,
                         void       *destination_buffer,
                         int         dest_bpp,
                         long        n)
{
  int conversions = babl_list_size (path);

  if (conversions == 1)
    {
      babl_conversion_process (BABL (babl_list_get_first (path)),
                               source_buffer,
                               destination_buffer,
                               n);
    }
  else
    {
      long j;

      void *temp_buffer = align_16 (alloca (MIN(n, MAX_BUFFER_SIZE) *
                                    sizeof (double) * 5 + 16));
      void *temp_buffer2 = NULL;

      if (conversions > 2)
        {
          /* We'll need one more auxiliary buffer */
          temp_buffer2 = align_16 (alloca (MIN(n, MAX_BUFFER_SIZE) *
                                   sizeof (double) * 5 + 16));
        }

      for (j = 0; j < n; j+= MAX_BUFFER_SIZE)
        {
          long c = MIN (n - j, MAX_BUFFER_SIZE);
          int i;

          void *aux1_buffer = temp_buffer;
          void *aux2_buffer = temp_buffer2;

          /* The first conversion goes from source_buffer to aux1_buffer */
          babl_conversion_process (babl_list_get_first (path),
                                   (void*)(((unsigned char*)source_buffer) +
                                                          (j * source_bpp)),
                                   aux1_buffer,
                                   c);

          /* Process, if any, conversions between the first and the last
           * conversion in the path, in a loop */
          for (i = 1; i < conversions - 1; i++)
            {
              babl_conversion_process (path->items[i],
                                       aux1_buffer,
                                       aux2_buffer,
                                       c);
              {
                /* Swap the auxiliary buffers */
                void *swap_buffer = aux1_buffer;
                aux1_buffer = aux2_buffer;
                aux2_buffer = swap_buffer;
              }
            }

          /* The last conversion goes from aux1_buffer to destination_buffer */
          babl_conversion_process (babl_list_get_last (path),
                                   aux1_buffer,
                                   (void*)((unsigned char*)destination_buffer +
                                                           (j * dest_bpp)),
                                   c);
        }
  }
}

static void
init_path_instrumentation (FishPathInstrumentation *fpi,
                           Babl                    *fmt_source,
                           Babl                    *fmt_destination)
{
  long   ticks_start = 0;
  long   ticks_end   = 0;

  const double *test_pixels = babl_get_path_test_pixels ();

  if (!fpi->fmt_rgba_double)
    {
      fpi->fmt_rgba_double =
          babl_format_with_space ("RGBA double",
                                  fmt_destination->format.space);
    }

  fpi->num_test_pixels = babl_get_num_path_test_pixels ();

  fpi->fish_rgba_to_source =
      babl_fish_reference (fpi->fmt_rgba_double, fmt_source);

  fpi->fish_reference =
      babl_fish_reference (fmt_source, fmt_destination);

  fpi->fish_destination_to_rgba =
      babl_fish_reference (fmt_destination, fpi->fmt_rgba_double);

  fpi->source =
      babl_calloc (fpi->num_test_pixels,
                   fmt_source->format.bytes_per_pixel);

  fpi->destination =
      babl_calloc (fpi->num_test_pixels,
                   fmt_destination->format.bytes_per_pixel);

  fpi->ref_destination =
      babl_calloc (fpi->num_test_pixels,
                   fmt_destination->format.bytes_per_pixel);

  fpi->destination_rgba_double =
      babl_calloc (fpi->num_test_pixels,
                   fpi->fmt_rgba_double->format.bytes_per_pixel);

  fpi->ref_destination_rgba_double =
      babl_calloc (fpi->num_test_pixels,
                   fpi->fmt_rgba_double->format.bytes_per_pixel);

  /* create sourcebuffer from testbuffer in the correct format */
  _babl_process (fpi->fish_rgba_to_source,
                 test_pixels, fpi->source,fpi->num_test_pixels);

  /* calculate the reference buffer of how it should be */
  ticks_start = babl_ticks ();
  _babl_process (fpi->fish_reference,
                 fpi->source, fpi->ref_destination,
                 fpi->num_test_pixels);
  ticks_end = babl_ticks ();
  fpi->reference_cost = (ticks_end - ticks_start) * BABL_TEST_ITER;

  /* transform the reference destination buffer to RGBA */
  _babl_process (fpi->fish_destination_to_rgba,
                 fpi->ref_destination, fpi->ref_destination_rgba_double,
                 fpi->num_test_pixels);
}

static void
destroy_path_instrumentation (FishPathInstrumentation *fpi)
{
  if (fpi->init_instrumentation_done)
    {
      babl_free (fpi->source);
      babl_free (fpi->destination);
      babl_free (fpi->destination_rgba_double);
      babl_free (fpi->ref_destination);
      babl_free (fpi->ref_destination_rgba_double);

      /* nulify the flag for potential new search */
      fpi->init_instrumentation_done = 0;
  }
}

static void
get_path_instrumentation (FishPathInstrumentation *fpi,
                          BablList                *path,
                          double                  *path_cost,
                          double                  *ref_cost,
                          double                  *path_error)
{
  long   ticks_start = 0;
  long   ticks_end   = 0;

  Babl *babl_source = fpi->source;
  Babl *babl_destination = fpi->destination;

  int source_bpp = 0;
  int dest_bpp = 0;

  switch (babl_source->instance.class_type)
    {
      case BABL_FORMAT:
        source_bpp = babl_source->format.bytes_per_pixel;
        break;
      case BABL_TYPE:
        source_bpp = babl_source->type.bits / 8;
        break;
      default:
        babl_log ("=eeek{%i}\n", babl_source->instance.class_type - BABL_MAGIC);
    }

  switch (babl_destination->instance.class_type)
    {
      case BABL_FORMAT:
        dest_bpp = babl_destination->format.bytes_per_pixel;
        break;
      case BABL_TYPE:
        dest_bpp = babl_destination->type.bits / 8;
        break;
      default:
        babl_log ("-eeek{%i}\n",
                  babl_destination->instance.class_type - BABL_MAGIC);
     }

  if (!fpi->init_instrumentation_done)
    {
      /* this initialization can be done only once since the
       * source and destination formats do not change during
       * the search */
      init_path_instrumentation (fpi, babl_source, babl_destination);
      fpi->init_instrumentation_done = 1;
    }

  /* calculate this path's view of what the result should be */
  ticks_start = babl_ticks ();
  for (int i = 0; i < BABL_TEST_ITER; i ++)
  process_conversion_path (path, fpi->source, source_bpp, fpi->destination,
                           dest_bpp, fpi->num_test_pixels);
  ticks_end = babl_ticks ();
  *path_cost = (ticks_end - ticks_start);

  /* transform the reference and the actual destination buffers to RGBA
   * for comparison with each other
   */
  _babl_process (fpi->fish_destination_to_rgba,
                 fpi->destination, fpi->destination_rgba_double,
                 fpi->num_test_pixels);

  *path_error = babl_rel_avg_error (fpi->destination_rgba_double,
                                    fpi->ref_destination_rgba_double,
                                    fpi->num_test_pixels * 4);

  *ref_cost = fpi->reference_cost;
}
