if OS_UNIX
CONCURRENCY_STRESS_TEST = concurrency-stress-test 
endif

C_TESTS =				\
	grayscale_to_rgb	\
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
	types			\
	palette \
	extract \
	nop \
	n_components		\
	models			\
	cairo-RGB24		\
	$(CONCURRENCY_STRESS_TEST)

if HAVE_INTROSPECTION
INTROSPECTION_TESTS = \
	introspection.py
endif

TESTS = \
	$(C_TESTS) \
	$(INTROSPECTION_TESTS)

TESTS_ENVIRONMENT = LD_LIBRARY_PATH=$(top_builddir)/babl:$LD_LIBRARY_PATH GI_TYPELIB_PATH=$(top_builddir)/babl BABL_PATH=$(top_builddir)/extensions/.libs

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/babl
if OS_UNIX
AM_LDFLAGS  = -pthread
endif

LDADD = $(top_builddir)/babl/libbabl-@BABL_API_VERSION@.la \
	$(MATH_LIB)

EXTRA_DIST=common.inc

dist_noinst_SCRIPTS = \
	introspection.py

noinst_PROGRAMS =		\
	introspect		\
	babl_fish_path_fitness	\
	babl_fish_path_dhtml	\
	babl-html-dump		\
	conversions		\
	formats			\
	$(C_TESTS)
