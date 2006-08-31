AUTOMAKE_OPTIONS = foreign dist-bzip2

SUBDIRS = babl		\
	  extensions 	\
	  tests		\
	  docs

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = babl.pc

built_dist_files = README

EXTRA_DIST = \
	$(built_dist_files)	\
	AUTHORS			\
	COPYING			\
	INSTALL			\
	NEWS			\
	ChangeLog		\
	autogen.sh		\
	babl.pc.in

DISTCLEANFILES = \
	$(built_dist_files)	\
	babl.pc

.PHONY: snapshot


if HAVE_W3M
dist-check-w3m:

README: all docs/index.html
	$(W3M) -cols 72 -dump $< > $@
else
dist-check-w3m:
	@echo "*** w3m must be available in order to make dist"
	@false
endif

snapshot:
	$(MAKE) dist distdir=$(PACKAGE)-`date +"%Y%m%d"`
snapcheck:
	$(MAKE) distcheck distdir=$(PACKAGE)-`date +"%Y%m%d"`

distclean-local:
	if test $(srcdir) = .; then :; else 	\
		rm -f $(BUILT_EXTRA_DIST);	\
	fi

dist-hook: dist-check-w3m
