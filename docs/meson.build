subdir('graphics')

host    = 'pippin.gimp.org'
location= 'public_html/babl'
scptarget = host + ':' + location + '/'


xml_insert = find_program(
  meson.source_root() / 'tools' / 'xml-insert.py',
  native: true
)

index_static_html = configure_file(
  input : 'index-static.html.in',
  output: 'index-static.html',
  configuration: conf,
)

babl_css = configure_file(
  input : 'babl.css',
  output : 'babl.css',
  copy: true
)

index_html_tmp_env = [
  'BABL_PATH=' + meson.build_root() / 'extensions',
]

# Don't build babl ref if cannot run compiled objects in this env
if env_bin.found() and cc_can_run # and not meson.is_cross_build()
  index_html_tmp = custom_target('index.html.tmp',
    input : [ babl_html_dump, ],
    output: [ 'index.html.tmp', ],
    command: [
      env_bin,
      index_html_tmp_env,
      babl_html_dump
    ],
    capture: true,
  )
else
  warning('Cannot create babl reference in this environment')
  index_html_tmp = 'index.html.tmp'
endif

TOC = files('toc')
html_files = {
  'index': [index_static_html, [
    ['AUTHORS', files(meson.source_root() / 'AUTHORS')],
    ['TODO',    files(meson.source_root() / 'TODO')],
    ['NEWS',    files(meson.source_root() / 'NEWS')],
    ['TOC',     TOC],
  ]],
  'Reference': ['auto', [
    ['BablBase', index_html_tmp],
    ['TOC',      TOC],
  ]],
  'CMYK': ['auto', [
    ['TOC', TOC]
  ]],
  'OldNews': ['auto', [
    ['TOC', TOC],
  ]],
  'Glossary': ['auto', [
    ['TOC', TOC],
  ]],
  'ColorManagement': ['auto', [
    ['TOC', TOC],
  ]],
  'UnifiedAlpha': ['auto', [
    ['TOC', TOC],
  ]],
}

foreach _file, _parms : html_files
  if '@0@'.format(_parms[0]) == 'auto'
    _in = '@0@-static.html'.format(_file)
  else
    _in = _parms[0]
  endif
  _out = '@0@.html'.format(_file)

  inserts = []
  foreach _parm : _parms[1]
    inserts += ['--insert', _parm[0], _parm[1]]
  endforeach

  _tgt = custom_target(_out,
    input: _in,
    output: _out,
    command: [
      xml_insert,
      '--output' , '@OUTPUT@',
      inserts,
      '@INPUT@',
    ],
    build_by_default: true
  )
  if _file == 'index'
    index_html = _tgt
  endif
endforeach


run_target('push_web',
  command: [
    'scp', index_html, index_static_html, babl_css, scptarget,
    '&&',
    'scp', graphic_files_install, scptarget + 'graphics/'
  ],
)
