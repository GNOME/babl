## Source directory

if OS_WIN32
win32_libs = -lgw32c -lole32 -luuid -lwsock32
no_undefined = -no-undefined
endif

SUBDIRS = base

c_sources =				\
        babl.c				\
	babl-core.c   			\
	babl-db.c			\
	babl-component.c		\
	babl-conversion.c		\
	babl-extension.c		\
	babl-fish.c			\
	babl-fish-simple.c		\
	babl-fish-stats.c		\
	babl-fish-reference.c		\
	babl-fish-path.c    	 	\
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
	babl-db.h			\
	babl-ids.h			\
	babl-internal.h			\
	babl-memory.h			\
	babl-util.h			\
	babl.h

library_includedir=$(includedir)/babl-$(BABL_API_VERSION)/babl
library_include_HEADERS =		\
	babl.h                          \
        babl-classes.h

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/babl/base

AM_CPPFLAGS = -DPREFIX=\""$(prefix)"\"

lib_LTLIBRARIES= libbabl.la
libbabl_la_SOURCES= $(h_sources) $(c_sources)
libbabl_la_LIBADD=\
	base/libbase.la \
	${win32_libs}

libbabl_la_LDFLAGS= \
	-version-info $(BABL_LIBRARY_VERSION) \
	-release $(BABL_RELEASE) \
	${no_undefined}

EXTRA_DIST := 			\
	.cvsignore
