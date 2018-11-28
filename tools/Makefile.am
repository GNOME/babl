AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/babl $(LCMS_CFLAGS)
AM_LDFLAGS = -no-install

LDADD = $(top_builddir)/babl/libbabl-@BABL_API_VERSION@.la \
	$(MATH_LIB) $(THREAD_LIB)

noinst_PROGRAMS =		\
	babl-verify		\
	babl-benchmark		\
	babl-icc-dump		\
	babl-icc-rewrite	\
	trc-validator		\
	introspect		\
	babl_fish_path_fitness	\
	babl-html-dump		\
	conversions		\
	formats	
if HAVE_SRANDOM
noinst_PROGRAMS +=		\
	babl-gen-test-pixels
endif
