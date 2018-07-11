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

/*
 * x86 bits Copyright (C) Manish Singh <yosh@gimp.org>
 */

/*
 * PPC CPU acceleration detection was taken from DirectFB but seems to be
 * originating from mpeg2dec with the following copyright:
 *
 * Copyright (C) 1999-2001 Aaron Holtzman <aholtzma@ess.engr.uvic.ca>
 */

#include "config.h"
#include <string.h>
#include <signal.h>
#include <setjmp.h>

#include "babl-cpuaccel.h"

typedef unsigned int gboolean;
typedef unsigned int guint32;
typedef int gint;
typedef char gchar;
typedef size_t gsize;
#define TRUE  1
#define FALSE 0
#define G_GNUC_CONST

static BablCpuAccelFlags  cpu_accel (void) G_GNUC_CONST;

static gboolean  use_cpu_accel = TRUE;


/**
 * babl_cpu_accel_get_support:
 *
 * Query for CPU acceleration support.
 *
 * Return value: #BablCpuAccelFlags as supported by the CPU.
 */
BablCpuAccelFlags
babl_cpu_accel_get_support (void)
{
  return use_cpu_accel ? cpu_accel () : BABL_CPU_ACCEL_NONE;
}

/**
 * babl_cpu_accel_set_use:
 * @use:  whether to use CPU acceleration features or not
 *
 * This function is for internal use only.
 */
void
babl_cpu_accel_set_use (gboolean use)
{
  use_cpu_accel = use ? TRUE : FALSE;
}


#if defined(ARCH_X86) && defined(USE_MMX) && defined(__GNUC__)

#define HAVE_ACCEL 1


typedef enum
{
  ARCH_X86_VENDOR_NONE,
  ARCH_X86_VENDOR_INTEL,
  ARCH_X86_VENDOR_AMD,
  ARCH_X86_VENDOR_CENTAUR,
  ARCH_X86_VENDOR_CYRIX,
  ARCH_X86_VENDOR_NSC,
  ARCH_X86_VENDOR_TRANSMETA,
  ARCH_X86_VENDOR_NEXGEN,
  ARCH_X86_VENDOR_RISE,
  ARCH_X86_VENDOR_UMC,
  ARCH_X86_VENDOR_SIS,
  ARCH_X86_VENDOR_UNKNOWN    = 0xff
} X86Vendor;

enum
{
  ARCH_X86_INTEL_FEATURE_MMX      = 1 << 23,
  ARCH_X86_INTEL_FEATURE_XMM      = 1 << 25,
  ARCH_X86_INTEL_FEATURE_XMM2     = 1 << 26,

  ARCH_X86_AMD_FEATURE_MMXEXT     = 1 << 22,
  ARCH_X86_AMD_FEATURE_3DNOW      = 1 << 31,

  ARCH_X86_CENTAUR_FEATURE_MMX    = 1 << 23,
  ARCH_X86_CENTAUR_FEATURE_MMXEXT = 1 << 24,
  ARCH_X86_CENTAUR_FEATURE_3DNOW  = 1 << 31,

  ARCH_X86_CYRIX_FEATURE_MMX      = 1 << 23,
  ARCH_X86_CYRIX_FEATURE_MMXEXT   = 1 << 24
};

enum
{
  ARCH_X86_INTEL_FEATURE_PNI      = 1 << 0,
  ARCH_X86_INTEL_FEATURE_SSSE3    = 1 << 9,
  ARCH_X86_INTEL_FEATURE_SSE4_1   = 1 << 19,
  ARCH_X86_INTEL_FEATURE_SSE4_2   = 1 << 20,
  ARCH_X86_INTEL_FEATURE_AVX      = 1 << 28,
  ARCH_X86_INTEL_FEATURE_F16C     = 1 << 29,
};

#if !defined(ARCH_X86_64) && (defined(PIC) || defined(__PIC__))
#define cpuid(op,eax,ebx,ecx,edx)  \
  __asm__ ("movl %%ebx, %%esi\n\t" \
           "cpuid\n\t"             \
           "xchgl %%ebx,%%esi"     \
           : "=a" (eax),           \
             "=S" (ebx),           \
             "=c" (ecx),           \
             "=d" (edx)            \
           : "0" (op))
#else
#define cpuid(op,eax,ebx,ecx,edx)  \
  __asm__ ("cpuid"                 \
           : "=a" (eax),           \
             "=b" (ebx),           \
             "=c" (ecx),           \
             "=d" (edx)            \
           : "0" (op))
#endif


