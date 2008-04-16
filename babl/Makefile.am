## Source directory

if OS_WIN32
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
	babl-util.c			\
	babl-list.c			\
	babl-hash-table.c

h_sources  =             		\
	babl-db.h			\
	babl-ids.h			\
	babl-internal.h			\
	babl-memory.h			\
	babl-util.h			\
	babl.h				\
	babl-list.h			\
	babl-hash-table.h

library_includedir=$(includedir)/babl-$(BABL_API_VERSION)/babl
library_include_HEADERS =		\
	babl.h				\
	babl-component.h		\
	babl-conversion.h		\
	babl-extension.h		\
	babl-fish.h			\
	babl-format.h			\
	babl-image.h			\
	babl-model.h			\
	babl-sampling.h 		\
	babl-type.h

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/babl/base

AM_CPPFLAGS = -DLIBDIR=\""$(libdir)"\"

lib_LTLIBRARIES= libbabl-@BABL_API_VERSION@.la
libbabl_@BABL_API_VERSION@_la_SOURCES= $(h_sources) $(c_sources)
libbabl_@BABL_API_VERSION@_la_LIBADD=\
	base/libbase.la \
	@LTLIBOBJS@

libbabl_@BABL_API_VERSION@_la_LDFLAGS= 		\
	${no_undefined} $(MATH_LIB)	\
	-version-info $(BABL_LIBRARY_VERSION)
