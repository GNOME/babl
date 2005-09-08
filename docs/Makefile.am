.SILENT:
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
	echo -n "HTML: $@"
	cp $< $@ 
	(TMPFILE=`tempfile`;\
	$(top_builddir)/tests/babl_html_dump > $$TMPFILE;\
	$(top_srcdir)/docs/tools/xml_insert.sh $@ BablBase $$TMPFILE;\
	rm -f $$TMPFILE )

	$(top_srcdir)/docs/tools/xml_insert.sh $@ AUTHORS $(top_srcdir)/AUTHORS
	$(top_srcdir)/docs/tools/xml_insert.sh $@ NEWS $(top_srcdir)/NEWS
	echo " [OK]"

distclean-local:
	if test $(srcdir) = .; then :; else 	\
		rm -f $(BUILT_EXTRA_DIST);	\
	fi


web: all graphics
	echo -n " WEB: "
	ping -c 1 $(WEBSITE_HOST) >/dev/null && \
	   (\
	      scp *.html *.css\
	          "$(WEBSITE_HOST):$(WEBSITE_LOCATION)/" >/dev/null && \
		  echo -n "."; \
	      scp graphics/*.png graphics/*.html\
	          "$(WEBSITE_HOST):$(WEBSITE_LOCATION)/graphics/" >/dev/null\
		&&\
	          echo "[OK]"\
		||\
	          echo "FAIL   scp failed (does dirs exist?)."\
	   )\
	|| \
	   (\
	   echo "FAIL   $(WEBSITE_HOST) not reachable."\
	   )
