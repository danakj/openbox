# util/epist/Makefile.am for Blackbox - an X11 Window manager

CPPFLAGS= @CPPFLAGS@ @DEBUG@

bin_PROGRAMS =		epist

epist_SOURCES =		epist.cc process.cc window.cc
epist_LDADD =	    ../../src/XAtom.o

MAINTAINERCLEANFILES =	Makefile.in

distclean-local:
	rm -f *\~ .\#*

# local dependencies

epist.o: epist.cc epist.hh process.hh
process.o: process.cc process.hh /usr/include/X11/Xlib.h epist.hh
