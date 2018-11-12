## Source directory

if PLATFORM_WIN32
win32_no_undefined = -no-undefined
endif

SUBDIRS = base

c_sources =				\
	babl.c				\
	babl-cache.c		\
	babl-component.c		\
	babl-conversion.c		\
	babl-core.c			\
	babl-db.c			\
	babl-extension.c		\
	babl-fish-path.c		\
	babl-fish-reference.c		\
	babl-fish-simple.c		\
	babl-fish.c			\
	babl-format.c			\
	babl-hash-table.c		\
	babl-icc.c			\
	babl-image.c			\
	babl-internal.c			\
	babl-introspect.c		\
	babl-list.c			\
	babl-memory.c			\
	babl-model.c			\
	babl-mutex.c			\
	babl-palette.c			\
	babl-polynomial.c		\
	babl-ref-pixels.c		\
	babl-sampling.c			\
	babl-sanity.c			\
	babl-space.c 			\
	babl-trc.c			\
	babl-type.c			\
	babl-util.c			\
	babl-cpuaccel.c			\
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
	babl-matrix.h			\
	babl-mutex.h			\
	babl-polynomial.h		\
	babl-ref-pixels.h		\
	babl-sampling.h			\
	babl-space.h			\
	babl-trc.h			\
	babl-type.h			\
	babl-types.h			\
	babl-util.h

library_includedir=$(includedir)/babl-$(BABL_API_VERSION)/babl
libinc_hdrs = \
	babl-introspect.h		\
	babl-macros.h			\
	babl-types.h			\
	babl.h

libinc_generated_hdrs =			\
	babl-version.h

library_include_HEADERS =		\
	$(libinc_hdrs) $(libinc_generated_hdrs)

AM_CPPFLAGS =				\
	-DLIBDIR=\""$(libdir)"\"	\
	-I$(top_srcdir)			\
	-I$(top_srcdir)/babl/base

lib_LTLIBRARIES= libbabl-@BABL_API_VERSION@.la

libbabl_@BABL_API_VERSION@_la_SOURCES= $(h_sources) $(c_sources)
libbabl_@BABL_API_VERSION@_la_CFLAGS= $(SSE2_EXTRA_CFLAGS) $(LCMS_CFLAGS)

libbabl_@BABL_API_VERSION@_la_LIBADD=	\
	base/libbase.la			\
	@LTLIBOBJS@


libbabl_@BABL_API_VERSION@_la_LDFLAGS=			\
	${win32_no_undefined} 				\
	$(MATH_LIB) $(THREAD_LIB) $(LOG_LIB) 		\
	$(LCMS_LIBS) \
	-export-symbols $(top_srcdir)/export-symbols 	\
	-version-info $(BABL_LIBRARY_VERSION)

EXTRA_DIST = babl-ref-pixels.inc 


# If git is available, always check if git-version.h should be
# updated. If git is not available, don't do anything if git-version.h
# already exists because then we are probably working with a tarball
# in which case the git-version.h we ship is correct.

libbabl_generated_sources = git-version.h
BUILT_SOURCES = git-version.h
CLEANFILES = git-version.h

git-version.h: update-git-version-header
	@if test -d "$(top_srcdir)/.git"; then \
	  git_version="`git --git-dir=$(top_srcdir)/.git describe --always`"; \
	  git_last_commit_year="`git --git-dir=$(top_srcdir)/.git log -n1 --reverse --pretty=%ci | cut -b 1-4`"; \
	elif test ! -f "$@"; then \
	  git_version="Unknown, shouldn't happen"; \
	  git_last_commit_timestamp=-1; \
	  git_last_commit_year="`date -u '+%Y'`"; \
	fi; \
	if test -n "$$git_version"; then \
	  echo "#ifndef __GIT_VERSION_H__" > "$@.tmp"; \
	  echo "#define __GIT_VERSION_H__" >> "$@.tmp"; \
	  echo "#define BABL_GIT_VERSION \"$$git_version\"" >> "$@.tmp"; \
	  echo "#define BABL_GIT_LAST_COMMIT_YEAR \"$$git_last_commit_year\"" >> "$@.tmp"; \
	  echo "#endif /* __GIT_VERSION_H__ */" >> "$@.tmp"; \
	fi
	@if ( test -f "$@.tmp" && test -f "$@" && cmp "$@.tmp" "$@" > /dev/null ); then \
	  rm -f "$@.tmp"; \
	elif test -f "$@.tmp"; then \
	  mv "$@.tmp" "$@"; \
	  echo "  git HEAD changed: $@ regenerated"; \
	fi

.PHONY: update-git-version-header
