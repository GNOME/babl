/*
 * This file was part of gggl, it implements a variety of pixel conversion
 * functions that are usable with babl, the file needs more cleanup, but the
 * conversion functions that are usable gets used by babl.
 *
 *    GGGL is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    GGGL is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with GGGL; if not, see <http://www.gnu.org/licenses/>.
 *
 *    Rights are granted to use this shared object in libraries covered by
 *    LGPL. (exception added, during import into babl CVS.)
 *
 *  Copyright 2003, 2004, 2005, 2007 Øyvind Kolås <pippin@gimp.org>
 */

/*
 * Implemented according to information read from:
 *
 * http://www.cinenet.net/~spitzak/conversion/sketches_0265.pdf
 *
 * initially ignoring any diffusion, to keep the implementation
 * smaller, and interchangeable with the non optimized version.
 *
 * due to ability to be able to relicence gggl under a different
 * licence than GPL, I avoided the temptation to look at the
 * source files in the same location, in case I was going to
 * need this piece of code for projects where GPL compatibility
 * was a must.
 *
 * TODO: error diffusion,
 */

#include <stdlib.h>
#include <stdint.h>

#include "config.h"
#include "babl.h"

#include "base/util.h"
#include "extensions/util.h"


#define INLINE    inline

/* lookup tables used in conversion */

static float         table_8_F[1 << 8];
static float         table_8g_F[1 << 8];
static unsigned char table_F_8[1 << 17];
static unsigned char table_F_8g[1 << 17];

static int           table_inited = 0;

static void
table_init (void)
{
  if (table_inited)
    return;
  table_inited = 1;

  /* fill tables for conversion from integer to float */
  {
    int i;
    for (i = 0; i < 1 << 8; i++)
      {
        float direct = i / 255.0;
        table_8_F[i]  = direct;
        table_8g_F[i] = gamma_2_2_to_linear (direct);
      }
  }
  /* fill tables for conversion from float to integer */
  {
    union
    {
      float    f;
      uint32_t s;
    } u;
    u.f = 0.0;

    //u.s[0] = 0;

    for (u.s = 0; u.s < 4294900000; u.s += 32768)
      {
        int c;
        int cg;

        if (u.f <= 0.0)
          {
            c  = 0;
            cg = 0;
          }
        else
          {
            c  = (u.f * 255.1619) + 0.5;
            cg = (linear_to_gamma_2_2 (u.f) * 255.1619) + 0.5;
            if (cg > 255) cg = 255;
            if (c > 255) c = 255;
          }

        table_F_8[(u.s >> 15) & ((1 << 17)-1)]  = c;
        table_F_8g[(u.s >> 15) & ((1 << 17)-1)] = cg;
      }
  }
#if 0
  /* fix tables to ensure 1:1 conversions back and forth */
  if (0)
    {
      int i;
      for (i = 0; i < 256; i++)
        {
          float           f;
          unsigned short *hi = ((unsigned short *) (void *) &f);
          unsigned short *lo = ((unsigned short *) (void *) &f);

          f = table_8_F[i];
          *lo              = 0;
          table_F_8[*hi] = i;
          f  = table_8g_F[i];
          *lo              = 0;
          table_F_8g[*hi] = i;
        }
    }
#endif
}

/* function to find the index in table for a float */
static unsigned int
gggl_float_to_index16 (float f)
{
  union
  {
    float          f;
    uint32_t       s;
  } u;
  u.f = f;
  return (u.s >> 15) & ((1 << 17)-1);
}

static INLINE long
conv_F_8 (unsigned char *src, unsigned char *dst, long samples)
{
  long n = samples;

  if (!table_inited)
    table_init ();
  while (n--)
    {
      register float f = (*(float *) src);
      *(unsigned char *) dst = table_F_8[gggl_float_to_index16 (f)];
      dst                   += 1;
      src                   += 4;
    }
  return samples;
}


