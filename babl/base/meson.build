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
]

babl_base = static_library('babl_base',
  babl_base_sources,
  include_directories: [rootInclude, bablInclude],
  dependencies: [math, lcms],
   c_args: common_c_flags
)

if host_cpu_family == 'x86_64'

  babl_base_x86_64_v2 = static_library('babl_base-x86-64-v2',
    babl_base_sources,
    include_directories: [rootInclude, bablInclude],
    dependencies: [math, lcms],
    c_args: common_c_flags + x86_64_v2_flags + '-DX86_64_V2'
  )

  babl_base_x86_64_v3 = static_library('babl_base-x86-64-v3',
    babl_base_sources,
    include_directories: [rootInclude, bablInclude],
    dependencies: [math, lcms],
    c_args: common_c_flags + x86_64_v3_flags + '-DX86_64_V3'
  )

endif
