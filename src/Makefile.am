localedir=$(datadir)/locale
DEFAULT_MENU=$(pkgdatadir)/menu
DEFAULT_STYLE=$(pkgdatadir)/styles/mbdtex

CPPFLAGS=`pkg-config --cflags xft` @CPPFLAGS@ \
-DDEFAULTMENU=\"$(DEFAULT_MENU)\" \
-DDEFAULTSTYLE=\"$(DEFAULT_STYLE)\" \
-DLOCALEDIR=\"$(localedir)\"

LDFLAGS=`pkg-config --libs xft`

INCLUDES= -I../otk

bin_PROGRAMS= openbox

openbox_LDADD=../otk/libotk.a @LIBINTL@

openbox_SOURCES= configuration.cc screen.cc openbox.cc \
                 util.cc window.cc workspace.cc xatom.cc blackbox.cc \
                 main.cc

MAINTAINERCLEANFILES= Makefile.in

distclean-local:
	rm -f *\~ *.orig *.rej .\#*

# local dependencies
