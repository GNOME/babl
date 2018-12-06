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

if test_bin.found() and git_bin.found()
  if run_command(
      [test_bin.path(), '-d', join_paths(meson.source_root(), '.git')]
    ).returncode() == 0
    # git repo and git is available
    git_version = run_command([git_bin.path(), 'describe', '--always']
      ).stdout().strip()
    git_last_commit_year = run_command([git_bin.path(), 
      'log', '-n1', '--date=format:%Y', '--pretty=%cd']
      ).stdout().strip()
  endif
else
  # Not a git repo so expect git-version.h to exist
  if test_bin.found() and git_bin.found() and
    run_command(
      [test_bin.path(), '-f', 'git-version.h']
    ).returncode() == 0
    git_version_h = 'git-version.h'
  else
    git_version = 'Unknown, shouldn\'t happen'
    git_last_commit_year = 0
  endif
endif
if not is_variable('git_version_h')
  git_conf = configuration_data()
  git_conf.set('BABL_GIT_VERSION', git_version)
  git_conf.set('BABL_GIT_LAST_COMMIT_YEAR', git_last_commit_year)
  git_version_h = configure_file(
    input:  'git-version.h.in',
    output: 'git-version.h',
    configuration: git_conf,
  )
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
  c_args:   [ '-DLIBDIR="' + join_paths(get_option('prefix'), get_option('libdir')) + '"', ],
  cpp_args: [ '-DLIBDIR="' + join_paths(get_option('prefix'), get_option('libdir')) + '"', ],
  link_whole: [ babl_base, ],
  link_args: [ babl_link_args, ],
  dependencies: [ math, thread, dl, lcms, ],
  link_depends: [ version_script_target, ],
  version: so_version,
  install: true,
)
