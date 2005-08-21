AUTOMAKE_OPTIONS = foreign

SUBDIRS = babl	\
	  tests	\
	  docs


DIST_SUBDIRS = babl	\
	       tests	\
	       docs

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = babl.pc

EXTRA_DIST = 		\
	.cvsignore	\
	COPYING		\
	ChangeLog	\
	INSTALL.in	\
	NEWS		\
	babl.pc.in	\
	autogen.sh

BUILT_EXTRA_DIST = 	\
	README		\
	babl.pc

.PHONY: snapshot

README:
	echo "Eek!" > $@

snapshot:
	$(MAKE) dist distdir=$(PACKAGE)-`date +"%Y%m%d"`
snapcheck:
	$(MAKE) distcheck distdir=$(PACKAGE)-`date +"%Y%m%d"`

distclean-local:
	if test $(srcdir) = .; then :; else 	\
		rm -f $(BUILT_EXTRA_DIST);	\
	fi