static X86Vendor
arch_get_vendor (void)
{
  guint32 eax, ebx, ecx, edx;
  guint32 intid[4];
  char *id = (char *) intid;

#ifndef ARCH_X86_64
  /* Only need to check this on ia32 */
  __asm__ ("pushfl\n\t"
           "pushfl\n\t"
           "popl %0\n\t"
           "movl %0,%1\n\t"
           "xorl $0x200000,%0\n\t"
           "pushl %0\n\t"
           "popfl\n\t"
           "pushfl\n\t"
           "popl %0\n\t"
           "popfl"
           : "=a" (eax),
             "=c" (ecx)
           :
           : "cc");

  if (eax == ecx)
    return ARCH_X86_VENDOR_NONE;
#endif

  cpuid (0, eax, ebx, ecx, edx);

  if (eax == 0)
    return ARCH_X86_VENDOR_NONE;

  intid[0] = ebx;
  intid[1] = edx;
  intid[2] = ecx;

  id[12] = '\0';

#ifdef ARCH_X86_64
  if (strcmp (id, "AuthenticAMD") == 0)
    return ARCH_X86_VENDOR_AMD;
  else if (strcmp (id, "GenuineIntel") == 0)
    return ARCH_X86_VENDOR_INTEL;
#else
  if (strcmp (id, "GenuineIntel") == 0)
    return ARCH_X86_VENDOR_INTEL;
  else if (strcmp (id, "AuthenticAMD") == 0)
    return ARCH_X86_VENDOR_AMD;
  else if (strcmp (id, "CentaurHauls") == 0)
    return ARCH_X86_VENDOR_CENTAUR;
  else if (strcmp (id, "CyrixInstead") == 0)
    return ARCH_X86_VENDOR_CYRIX;
  else if (strcmp (id, "Geode by NSC") == 0)
    return ARCH_X86_VENDOR_NSC;
  else if (strcmp (id, "GenuineTMx86") == 0 ||
           strcmp (id, "TransmetaCPU") == 0)
    return ARCH_X86_VENDOR_TRANSMETA;
  else if (strcmp (id, "NexGenDriven") == 0)
    return ARCH_X86_VENDOR_NEXGEN;
  else if (strcmp (id, "RiseRiseRise") == 0)
    return ARCH_X86_VENDOR_RISE;
  else if (strcmp (id, "UMC UMC UMC ") == 0)
    return ARCH_X86_VENDOR_UMC;
  else if (strcmp (id, "SiS SiS SiS ") == 0)
    return ARCH_X86_VENDOR_SIS;
#endif

  return ARCH_X86_VENDOR_UNKNOWN;
}

static guint32
arch_accel_intel (void)
{
  guint32 caps = 0;

#ifdef USE_MMX
  {
    guint32 eax, ebx, ecx, edx;

    cpuid (1, eax, ebx, ecx, edx);

    if ((edx & ARCH_X86_INTEL_FEATURE_MMX) == 0)
      return 0;

    caps = BABL_CPU_ACCEL_X86_MMX;

#ifdef USE_SSE
    if (edx & ARCH_X86_INTEL_FEATURE_XMM)
      caps |= BABL_CPU_ACCEL_X86_SSE | BABL_CPU_ACCEL_X86_MMXEXT;

    if (edx & ARCH_X86_INTEL_FEATURE_XMM2)
      caps |= BABL_CPU_ACCEL_X86_SSE2;

    if (ecx & ARCH_X86_INTEL_FEATURE_PNI)
      caps |= BABL_CPU_ACCEL_X86_SSE3;

    if (ecx & ARCH_X86_INTEL_FEATURE_SSSE3)
      caps |= BABL_CPU_ACCEL_X86_SSSE3;

    if (ecx & ARCH_X86_INTEL_FEATURE_SSE4_1)
      caps |= BABL_CPU_ACCEL_X86_SSE4_1;

    if (ecx & ARCH_X86_INTEL_FEATURE_F16C)
      caps |= BABL_CPU_ACCEL_X86_F16C;
#endif /* USE_SSE */
  }
#endif /* USE_MMX */

  return caps;
}

static guint32
arch_accel_amd (void)
{
  guint32 caps;

  caps = arch_accel_intel ();

#ifdef USE_MMX
  {
    guint32 eax, ebx, ecx, edx;

    cpuid (0x80000000, eax, ebx, ecx, edx);

    if (eax < 0x80000001)
      return caps;

#ifdef USE_SSE
    cpuid (0x80000001, eax, ebx, ecx, edx);

    if (edx & ARCH_X86_AMD_FEATURE_3DNOW)
      caps |= BABL_CPU_ACCEL_X86_3DNOW;

    if (edx & ARCH_X86_AMD_FEATURE_MMXEXT)
      caps |= BABL_CPU_ACCEL_X86_MMXEXT;
#endif /* USE_SSE */
  }
#endif /* USE_MMX */

  return caps;
}

static guint32
arch_accel_centaur (void)
{
  guint32 caps;

  caps = arch_accel_intel ();

#ifdef USE_MMX
  {
    guint32 eax, ebx, ecx, edx;

    cpuid (0x80000000, eax, ebx, ecx, edx);

    if (eax < 0x80000001)
      return caps;

    cpuid (0x80000001, eax, ebx, ecx, edx);

    if (edx & ARCH_X86_CENTAUR_FEATURE_MMX)
      caps |= BABL_CPU_ACCEL_X86_MMX;

#ifdef USE_SSE
    if (edx & ARCH_X86_CENTAUR_FEATURE_3DNOW)
      caps |= BABL_CPU_ACCEL_X86_3DNOW;

    if (edx & ARCH_X86_CENTAUR_FEATURE_MMXEXT)
      caps |= BABL_CPU_ACCEL_X86_MMXEXT;
#endif /* USE_SSE */
  }
#endif /* USE_MMX */

  return caps;
}

