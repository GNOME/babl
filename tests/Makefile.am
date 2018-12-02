if OS_UNIX
CONCURRENCY_STRESS_TESTS =		\
	concurrency-stress-test 	\
	palette-concurrency-stress-test
endif

C_TESTS =				\
	cairo_cmyk_hack \
	grayscale_to_rgb	\
	chromaticities          \
	rgb_to_bgr       	\
	rgb_to_ycbcr		\
	srgb_to_lab_u8		\
	sanity			\
	babl_class_name		\
	extract \
	floatclamp \
	float-to-8bit \
	hsl    \
	hsva   \
	types	\
	format_with_space   \
	palette \
	extract \
	nop \
	n_components		\
	n_components_cast \
	models			\
	cairo-RGB24		\
	transparent		\
	$(CONCURRENCY_STRESS_TESTS)

TESTS = \
	$(C_TESTS)

TESTS_ENVIRONMENT = LD_LIBRARY_PATH=$(top_builddir)/babl:$LD_LIBRARY_PATH GI_TYPELIB_PATH=$(top_builddir)/babl BABL_PATH=$(top_builddir)/extensions/.libs

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/babl $(LCMS_CFLAGS)
AM_LDFLAGS  = $(THREAD_LIB) -no-install

LDADD = $(top_builddir)/babl/libbabl-@BABL_API_VERSION@.la \
	$(MATH_LIB) $(THREAD_LIB)

EXTRA_DIST=common.inc

noinst_PROGRAMS =		\
	$(C_TESTS)
