h_sources =  			\
	babl-base.h

c_sources =			\
	babl-base.c		\
	type-double.c		\
	type-float.c		\
	type-u8.c		\
	type-u16.c		\
	model-rgb.c		\
	model-grayscale.c

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/babl

noinst_LTLIBRARIES= libbase.la
libbase_la_SOURCES= $(h_sources) $(c_sources) 

EXTRA_DIST := 		\
	.cvsignore	\
	rgb-constants.h	\
	util.h
	

LDADD = -lm
