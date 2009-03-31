TESTS =				\
	grayscale_to_rgb	\
	rgb_to_bgr       	\
	rgb_to_ycbcr		\
	srgb_to_lab_u8		\
	sanity			\
	babl_class_name		\
	types			\
	models	

TESTS_ENVIRONMENT = BABL_PATH=$(top_builddir)/extensions/.libs

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/babl

LDADD = $(top_builddir)/babl/libbabl-@BABL_API_VERSION@.la \
	$(MATH_LIB)

noinst_PROGRAMS =		\
	introspect		\
	babl_fish_path_fitness	\
	babl_fish_path_dhtml	\
	babl-html-dump		\
	conversions		\
	formats			\
	nop			\
	$(TESTS)
