# util/Makefile.am for Blackbox - an X11 Window manager

CPPFLAGS= @CPPFLAGS@

#SUBDIRS = 		epist
bin_SCRIPTS = 		bsetbg
bin_PROGRAMS =		bsetroot xftlsfonts

bsetroot_SOURCES =	bsetroot.cc
bsetroot_LDADD =	../src/basedisplay.o ../otk/color.o ../otk/gccache.o \
			../otk/texture.o ../src/timer.o ../otk/image.o \
			../otk/imagecontrol.o ../src/util.o ../src/screeninfo.o

xftlsfonts_SOURCES = xftlsfonts.cc

INCLUDES= -I../src -I../otk

MAINTAINERCLEANFILES =	Makefile.in

distclean-local:
	rm -f *\~ .\#*

# local dependencies
