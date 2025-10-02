subdir('graphics')

xml_insert = find_program(
  xml_insert_file,
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

# Don't build babl ref if cannot run compiled objects in this env
if can_run_host_binaries
  babl_run_env=environment()
  babl_run_env.set('BABL_PATH', babl_extensions_build_dir)
  
  index_html_tmp = custom_target('index.html.tmp',
    env: babl_run_env,
    input : babl_html_dump,
    output:'index.html.tmp',
    command: [babl_html_dump],
    capture: true,
    depends: babl_extensions,
  )
else
  warning('Cannot create babl reference in this environment')
  index_html_tmp = 'index.html.tmp'
endif

TOC = files('toc')
html_files = {
  'index': ['index-static.html.in', [
    ['AUTHORS', authors_file],
    ['TODO',    todo_file],
    ['NEWS',    news_file],
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

# gi-docgen API reference

if build_gi_docgen
  subdir('reference')
endif
