CPPFLAGS=`pkg-config --cflags xft` @CPPFLAGS@

INCLUDES= -I../src

noinst_LIBRARIES=libotk.a

libotk_a_SOURCES= color.cc display.cc font.cc gccache.cc image.cc \
                  imagecontrol.cc rect.cc screeninfo.cc texture.cc

MAINTAINERCLEANFILES= Makefile.in

distclean-local:
	rm -f *\~ *.orig *.rej .\#*

# local dependencies
color.o: color.cc color.hh ../src/basedisplay.hh ../src/timer.hh ../src/util.hh
font.o: font.cc font.hh ../src/screen.hh color.hh texture.hh ../src/util.hh image.hh \
 ../src/timer.hh ../src/basedisplay.hh ../src/workspace.hh ../src/xatom.hh ../src/blackbox.hh \
 ../src/configuration.hh gccache.hh
gccache.o: gccache.cc gccache.hh ../src/basedisplay.hh ../src/timer.hh ../src/util.hh \
 color.hh
image.o: image.cc ../src/blackbox.hh ../src/basedisplay.hh ../src/timer.hh ../src/util.hh \
 ../src/configuration.hh ../src/xatom.hh gccache.hh color.hh image.hh texture.hh
imagecontrol.o: imagecontrol.cc ../src/blackbox.hh ../src/basedisplay.hh ../src/timer.hh \
 ../src/util.hh ../src/configuration.hh ../src/xatom.hh color.hh image.hh texture.hh
texture.o: texture.cc texture.hh color.hh ../src/util.hh ../src/basedisplay.hh \
 ../src/timer.hh image.hh ../src/screen.hh ../src/workspace.hh ../src/xatom.hh \
 ../src/blackbox.hh ../src/configuration.hh
