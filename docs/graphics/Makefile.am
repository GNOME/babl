.SILENT:

if HAVE_INKSCAPE
PNG_FILES = 			\
	babl-a4poster.png	\
	babl-16x16.png 		\
	babl-48x48.png
endif

SVG_FILES = 			\
	babl-a4poster.svg	\
	babl-16x16.svg 		\
	babl-48x48.svg

EXTRA_DIST = 			\
	.cvsignore		\
	index.html		\
	$(PNG_FILES)		\
	$(SVG_FILES)

MAINTAINERCLEANFILES = $(PNG_FILES)

all: $(MAINTAINERCLEANFILES)

INKSCAPE_CMD = echo -n " SVG: $@" ;\
  $(INKSCAPE) -e $@ $< 2&>/dev/null\
    &&\
      echo " [OK]"\
    ||\
      echo " FAIL"

if HAVE_INKSCAPE
dist-check-inkscape:

%.png: %.svg
	$(INKSCAPE_CMD)
#-e $@ $< 2>/dev/null

# the png version of the a4 poster is used as a webgraphic
# , thus it is rendered with the background alpha set to
# 0.0
babl-a4poster.png: babl-a4poster.svg
	echo -n " SVG: $@" ;\
	$(INKSCAPE) -e $@ $< -w 256 -y 0 2&>/dev/null\
	  &&\
	    echo " [OK]"\
	  ||\
	    echo " FAIL"
else
dist-check-inkscape:
	@echo "*** inkscape must be available in order to make dist"
	@false
endif


dist-hook: dist-check-inkscape