static INLINE long
conv_F_8g (unsigned char *src, unsigned char *dst, long samples)
{
  long n = samples;

  if (!table_inited)
    table_init ();
  while (n--)
    {
      register float f = (*(float *) src);
      *(unsigned char *) dst = table_F_8g[gggl_float_to_index16 (f)];
      dst                   += 1;
      src                   += 4;
    }
  return samples;
}


static INLINE long
conv_8_F (unsigned char *src, unsigned char *dst, long samples)
{
  long n = samples;

  if (!table_inited)
    table_init ();
  while (n--)
    {
      (*(float *) dst) = table_8_F[*(unsigned char *) src];
      dst             += 4;
      src             += 1;
    }
  return samples;
}

static float u8_gamma_minimums[257] =
{
0.0,
0x1.3e6p-13, /* 0.000152, 1 */
0x1.dd7p-12, /* 0.000455, 2 */
0x1.8dd8p-11, /* 0.000759, 3 */
0x1.168p-10, /* 0.001062, 4 */
0x1.661p-10, /* 0.001366, 5 */
0x1.b5ap-10, /* 0.001669, 6 */
0x1.029ap-9, /* 0.001973, 7 */
0x1.2a62p-9, /* 0.002276, 8 */
0x1.522ap-9, /* 0.002580, 9 */
0x1.79f4p-9, /* 0.002884, 10 */
0x1.a1e6p-9, /* 0.003188, 11 */
0x1.cbf8p-9, /* 0.003509, 12 */
0x1.f86ap-9, /* 0.003848, 13 */
0x1.13a1p-8, /* 0.004206, 14 */
0x1.2c47p-8, /* 0.004582, 15 */
0x1.462ap-8, /* 0.004977, 16 */
0x1.614fp-8, /* 0.005391, 17 */
0x1.7dbap-8, /* 0.005825, 18 */
0x1.9b6fp-8, /* 0.006278, 19 */
0x1.ba73p-8, /* 0.006751, 20 */
0x1.dacap-8, /* 0.007245, 21 */
0x1.fc77p-8, /* 0.007759, 22 */
0x1.0fbf8p-7, /* 0.008293, 23 */
0x1.21f28p-7, /* 0.008848, 24 */
0x1.34d68p-7, /* 0.009425, 25 */
0x1.486ep-7, /* 0.010023, 26 */
0x1.5cbap-7, /* 0.010642, 27 */
0x1.71bc8p-7, /* 0.011283, 28 */
0x1.87778p-7, /* 0.011947, 29 */
0x1.9dedp-7, /* 0.012632, 30 */
0x1.b51ep-7, /* 0.013340, 31 */
0x1.cd0dp-7, /* 0.014070, 32 */
0x1.e5bbp-7, /* 0.014823, 33 */
0x1.ff2a8p-7, /* 0.015600, 34 */
0x1.0cae4p-6, /* 0.016399, 35 */
0x1.1a294p-6, /* 0.017222, 36 */
0x1.28074p-6, /* 0.018068, 37 */
0x1.3649p-6, /* 0.018938, 38 */
0x1.44ef8p-6, /* 0.019832, 39 */
0x1.53fbp-6, /* 0.020751, 40 */
0x1.636ccp-6, /* 0.021693, 41 */
0x1.73454p-6, /* 0.022661, 42 */
0x1.83858p-6, /* 0.023652, 43 */
0x1.942dcp-6, /* 0.024669, 44 */
0x1.a53f8p-6, /* 0.025711, 45 */
0x1.b6bacp-6, /* 0.026778, 46 */
0x1.c8a08p-6, /* 0.027870, 47 */
0x1.daf18p-6, /* 0.028988, 48 */
0x1.edae8p-6, /* 0.030132, 49 */
0x1.006cp-5, /* 0.031301, 50 */
0x1.0a378p-5, /* 0.032497, 51 */
0x1.1439ep-5, /* 0.033719, 52 */
0x1.1e73ap-5, /* 0.034967, 53 */
0x1.28e52p-5, /* 0.036242, 54 */
0x1.338eap-5, /* 0.037544, 55 */
0x1.3e706p-5, /* 0.038872, 56 */
0x1.498aep-5, /* 0.040227, 57 */
0x1.54de6p-5, /* 0.041610, 58 */
0x1.606b2p-5, /* 0.043020, 59 */
0x1.6c318p-5, /* 0.044457, 60 */
0x1.7831ep-5, /* 0.045922, 61 */
0x1.846c8p-5, /* 0.047415, 62 */
0x1.90e1ap-5, /* 0.048936, 63 */
0x1.9d91ap-5, /* 0.050484, 64 */
0x1.aa7dp-5, /* 0.052062, 65 */
0x1.b7a3cp-5, /* 0.053667, 66 */
0x1.c5064p-5, /* 0.055301, 67 */
0x1.d2a4ep-5, /* 0.056963, 68 */
0x1.e08p-5, /* 0.058655, 69 */
0x1.ee97ap-5, /* 0.060375, 70 */
0x1.fcec6p-5, /* 0.062124, 71 */
0x1.05bf3p-4, /* 0.063903, 72 */
0x1.0d26fp-4, /* 0.065711, 73 */
0x1.14adap-4, /* 0.067548, 74 */
0x1.1c536p-4, /* 0.069415, 75 */
0x1.24185p-4, /* 0.071312, 76 */
0x1.2bfcap-4, /* 0.073239, 77 */
0x1.34007p-4, /* 0.075196, 78 */
0x1.3c23ep-4, /* 0.077183, 79 */
0x1.44671p-4, /* 0.079200, 80 */
0x1.4cca3p-4, /* 0.081248, 81 */
0x1.554d5p-4, /* 0.083326, 82 */
0x1.5df0ap-4, /* 0.085435, 83 */
0x1.66b44p-4, /* 0.087574, 84 */
0x1.6f984p-4, /* 0.089745, 85 */
0x1.789cep-4, /* 0.091946, 86 */
0x1.81c23p-4, /* 0.094179, 87 */
0x1.8b085p-4, /* 0.096443, 88 */
0x1.946f7p-4, /* 0.098739, 89 */
0x1.9df7bp-4, /* 0.101066, 90 */
0x1.a7a12p-4, /* 0.103425, 91 */
0x1.b16bfp-4, /* 0.105816, 92 */
0x1.bb584p-4, /* 0.108238, 93 */
0x1.c5662p-4, /* 0.110693, 94 */
0x1.cf95cp-4, /* 0.113180, 95 */
0x1.d9e74p-4, /* 0.115699, 96 */
0x1.e45abp-4, /* 0.118251, 97 */
0x1.eef03p-4, /* 0.120835, 98 */
0x1.f9a8p-4, /* 0.123451, 99 */
0x1.02411p-3, /* 0.126101, 100 */
0x1.07bf68p-3, /* 0.128783, 101 */
0x1.0d4f08p-3, /* 0.131498, 102 */
0x1.12eff8p-3, /* 0.134247, 103 */
0x1.18a25p-3, /* 0.137028, 104 */
0x1.1e6628p-3, /* 0.139843, 105 */
0x1.243b88p-3, /* 0.142692, 106 */
0x1.2a228p-3, /* 0.145574, 107 */
0x1.301b28p-3, /* 0.148489, 108 */
0x1.36258p-3, /* 0.151439, 109 */
0x1.3c41ap-3, /* 0.154422, 110 */
0x1.426f98p-3, /* 0.157439, 111 */
0x1.48af7p-3, /* 0.160491, 112 */
0x1.4f014p-3, /* 0.163577, 113 */
0x1.556508p-3, /* 0.166697, 114 */
0x1.5bdaep-3, /* 0.169851, 115 */
0x1.6262dp-3, /* 0.173040, 116 */
0x1.68fcfp-3, /* 0.176264, 117 */
0x1.6fa948p-3, /* 0.179522, 118 */
0x1.7667ep-3, /* 0.182815, 119 */
0x1.7d38d8p-3, /* 0.186144, 120 */
0x1.841c28p-3, /* 0.189507, 121 */
0x1.8b11f8p-3, /* 0.192905, 122 */
0x1.921a4p-3, /* 0.196339, 123 */
0x1.99352p-3, /* 0.199808, 124 */
0x1.a06298p-3, /* 0.203313, 125 */
0x1.a7a2b8p-3, /* 0.206853, 126 */
0x1.aef598p-3, /* 0.210429, 127 */
0x1.b65b38p-3, /* 0.214041, 128 */
0x1.bdd3bp-3, /* 0.217689, 129 */
0x1.c55f08p-3, /* 0.221373, 130 */
0x1.ccfd5p-3, /* 0.225093, 131 */
0x1.d4ae88p-3, /* 0.228849, 132 */
0x1.dc72dp-3, /* 0.232641, 133 */
0x1.e44a28p-3, /* 0.236470, 134 */
0x1.ec34ap-3, /* 0.240335, 135 */
0x1.f43258p-3, /* 0.244237, 136 */
0x1.fc435p-3, /* 0.248175, 137 */
0x1.0233c4p-2, /* 0.252151, 138 */
0x1.064f98p-2, /* 0.256163, 139 */
0x1.0a7518p-2, /* 0.260212, 140 */
0x1.0ea45p-2, /* 0.264299, 141 */
0x1.12dd44p-2, /* 0.268422, 142 */
0x1.172p-2, /* 0.272583, 143 */
0x1.1b6c84p-2, /* 0.276781, 144 */
0x1.1fc2ep-2, /* 0.281017, 145 */
0x1.242314p-2, /* 0.285290, 146 */
0x1.288d28p-2, /* 0.289601, 147 */
0x1.2d0124p-2, /* 0.293950, 148 */
0x1.317f0cp-2, /* 0.298336, 149 */
0x1.3606e8p-2, /* 0.302761, 150 */
0x1.3a98cp-2, /* 0.307223, 151 */
0x1.3f3498p-2, /* 0.311724, 152 */
0x1.43da74p-2, /* 0.316263, 153 */
0x1.488a5cp-2, /* 0.320840, 154 */
0x1.4d4454p-2, /* 0.325456, 155 */
0x1.520864p-2, /* 0.330110, 156 */
0x1.56d694p-2, /* 0.334803, 157 */
0x1.5baee8p-2, /* 0.339534, 158 */
0x1.60916p-2, /* 0.344305, 159 */
0x1.657e0cp-2, /* 0.349114, 160 */
0x1.6a74fp-2, /* 0.353962, 161 */
0x1.6f760cp-2, /* 0.358849, 162 */
0x1.748168p-2, /* 0.363775, 163 */
0x1.79971p-2, /* 0.368740, 164 */
0x1.7eb704p-2, /* 0.373745, 165 */
0x1.83e15p-2, /* 0.378789, 166 */
0x1.8915fp-2, /* 0.383873, 167 */
0x1.8e54f4p-2, /* 0.388996, 168 */
0x1.939e6p-2, /* 0.394159, 169 */
0x1.98f238p-2, /* 0.399361, 170 */
0x1.9e508p-2, /* 0.404604, 171 */
0x1.a3b94p-2, /* 0.409886, 172 */
0x1.a92c8p-2, /* 0.415209, 173 */
0x1.aeaa44p-2, /* 0.420571, 174 */
0x1.b4329p-2, /* 0.425974, 175 */
0x1.b9c56cp-2, /* 0.431417, 176 */
0x1.bf62d8p-2, /* 0.436900, 177 */
0x1.c50aep-2, /* 0.442424, 178 */
0x1.cabd88p-2, /* 0.447989, 179 */
0x1.d07ad4p-2, /* 0.453594, 180 */
0x1.d642c8p-2, /* 0.459239, 181 */
0x1.dc156cp-2, /* 0.464925, 182 */
0x1.e1f2c4p-2, /* 0.470653, 183 */
0x1.e7dad8p-2, /* 0.476421, 184 */
0x1.edcdacp-2, /* 0.482230, 185 */
0x1.f3cb4cp-2, /* 0.488080, 186 */
0x1.f9d3bcp-2, /* 0.493972, 187 */
0x1.ffe70cp-2, /* 0.499905, 188 */
0x1.03028cp-1, /* 0.505879, 189 */
0x1.06170ap-1, /* 0.511895, 190 */
0x1.0930f8p-1, /* 0.517952, 191 */
0x1.0c5058p-1, /* 0.524050, 192 */
0x1.0f752ep-1, /* 0.530191, 193 */
0x1.129f7cp-1, /* 0.536373, 194 */
0x1.15cf44p-1, /* 0.542597, 195 */
0x1.19048cp-1, /* 0.548863, 196 */
0x1.1c3f56p-1, /* 0.555171, 197 */
0x1.1f7fa4p-1, /* 0.561521, 198 */
0x1.22c578p-1, /* 0.567913, 199 */
0x1.2610d8p-1, /* 0.574347, 200 */
0x1.2961c2p-1, /* 0.580824, 201 */
0x1.2cb83ep-1, /* 0.587343, 202 */
0x1.30144cp-1, /* 0.593905, 203 */
0x1.3375eep-1, /* 0.600509, 204 */
0x1.36dd26p-1, /* 0.607156, 205 */
0x1.3a49fap-1, /* 0.613846, 206 */
0x1.3dbc6ap-1, /* 0.620578, 207 */
0x1.413478p-1, /* 0.627353, 208 */
0x1.44b228p-1, /* 0.634172, 209 */
0x1.48357cp-1, /* 0.641033, 210 */
0x1.4bbe76p-1, /* 0.647937, 211 */
0x1.4f4d18p-1, /* 0.654885, 212 */
0x1.52e168p-1, /* 0.661876, 213 */
0x1.567b64p-1, /* 0.668910, 214 */
0x1.5a1b1p-1, /* 0.675988, 215 */
0x1.5dc07p-1, /* 0.683109, 216 */
0x1.616b86p-1, /* 0.690273, 217 */
0x1.651c54p-1, /* 0.697482, 218 */
0x1.68d2dcp-1, /* 0.704734, 219 */
0x1.6c8f22p-1, /* 0.712030, 220 */
0x1.705126p-1, /* 0.719369, 221 */
0x1.7418eep-1, /* 0.726753, 222 */
0x1.77e67ap-1, /* 0.734180, 223 */
0x1.7bb9cep-1, /* 0.741652, 224 */
0x1.7f92eap-1, /* 0.749168, 225 */
0x1.8371d6p-1, /* 0.756728, 226 */
0x1.87568ep-1, /* 0.764332, 227 */
0x1.8b411ap-1, /* 0.771981, 228 */
0x1.8f317ap-1, /* 0.779674, 229 */
0x1.9327bp-1, /* 0.787412, 230 */
0x1.9723cp-1, /* 0.795195, 231 */
0x1.9b25aap-1, /* 0.803022, 232 */
0x1.9f2d74p-1, /* 0.810894, 233 */
0x1.a33b1ep-1, /* 0.818810, 234 */
0x1.a74eacp-1, /* 0.826772, 235 */
0x1.ab682p-1, /* 0.834779, 236 */
0x1.af877cp-1, /* 0.842831, 237 */
0x1.b3accp-1, /* 0.850927, 238 */
0x1.b7d7f2p-1, /* 0.859069, 239 */
0x1.bc0914p-1, /* 0.867257, 240 */
0x1.c04026p-1, /* 0.875489, 241 */
0x1.c47d2ap-1, /* 0.883767, 242 */
0x1.c8c024p-1, /* 0.892091, 243 */
0x1.cd0918p-1, /* 0.900460, 244 */
0x1.d15804p-1, /* 0.908875, 245 */
0x1.d5acecp-1, /* 0.917335, 246 */
0x1.da07d4p-1, /* 0.925841, 247 */
0x1.de68bcp-1, /* 0.934393, 248 */
0x1.e2cfa8p-1, /* 0.942991, 249 */
0x1.e73c9ap-1, /* 0.951634, 250 */
0x1.ebaf96p-1, /* 0.960324, 251 */
0x1.f0289ep-1, /* 0.969060, 252 */
0x1.f4a7b6p-1, /* 0.977842, 253 */
0x1.f92cep-1, /* 0.986670, 254 */
0x1.fdb822p-1, /* 0.995545, 255 */
999.0
};

