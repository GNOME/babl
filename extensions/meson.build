extension_names = [
  '16bit',
  'cairo',
  'CIE',
  'fast-float',
  'float-half',
  'float',
  'gegl-fixups',
  'gggl-lies',
  'gggl-table-lies',
  'gggl-table',
  'gggl',
  'gimp-8bit',
  'grey',
  'HCY',
  'HSL',
  'HSV',
  'naive-CMYK',
  'simple',
  'sse-half',
  'sse2-float',
  'sse2-int16',
  'sse2-int8',
  'sse4-int8',
  'two-table',
  'ycbcr',
]

foreach extension_name : extension_names
  extension = library(extension_name,
    extension_name + '.c',
    include_directories: [ rootInclude, bablInclude, ],
    link_with: [ babl, ],
    dependencies: [ math, thread, ],
    name_prefix: '',
    install: true,
    install_dir: join_paths(get_option('libdir'), lib_name),
  )
endforeach

# sse2_float_la_CFLAGS = $(SSE2_EXTRA_CFLAGS)
# sse2_int8_la_CFLAGS = $(SSE2_EXTRA_CFLAGS)
# sse2_int16_la_CFLAGS = $(SSE2_EXTRA_CFLAGS)
# sse4_int8_la_CFLAGS = $(SSE4_1_EXTRA_CFLAGS)
# sse_half_la_CFLAGS = $(SSE4_1_EXTRA_CFLAGS) $(F16C_EXTRA_CFLAGS)
