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