static guint32
arch_accel_cyrix (void)
{
  guint32 caps;

  caps = arch_accel_intel ();

#ifdef USE_MMX
  {
    guint32 eax, ebx, ecx, edx;

    cpuid (0, eax, ebx, ecx, edx);

    if (eax != 2)
      return caps;

    cpuid (0x80000001, eax, ebx, ecx, edx);

    if (edx & ARCH_X86_CYRIX_FEATURE_MMX)
      caps |= BABL_CPU_ACCEL_X86_MMX;

#ifdef USE_SSE
    if (edx & ARCH_X86_CYRIX_FEATURE_MMXEXT)
      caps |= BABL_CPU_ACCEL_X86_MMXEXT;
#endif /* USE_SSE */
  }
#endif /* USE_MMX */

  return caps;
}

#ifdef USE_SSE
static jmp_buf sigill_return;

static void
sigill_handler (gint n)
{
  longjmp (sigill_return, 1);
}

static gboolean
arch_accel_sse_os_support (void)
{
  if (setjmp (sigill_return))
    {
      return FALSE;
    }
  else
    {
      signal (SIGILL, sigill_handler);
      __asm__ __volatile__ ("xorps %xmm0, %xmm0");
      signal (SIGILL, SIG_DFL);
    }

  return TRUE;
}
#endif /* USE_SSE */

static guint32
arch_accel (void)
{
  guint32 caps;
  X86Vendor vendor;

  vendor = arch_get_vendor ();

  switch (vendor)
    {
    case ARCH_X86_VENDOR_NONE:
      caps = 0;
      break;

    case ARCH_X86_VENDOR_AMD:
      caps = arch_accel_amd ();
      break;

    case ARCH_X86_VENDOR_CENTAUR:
      caps = arch_accel_centaur ();
      break;

    case ARCH_X86_VENDOR_CYRIX:
    case ARCH_X86_VENDOR_NSC:
      caps = arch_accel_cyrix ();
      break;

    /* check for what Intel speced, even if UNKNOWN */
    default:
      caps = arch_accel_intel ();
      break;
    }

#ifdef USE_SSE
  if ((caps & BABL_CPU_ACCEL_X86_SSE) && !arch_accel_sse_os_support ())
    caps &= ~(BABL_CPU_ACCEL_X86_SSE   |
              BABL_CPU_ACCEL_X86_SSE2  |
              BABL_CPU_ACCEL_X86_SSE3  |
              BABL_CPU_ACCEL_X86_SSSE3 |
              BABL_CPU_ACCEL_X86_SSE4_1);
#endif

  return caps;
}

#endif /* ARCH_X86 && USE_MMX && __GNUC__ */


#if defined(ARCH_PPC) && defined (USE_ALTIVEC)

#if defined(HAVE_ALTIVEC_SYSCTL)

#include <sys/sysctl.h>

#define HAVE_ACCEL 1

static guint32
arch_accel (void)
{
  gint     sels[2] = { CTL_HW, HW_VECTORUNIT };
  gboolean has_vu  = FALSE;
  gsize    length  = sizeof(has_vu);
  gint     err;

  err = sysctl (sels, 2, &has_vu, &length, NULL, 0);

  if (err == 0 && has_vu)
    return BABL_CPU_ACCEL_PPC_ALTIVEC;

  return 0;
}

#elif defined(__GNUC__)

#define HAVE_ACCEL 1

static          sigjmp_buf   jmpbuf;
static volatile sig_atomic_t canjump = 0;

static void
sigill_handler (gint sig)
{
  if (!canjump)
    {
      signal (sig, SIG_DFL);
      raise (sig);
    }

  canjump = 0;
  siglongjmp (jmpbuf, 1);
}

static guint32
arch_accel (void)
{
  signal (SIGILL, sigill_handler);

  if (sigsetjmp (jmpbuf, 1))
    {
      signal (SIGILL, SIG_DFL);
      return 0;
    }

  canjump = 1;

  asm volatile ("mtspr 256, %0\n\t"
                "vand %%v0, %%v0, %%v0"
                :
                : "r" (-1));

  signal (SIGILL, SIG_DFL);

  return BABL_CPU_ACCEL_PPC_ALTIVEC;
}
#endif /* __GNUC__ */

#endif /* ARCH_PPC && USE_ALTIVEC */


static BablCpuAccelFlags
cpu_accel (void)
{
#ifdef HAVE_ACCEL
  static guint32 accel = ~0U;

  if (accel != ~0U)
    return accel;

  accel = arch_accel ();

#if defined(ARCH_X86_64)
  accel |= BABL_CPU_ACCEL_X86_64;
#endif

  return (BablCpuAccelFlags) accel;

#else /* !HAVE_ACCEL */
  return BABL_CPU_ACCEL_NONE;
#endif
}
