bablInclude = include_directories('.')
subdir('base')

version_script = 'babl.map'
export_symbols = join_paths(meson.source_root(), 'export-symbols')
version_script_target = custom_target(version_script,
  input : [ export_symbols, ] ,
  output: [ version_script ] ,
  command: [
  'python',
  join_paths(meson.source_root(), 'gen_babl_map.py'),
  export_symbols,
  version_script,
  ],
  capture: true,
)

# c compiler arguments
babl_c_args = [
  sse2_cflags,
  '-DLIBDIR="@0@"'.format(babl_libdir),
]

# Linker arguments
babl_link_args = [
  '-Wl,--version-script,' + version_script,
]
if platform_win32
  babl_link_args += '-Wl,--no-undefined'
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

is_git_repo = (
  test_bin.found() and
  git_bin.found() and
  run_command(test_bin, '-d', meson.source_root() / '.git').returncode() == 0
)

if is_git_repo
  git_version_h = vcs_tag(
    input : 'git-version.h.in',
    output: 'git-version.h',
    replace_string: '@BABL_GIT_VERSION@',
    command: [ git_bin.path(), 'describe', '--always' ],
  )

  meson.add_dist_script(
    [ 'ninja', 'babl/git-version.h', ],
  )
  meson.add_dist_script(
    [ 'sh', '-c', ' '.join(
    [ 'cp', git_version_h.full_path(), '${MESON_DIST_ROOT}/babl' ]
    )]
  )

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
  include_directories: [ rootInclude, bablBaseInclude],
  c_args: babl_c_args,
  link_whole: [ babl_base, ],
  link_args: [ babl_link_args, ],
  dependencies: [ math, thread, dl, lcms, ],
  link_depends: [ version_script_target, ],
  version: so_version,
  install: true,
)
