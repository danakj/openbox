# util/Makefile.am for Blackbox - an X11 Window manager

CPPFLAGS= @CPPFLAGS@ @DEBUG@ @NLS@ @TIMEDCACHE@ @ORDEREDPSEUDO@

bin_SCRIPTS = 		bsetbg
bin_PROGRAMS =		bsetroot

bsetroot_SOURCES =	bsetroot.cc
bsetroot_LDADD =	../src/BaseDisplay.o ../src/Color.o ../src/GCCache.o ../src/Texture.o ../src/Timer.o ../src/Image.o ../src/ImageControl.o ../src/i18n.o ../src/Util.o

MAINTAINERCLEANFILES =	Makefile.in

distclean-local:
	rm -f *\~

# local dependencies

bsetroot.o: bsetroot.cc ../config.h ../src/i18n.hh \
 ../src/../nls/blackbox-nls.hh ../src/GCCache.hh ../src/BaseDisplay.hh \
 ../src/Timer.hh ../src/Color.hh ../src/Texture.hh ../src/Util.hh \
 bsetroot.hh ../src/Image.hh
