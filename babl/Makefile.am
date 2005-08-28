## Source directory

SUBDIRS = base

c_sources =				\
        babl.c				\
	babl-core.c   			\
	babl-component.c		\
	babl-conversion.c		\
	babl-extension.c		\
	babl-fish.c			\
	babl-format.c			\
	babl-image.c			\
	babl-internal.c			\
	babl-introspect.c		\
	babl-memory.c			\
	babl-model.c			\
	babl-sampling.c			\
	babl-sanity.c			\
	babl-type.c			\
	babl-util.c

h_sources  =             		\
	babl-classes.h			\
	babl-component.h		\
	babl-core.h   			\
	babl-conversion.h		\
	babl-db.h			\
	babl-extension.h		\
	babl-format.h			\
	babl-ids.h			\
	babl-instance.h			\
	babl-internal.h			\
	babl-introspect.h		\
	babl-image.h			\
	babl-fish.h			\
	babl-memory.h			\
	babl-model.h			\
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
	babl-extension.h		\
	babl-fish.h			\
	babl-ids.h			\
	babl-image.h			\
	babl-instance.h			\
	babl-model.h			\
	babl-format.h			\
	babl-sampling.h			\
	babl-type.h

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/babl/base

lib_LTLIBRARIES= libbabl.la
libbabl_la_SOURCES= $(h_sources) $(c_sources)
libbabl_la_LIBADD=\
	base/libbase.la

libbabl_la_LDFLAGS= \
	-version-info $(BABL_LIBRARY_VERSION) \
	-release $(BABL_RELEASE)

EXTRA_DIST := 			\
	.cvsignore
