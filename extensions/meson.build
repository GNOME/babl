babl_extensions_build_dir = meson.current_build_dir()

no_cflags = []

# Dependencies
babl_ext_dep = [
  math,
  thread,
]

# Include directories
babl_ext_inc = [
  rootInclude,
  bablInclude,
]

# Linker arguments
babl_ext_link_args = [
]
if platform_win32 and cc.has_link_argument('-Wl,--no-undefined')
  babl_ext_link_args += '-Wl,--no-undefined'
endif


extensions = [
  ['u16', no_cflags],
  ['u32', no_cflags],
  ['cairo', no_cflags],
  ['CIE', sse2_cflags],
  ['double', no_cflags],
  ['fast-float', no_cflags],
  ['half', no_cflags],
  ['float', no_cflags],
  ['gegl-fixups', no_cflags],
  ['gggl-lies', no_cflags],
  ['gggl-table-lies', no_cflags],
  ['gggl-table', no_cflags],
  ['gggl', no_cflags],
  ['gimp-8bit', no_cflags],
  ['grey', no_cflags],
  ['HCY', no_cflags],
  ['HSL', no_cflags],
  ['HSV', no_cflags],
  ['naive-CMYK', no_cflags],
  ['simple', no_cflags],
  ['sse-half', [sse4_1_cflags, f16c_cflags]],
  ['sse2-float', sse2_cflags],
  ['sse2-int16', sse2_cflags],
  ['sse2-int8', sse2_cflags],
  ['sse4-int8', sse4_1_cflags],
  ['avx2-int8', avx2_cflags],
  ['two-table', sse2_cflags],
  ['ycbcr', sse2_cflags],
]

foreach ext : extensions
  library(
    ext[0],
    ext[0] + '.c',
    c_args: ext[1],
    include_directories: babl_ext_inc,
    link_with: babl,
    link_args: babl_ext_link_args,
    dependencies: babl_ext_dep,
    name_prefix: '',
    install: true,
    install_dir: babl_libdir / lib_name,
  )
endforeach
