localedir=$(datadir)/locale
DEFAULT_MENU=$(pkgdatadir)/menu
DEFAULT_STYLE=$(pkgdatadir)/styles/mbdtex

CPPFLAGS=$(XFT_CFLAGS) $(GUILE_CFLAGS) @CPPFLAGS@ \
-DDEFAULTMENU=\"$(DEFAULT_MENU)\" \
-DDEFAULTSTYLE=\"$(DEFAULT_STYLE)\" \
-DLOCALEDIR=\"$(localedir)\"

LIBS=$(XFT_LIBS) $(GUILE_LDFLAGS) @LIBS@

INCLUDES= -I../otk

bin_PROGRAMS= openbox3

openbox3_LDADD=../otk/libotk.a @LIBINTL@

openbox3_SOURCES= actions.cc client.cc frame.cc openbox.cc screen.cc \
                  main.cc rootwindow.cc backgroundwidget.cc labelwidget.cc \
                  buttonwidget.cc

MAINTAINERCLEANFILES= Makefile.in

distclean-local:
	rm -f *\~ *.orig *.rej .\#*

# local dependencies
