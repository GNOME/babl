subdir('graphics')

host    = 'pippin.gimp.org'
location= 'public_html/babl'
scptarget = host + ':' + location + '/'


xml_insert = find_program(join_paths('tools', 'xml_insert.sh'))

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
  'BABL_PATH='+ join_paths(meson.build_root(), 'extensions'),
]

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

index_html = custom_target('index.html',
  input : [
    index_static_html,
    join_paths(meson.source_root(), 'AUTHORS'),
    join_paths(meson.source_root(), 'TODO'),
    join_paths(meson.source_root(), 'NEWS'),
    'toc',
  ],
  output: [ 'index.html', ],
  command: [
    env_bin,
    'cp', '@INPUT0@', '@OUTPUT@',
    '&&', xml_insert, '@OUTPUT@', 'AUTHORS',  '@INPUT1@',
    '&&', xml_insert, '@OUTPUT@', 'TODO',     '@INPUT2@',
    '&&', xml_insert, '@OUTPUT@', 'NEWS',     '@INPUT3@',
    '&&', xml_insert, '@OUTPUT@', 'TOC',     '@INPUT4@',
  ],
  build_by_default: true,
)

Reference_html = custom_target('Reference.html',
  input : [
    'Reference-static.html',
    'toc',
    index_html_tmp,
  ],
  output: [ 'Reference.html', ],
  command: [
    env_bin,
    'cp', '@INPUT0@', '@OUTPUT@',
    '&&', xml_insert, '@OUTPUT@', 'TOC', '@INPUT1@',
    '&&', xml_insert, '@OUTPUT@', 'BablBase', '@INPUT2@',
  ],
  build_by_default: true,
)


CMYK_html = custom_target('CMYK.html',
  input : [
    'CMYK-static.html',
    'toc',
  ],
  output: [ 'CMYK.html', ],
  command: [
    env_bin,
    'cp', '@INPUT0@', '@OUTPUT@',
    '&&', xml_insert, '@OUTPUT@', 'TOC', '@INPUT1@',
  ],
  build_by_default: true,
)


Glossary_html = custom_target('Glossary.html',
  input : [
    'Glossary-static.html',
    'toc',
  ],
  output: [ 'Glossary.html', ],
  command: [
    env_bin,
    'cp', '@INPUT0@', '@OUTPUT@',
    '&&', xml_insert, '@OUTPUT@', 'TOC', '@INPUT1@',
  ],
  build_by_default: true,
)


ColorManagement_html = custom_target('ColorManagement.html',
  input : [
    'ColorManagement-static.html',
    'toc',
  ],
  output: [ 'ColorManagement.html', ],
  command: [
    env_bin,
    'cp', '@INPUT0@', '@OUTPUT@',
    '&&', xml_insert, '@OUTPUT@', 'TOC',     '@INPUT1@',
  ],
  build_by_default: true,
)

SymmetricAlpha_html = custom_target('SymmetricAlpha.html',
  input : [
    'SymmetricAlpha-static.html',
    'toc',
  ],
  output: [ 'SymmetricAlpha.html', ],
  command: [
    env_bin,
    'cp', '@INPUT0@', '@OUTPUT@',
    '&&', xml_insert, '@OUTPUT@', 'TOC',     '@INPUT1@',
  ],
  build_by_default: true,
)


run_target('push_web',
  command: [
    'scp', index_html, index_static_html, babl_css, scptarget,
    '&&',
    'scp', graphic_files_install, scptarget + 'graphics/'
  ],
)
