CPPFLAGS=`pkg-config --cflags xft` @CPPFLAGS@

INCLUDES= -I../src

noinst_LIBRARIES=libotk.a

libotk_a_SOURCES= color.cc display.cc font.cc gccache.cc image.cc \
                  imagecontrol.cc rect.cc screeninfo.cc texture.cc timer.cc

MAINTAINERCLEANFILES= Makefile.in

distclean-local:
	rm -f *\~ *.orig *.rej .\#*

# local dependencies
color.o: color.cc color.hh display.hh screeninfo.hh rect.hh
gccache.o: gccache.cc gccache.hh display.hh color.hh assassin.hh \
 screeninfo.hh rect.hh
rect.o: rect.cc rect.hh
