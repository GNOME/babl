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
	-I$(top_srcdir)/extensions	\
	$(LCMS_CFLAGS)

extdir = $(libdir)/babl-@BABL_API_VERSION@
ext_LTLIBRARIES = \
	u16.la        \
	u32.la        \
	cairo.la        \
	CIE.la          \
	half.la   \
	gegl-fixups.la  \
	gggl-lies.la    \
	gggl-table.la   \
	gggl-table-lies.la \
	gggl.la         \
	gimp-8bit.la    \
	grey.la         \
	double.la       \
	float.la        \
	fast-float.la   \
	naive-CMYK.la   \
	HCY.la          \
	HSL.la          \
	HSV.la          \
	simple.la       \
	sse2-float.la   \
	sse2-int8.la    \
	sse2-int16.la   \
	sse4-int8.la    \
	sse-half.la     \
	two-table.la	\
	ycbcr.la

cairo_la_SOURCES = cairo.c
CIE_la_SOURCES = CIE.c
simple_la_SOURCES = simple.c
float_half_la_SOURCES = float-half.c
gegl_fixups_la_SOURCES = gegl-fixups.c
gggl_lies_la_SOURCES = gggl-lies.c
gggl_table_lies_la_SOURCES = gggl-table-lies.c
gggl_table_la_SOURCES = gggl-table.c
gggl_la_SOURCES = gggl.c
gimp_8bit_la_SOURCES = gimp-8bit.c
grey_la_SOURCES = grey.c
naive_CMYK_la_SOURCES = naive-CMYK.c
HCY_la_SOURCES = HCY.c
HSL_la_SOURCES = HSL.c
HSV_la_SOURCES = HSV.c
sse2_float_la_SOURCES = sse2-float.c
sse2_int8_la_SOURCES = sse2-int8.c
sse2_int16_la_SOURCES = sse2-int16.c
sse4_int8_la_SOURCES = sse4-int8.c
sse_half_la_SOURCES = sse-half.c
two_table_la_SOURCES = two-table.c two-table-tables.h
ycbcr_la_SOURCES = ycbcr.c
float_la_SOURCES = float.c
fast_float_la_SOURCES = fast-float.c

LIBS =  $(top_builddir)/babl/libbabl-@BABL_API_VERSION@.la \
	$(MATH_LIB) $(THREAD_LIB)

CIE_la_CFLAGS = $(SSE2_EXTRA_CFLAGS)
sse2_float_la_CFLAGS = $(SSE2_EXTRA_CFLAGS)
sse2_int8_la_CFLAGS = $(SSE2_EXTRA_CFLAGS)
sse2_int16_la_CFLAGS = $(SSE2_EXTRA_CFLAGS)
sse4_int8_la_CFLAGS = $(SSE4_1_EXTRA_CFLAGS)
sse_half_la_CFLAGS = $(SSE4_1_EXTRA_CFLAGS) $(F16C_EXTRA_CFLAGS)