static INLINE long
conv_rgbaF_rgb8 (unsigned char *src, unsigned char *dst, long samples)
{
  long n = samples;

  while (n--)
    {
      register float f = (*(float *) src);
      *(unsigned char *) dst = table_F_8g[gggl_float_to_index16 (f)];
      src                   += 4;
      dst                   += 1;

      f                      = (*(float *) src);
      *(unsigned char *) dst = table_F_8g[gggl_float_to_index16 (f)];
      src                   += 4;
      dst                   += 1;

      f                      = (*(float *) src);
      *(unsigned char *) dst = table_F_8g[gggl_float_to_index16 (f)];
      src                   += 4;
      dst                   += 1;

      src += 4;
    }
  return samples;
}


static INLINE long
conv_rgbaF_rgba8 (unsigned char *src, unsigned char *dst, long samples)
{
  long n = samples;

  while (n--)
    {
      register float f = (*(float *) src);
      *(unsigned char *) dst = table_F_8g[gggl_float_to_index16 (f)];
      src                   += 4;
      dst                   += 1;

      f                      = (*(float *) src);
      *(unsigned char *) dst = table_F_8g[gggl_float_to_index16 (f)];
      src                   += 4;
      dst                   += 1;

      f                      = (*(float *) src);
      *(unsigned char *) dst = table_F_8g[gggl_float_to_index16 (f)];
      src                   += 4;
      dst                   += 1;

      f                      = (*(float *) src);
      *(unsigned char *) dst = table_F_8[gggl_float_to_index16 (f)];
      src                   += 4;
      dst                   += 1;
    }
  return samples;
}

