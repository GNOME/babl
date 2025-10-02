# Extra markdown files
babl_doc_content_files = [
]

babl_doc_toml = configure_file(
  input: 'babl.toml.in',
  output: '@BASENAME@',
  configuration: {
    'BABL_VERSION':   version,
    'BABL_LOGO_PATH': 'https://gegl.org/babl/graphics/babl-a4poster.png',
  },
)

babl_docs = custom_target('babl-docs',
  input: babl_gir[0],
  output: lib_name,
  command: [
    gi_docgen,
    'generate',
    '--quiet',
    '--fatal-warnings',
    '--config', babl_doc_toml,
    '--output-dir=@OUTPUT@',
    '--no-namespace-dir',
    '--content-dir=@0@'.format(meson.current_source_dir()),
    '@INPUT@',
  ],
  depend_files: [
    babl_doc_toml,
    babl_doc_content_files,
  ],
  build_by_default: true,
  install: true,
  install_dir: get_option('datadir') / 'doc',
  install_tag: 'docs'
)
