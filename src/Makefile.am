localedir=$(datadir)/locale
scriptdir = $(libdir)/openbox/python
DEFAULT_MENU=$(pkgdatadir)/menu
DEFAULT_STYLE=$(pkgdatadir)/styles/mbdtex

CXXFLAGS=$(XFT_CFLAGS) $(PYTHON_CFLAGS) @CXXFLAGS@ \
-DDEFAULTMENU=\"$(DEFAULT_MENU)\" \
-DDEFAULTSTYLE=\"$(DEFAULT_STYLE)\" \
-DLOCALEDIR=\"$(localedir)\" \
-DSCRIPTDIR=\"$(scriptdir)\"

LIBS=$(XFT_LIBS) $(PYTHON_LIBS) @LIBS@

INCLUDES= -I..

bin_PROGRAMS= openbox3

openbox3_LDADD=-L../otk -lotk @LIBINTL@

openbox3_SOURCES= actions.cc client.cc frame.cc openbox.cc screen.cc \
                  main.cc backgroundwidget.cc labelwidget.cc \
                  buttonwidget.cc python.cc bindings.cc \
                  openbox_wrap.cc
openbox3_LDFLAGS= $(PYTHON_LDFLAGS)

script_DATA = ob.py

EXTRA_DIST = $(script_DATA)

MAINTAINERCLEANFILES= Makefile.in

distclean-local:
	rm -f *\~ *.orig *.rej .\#*

openbox.i: openbox.hh screen.hh client.hh python.hh frame.hh
	touch $@

openbox_wrap.cc: openbox.i
	swig -I../otk $(filter -I%,$(CXXFLAGS)) -python -shadow -c++ -nodefault -o $@ $<

# local dependencies
