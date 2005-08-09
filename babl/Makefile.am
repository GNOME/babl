## Source directory

SUBDIRS = babl-base

c_sources =				\
        babl.c				\
	babl-classes.c			\
	babl-component.c		\
	babl-conversion.c		\
	babl-fish.c			\
	babl-image.c			\
	babl-introspect.c		\
	babl-memory.c			\
	babl-model.c			\
	babl-pixel-format.c		\
	babl-sampling.c			\
	babl-sanity.c			\
	babl-type.c			\
	babl-util.c

h_sources  =             		\
	babl-classes.h			\
	babl-component.h		\
	babl-conversion.h		\
	babl-db.h			\
	babl-ids.h			\
	babl-internal.h			\
	babl-introspect.h		\
	babl-image.h			\
	babl-fish.h			\
	babl-memory.h			\
	babl-model.h			\
	babl-pixel-format.h		\
	babl-sampling.h			\
	babl-sanity.h			\
	babl-type.h			\
	babl-util.h			\
	babl.h

library_includedir=$(includedir)/babl-$(BABL_API_VERSION)/babl
library_include_HEADERS =		\
	babl.h  			\
	babl-classes.h			\
	babl-component.h		\
	babl-conversion.h		\
	babl-fish.h			\
	babl-image.h			\
	babl-model.h			\
	babl-pixel-format.h		\
	babl-sampling.h			\
	babl-type.h

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/babl/babl-base

lib_LTLIBRARIES= libbabl.la
libbabl_la_SOURCES= $(h_sources) $(c_sources)
libbabl_la_LIBADD=\
	babl-base/libbabl_base.la

libbabl_la_LDFLAGS= \
	-version-info $(BABL_LIBRARY_VERSION) \
	-release $(BABL_RELEASE)

EXTRA_DIST := $(wildcard *.h)	\
	.cvsignore		\
	Makefile-mini

