AC_INIT(babl/babl.h)
AM_CONFIG_HEADER(config.h)

BABL_MAJOR_VERSION=0
BABL_MINOR_VERSION=0
BABL_MICRO_VERSION=1

BABL_API_VERSION=0.0
AC_SUBST(BABL_API_VERSION)

BABL_LIBRARY_VERSION=0:0:0

AC_SUBST(BABL_LIBRARY_VERSION)

PACKAGE=babl

BABL_VERSION=$BABL_MAJOR_VERSION.$BABL_MINOR_VERSION.$BABL_MICRO_VERSION
BABL_RELEASE=$BABL_MAJOR_VERSION.$BABL_MINOR_VERSION
AC_SUBST(BABL_RELEASE)
AC_SUBST(BABL_VERSION)

VERSION=$BABL_VERSION

AM_INIT_AUTOMAKE($PACKAGE, $VERSION, no-define)

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
INSTALL
)

AC_OUTPUT