#define conv_rgbaF_rgbP8    conv_rgbaF_rgba8

static INLINE long
conv_rgbF_rgb8 (unsigned char *src, unsigned char *dst, long samples)
{
  conv_F_8g (src, dst, samples * 3);
  return samples;
}

static INLINE long
conv_gaF_ga8 (unsigned char *src, unsigned char *dst, long samples)
{
  conv_F_8 (src, dst, samples * 2);
  return samples;
}

#define conv_rgbAF_rgbA8    conv_rgbaF_rgba8
#define conv_gF_g8          conv_F_8
#define conv_gAF_gA8        conv_gaF_ga8


static INLINE long
conv_rgba8_rgbaF (unsigned char *src, unsigned char *dst, long samples)
{
  long n = samples;

  while (n--)
    {
      (*(float *) dst) = table_8g_F[*(unsigned char *) src];
      dst             += 4;
      src             += 1;

      (*(float *) dst) = table_8g_F[*(unsigned char *) src];
      dst             += 4;
      src             += 1;

      (*(float *) dst) = table_8g_F[*(unsigned char *) src];
      dst             += 4;
      src             += 1;

      (*(float *) dst) = table_8_F[*(unsigned char *) src];
      dst             += 4;
      src             += 1;
    }
  return samples;
}

static INLINE long
conv_rgb8_rgbaF (unsigned char *src, unsigned char *dst, long samples)
{
  long n = samples;

  while (n--)
    {
      (*(float *) dst) = table_8g_F[*(unsigned char *) src];
      dst             += 4;
      src             += 1;

      (*(float *) dst) = table_8g_F[*(unsigned char *) src];
      dst             += 4;
      src             += 1;

      (*(float *) dst) = table_8g_F[*(unsigned char *) src];
      dst             += 4;
      src             += 1;

      (*(float *) dst) = 1.0;
      dst             += 4;
    }
  return samples;
}

