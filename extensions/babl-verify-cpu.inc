#include "babl-cpuaccel.h"

#ifdef X86_64_V2
#define BABL_SIMD_x86_64_v2
#define BABL_SIMD_SUFFIX(symbol) symbol##_x86_64_v2
#else
#ifdef X86_64_V3
#define BABL_SIMD_x86_64_v3
#define BABL_SIMD_SUFFIX(symbol) symbol##_x86_64_v3
#else
#define BABL_SIMD_generic
#define BABL_SIMD_SUFFIX(symbol) symbol##_generic
#endif
#endif

#define BABL_VERIFY_CPU()  do{}while(0)


#ifdef BABL_SIMDFREE

#ifdef ARCH_X86_64
  
#undef BABL_VERIFY_CPU
#define BABL_VERIFY_CPU()\
  if ((babl_cpu_accel_get_support() & BABL_CPU_ACCEL_X86_64_V2)\
                                       == BABL_CPU_ACCEL_X86_64_V2) return 0;\
  if ((babl_cpu_accel_get_support() & BABL_CPU_ACCEL_X86_64_V3)\
                                       == BABL_CPU_ACCEL_X86_64_V3) return 0;

#endif

#ifdef ARCH_ARM
#undef BABL_VERIFY_CPU
#define BABL_VERIFY_CPU()\
  if ((babl_cpu_accel_get_support() & BABL_CPU_ACCEL_ARM_NEON)\
                                       == BABL_CPU_ACCEL_ARM_NEON) return 0;

#endif
#endif

#ifdef X86_64_V2
#undef BABL_VERIFY_CPU
#define BABL_VERIFY_CPU()\
  if ((babl_cpu_accel_get_support() & BABL_CPU_ACCEL_X86_64_V2)\
                                       != BABL_CPU_ACCEL_X86_64_V2) return 0;\
  if ((babl_cpu_accel_get_support() & BABL_CPU_ACCEL_X86_64_V3)\
                                       == BABL_CPU_ACCEL_X86_64_V3) return 0;
#endif

#ifdef X86_64_V3
#undef BABL_VERIFY_CPU
#define BABL_VERIFY_CPU()\
  if ((babl_cpu_accel_get_support() & BABL_CPU_ACCEL_X86_64_V3)\
                                       != BABL_CPU_ACCEL_X86_64_V3) return 0;
#endif


#ifdef ARM_NEON
#undef BABL_VERIFY_CPU
#define BABL_VERIFY_CPU()\
  if ((babl_cpu_accel_get_support() & BABL_CPU_ACCEL_ARM_NEON)\
                                       != BABL_CPU_ACCEL_ARM_NEON) return 0;
#endif
