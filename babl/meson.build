bablInclude = include_directories('.')
subdir('base')

babl_version_h = configure_file(
  input : 'babl-version.h.in',
  output: 'babl-version.h',
  configuration: conf,
)

gitversion_h1 = vcs_tag(
  input : 'git-version.h.in',
  output: 'git-version.h.in.1',
  command: [ 'git', 'describe', '--always', ],
  replace_string: '@BABL_GIT_VERSION@',
  fallback: '',
)
gitversion_h2 = vcs_tag(
  input : gitversion_h1,
  output: 'git-version.h.in.2',
  command: [ 'git', 'rev-parse', '--short', 'HEAD', ],
  replace_string: '@BABL_GIT_VERSION_ABBREV@',
  fallback: '',
)
gitversion_h = vcs_tag(
  input : gitversion_h2,
  output: 'git-version.h',
  command: [ 'git', 'log', '-n1', '--date=format:%Y', '--pretty=%cd', ],
  replace_string: '@BABL_GIT_LAST_COMMIT_YEAR@',
  fallback: '',
)

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
  gitversion_h,
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

babl = library(lib_name,
  babl_sources,
  include_directories: [ rootInclude, bablBaseInclude],
  c_args:   [ '-DLIBDIR="' + get_option('libdir') + '"', ],
  cpp_args: [ '-DLIBDIR="' + get_option('libdir') + '"', ],
  link_with: [ babl_base, ],
  link_args: [ '-Wl,--version-script,' + version_script, ],
  dependencies: [ math, thread, dl, ],
  install: true,
  version: so_version,
)
