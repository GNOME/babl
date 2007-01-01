AC_PREREQ(2.54)

m4_define([babl_major_version], 0)
m4_define([babl_minor_version], 0)
m4_define([babl_micro_version], 9)
m4_define([babl_real_version],
          [babl_major_version.babl_minor_version.babl_micro_version])
m4_define([babl_version], [babl_real_version])

AC_INIT(babl, babl_major_version.babl_minor_version.babl_micro_version)
AC_CONFIG_SRCDIR(babl/babl.h)

BABL_MAJOR_VERSION=babl_major_version
BABL_MINOR_VERSION=babl_minor_version
BABL_MICRO_VERSION=babl_micro_version
BABL_REAL_VERSION=babl_real_version
BABL_VERSION=babl_version
AC_SUBST(BABL_REAL_VERSION)

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

AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE(no-define)

AC_PROG_CC
AC_LIBTOOL_WIN32_DLL
AM_PROG_LIBTOOL

dnl Initialize maintainer mode
AM_MAINTAINER_MODE

AC_STDC_HEADERS
AM_SANITY_CHECK

WEBSITE_HOST=pippin.gimp.org
AC_SUBST(WEBSITE_HOST)
WEBSITE_LOCATION=public_html/babl/
AC_SUBST(WEBSITE_LOCATION)

if eval "test x$GCC = xyes"; then
  case " $CFLAGS " in
  *[\ \	]-Wall[\ \	]*) ;;
  *) CFLAGS="$CFLAGS -Wall" ;;
  esac
fi

babl_extension_dir="$libdir/babl"
AC_SUBST(babl_extension_dir)

AC_PATH_PROG(INKSCAPE, inkscape, no)
AM_CONDITIONAL(HAVE_INKSCAPE, test "x$INKSCAPE" != "xno")

AC_PATH_PROG(W3M, w3m, no)
AM_CONDITIONAL(HAVE_W3M, test "x$W3M" != "xno")

###########################
# Check target architecture
###########################

AC_MSG_CHECKING([for target architecture])
case x"$target" in
  xNONE | x)
    target_or_host="$host" ;;
  *)
    target_or_host="$target" ;;
esac
AC_MSG_RESULT([$target_or_host])

case "$target_or_host" in
  i*86-*-*)
    have_x86=yes
    AC_DEFINE(ARCH_X86, 1, [Define to 1 if you are compiling for ix86.])
    ;;
  x86_64-*-*)
    have_x86=yes
    AC_DEFINE(ARCH_X86, 1, [Define to 1 if you are compiling for ix86.])
    AC_DEFINE(ARCH_X86_64, 1, [Define to 1 if you are compiling for amd64.])
    ;;
  ppc-*-* | powerpc-*)
    have_ppc=yes
    AC_DEFINE(ARCH_PPC, 1, [Define to 1 if you are compiling for PowerPC.])
    ;;
  ppc64-*-* | powerpc64-*)
    have_ppc=yes
    AC_DEFINE(ARCH_PPC, 1, [Define to 1 if you are compiling for PowerPC.])
    AC_DEFINE(ARCH_PPC64, 1, [Define to 1 if you are compiling for PowerPC64.])
    ;;
  *)
    ;;
esac


#################
# Check for Win32
#################

AC_MSG_CHECKING([for some Win32 platform])
case "$target_or_host" in
  *-*-mingw* | *-*-cygwin*)
    platform_win32=yes
    ;;
  *)
    platform_win32=no
    ;;
esac
AC_MSG_RESULT([$platform_win32])
AM_CONDITIONAL(PLATFORM_WIN32, test "$platform_win32" = "yes")

MATH_LIB=-lm
AC_MSG_CHECKING([for native Win32])
case "$target_or_host" in
  *-*-mingw*)
    os_win32=yes
    PATHSEP=';'
    MATH_LIB=
    ;;
  *)
    os_win32=no
    PATHSEP=':'
    ;;
esac
AC_MSG_RESULT([$os_win32])
AC_SUBST(PATHSEP)
AC_SUBST(MATH_LIB)

AM_CONDITIONAL(OS_WIN32, test "$os_win32" = "yes")
AM_CONDITIONAL(OS_UNIX, test "$os_win32" != "yes")

dnl ===========================================================================

AC_CHECK_LIB([dl], [dlopen], [DL_LIB="-ldl"])
AC_SUBST(DL_LIB)

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
