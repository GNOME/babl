if OS_WIN32
AM_LDFLAGS = -module -no-undefined
else
AM_LDFLAGS = -module -avoid-version
endif

noinst_HEADERS = util.h

AM_CPPFLAGS = \
	-I$(top_builddir)		\
	-I$(top_srcdir)			\
	-I$(top_srcdir)/babl		\
	-I$(top_srcdir)/extensions

extdir = $(libdir)/babl-@BABL_API_VERSION@
ext_LTLIBRARIES = \
	CIE-Lab.la      \
	gegl-fixups.la  \
	gggl-lies.la    \
	gggl.la         \
	gimp-8bit.la    \
	naive-CMYK.la   \
	sse-fixups.la

LIBS = $(top_builddir)/babl/libbabl-@BABL_API_VERSION@.la $(MATH_LIB)

sse_fixups_la_CFLAGS = $(MMX_EXTRA_CFLAGS) $(SSE_EXTRA_CFLAGS)
