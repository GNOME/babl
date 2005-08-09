AUTOMAKE_OPTIONS = foreign

SUBDIRS=       babl	\
	       tests


DIST_SUBDIRS = babl	\
	       tests

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = babl.pc

EXTRA_DIST = 		\
	.cvsignore	\
	COPYING		\
	ChangeLog	\
	README.in	\
	INSTALL.in	\
	NEWS		\
	babl.pc.in	\
	autogen.sh	\
	Makefile-mini

BUILT_EXTRA_DIST = 	\
	README		\
	babl.pc

.PHONY: snapshot

snapshot:
	$(MAKE) dist distdir=$(PACKAGE)-`date +"%Y%m%d"`