static long
conv_rgbAF_rgb8 (unsigned char *srcc,
                 unsigned char *dstc,
                 long           samples)
{
  float         *src = (void *) srcc;
  unsigned char *dst = (void *) dstc;
  long           n   = samples;

  while (n--)
    {
      float alpha = src[3];
      if (alpha < BABL_ALPHA_THRESHOLD)
        {
          dst[0] = 0;
          dst[1] = 0;
          dst[2] = 0;
        }
      else
        {
          float alpha_recip = 1.0 / alpha;
          dst[0] = table_F_8g[gggl_float_to_index16 (src[0] * alpha_recip)];
          dst[0] = src[0] * alpha_recip < u8_gamma_minimums[dst[0]] ? dst[0] - 1 : src[0] * alpha_recip >= u8_gamma_minimums[dst[0] + 1] ?  dst[0] + 1 : dst[0];
          dst[1] = table_F_8g[gggl_float_to_index16 (src[1] * alpha_recip)];
          dst[1] = src[1] * alpha_recip < u8_gamma_minimums[dst[1]] ? dst[1] - 1 : src[1] * alpha_recip >= u8_gamma_minimums[dst[1] + 1] ?  dst[1] + 1 : dst[1];
          dst[2] = table_F_8g[gggl_float_to_index16 (src[2] * alpha_recip)];
          dst[2] = src[2] * alpha_recip < u8_gamma_minimums[dst[2]] ? dst[2] - 1 : src[2] * alpha_recip >= u8_gamma_minimums[dst[2] + 1] ?  dst[2] + 1 : dst[2];
        }
      src += 4;
      dst += 3;
    }
  return samples;
}

