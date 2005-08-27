h_sources =  			\
	babl-base.h		\
	cpercep.h

c_sources =			\
	babl-base.c		\
	cpercep.c		\
	type-float.c		\
	type-u8.c		\
	type-u16.c		\
	model-rgb.c		\
	model-gray.c		\
	model-lab.c		\
	model-ycbcr.c

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/babl

noinst_LTLIBRARIES= libbase.la
libbase_la_SOURCES= $(h_sources) $(c_sources) 

EXTRA_DIST := 		\
	.cvsignore	\
	rgb-constants.h	\
	util.h

LDADD = -lm
