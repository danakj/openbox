# util/Makefile.am for Blackbox - an X11 Window manager

CPPFLAGS= @CPPFLAGS@ @DEBUG@

SUBDIRS = 		epist
bin_SCRIPTS = 		bsetbg

if XFT
if XFT2
else
TMP_PROGS =	xftlsfonts
endif
endif

bin_PROGRAMS =		bsetroot $(TMP_PROGS)

bsetroot_SOURCES =	bsetroot.cc
bsetroot_LDADD =	../src/BaseDisplay.o ../src/Color.o ../src/GCCache.o ../src/Texture.o ../src/Timer.o ../src/Image.o ../src/ImageControl.o ../src/i18n.o ../src/Util.o

MAINTAINERCLEANFILES =	Makefile.in

distclean-local:
	rm -f *\~ .\#*

# local dependencies

bsetroot.o: bsetroot.cc ../config.h ../src/i18n.hh \
 ../src/../nls/blackbox-nls.hh ../src/GCCache.hh ../src/BaseDisplay.hh \
 ../src/Timer.hh ../src/Color.hh ../src/Texture.hh ../src/Util.hh \
 bsetroot.hh ../src/Image.hh
xftlsfonts.o: xftlsfonts.cc