static long
conv_rgbAF_rgba8 (unsigned char *srcc,
                 unsigned char *dstc,
                 long           samples)
{
  float         *src = (void *) srcc;
  unsigned char *dst = (void *) dstc;
  long           n   = samples;

  while (n--)
    {
      float alpha = src[3];
      if (alpha < BABL_ALPHA_THRESHOLD)
        {
          dst[0] = 0;
          dst[1] = 0;
          dst[2] = 0;
          dst[3] = 0;
        }
      else
        {
          float alpha_recip = 1.0 / alpha;
          dst[0] = table_F_8g[gggl_float_to_index16 (src[0] * alpha_recip)];
          dst[0] = src[0] * alpha_recip < u8_gamma_minimums[dst[0]] ? dst[0] - 1 : src[0] * alpha_recip >= u8_gamma_minimums[dst[0] + 1] ?  dst[0] + 1 : dst[0];
          dst[1] = table_F_8g[gggl_float_to_index16 (src[1] * alpha_recip)];
          dst[1] = src[1] * alpha_recip < u8_gamma_minimums[dst[1]] ? dst[1] - 1 : src[1] * alpha_recip >= u8_gamma_minimums[dst[1] + 1] ?  dst[1] + 1 : dst[1];
          dst[2] = table_F_8g[gggl_float_to_index16 (src[2] * alpha_recip)];
          dst[2] = src[2] * alpha_recip < u8_gamma_minimums[dst[2]] ? dst[2] - 1 : src[2] * alpha_recip >= u8_gamma_minimums[dst[2] + 1] ?  dst[2] + 1 : dst[2];
          dst[3] = 0xFF * alpha + 0.5;
        }
      src += 4;
      dst += 4;
    }
  return samples;
}

