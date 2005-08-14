TESTS =				\
	float_to_u8		\
	grayscale_to_rgb	\
	u8_to_float

float_to_u8_SOURCES      =  float_to_u8.c
u8_to_float_SOURCES      =  u8_to_float.c
grayscale_to_rgb_SOURCES =  grayscale_to_rgb.c


AM_CFLAGS = -I$(top_srcdir) -I$(top_srcdir)/babl
LDADD = $(top_builddir)/babl/libbabl.la -lm


noinst_PROGRAMS =	\
	introspect	\
	nop		\
	$(TESTS)

introspect_SOURCES  =  introspect.c
nop_SOURCES         =  nop.c


EXTRA_DIST = .cvsignore

