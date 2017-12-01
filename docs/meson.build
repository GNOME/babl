subdir('graphics')

host    = 'pippin.gimp.org'
location= 'public_html/babl'
scptarget = host + ':' + location + '/'


babl_css = files('babl.css')

xml_insert = find_program(join_paths('tools', 'xml_insert.sh'))

index_static_html = configure_file(
  input : 'index-static.html.in',
  output: 'index-static.html',
  configuration: conf,
)

index_html_tmp = custom_target('index.html.tmp',
  input : [ babl_html_dump, ],
  output: [ 'index.html.tmp', ],
  command: [ 'bash', '-c',
    'BABL_PATH="'+ join_paths(meson.build_root(), 'extensions') +'" '+ babl_html_dump.full_path()
  ],
  capture: true,
)

index_html = custom_target('index.html',
  input : [
    index_html_tmp,
    index_static_html,
    join_paths(meson.source_root(), 'AUTHORS'),
    join_paths(meson.source_root(), 'TODO'),
    join_paths(meson.source_root(), 'NEWS'),
  ],
  output: [ 'index.html', ],
  command: [ 'bash', '-c',
    'cp '+ '@0@'.format(index_static_html) +' @OUTPUT@'
    +' && '+ xml_insert.path() +' @OUTPUT@ BablBase '+ index_html_tmp.full_path()
    +' && '+ xml_insert.path() +' @OUTPUT@ AUTHORS  '+ join_paths(meson.source_root(), 'AUTHORS')
    +' && '+ xml_insert.path() +' @OUTPUT@ TODO     '+ join_paths(meson.source_root(), 'TODO')
    +' && '+ xml_insert.path() +' @OUTPUT@ NEWS     '+ join_paths(meson.source_root(), 'NEWS')
  ],
)


run_target('push_web',
  command: [
    'scp', index_html, index_static_html, babl_css, scptarget,
    '&&',
    'scp', graphic_files_install, scptarget + 'graphics/'
  ],
)
