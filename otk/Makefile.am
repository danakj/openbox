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

otk_test: libotk.a otk_test.cc
	$(CXX) $(CPPFLAGS) -DHAVE_CONFIG_H -I. -I. -I.. -I../src $(XFT_CFLAGS) -Wall -W -pedantic -DNDEBUG -g -O2 -o otk_test otk_test.cc $(XFT_LIBS) ./libotk.a 

# local dependencies
button.o: button.cc button.hh focuswidget.hh widget.hh rect.hh \
 point.hh texture.hh color.hh util.hh style.hh font.hh image.hh \
 screeninfo.hh timer.hh configuration.hh
color.o: color.cc color.hh display.hh screeninfo.hh rect.hh point.hh
configuration.o: configuration.cc configuration.hh util.hh
focuswidget.o: focuswidget.cc focuswidget.hh widget.hh rect.hh \
 point.hh texture.hh color.hh util.hh style.hh font.hh image.hh \
 screeninfo.hh timer.hh configuration.hh
font.o: font.cc font.hh util.hh gccache.hh display.hh color.hh \
 screeninfo.hh rect.hh point.hh
gccache.o: gccache.cc gccache.hh display.hh color.hh assassin.hh \
 screeninfo.hh rect.hh point.hh
image.o: image.cc display.hh gccache.hh color.hh image.hh \
 screeninfo.hh rect.hh point.hh timer.hh texture.hh util.hh
imagecontrol.o: imagecontrol.cc display.hh color.hh image.hh \
 screeninfo.hh rect.hh point.hh timer.hh texture.hh util.hh
otk_test.o: otk_test.cc focuswidget.hh widget.hh rect.hh point.hh \
 texture.hh color.hh util.hh style.hh font.hh image.hh screeninfo.hh \
 timer.hh configuration.hh button.hh display.hh timerqueuemanager.hh \
 timerqueue.hh
property.o: property.cc property.hh screeninfo.hh rect.hh point.hh \
 display.hh
rect.o: rect.cc rect.hh point.hh
screeninfo.o: screeninfo.cc screeninfo.hh rect.hh point.hh display.hh \
 util.hh
style.o: style.cc display.hh util.hh style.hh color.hh font.hh \
 texture.hh image.hh screeninfo.hh rect.hh point.hh timer.hh \
 configuration.hh
texture.o: texture.cc texture.hh color.hh util.hh display.hh image.hh \
 screeninfo.hh rect.hh point.hh timer.hh
timer.o: timer.cc timer.hh timerqueuemanager.hh timerqueue.hh
timerqueuemanager.o: timerqueuemanager.cc timerqueuemanager.hh \
 timerqueue.hh timer.hh display.hh
util.o: util.cc util.hh
widget.o: widget.cc widget.hh rect.hh point.hh texture.hh color.hh \
 util.hh style.hh font.hh image.hh screeninfo.hh timer.hh \
 configuration.hh display.hh assassin.hh
