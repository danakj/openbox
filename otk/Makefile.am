SUBDIRS = wrap

buttonsdir = $(pkgdatadir)/buttons
includeotkdir = $(includedir)/otk
pkgconfigdir = $(libdir)/pkgconfig

CXXFLAGS=$(XFT_CFLAGS) @CXXFLAGS@ \
-DBUTTONSDIR=\"$(buttonsdir)\"

#noinst_LIBRARIES=libotk.a
lib_LTLIBRARIES=libotk.la

libotk_la_SOURCES=rendercontrol.cc truerendercontrol.cc surface.cc \
                  renderstyle.cc rendercolor.cc pseudorendercontrol.cc \
                  display.cc font.cc \
                  property.cc rect.cc screeninfo.cc \
                  timer.cc \
                  util.cc widget.cc focuswidget.cc \
                  button.cc eventhandler.cc eventdispatcher.cc ustring.cc \
                  label.cc focuslabel.cc application.cc appwidget.cc
includeotk_HEADERS=application.hh appwidget.hh assassin.hh button.hh \
                   display.hh eventdispatcher.hh eventhandler.hh \
                   focuslabel.hh focuswidget.hh font.hh label.hh otk.hh \
                   point.hh property.hh pseudorendercontrol.hh rect.hh \
                   rendercolor.hh rendercontrol.hh renderstyle.hh \
                   rendertexture.hh screeninfo.hh strut.hh surface.hh \
                   timer.hh truerendercontrol.hh ustring.hh util.hh widget.hh \
                   ../config.h

MAINTAINERCLEANFILES= Makefile.in

pkgconfig_DATA = otk.pc

otk.pc: otk.pc.in
	@regex_cmd@ -e "s,\@prefix\@,$(prefix)," \
		-e "s,\@version\@,$(VERSION)," \
		@srcdir@/$^ > $@

distclean-local:
	rm -f *\~ *.orig *.rej .\#*

otk_test: libotk.la otk_test.cc
	$(CXX) $(CXXFLAGS) -DHAVE_CONFIG_H -I. -I. -I.. -I../src $(XFT_CFLAGS) -Wall -W -pedantic -DNDEBUG -g -O2 -o otk_test otk_test.cc $(XFT_LIBS) -L. -lotk @LIBS@

rendertest: libotk.la rendertest.cc
	$(CXX) $(CXXFLAGS) -DHAVE_CONFIG_H -I. -I. -I.. -I../src $(XFT_CFLAGS) -Wall -W -pedantic -DNDEBUG -g -O2 -o $@ rendertest.cc $(XFT_LIBS) -L. -lotk @LIBS@

# local dependencies
