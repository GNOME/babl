babl_extensions_build_dir = meson.current_build_dir()

no_cflags = []

# Dependencies
babl_ext_dep = [
  math,
  thread,
  lcms,
]

# Include directories
babl_ext_inc = [
  rootInclude,
  bablInclude,
]

# Linker arguments
babl_ext_link_args = [
]
if platform_win32
  babl_ext_link_args += no_undefined
  if cc.get_argument_syntax() == 'msvc'
    babl_ext_link_args += '/EXPORT:init'
  endif
endif

autosimd_extensions = [
  ['u16', no_cflags],
  ['u32', no_cflags],
  ['cairo', no_cflags],
  ['grey', no_cflags],
  ['gggl', no_cflags],
  ['gggl-lies', no_cflags],
  ['gegl-fixups', no_cflags],
  ['CIE', sse2_cflags],
  ['float', no_cflags],
  ['double', no_cflags],
  ['simple', no_cflags],
  ['ycbcr', no_cflags],
]

extensions = [
  ['u16', no_cflags],
  ['u32', no_cflags],
  ['cairo', no_cflags],
  ['oklab', no_cflags],
  ['CIE', sse2_cflags],
  ['double', no_cflags],
  ['fast-float', no_cflags],
  ['half', no_cflags],
  ['float', no_cflags],
  ['formats', no_cflags],
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

babl_extensions = []
foreach ext : extensions
  babl_extensions += shared_module(
    ext[0],
    ext[0] + '.c',
    c_args: [ext[1], '-DBABL_SIMDFREE' ],
    include_directories: babl_ext_inc,
    link_with: babl,
    link_args: babl_ext_link_args,
    dependencies: babl_ext_dep,
    name_prefix: '',
    install: true,
    install_dir: babl_libdir / lib_name,
  )
endforeach

if host_cpu_family == 'x86_64'
 
  foreach ext : autosimd_extensions
    shared_module(
      'x86-64-v2-' + ext[0],
      ext[0] + '.c',
      c_args: [ext[1]] + x86_64_v2_flags,
      include_directories: babl_ext_inc,
      link_with: babl,
      link_args: babl_ext_link_args,
      dependencies: babl_ext_dep,
      name_prefix: '',
      install: true,
      install_dir: babl_libdir / lib_name,
   )
  endforeach

  foreach ext : autosimd_extensions
    shared_module(
      'x86-64-v3-' + ext[0],
      ext[0] + '.c',
      c_args: [ext[1]] + x86_64_v3_flags,
      include_directories: babl_ext_inc,
      link_with: babl,
      link_args: babl_ext_link_args,
      dependencies: babl_ext_dep,
      name_prefix: '',
      install: true,
      install_dir: babl_libdir / lib_name,
   )
  endforeach

elif host_cpu_family == 'arm'
  

  foreach ext : autosimd_extensions
    shared_module(
      'arm-neon-' + ext[0],
      ext[0] + '.c',
      c_args: [ext[1]] + arm_neon_flags,
      include_directories: babl_ext_inc,
      link_with: babl,
      link_args: babl_ext_link_args,
      dependencies: babl_ext_dep,
      name_prefix: '',
      install: true,
      install_dir: babl_libdir / lib_name,
   )
  endforeach

endif
