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

babl_path_src = join_paths(meson.source_root() + 'extensions')
babl_path_bin = join_paths(meson.build_root()  + 'extensions')

index_html = custom_target('index.html',
  input : [
    index_static_html,
    '../AUTHORS',
    '../TODO',
    '../NEWS',
  ],
  output: [ 'index.html', ],
  command: [
    # cp $< $@
    # (which mktemp > /dev/null 2>&1 && TMPFILE=`mktemp` || TMPFILE="/tmp/babl_build_tempfile" ;\
    # export BABL_PATH="$(top_builddir)/extensions:$(top_builddir)/extensions/.libs"; $(babl_html_dump) > $$TMPFILE;\
    # $(SHELL) $(top_srcdir)/docs/tools/xml_insert.sh $@ BablBase $$TMPFILE;\
    # rm -f $$TMPFILE )
    # echo -n "."
    #
    # $(SHELL) $(top_srcdir)/docs/tools/xml_insert.sh $@ AUTHORS $(top_srcdir)/AUTHORS
    # $(SHELL) $(top_srcdir)/docs/tools/xml_insert.sh $@ TODO $(top_srcdir)/TODO
    # $(SHELL) $(top_srcdir)/docs/tools/xml_insert.sh $@ NEWS $(top_srcdir)/NEWS
    # echo " [OK]"
  ],
)

babl_css = files('babl.css')

custom_target('push_web.sh',
  input : [
    index_html,
    index_static_html,
    babl_css,
    graphic_files_install,
  ],
  output: [
    'push_web.sh',
  ],
  command: [
    'echo', '#!/bin/bash',
    '\n' + 'scp', index_html, index_static_html, babl_css, scptarget,
    '\n' + 'scp', graphic_files_install, scptarget + 'graphics/'
  ],
  capture: true,
)
