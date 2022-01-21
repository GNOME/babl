/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2005-2008, Øyvind Kolås and others.
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

#ifndef _BABL_CPU_ACCEL_H
#define _BABL_CPU_ACCEL_H

typedef enum
{
  BABL_CPU_ACCEL_NONE        = 0x0,

  /* x86 accelerations */
  BABL_CPU_ACCEL_X86_MMX     = 0x80000000,
  BABL_CPU_ACCEL_X86_3DNOW   = 0x40000000,
  BABL_CPU_ACCEL_X86_MMXEXT  = 0x20000000,
  BABL_CPU_ACCEL_X86_SSE     = 0x10000000,
  BABL_CPU_ACCEL_X86_SSE2    = 0x08000000,
  BABL_CPU_ACCEL_X86_SSE3    = 0x04000000,
  BABL_CPU_ACCEL_X86_SSSE3   = 0x02000000,
  BABL_CPU_ACCEL_X86_SSE4_1  = 0x01000000,
  BABL_CPU_ACCEL_X86_SSE4_2  = 0x00800000,
  BABL_CPU_ACCEL_X86_AVX     = 0x00400000,
  BABL_CPU_ACCEL_X86_POPCNT  = 0x00200000,
  BABL_CPU_ACCEL_X86_FMA     = 0x00100000,
  BABL_CPU_ACCEL_X86_MOVBE   = 0x00080000,
  BABL_CPU_ACCEL_X86_F16C    = 0x00040000,
  BABL_CPU_ACCEL_X86_XSAVE   = 0x00020000,
  BABL_CPU_ACCEL_X86_OSXSAVE = 0x00010000,
  BABL_CPU_ACCEL_X86_BMI1    = 0x00008000,
  BABL_CPU_ACCEL_X86_BMI2    = 0x00004000,
  BABL_CPU_ACCEL_X86_AVX2    = 0x00002000,

  BABL_CPU_ACCEL_X86_64_V2 =
    (BABL_CPU_ACCEL_X86_POPCNT|
     BABL_CPU_ACCEL_X86_SSE4_1|
     BABL_CPU_ACCEL_X86_SSE4_2|
     BABL_CPU_ACCEL_X86_SSSE3),

  BABL_CPU_ACCEL_X86_64_V3 =
    (BABL_CPU_ACCEL_X86_64_V2|
     BABL_CPU_ACCEL_X86_BMI1|
     BABL_CPU_ACCEL_X86_BMI2|
     BABL_CPU_ACCEL_X86_AVX|
     BABL_CPU_ACCEL_X86_FMA|
     BABL_CPU_ACCEL_X86_F16C|
     BABL_CPU_ACCEL_X86_AVX2|
     BABL_CPU_ACCEL_X86_OSXSAVE|
     BABL_CPU_ACCEL_X86_MOVBE),

  /* powerpc accelerations */
  BABL_CPU_ACCEL_PPC_ALTIVEC = 0x00000010,

  /* arm accelerations */
  BABL_CPU_ACCEL_ARM_NEON    = 0x00000020,

  /* x86_64 arch */
  BABL_CPU_ACCEL_X86_64      = 0x00000040
} BablCpuAccelFlags;



BablCpuAccelFlags  babl_cpu_accel_get_support (void);
void               babl_cpu_accel_set_use     (unsigned int use);


#endif  /* _BABL_CPU_ACCEL_H */
