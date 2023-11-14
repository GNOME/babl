babl_sources = files(
  'babl.c',
  '../babl/babl-internal.c',
  '../babl/babl-memory.c',
  '../babl/babl-mutex.c',
  '../babl/babl-util.c',
)

babl_bin = executable('babl',
  babl_sources,
  include_directories: [ rootInclude, bablInclude ],
  dependencies: [ thread ],
  link_with: babl,
  install: true,
)
