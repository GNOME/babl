noinst_PROGRAMS =	\
	introspect	\
	nop
 
AM_CFLAGS = -I$(top_srcdir) -I$(top_srcdir)/babl

introspect_SOURCES = \
	introspect.c
introspect_LDADD = $(top_builddir)/babl/libbabl.la

nop_SOURCES = \
	nop.c
nop_LDADD = $(top_builddir)/babl/libbabl.la

EXTRA_DIST=		\
	.cvsignore

TESTS=nop introspect
