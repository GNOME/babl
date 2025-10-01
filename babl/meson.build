
babl_library_build_dir = meson.current_build_dir()
bablInclude = include_directories('.')

subdir('base')


# c compiler arguments
babl_c_args = [
  sse2_cflags,
  '-DLIBDIR="@0@"'.format(babl_libdir),
]

# symbol maps
version_script = custom_target('babl.map',
  input : export_symbols_file,
  output: ['babl.map', 'babl.map.clang'],
  command: [
  find_program(gen_babl_map_file),
  '@INPUT@',
  '@OUTPUT0@',
  ],
)

# Linker arguments
if cc.links('', 
    name: '-Wl,--version-script', 
    args: ['-shared', '-Wl,--version-script=' 
      + meson.current_source_dir() / 'test-gnu.map']
  ) and cc.get_argument_syntax() != 'msvc'
  babl_link_args = [
    '-Wl,--version-script=' + version_script[0].full_path()
  ]
elif cc.get_id() == 'clang'
  if cc.links('', 
      name: '-Wl,-exported_symbols_list', 
      args: ['-Wl,-exported_symbols_list', 
        meson.current_source_dir() / 'test-clang.map']
    )
    # Clang on Darwin
    babl_link_args = [
      '-Wl,-exported_symbols_list', version_script[1].full_path()
    ]
  else  
    # Clang on msys/mingw
    babl_link_args = []
  endif
elif cc.get_argument_syntax() == 'msvc'
  babl_link_args = [
    '/VERSION:' + api_version
  ]
else
  error(
    'Linker doesn\'t support --version-script or -exported_symbols_list'
  )
endif
if platform_win32
  babl_link_args += no_undefined
endif

# sources
babl_version_h = configure_file(
  input:  'babl-version.h.in',
  output: 'babl-version.h',
  configuration: conf,
)

babl_sources = files(
  'babl-cache.c',
  'babl-component.c',
  'babl-conversion.c',
  'babl-core.c',
  'babl-cpuaccel.c',
  'babl-db.c',
  'babl-extension.c',
  'babl-fish-path.c',
  'babl-fish-reference.c',
  'babl-fish-simple.c',
  'babl-fish.c',
  'babl-format.c',
  'babl-hash-table.c',
  'babl-icc.c',
  'babl-image.c',
  'babl-internal.c',
  'babl-introspect.c',
  'babl-list.c',
  'babl-memory.c',
  'babl-model.c',
  'babl-mutex.c',
  'babl-palette.c',
  'babl-polynomial.c',
  'babl-ref-pixels.c',
  'babl-sampling.c',
  'babl-sanity.c',
  'babl-space.c',
  'babl-type.c',
  'babl-shared-util.c',
  'babl-util.c',
  'babl-version.c',
  'babl.c',
) + [
  babl_version_h,
  git_version_h,
]

babl_headers = files(
  'babl-introspect.h',
  'babl-macros.h',
  'babl-types.h',
  'babl.h',
) + [
  babl_version_h,
]

install_headers(babl_headers,
  subdir: lib_name / 'babl'
)
# copy external headers to babl subdirectory for subproject builds as
# we don't want to expose the project root folder due to potential
# name clashes.
if meson.is_subproject()
  subdir('babl')
endif

babl_deps = [math, thread, dl, lcms, log]
babl_includes = [rootInclude, bablBaseInclude]

if host_cpu_family == 'x86_64'
  simd_extra = [babl_base_x86_64_v2, babl_base_x86_64_v3]
elif host_cpu_family == 'arm'
  simd_extra = [babl_base_arm_neon]
else
  simd_extra = []
endif

# build library
babl = library(
  lib_name,
  babl_sources,
  include_directories: babl_includes,
  c_args: babl_c_args,
  vs_module_defs: meson.project_source_root() / 'export-symbols.def',
  link_whole: babl_base,
  link_args: babl_link_args,
  link_with: simd_extra,
  dependencies: babl_deps,
  link_depends: version_script[0],
  version: so_version,
  install: true,
)

if build_gir
  # identity filter, so GIR doesn't choke on the Babl type
  # (since it has the same name as the Babl namespace)
  babl_gir = gnome.generate_gir(
    babl,
    sources: babl_headers,
    extra_args: [
      '--identifier-filter-cmd=@0@ @1@'.format(python.full_path(), 
      '"' + meson.current_source_dir() / 'identfilter.py' + '"'),
      '-DBABL_IS_BEING_COMPILED',
      '--quiet',
    ],
    namespace: 'Babl',
    nsversion: api_version,
    header: 'babl.h',
    export_packages: 'babl-0.1',
    install: true,
  )

  if build_vapi
    babl_vapi = gnome.generate_vapi(lib_name,
      sources: babl_gir[0],
      install: true,
    )
  endif
else
  babl_gir = []
endif
