PNG_FILES = 			\
	babl-a4poster.png	\
	babl-16x16.png 		\
	babl-48x48.png

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

%.png : %.svg
	inkscape -e $@ $<

# the png version of the a4 poster is used as a webgraphic
# , thus it is rendered with the background alpha set to
# 0.0
babl-a4poster.png: babl-a4poster.svg
	inkscape -e $@ $< -w 256 -y 0

