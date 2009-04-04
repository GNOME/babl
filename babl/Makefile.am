## Source directory

if OS_WIN32
no_undefined = -no-undefined
endif

SUBDIRS = base

c_sources =				\
	babl.c				\
	babl-component.c		\
	babl-conversion.c		\
	babl-core.c			\
	babl-cpuaccel.c			\
	babl-db.c			\
	babl-extension.c		\
	babl-fish-path.c		\
	babl-fish-reference.c		\
	babl-fish-simple.c		\
	babl-fish-stats.c		\
	babl-fish.c			\
	babl-format.c			\
	babl-hash-table.c		\
	babl-image.c			\
	babl-internal.c			\
	babl-introspect.c		\
	babl-list.c			\
	babl-memory.c			\
	babl-model.c			\
	babl-sampling.c			\
	babl-sanity.c			\
	babl-type.c			\
	babl-util.c			\
	babl-version.c

h_sources  =				\
	babl.h				\
	babl-class.h			\
	babl-classes.h			\
	babl-component.h		\
	babl-conversion.h		\
	babl-cpuaccel.h			\
	babl-db.h			\
	babl-extension.h		\
	babl-fish.h			\
	babl-format.h			\
	babl-hash-table.h		\
	babl-ids.h			\
	babl-image.h			\
	babl-internal.h			\
	babl-introspect.h		\
	babl-list.h			\
	babl-macros.h			\
	babl-memory.h			\
	babl-model.h			\
	babl-sampling.h			\
	babl-type.h			\
	babl-types.h			\
	babl-util.h

library_includedir=$(includedir)/babl-$(BABL_API_VERSION)/babl
library_include_HEADERS =		\
	babl-macros.h			\
	babl-types.h                    \
	babl-version.h			\
	babl.h

AM_CPPFLAGS = \
	-DLIBDIR=\""$(libdir)"\"	\
	-I$(top_srcdir)			\
	-I$(top_srcdir)/babl/base

lib_LTLIBRARIES= libbabl-@BABL_API_VERSION@.la
libbabl_@BABL_API_VERSION@_la_SOURCES= $(h_sources) $(c_sources)
libbabl_@BABL_API_VERSION@_la_LIBADD=\
	base/libbase.la \
	@LTLIBOBJS@

libbabl_@BABL_API_VERSION@_la_LDFLAGS= 		\
	${no_undefined} $(MATH_LIB)	\
	-version-info $(BABL_LIBRARY_VERSION)
