CPPFLAGS=$(XFT_CFLAGS) @CPPFLAGS@

INCLUDES= -I../src

noinst_LIBRARIES=libotk.a

libotk_a_SOURCES= color.cc display.cc font.cc gccache.cc image.cc property.cc \
                  imagecontrol.cc rect.cc screeninfo.cc texture.cc timer.cc \
                  timerqueuemanager.cc style.cc configuration.cc util.cc \
                  widget.cc focuswidget.cc button.cc

MAINTAINERCLEANFILES= Makefile.in

distclean-local:
	rm -f *\~ *.orig *.rej .\#*

# local dependencies
color.o: color.cc color.hh display.hh screeninfo.hh rect.hh
gccache.o: gccache.cc gccache.hh display.hh color.hh assassin.hh \
 screeninfo.hh rect.hh
rect.o: rect.cc rect.hh
configuration.o: configuration.cc configuration.hh
style.o: style.cc style.hh
util.o: util.cc util.hh
widget.o: widget.cc widget.hh
focuswidget.o: focuswidget.cc focuswidget.hh
button.o: button.cc button.hh
