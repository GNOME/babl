AC_PREREQ(2.54)

m4_define([babl_major_version], 0)
m4_define([babl_minor_version], 0)
m4_define([babl_micro_version], 2)

AC_INIT(babl, babl_major_version.babl_minor_version.babl_micro_version)
AC_CONFIG_SRCDIR(babl/babl.h)
AC_CONFIG_HEADERS(config.h)

BABL_MAJOR_VERSION=babl_major_version
BABL_MINOR_VERSION=babl_minor_version
BABL_MICRO_VERSION=babl_micro_version
BABL_REAL_VERSION=babl_real_version
BABL_VERSION=babl_version

BABL_API_VERSION=babl_major_version.babl_minor_version
AC_SUBST(BABL_API_VERSION)

BABL_LIBRARY_VERSION=0:0:0

AC_SUBST(BABL_LIBRARY_VERSION)

PACKAGE_NAME=babl
AC_SUBST(PACKAGE_NAME)

BABL_VERSION=$BABL_MAJOR_VERSION.$BABL_MINOR_VERSION.$BABL_MICRO_VERSION
BABL_RELEASE=$BABL_MAJOR_VERSION.$BABL_MINOR_VERSION
AC_SUBST(BABL_RELEASE)
AC_SUBST(BABL_VERSION)

AM_INIT_AUTOMAKE(no-define)

AC_PROG_CC
AM_PROG_LIBTOOL
AC_STDC_HEADERS
AM_SANITY_CHECK

if eval "test x$GCC = xyes"; then
  case " $CFLAGS " in
  *[\ \	]-Wall[\ \	]*) ;;
  *) CFLAGS="$CFLAGS -Wall" ;;
  esac
fi

babl_extension_dir="$(libdir)/babl"
AC_SUBST(babl_extension_dir)

AC_PATH_PROG(INKSCAPE, inkscape, no)
AM_CONDITIONAL(HAVE_INKSCAPE, test "x$INKSCAPE" != "xno")

AC_PATH_PROG(W3M, w3m, no)
AM_CONDITIONAL(HAVE_W3M, test "x$W3M" != "xno")

dnl ===========================================================================

AC_DEFINE_UNQUOTED(BABL_PATH, "~/.babl-$BABL_API_VERSION:/usr/local/lib/babl-$BABL_API_VERSION:/usr/lib/babl-$BABL_API_VERSION", [search path for babl extensions (default value of enviroment variable)])
AC_DEFINE(BABL_PATH_SEPERATOR, "/", [string used to seperate directories in a path string]) 
AC_DEFINE(BABL_LIST_SEPERATOR, ':', [seperator between paths in BABL_PATH])

dnl ===========================================================================


dnl ===========================================================================

AC_CONFIG_FILES(
Makefile
babl.pc
babl/Makefile
babl/base/Makefile
docs/Makefile
docs/index-static.html
docs/graphics/Makefile
tests/Makefile
extensions/Makefile
INSTALL
)

AC_OUTPUT
