# util/Makefile.am for Blackbox - an X11 Window manager

CXXFLAGS= @CXXFLAGS@

#SUBDIRS = 		epist
bin_SCRIPTS = 		bsetbg
bin_PROGRAMS =		bsetroot xftlsfonts

bsetroot_SOURCES =	bsetroot.cc
bsetroot_LDADD =	../src/basedisplay.o ../src/color.o ../src/gccache.o \
			../src/texture.o ../src/timer.o ../src/image.o \
			../src/imagecontrol.o ../src/util.o

xftlsfonts_SOURCES = xftlsfonts.cc

MAINTAINERCLEANFILES =	Makefile.in

distclean-local:
	rm -f *\~ .\#*

# local dependencies