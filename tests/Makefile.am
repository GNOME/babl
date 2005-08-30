TESTS =				\
	grayscale_to_rgb	\
	rgb_to_bgr       	\
	rgb_to_ycbcr		\
	srgb_to_lab_u8		\
	sanity			\
	types			\
	models			\
	babl_class_name

grayscale_to_rgb_SOURCES    = grayscale_to_rgb.c
rgb_to_bgr_SOURCES          = rgb_to_bgr.c
srgb_to_lab_u8_SOURCES      = srgb_to_lab_u8.c
rgb_to_ycbcr_SOURCES        = rgb_to_ycbcr.c
babl_class_name_SOURCES     = babl_class_name.c
sanity_SOURCES              = sanity.c
types_SOURCES               = types.c
models_SOURCES              = models.c


AM_CFLAGS = -I$(top_srcdir) -I$(top_srcdir)/babl
LDADD = $(top_builddir)/babl/libbabl.la -lm -ldl

noinst_PROGRAMS =	\
	introspect	\
	babl_html_dump	\
	nop		\
	$(TESTS)

introspect_SOURCES     = introspect.c
babl_html_dump_SOURCES = babl-html-dump.c
nop_SOURCES            = nop.c

EXTRA_DIST = .cvsignore
