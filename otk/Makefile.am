scriptdir = $(libdir)/openbox/python

CXXFLAGS=$(XFT_CFLAGS) $(PYTHON_CFLAGS) @CXXFLAGS@

INCLUDES= -I../src

#noinst_LIBRARIES=libotk.a
noinst_LTLIBRARIES=libotk.la

libotk_la_SOURCES= color.cc display.cc font.cc gccache.cc image.cc \
                  property.cc imagecontrol.cc rect.cc screeninfo.cc \
                  texture.cc timer.cc timerqueuemanager.cc style.cc \
                  configuration.cc util.cc widget.cc focuswidget.cc \
                  button.cc eventhandler.cc eventdispatcher.cc \
                  label.cc focuslabel.cc application.cc appwidget.cc \
                  otk_wrap.cc

libotk_la_LDFLAGS = $(XFT_LIBS) $(PYTHON_LIBS)

script_DATA = otk.py

EXTRA_DIST = $(script_DATA)

MAINTAINERCLEANFILES= Makefile.in

distclean-local:
	rm -f *\~ *.orig *.rej .\#*

otk_test: libotk.a otk_test.cc
	$(CXX) $(CXXFLAGS) -DHAVE_CONFIG_H -I. -I. -I.. -I../src $(XFT_CFLAGS) -Wall -W -pedantic -DNDEBUG -g -O2 -o otk_test otk_test.cc $(XFT_LIBS) ./libotk.a 

otk.i: $(wildcard *.hh)
	touch $@

otk_wrap.cc: otk.i
	swig $(filter -I%,$(CXXFLAGS)) -python -shadow -c++ -o $@ $<

# local dependencies
application.o: application.cc application.hh eventdispatcher.hh \
 eventhandler.hh display.hh configuration.hh timerqueuemanager.hh \
 timerqueue.hh timer.hh image.hh color.hh screeninfo.hh rect.hh \
 point.hh style.hh font.hh texture.hh util.hh widget.hh
appwidget.o: appwidget.cc appwidget.hh widget.hh rect.hh point.hh \
 texture.hh color.hh util.hh style.hh font.hh image.hh screeninfo.hh \
 timer.hh configuration.hh eventdispatcher.hh eventhandler.hh \
 application.hh display.hh timerqueuemanager.hh timerqueue.hh
button.o: button.cc button.hh focuslabel.hh focuswidget.hh widget.hh \
 rect.hh point.hh texture.hh color.hh util.hh style.hh font.hh \
 image.hh screeninfo.hh timer.hh configuration.hh eventdispatcher.hh \
 eventhandler.hh application.hh display.hh timerqueuemanager.hh \
 timerqueue.hh
color.o: color.cc color.hh display.hh screeninfo.hh rect.hh point.hh
configuration.o: configuration.cc configuration.hh util.hh
eventdispatcher.o: eventdispatcher.cc eventdispatcher.hh \
 eventhandler.hh display.hh
eventhandler.o: eventhandler.cc eventhandler.hh
focuslabel.o: focuslabel.cc focuslabel.hh focuswidget.hh widget.hh \
 rect.hh point.hh texture.hh color.hh util.hh style.hh font.hh \
 image.hh screeninfo.hh timer.hh configuration.hh eventdispatcher.hh \
 eventhandler.hh application.hh display.hh timerqueuemanager.hh \
 timerqueue.hh
focuswidget.o: focuswidget.cc focuswidget.hh widget.hh rect.hh \
 point.hh texture.hh color.hh util.hh style.hh font.hh image.hh \
 screeninfo.hh timer.hh configuration.hh eventdispatcher.hh \
 eventhandler.hh application.hh display.hh timerqueuemanager.hh \
 timerqueue.hh
font.o: font.cc font.hh util.hh display.hh color.hh screeninfo.hh \
 rect.hh point.hh
gccache.o: gccache.cc gccache.hh display.hh color.hh assassin.hh \
 screeninfo.hh rect.hh point.hh
image.o: image.cc display.hh gccache.hh color.hh image.hh \
 screeninfo.hh rect.hh point.hh timer.hh texture.hh util.hh
imagecontrol.o: imagecontrol.cc display.hh color.hh image.hh \
 screeninfo.hh rect.hh point.hh timer.hh texture.hh util.hh
label.o: label.cc label.hh widget.hh rect.hh point.hh texture.hh \
 color.hh util.hh style.hh font.hh image.hh screeninfo.hh timer.hh \
 configuration.hh eventdispatcher.hh eventhandler.hh application.hh \
 display.hh timerqueuemanager.hh timerqueue.hh
otk_test.o: otk_test.cc application.hh eventdispatcher.hh \
 eventhandler.hh display.hh configuration.hh timerqueuemanager.hh \
 timerqueue.hh timer.hh image.hh color.hh screeninfo.hh rect.hh \
 point.hh style.hh font.hh texture.hh util.hh focuswidget.hh widget.hh \
 appwidget.hh button.hh focuslabel.hh
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
 configuration.hh eventdispatcher.hh eventhandler.hh application.hh \
 display.hh timerqueuemanager.hh timerqueue.hh assassin.hh
