localedir=$(datadir)/locale
DEFAULT_MENU=$(pkgdatadir)/menu
DEFAULT_STYLE=$(pkgdatadir)/styles/mbdtex

CPPFLAGS=$(XFT_CFLAGS) $(GUILE_CFLAGS) $(PYTHON_CFLAGS) @CPPFLAGS@ \
-DDEFAULTMENU=\"$(DEFAULT_MENU)\" \
-DDEFAULTSTYLE=\"$(DEFAULT_STYLE)\" \
-DLOCALEDIR=\"$(localedir)\"

LIBS=$(XFT_LIBS) $(GUILE_LDFLAGS) $(PYTHON_LDFLAGS) @LIBS@

INCLUDES= -I../otk

bin_PROGRAMS= openbox3

openbox3_LDADD=../otk/libotk.a @LIBINTL@

openbox3_SOURCES= actions.cc client.cc frame.cc openbox.cc screen.cc \
                  main.cc rootwindow.cc backgroundwidget.cc labelwidget.cc \
                  buttonwidget.cc openbox_wrap.cc
# guile.cc

MAINTAINERCLEANFILES= Makefile.in

distclean-local:
	rm -f *\~ *.orig *.rej .\#*

openbox.i: openbox.hh screen.hh client.hh
	touch openbox.i

openbox_wrap.cc: openbox.i
	swig -python -c++ -nodefault -module openbox -o openbox_wrap.cc openbox.i
#	swig -guile -c++ -nodefault -o openbox_wrap.cc openbox.i

# local dependencies
