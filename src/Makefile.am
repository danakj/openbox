localedir=$(datadir)/locale
DEFAULT_MENU=$(pkgdatadir)/menu
DEFAULT_STYLE=$(pkgdatadir)/styles/mbdtex

CPPFLAGS=$(XFT_CFLAGS) $(PYTHON_CFLAGS) @CPPFLAGS@ \
-DDEFAULTMENU=\"$(DEFAULT_MENU)\" \
-DDEFAULTSTYLE=\"$(DEFAULT_STYLE)\" \
-DLOCALEDIR=\"$(localedir)\"

LIBS=$(XFT_LIBS) $(PYTHON_LIBS) @LIBS@

INCLUDES= -I../otk

bin_PROGRAMS= openbox3

openbox3_LDADD=-L../otk -lotk @LIBINTL@

openbox3_SOURCES= actions.cc client.cc frame.cc openbox.cc screen.cc \
                  main.cc rootwindow.cc backgroundwidget.cc labelwidget.cc \
                  buttonwidget.cc python.cc openbox_wrap.cc

MAINTAINERCLEANFILES= Makefile.in

distclean-local:
	rm -f *\~ *.orig *.rej .\#*

openbox.i: openbox.hh screen.hh client.hh
	touch $@

openbox_wrap.cc: openbox.i
	swig $(filter -I%,$(CFLAGS)) -python -c++ -nodefault -o $@ $<

# local dependencies
