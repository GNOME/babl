babl_sources = files(
  'babl.c',
  '../babl/babl-util.c'
)

babl_bin = executable('babl',
  babl_sources,
  include_directories: [ rootInclude, bablInclude ],
  link_with: babl,
  install: true,
)
