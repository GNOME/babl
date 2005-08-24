TESTS =				\
	float_to_u8		\
	grayscale_to_rgb	\
	u8_to_float		\
	rgb_to_lab_to_rgb	\
	rgb_to_ycbcr		\
	rgb_to_ycbcr_to_rgb	\
	srgb_to_lab_u8		\
	sanity			\
	babl_class_name

float_to_u8_SOURCES         = float_to_u8.c
u8_to_float_SOURCES         = u8_to_float.c
grayscale_to_rgb_SOURCES    = grayscale_to_rgb.c
rgb_to_lab_to_rgb_SOURCES   = rgb_to_lab_to_rgb.c
srgb_to_lab_u8_SOURCES      = srgb_to_lab_u8.c
rgb_to_ycbcr_SOURCES        = rgb_to_ycbcr.c
rgb_to_ycbcr_to_rgb_SOURCES = rgb_to_ycbcr_to_rgb.c
babl_class_name_SOURCES     = babl_class_name.c
sanity_SOURCES              = sanity.c


AM_CFLAGS = -I$(top_srcdir) -I$(top_srcdir)/babl
LDADD = $(top_builddir)/babl/libbabl.la -lm


noinst_PROGRAMS =	\
	introspect	\
	babl_html_dump	\
	nop		\
	$(TESTS)

introspect_SOURCES     = introspect.c
babl_html_dump_SOURCES = babl-html-dump.c
nop_SOURCES            = nop.c

EXTRA_DIST = .cvsignore
