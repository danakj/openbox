DEFAULT_MENU=$(pkgdatadir)/menu
DEFAULT_STYLE=$(pkgdatadir)/styles/mbdtex

CXXFLAGS=`pkg-config --cflags xft` @CXXFLAGS@ \
-DDEFAULTMENU=\"$(DEFAULT_MENU)\" \
-DDEFAULTSTYLE=\"$(DEFAULT_STYLE)\" \
-Dmultibyte\(\)=True

LDFLAGS=`pkg-config --libs xft`

bin_PROGRAMS= openbox

openbox_SOURCES= basedisplay.cc color.cc configuration.cc font.cc gccache.cc image.cc imagecontrol.cc screen.cc texture.cc timer.cc util.cc window.cc workspace.cc xatom.cc blackbox.cc main.cc

MAINTAINERCLEANFILES= Makefile.in

distclean-local:
	rm -f *\~ *.orig *.rej .\#*

# local dependencies
basedisplay.o: basedisplay.cc basedisplay.hh timer.hh util.hh \
 gccache.hh color.hh
blackbox.o: blackbox.cc blackbox.hh basedisplay.hh timer.hh util.hh \
 configuration.hh xatom.hh gccache.hh color.hh image.hh screen.hh \
 texture.hh workspace.hh window.hh
color.o: color.cc color.hh basedisplay.hh timer.hh util.hh
configuration.o: configuration.cc configuration.hh util.hh
font.o: font.cc font.hh screen.hh color.hh texture.hh util.hh image.hh \
 timer.hh basedisplay.hh workspace.hh xatom.hh blackbox.hh \
 configuration.hh gccache.hh
gccache.o: gccache.cc gccache.hh basedisplay.hh timer.hh util.hh \
 color.hh
image.o: image.cc blackbox.hh basedisplay.hh timer.hh util.hh \
 configuration.hh xatom.hh gccache.hh color.hh image.hh texture.hh
imagecontrol.o: imagecontrol.cc blackbox.hh basedisplay.hh timer.hh \
 util.hh configuration.hh xatom.hh color.hh image.hh texture.hh
main.o: main.cc ../version.h blackbox.hh basedisplay.hh timer.hh \
 util.hh configuration.hh xatom.hh
screen.o: screen.cc blackbox.hh basedisplay.hh timer.hh util.hh \
 configuration.hh xatom.hh font.hh screen.hh color.hh texture.hh \
 image.hh workspace.hh gccache.hh window.hh
texture.o: texture.cc texture.hh color.hh util.hh basedisplay.hh \
 timer.hh image.hh screen.hh workspace.hh xatom.hh blackbox.hh \
 configuration.hh
timer.o: timer.cc basedisplay.hh timer.hh util.hh
util.o: util.cc util.hh
window.o: window.cc blackbox.hh basedisplay.hh timer.hh util.hh \
 configuration.hh xatom.hh font.hh screen.hh color.hh texture.hh \
 image.hh workspace.hh gccache.hh window.hh
workspace.o: workspace.cc blackbox.hh basedisplay.hh timer.hh util.hh \
 configuration.hh xatom.hh font.hh screen.hh color.hh texture.hh \
 image.hh workspace.hh window.hh
xatom.o: xatom.cc ../config.h xatom.hh screen.hh color.hh texture.hh \
 util.hh image.hh timer.hh basedisplay.hh workspace.hh blackbox.hh \
 configuration.hh
