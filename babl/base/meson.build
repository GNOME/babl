bablBaseInclude = include_directories('.')

babl_base_sources = [
  'babl-base.c',
  'formats.c',
  'model-cmyk.c',
  'model-gray.c',
  'model-rgb.c',
  'model-ycbcr.c',
  'pow-24.c',
  'type-float.c',
  'type-half.c',
  'type-u15.c',
  'type-u16.c',
  'type-u32.c',
  'type-u8.c',
  'babl-trc.c',
  'babl-rgb-converter.c',
]

babl_base = static_library('babl_base',
  babl_base_sources,
  include_directories: [rootInclude, bablInclude],
  dependencies: [math, lcms, log],
   c_args: common_c_flags + [sse2_cflags]
) 

if host_cpu_family == 'x86_64'

  babl_base_x86_64_v2 = static_library('babl_base-x86-64-v2',
    babl_base_sources,
    include_directories: [rootInclude, bablInclude],
    dependencies: [math, lcms, log],
    c_args: common_c_flags + x86_64_v2_flags
  )

  babl_base_x86_64_v3 = static_library('babl_base-x86-64-v3',
    babl_base_sources,
    include_directories: [rootInclude, bablInclude],
    dependencies: [math, lcms, log],
    c_args: common_c_flags + x86_64_v3_flags
  )

endif


if host_cpu_family == 'arm'

  babl_base_arm_neon = static_library('babl_base-arm-neon',
    babl_base_sources,
    include_directories: [rootInclude, bablInclude],
    dependencies: [math, lcms, log],
    c_args: common_c_flags + arm_neon_flags
  )

endif
