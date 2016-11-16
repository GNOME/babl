AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/babl

LDADD = $(top_builddir)/babl/libbabl-@BABL_API_VERSION@.la \
	$(MATH_LIB) $(THREAD_LIB)

noinst_PROGRAMS = babl-verify
if HAVE_SRANDOM
noinst_PROGRAMS +=		\
	babl-gen-test-pixels
endif
