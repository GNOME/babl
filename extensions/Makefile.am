# General minimalistic compile file for self contained single
# file babl_extensions

if OS_WIN32
no_undefined = -no-undefined
libbabldlla=$(top_builddir)/babl/.libs/libbabl-$(BABL_API_VERSION).dll.a
endif

CFILES          = $(wildcard $(srcdir)/*.c)
SOBJS           = $(subst $(srcdir)/,,$(CFILES:.c=$(SHREXT)))
INSTALLED_ITEMS = $(subst $(srcdir),$(ext_dir),$(CFILES:.c=$(SHREXT)))
EXTRA_DIST      = $(wildcard *.[ch])

all-local: $(SOBJS)

CFLAGS  += -I $(top_srcdir)/babl -I $(top_srcdir)/extensions -fPIC

LDFLAGS += -shared

LDADD    = $(no-undefined) $(libbabldlla)

%$(SHREXT): %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LDADD)

# if extension needing external libraries are to be # compiled with this make
# file, each of them can be added according to the this pattern:
# extra$(SHREXT): extra.c
#	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<  [own compile and link flags]

CIE-Lab$(SHREXT): CIE-Lab.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LDADD) $(MATH_LIB)
#lcms$(SHREXT): lcms.c
#	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LDADD) `pkg-config lcms --cflags --libs`
#############################################################################

clean-local:
	rm -f $(SOBJS)

ext_dir = $(DESTDIR)$(libdir)/babl-@BABL_API_VERSION@

install-exec-local: all-local
	$(INSTALL) -d $(ext_dir)
	$(INSTALL) $(SOBJS) $(ext_dir)

uninstall-local:
	rm -f $(INSTALLED_ITEMS)

check-local: all-local
