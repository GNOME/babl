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
	grey.la         \
	float.la        \
	fast-float.la   \
	naive-CMYK.la   \
	HSL.la          \
	HSV.la          \
	simple.la       \
	sse2-float.la   \
	sse2-int8.la    \
	sse2-int16.la   \
	two-table.la	\
	ycbcr.la

cairo_la_SOURCES = cairo.c cairo-tables.h
CIE_la_SOURCES = CIE.c
simple_la_SOURCES = simple.c
gegl_fixups_la_SOURCES = gegl-fixups.c
gggl_lies_la_SOURCES = gggl-lies.c
gggl_la_SOURCES = gggl.c
gimp_8bit_la_SOURCES = gimp-8bit.c
grey_la_SOURCES = grey.c
naive_CMYK_la_SOURCES = naive-CMYK.c
HSL_la_SOURCES = HSL.c
HSV_la_SOURCES = HSV.c
sse2_float_la_SOURCES = sse2-float.c
sse2_int8_la_SOURCES = sse2-int8.c
sse2_int16_la_SOURCES = sse2-int16.c
two_table_la_SOURCES = two-table.c two-table-tables.h
ycbcr_la_SOURCES = ycbcr.c
float_la_SOURCES = float.c
fast_float_la_SOURCES = fast-float.c

LIBS = $(top_builddir)/babl/libbabl-@BABL_API_VERSION@.la $(MATH_LIB) \
	$(THREAD_LIB)

sse2_float_la_CFLAGS = $(SSE2_EXTRA_CFLAGS)
sse2_int8_la_CFLAGS = $(SSE2_EXTRA_CFLAGS)
sse2_int16_la_CFLAGS = $(SSE2_EXTRA_CFLAGS)
