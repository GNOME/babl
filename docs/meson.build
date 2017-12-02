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
  command: [ find_program('env'),
    'BABL_PATH='+ join_paths(meson.build_root(), 'extensions'),
    babl_html_dump
  ],
  capture: true,
)

index_html = custom_target('index.html',
  input : [
    index_static_html,
    index_html_tmp,
    join_paths(meson.source_root(), 'AUTHORS'),
    join_paths(meson.source_root(), 'TODO'),
    join_paths(meson.source_root(), 'NEWS'),
  ],
  output: [ 'index.html', ],
  command: [ find_program('env'),
    'cp', '@INPUT0@', '@OUTPUT@',
    '&&', xml_insert, '@OUTPUT@', 'BablBase', '@INPUT1@',
    '&&', xml_insert, '@OUTPUT@', 'AUTHORS',  '@INPUT2@',
    '&&', xml_insert, '@OUTPUT@', 'TODO',     '@INPUT3@',
    '&&', xml_insert, '@OUTPUT@', 'NEWS',     '@INPUT4@',
  ],
)


run_target('push_web',
  command: [
    'scp', index_html, index_static_html, babl_css, scptarget,
    '&&',
    'scp', graphic_files_install, scptarget + 'graphics/'
  ],
)
