TESTS =				\
	grayscale_to_rgb	\
	rgb_to_bgr       	\
	rgb_to_ycbcr		\
	srgb_to_lab_u8		\
	sanity			\
	babl_class_name		\
	types			\
	models	

TESTS_ENVIRONMENT = BABL_PATH=$(top_builddir)/extensions

grayscale_to_rgb_SOURCES        = grayscale_to_rgb.c
rgb_to_bgr_SOURCES              = rgb_to_bgr.c
srgb_to_lab_u8_SOURCES          = srgb_to_lab_u8.c
rgb_to_ycbcr_SOURCES            = rgb_to_ycbcr.c
babl_class_name_SOURCES         = babl_class_name.c
sanity_SOURCES                  = sanity.c
types_SOURCES                   = types.c
babl_fish_path_fitness_SOURCES  = babl_fish_path_fitness.c
babl_fish_path_dhtml_SOURCES    = babl_fish_path_dhtml.c
formats_SOURCES                 = formats.c
models_SOURCES                  = models.c

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/babl

LDADD = $(top_builddir)/babl/libbabl-@BABL_API_VERSION@.la \
	$(MATH_LIB)

noinst_PROGRAMS =		\
	introspect		\
	babl_fish_path_fitness	\
	babl_fish_path_dhtml	\
	babl_html_dump		\
	conversions		\
	formats			\
	nop			\
	$(TESTS)

introspect_SOURCES     = introspect.c
babl_html_dump_SOURCES = babl-html-dump.c
nop_SOURCES            = nop.c
