localedir=$(datadir)/locale
DEFAULT_MENU=$(pkgdatadir)/menu
DEFAULT_STYLE=$(pkgdatadir)/styles/mbdtex

CPPFLAGS=$(XFT_CFLAGS) @CPPFLAGS@ \
-DDEFAULTMENU=\"$(DEFAULT_MENU)\" \
-DDEFAULTSTYLE=\"$(DEFAULT_STYLE)\" \
-DLOCALEDIR=\"$(localedir)\"

LIBS=$(XFT_LIBS) $(PYTHON_LDFLAGS) @LIBS@

INCLUDES= -I../otk

bin_PROGRAMS= openbox3

openbox3_LDADD=../otk/libotk.a @LIBINTL@

openbox3_SOURCES= actions.cc client.cc frame.cc openbox.cc screen.cc \
                  main.cc rootwindow.cc backgroundwidget.cc labelwidget.cc \
                  buttonwidget.cc python.cc python_client.cc

MAINTAINERCLEANFILES= Makefile.in

distclean-local:
	rm -f *\~ *.orig *.rej .\#*

# local dependencies