static long
conv_bgrA8_rgba8 (unsigned char *srcc,
                  unsigned char *dstc,
                  long           samples)
{
  unsigned char *src = (void *) srcc;
  unsigned char *dst = (void *) dstc;
  long           n   = samples;

  while (n--)
    {
      unsigned char alpha = src[3];
      dst[0] = alpha ? (src[2] * 255 / alpha) : 0;
      dst[1] = alpha ? (src[1] * 255 / alpha) : 0;
      dst[2] = alpha ? (src[0] * 255 / alpha) : 0;
      dst[3] = alpha;
      src   += 4;
      dst   += 4;
    }
  return samples;
}


static long
conv_rgbaF_rgbAF (unsigned char *srcc,
                  unsigned char *dstc,
                  long           samples)
{
  float *src = (void *) srcc;
  float *dst = (void *) dstc;
  long           n   = samples;

  while (n--)
    {
      float alpha = src[3];
      dst[0] = src[0] * alpha;
      dst[1] = src[1] * alpha;
      dst[2] = src[2] * alpha;
      dst[3] = alpha;
      src   += 4;
      dst   += 4;
    }
  return samples;
}


static long
conv_rgbAF_rgbaF (unsigned char *srcc,
                  unsigned char *dstc,
                  long           samples)
{
  float *src = (void *) srcc;
  float *dst = (void *) dstc;
  long           n   = samples;

  while (n--)
    {
      float alpha = src[3];
      float recip;
      if (alpha < BABL_ALPHA_THRESHOLD)
        recip = 0.0;
      else
        recip = 1.0/alpha;
      dst[0] = src[0] * recip;
      dst[1] = src[1] * recip;
      dst[2] = src[2] * recip;
      dst[3] = alpha;
      src   += 4;
      dst   += 4;
    }
  return samples;
}



