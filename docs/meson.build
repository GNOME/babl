subdir('graphics')

host    = 'pippin.gimp.org'
location= 'public_html/babl'
scptarget = host + ':' + location + '/'


# The patterns used for html creation in the babl build system
# is very small, and should probably be documented in a tutorial.

index_static_html = configure_file(
  input : 'index-static.html.in',
  output: 'index-static.html',
  configuration: conf,
)

index_html = custom_target('index.html',
  input : [
    index_static_html,
    '../AUTHORS',
    '../TODO',
    '../NEWS',
  ],
  output: [ 'index.html', ],
  command: [

  ],
)

# push_web_root = custom_target('push_web_root',
#   input : [
#     index_html,
#     index_static_html,
#     'babl.css',
#   ],
#   output: [ 'push_web_root' ],
#   command: [
#     'scp',
#     '@INPUT@',
#     scptarget,
#   ],
# )
# push_web_graphics = custom_target('push_web_graphics',
#   input : [
#     graphic_files_install,
#   ],
#   output: [ 'push_web_graphics' ],
#   command: [
#     'scp',
#     '@INPUT@',
#     scptarget + 'graphics/',
#   ],
# )
#
# web = custom_target('web',
#   input: [
#     push_web_root,
#     push_web_graphics
#   ],
#   output: [],
#   command: [],
# )