buttonsdir = $(pkgdatadir)/buttons
scriptdir = $(libdir)/openbox/python
includeotkdir = $(includedir)/otk

CXXFLAGS=$(XFT_CFLAGS) $(PYTHON_CFLAGS) @CXXFLAGS@ \
-DBUTTONSDIR=\"$(buttonsdir)\" -DSWIG_GLOBAL
# -DSWIG_GLOBAL means we don't have to link libswigpy cuz libotk will export
# everything it would

#INCLUDES= -I../src

#noinst_LIBRARIES=libotk.a
lib_LTLIBRARIES=libotk.la

libotk_la_SOURCES=rendercontrol.cc truerendercontrol.cc surface.cc \
                  renderstyle.cc rendercolor.cc pseudorendercontrol.cc \
                  display.cc font.cc \
                  property.cc rect.cc screeninfo.cc \
                  timer.cc \
                  util.cc widget.cc focuswidget.cc \
                  button.cc eventhandler.cc eventdispatcher.cc ustring.cc \
                  label.cc focuslabel.cc application.cc appwidget.cc \
                  otk_wrap.cc
includeotk_HEADERS=application.hh appwidget.hh assassin.hh button.hh \
                   display.hh eventdispatcher.hh eventhandler.hh \
                   focuslabel.hh focuswidget.hh font.hh label.hh otk.hh \
                   point.hh property.hh pseudorendercontrol.hh rect.hh \
                   rendercolor.hh rendercontrol.hh renderstyle.hh \
                   rendertexture.hh screeninfo.hh strut.hh surface.hh \
                   timer.hh truerendercontrol.hh ustring.hh util.hh widget.hh \
                   ../config.h
libotk_la_LDFLAGS = $(XFT_LIBS) $(PYTHON_LIBS) @LIBS@

MAINTAINERCLEANFILES= Makefile.in

script_DATA = otk.py

EXTRA_DIST = $(script_DATA)

distclean-local:
	rm -f *\~ *.orig *.rej .\#*

otk_test: libotk.la otk_test.cc
	$(CXX) $(CXXFLAGS) -DHAVE_CONFIG_H -I. -I. -I.. -I../src $(XFT_CFLAGS) -Wall -W -pedantic -DNDEBUG -g -O2 -o otk_test otk_test.cc $(XFT_LIBS) -L. -lotk @LIBS@

rendertest: libotk.la rendertest.cc
	$(CXX) $(CXXFLAGS) -DHAVE_CONFIG_H -I. -I. -I.. -I../src $(XFT_CFLAGS) -Wall -W -pedantic -DNDEBUG -g -O2 -o $@ rendertest.cc $(XFT_LIBS) -L. -lotk @LIBS@

%.py: %_wrap.cc

otk.i: $(wildcard *.hh)
	touch $@

%_wrap.cc: %.i
	swig $(filter -I%,$(CXXFLAGS)) -python -shadow -c++ -nodefault -o $@ $<

# local dependencies
