buttonsdir = $(pkgdatadir)/buttons
includeotkdir = $(includedir)/otk
pkgconfigdir = $(libdir)/pkgconfig

CPPFLAGS=$(XFT_CFLAGS) @CPPFLAGS@ -DBUTTONSDIR=\"$(buttonsdir)\"

lib_LTLIBRARIES=libotk.la

libotk_la_SOURCES=rendercontrol.cc truerendercontrol.cc surface.cc util.cc \
                  renderstyle.cc rendercolor.cc pseudorendercontrol.cc \
                  display.cc font.cc screeninfo.cc property.cc timer.cc \
                  eventdispatcher.cc eventhandler.cc ustring.cc \
                  widget.cc application.cc label.cc appwidget.cc button.cc \
                  otk.cc messagedialog.cc

#focuswidget.cc focuslabel.cc

includeotk_HEADERS=application.hh appwidget.hh assassin.hh button.hh \
                   display.hh eventdispatcher.hh eventhandler.hh font.hh \
                   label.hh otk.hh point.hh property.hh pseudorendercontrol.hh\
                   rect.hh rendercolor.hh rendercontrol.hh renderstyle.hh \
                   rendertexture.hh screeninfo.hh size.hh strut.hh surface.hh \
                   timer.hh truerendercontrol.hh ustring.hh util.hh widget.hh \
                   messagedialog.hh ../config.h

EXTRA_DIST = otk.pc.in

DISTCLEANFILES = otk.pc
MAINTAINERCLEANFILES= Makefile.in

pkgconfig_DATA = otk.pc

otk.pc: otk.pc.in
	@regex_cmd@ -e "s,\@prefix\@,$(prefix)," \
		-e "s,\@version\@,$(VERSION)," \
		@srcdir@/$^ > $@

distclean-local:
	$(RM) *\~ *.orig *.rej .\#*

uninstall-am:
	-rmdir -p $(includeotkdir)
	-rmdir -p $(pkgconfigdir)

otk_test: libotk.la otk_test.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -DHAVE_CONFIG_H -I. -I. -I.. -I../src $(XFT_CFLAGS) -Wall -W -pedantic -DNDEBUG -g -O2 -o otk_test otk_test.cc $(XFT_LIBS) -L. -lotk @LIBS@

rendertest: libotk.la rendertest.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -DHAVE_CONFIG_H -I. -I. -I.. -I../src $(XFT_CFLAGS) -Wall -W -pedantic -DNDEBUG -g -O2 -o $@ rendertest.cc $(XFT_LIBS) -L. -lotk @LIBS@

# local dependencies
