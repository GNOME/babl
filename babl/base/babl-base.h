/* babl - dynamically extendable universal pixel conversion library.
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

#ifndef _BABL_BASE_H
#define _BABL_BASE_H

#ifdef ARM_NEON
#define BABL_SIMD_SUFFIX(symbol) symbol##_arm_neon
#else
#ifdef X86_64_V2
#define BABL_SIMD_SUFFIX(symbol) symbol##_x86_64_v2
#else 
#ifdef X86_64_V3
#define BABL_SIMD_SUFFIX(symbol) symbol##_x86_64_v3
#else
#define BABL_SIMD_SUFFIX(symbol) symbol##_generic
#endif
#endif
#endif

extern void (*babl_base_init)    (void);

void BABL_SIMD_SUFFIX(babl_base_init)    (void);
void BABL_SIMD_SUFFIX(babl_base_destroy) (void);
void BABL_SIMD_SUFFIX(babl_formats_init) (void);

void BABL_SIMD_SUFFIX(babl_base_type_half) (void);
void BABL_SIMD_SUFFIX(babl_base_type_float)  (void);
void BABL_SIMD_SUFFIX(babl_base_type_u8)     (void);
void BABL_SIMD_SUFFIX(babl_base_type_u16)    (void);
void BABL_SIMD_SUFFIX(babl_base_type_u15)    (void);
void BABL_SIMD_SUFFIX(babl_base_type_u32)    (void);

void BABL_SIMD_SUFFIX(babl_base_model_rgb)   (void);
void BABL_SIMD_SUFFIX(babl_base_model_cmyk)  (void);
void BABL_SIMD_SUFFIX(babl_base_model_gray)  (void);
void BABL_SIMD_SUFFIX(babl_base_model_ycbcr) (void);

#endif
