# util/epist/Makefile.am for Blackbox - an X11 Window manager

CPPFLAGS= @CPPFLAGS@ @DEBUG@

bin_PROGRAMS =		epist

epist_SOURCES =		epist.cc window.cc screen.cc main.cc
epist_LDADD =		../../src/XAtom.o ../../src/BaseDisplay.o \
					../../src/Util.o ../../src/i18n.o \
					../../src/GCCache.o ../../src/Color.o ../../src/Texture.o \
					../../src/Timer.o ../../src/Image.o ../../src/ImageControl.o

MAINTAINERCLEANFILES =	Makefile.in

distclean-local:
	rm -f *\~ .\#*

# local dependencies

epist.o: epist.cc epist.hh ../../src/BaseDisplay.hh ../../src/Timer.hh \
  ../../src/Util.hh process.hh screen.hh window.hh ../../src/XAtom.hh
main.o: main.cc epist.hh ../../src/BaseDisplay.hh ../../src/Timer.hh \
  ../../src/Util.hh ../../src/i18n.hh ../../nls/blackbox-nls.hh
process.o: process.cc
screen.o: screen.cc ../../src/XAtom.hh screen.hh window.hh epist.hh \
  ../../src/BaseDisplay.hh ../../src/Timer.hh ../../src/Util.hh
window.o: window.cc window.hh epist.hh ../../src/BaseDisplay.hh \
  ../../src/Timer.hh ../../src/Util.hh ../../src/XAtom.hh