static long
conv_rgbAF_lrgba8 (unsigned char *srcc,
                   unsigned char *dstc,
                   long           samples)
{
  float *src = (void *) srcc;
  unsigned char *dst = (void *) dstc;
  long           n   = samples;

  while (n--)
    {
      float alpha = src[3];
      float recip = (1.0/alpha);
      if (alpha < BABL_ALPHA_THRESHOLD)
        {
          dst[0] = dst[1] = dst[2] = dst[3] = 0;
        }
      else
        {
          dst[0] = table_F_8[gggl_float_to_index16 (src[0] * recip)];
          dst[1] = table_F_8[gggl_float_to_index16 (src[1] * recip)];
          dst[2] = table_F_8[gggl_float_to_index16 (src[2] * recip)];
          dst[3] = table_F_8[gggl_float_to_index16 (alpha)];
        }
      src   += 4;
      dst   += 4;
    }
  return samples;
}

#define conv_rgb8_rgbAF    conv_rgb8_rgbaF

int init (void);

int
init (void)
{
  const Babl *rgbaF = babl_format_new (
    babl_model ("RGBA"),
    babl_type ("float"),
    babl_component ("R"),
    babl_component ("G"),
    babl_component ("B"),
    babl_component ("A"),
    NULL);
  const Babl *rgbAF = babl_format_new (
    babl_model ("RaGaBaA"),
    babl_type ("float"),
    babl_component ("Ra"),
    babl_component ("Ga"),
    babl_component ("Ba"),
    babl_component ("A"),
    NULL);

  const Babl *lrgba8 = babl_format_new (
    babl_model ("RGBA"),
    babl_type ("u8"),
    babl_component ("R"),
    babl_component ("G"),
    babl_component ("B"),
    babl_component ("A"),
    NULL);

  const Babl *rgba8 = babl_format_new (
    babl_model ("R'G'B'A"),
    babl_type ("u8"),
    babl_component ("R'"),
    babl_component ("G'"),
    babl_component ("B'"),
    babl_component ("A"),
    NULL);
  const Babl *bgrA8 = babl_format_new (
    "name", "B'aG'aR'aA u8",
    babl_model ("R'aG'aB'aA"),
    babl_type ("u8"),
    babl_component ("B'a"),
    babl_component ("G'a"),
    babl_component ("R'a"),
    babl_component ("A"),
    NULL);
  const Babl *rgb8 = babl_format_new (
    babl_model ("R'G'B'"),
    babl_type ("u8"),
    babl_component ("R'"),
    babl_component ("G'"),
    babl_component ("B'"),
    NULL);

  table_init ();

#define o(src, dst) \
  babl_conversion_new (src, dst, "linear", conv_ ## src ## _ ## dst, NULL)

  o (rgbaF, rgbAF);
  o (rgbAF, rgbaF);
  o (rgbAF, lrgba8);
  o (rgb8, rgbaF);
  o (rgb8, rgbAF);
  o (rgba8, rgbaF);
  o (rgbaF, rgb8);
  o (rgbAF, rgb8);
  o (rgbAF, rgba8);
  o (bgrA8, rgba8);

  return 0;
}
