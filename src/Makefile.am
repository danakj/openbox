localedir=$(datadir)/locale
DEFAULT_MENU=$(pkgdatadir)/menu
DEFAULT_STYLE=$(pkgdatadir)/styles/mbdtex

CPPFLAGS=`pkg-config --cflags xft` @CPPFLAGS@ \
-DDEFAULTMENU=\"$(DEFAULT_MENU)\" \
-DDEFAULTSTYLE=\"$(DEFAULT_STYLE)\" \
-DLOCALEDIR=\"$(localedir)\"

LDFLAGS=`pkg-config --libs xft`

INCLUDES= -I../otk

bin_PROGRAMS= openbox3

openbox3_LDADD=../otk/libotk.a @LIBINTL@

openbox3_SOURCES= client.cc configuration.cc screen.cc openbox.cc \
                 util.cc bbwindow.cc workspace.cc blackbox.cc \
                 main.cc xeventhandler.cc

MAINTAINERCLEANFILES= Makefile.in

distclean-local:
	rm -f *\~ *.orig *.rej .\#*

# local dependencies
