localedir=$(datadir)/locale
scriptdir = $(libdir)/openbox/python
DEFAULT_MENU=$(pkgdatadir)/menu
DEFAULT_STYLE=$(pkgdatadir)/styles/mbdtex

CPPFLAGS=$(XFT_CFLAGS) $(PYTHON_CFLAGS) @CPPFLAGS@ \
-DDEFAULTMENU=\"$(DEFAULT_MENU)\" \
-DDEFAULTSTYLE=\"$(DEFAULT_STYLE)\" \
-DLOCALEDIR=\"$(localedir)\" \
-DSCRIPTDIR=\"$(scriptdir)\"

LIBS=$(XFT_LIBS) $(PYTHON_LIBS) @LIBS@ $(SWIG_PYTHON_LIB)

bin_PROGRAMS= openbox3

openbox3_LDADD=-L../otk -lotk @LIBINTL@
openbox3_LDFLAGS=-export-dynamic
openbox3_SOURCES= actions.cc client.cc frame.cc openbox.cc screen.cc \
                  main.cc python.cc bindings.cc config.cc
noinst_HEADERS= actions.hh bindings.hh client.hh frame.hh openbox.hh \
                python.hh screen.hh config.hh

MAINTAINERCLEANFILES= Makefile.in

distclean-local:
	$(RM) *\~ *.orig *.rej .\#*
