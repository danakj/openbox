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

openbox_SOURCES= basedisplay.cc configuration.cc screen.cc timer.cc util.cc \
                 window.cc workspace.cc xatom.cc blackbox.cc main.cc

MAINTAINERCLEANFILES= Makefile.in

distclean-local:
	rm -f *\~ *.orig *.rej .\#*

# local dependencies
basedisplay.o: basedisplay.cc basedisplay.hh timer.hh util.hh \
 ../otk/gccache.hh ../otk/color.hh
blackbox.o: blackbox.cc blackbox.hh basedisplay.hh timer.hh util.hh \
 configuration.hh xatom.hh ../otk/gccache.hh ../otk/color.hh ../otk/image.hh screen.hh \
 ../otk/texture.hh workspace.hh window.hh
configuration.o: configuration.cc configuration.hh util.hh
main.o: main.cc ../version.h blackbox.hh basedisplay.hh timer.hh \
 util.hh configuration.hh xatom.hh
screen.o: screen.cc blackbox.hh basedisplay.hh timer.hh util.hh \
 configuration.hh xatom.hh ../otk/font.hh screen.hh ../otk/color.hh ../otk/texture.hh \
 ../otk/image.hh workspace.hh ../otk/gccache.hh window.hh
timer.o: timer.cc basedisplay.hh timer.hh util.hh
util.o: util.cc util.hh
window.o: window.cc blackbox.hh basedisplay.hh timer.hh util.hh \
 configuration.hh xatom.hh ../otk/font.hh screen.hh ../otk/color.hh ../otk/texture.hh \
 ../otk/image.hh workspace.hh ../otk/gccache.hh window.hh
workspace.o: workspace.cc blackbox.hh basedisplay.hh timer.hh util.hh \
 configuration.hh xatom.hh ../otk/font.hh screen.hh ../otk/color.hh ../otk/texture.hh \
 ../otk/image.hh workspace.hh window.hh
xatom.o: xatom.cc ../config.h xatom.hh screen.hh ../otk/color.hh ../otk/texture.hh \
 util.hh ../otk/image.hh timer.hh basedisplay.hh workspace.hh blackbox.hh \
 configuration.hh
