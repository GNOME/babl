
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
  input : meson.source_root() / 'export-symbols',
  output: ['babl.map', 'babl.map.clang'],
  command: [
  find_program(meson.source_root() / 'gen_babl_map.py'),
  '@INPUT@',
  '@OUTPUT0@',
  ],
)

# Linker arguments
if cc.links('', 
    name: '-Wl,--version-script', 
    args: ['-shared', '-Wl,--version-script=' 
      + meson.current_source_dir() / 'test-gnu.map']
  )
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
else
  error(
    'Linker doesn\'t support --version-script or -exported_symbols_list'
  )
endif
if platform_win32
  babl_link_args += no_undefined
endif

babl_version_h = configure_file(
  input:  'babl-version.h.in',
  output: 'babl-version.h',
  configuration: conf,
)

# If git is available, always check if git-version.h should be
# updated. If git is not available, don't do anything if git-version.h
# already exists because then we are probably working with a tarball
# in which case the git-version.h we ship is correct.
if git_bin.found() and run_command(
    git_bin,
    'rev-parse',
    '--is-inside-work-tree',
).returncode() == 0
  git_version_h = vcs_tag(
    input : 'git-version.h.in',
    output: 'git-version.h',
    replace_string: '@BABL_GIT_VERSION@',
    command: [ git_bin.path(), 'describe', '--always' ],
  )

  if env_bin.found()
    meson.add_dist_script(
      [ 'ninja', 'babl/git-version.h', ],
    )
    meson.add_dist_script(
      [ 'sh', '-c', ' '.join(
      [ 'cp', git_version_h.full_path(), '${MESON_DIST_ROOT}/babl' ]
      )]
    )
  endif
else
  git_version_h = files('git-version.h')
endif

babl_sources = [
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
  'babl-trc.c',
  'babl-type.c',
  'babl-util.c',
  'babl-version.c',
  'babl.c',
  babl_version_h,
  git_version_h,
]

babl_headers = [
  'babl-introspect.h',
  'babl-macros.h',
  'babl-types.h',
  'babl.h',
  babl_version_h,
]

install_headers(babl_headers,
  subdir: join_paths(lib_name, 'babl')
)

babl = library(
  lib_name,
  babl_sources,
  include_directories: [rootInclude, bablBaseInclude],
  c_args: babl_c_args,
  link_whole: babl_base,
  link_args: babl_link_args,
  dependencies: [math, thread, dl, lcms],
  link_depends: version_script,
  version: so_version,
  install: true,
)

if build_gir
  # identity filter, so GIR doesn't choke on the Babl type
  # (since it has the same name as the Babl namespace)
  identfilter_py = join_paths(meson.current_source_dir(), 'identfilter.py')

  babl_gir = gnome.generate_gir(babl,
    sources: babl_headers,
    extra_args: [
      '--identifier-filter-cmd=@0@ @1@'.format(python.path(), identfilter_py),
      '-DBABL_IS_BEING_COMPILED',
    ],
    namespace: 'Babl',
    nsversion: '0.1',
    header: 'babl.h',
    install: true,
  )

  if get_option('enable-vapi')
    gnome.generate_vapi(lib_name,
      sources: babl_gir[0],
      install: true,
    )
  endif
endif
