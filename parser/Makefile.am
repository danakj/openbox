localedir=$(datadir)/locale
rcdir=$(datadir)/openbox

CPPFLAGS=$(GLIB_CFLAGS) $(XML_CFLAGS) @CPPFLAGS@ \
         -DG_LOG_DOMAIN=\"Parser\" \
         -DLOCALEDIR=\"$(localedir)\" \
         -DRCDIR=\"$(rcdir)\"

INCLUDES=-I..
LIBS=$(GLIB_LIBS) $(XML_LIBS) @LIBS@

lib_LTLIBRARIES=libobparser.la
libobparser_la_SOURCES=parse.c

noinst_HEADERS=parse.h

MAINTAINERCLEANFILES=Makefile.in

distclean-local:
	$(RM) *\~ *.orig *.rej .\#*
