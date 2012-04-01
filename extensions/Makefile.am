if PLATFORM_WIN32
AM_LDFLAGS = -module -avoid-version -no-undefined
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
	cairo.la        \
	CIE.la          \
	gegl-fixups.la  \
	gggl-lies.la    \
	gggl.la         \
	gimp-8bit.la    \
	float.la    \
	fast-float.la    \
	naive-CMYK.la   \
	sse-fixups.la

cairo_la_SOURCES = cairo.c
CIE_la_SOURCES = CIE.c
gegl_fixups_la_SOURCES = gegl-fixups.c
gggl_lies_la_SOURCES = gggl-lies.c
gggl_la_SOURCES = gggl.c
gimp_8bit_la_SOURCES = gimp-8bit.c
naive_CMYK_la_SOURCES = naive-CMYK.c
sse_fixups_la_SOURCES = sse-fixups.c
float_la_SOURCES = float.c
fast_float_la_SOURCES = fast-float.c

LIBS = $(top_builddir)/babl/libbabl-@BABL_API_VERSION@.la $(MATH_LIB)

sse_fixups_la_CFLAGS = $(MMX_EXTRA_CFLAGS) $(SSE_EXTRA_CFLAGS)
