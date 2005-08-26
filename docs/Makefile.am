SUBDIRS=      graphics
DIST_SUBDIRS= graphics

# The patterns used for html creation in the babl build system
# is very small, and should probably be documented in a tutorial.
#
#
#
HTML_FILES = index-static.html 	\
	     index.html

EXTRA_DIST= 			\
	.cvsignore		\
	babl.css  		\
	tools/xml_insert.sh	\
	index-static.html.in	\
	index.html		\
	LGPL

BUILT_EXTRA_DIST = index.html

CLEANFILES = README
DISTCLEANFILES = index-static.html 
MAINTAINERCLEANFILES = index.html

all: $(HTML_FILES)

index.html: index-static.html $(top_builddir)/tests/babl_html_dump $(top_srcdir)/AUTHORS  $(top_srcdir)/NEWS
	cp $< $@ 
	(TMPFILE=`tempfile`;\
	$(top_builddir)/tests/babl_html_dump > $$TMPFILE;\
	$(top_srcdir)/docs/tools/xml_insert.sh $@ BablBase $$TMPFILE;\
	rm -f $$TMPFILE )

	$(top_srcdir)/docs/tools/xml_insert.sh $@ AUTHORS $(top_srcdir)/AUTHORS
	$(top_srcdir)/docs/tools/xml_insert.sh $@ NEWS $(top_srcdir)/NEWS

distclean-local:
	if test $(srcdir) = .; then :; else 	\
		rm -f $(BUILT_EXTRA_DIST);	\
	fi
