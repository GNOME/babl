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
  include_directories: [ rootInclude, bablInclude, ],
  dependencies: [ math, lcms],
)
