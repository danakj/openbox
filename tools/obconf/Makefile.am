localedir=$(datadir)/locale
plugindir=$(libdir)/openbox/plugins
rcdir=$(datadir)/openbox

CPPFLAGS=$(GTK_CFLAGS) $(GLADE_CFLAGS) $(GMODULE_CFLAGS) $(XML_CFLAGS) \
         @CPPFLAGS@ \
         -DLOCALEDIR=\"$(localedir)\" \
         -DRCDIR=\"$(rcdir)\" \
         -DPLUGINDIR=\"$(plugindir)\" \
         -DG_LOG_DOMAIN=\"ObConf\"

INCLUDES=-I../..
LIBS=$(GTK_LIBS) $(GLADE_LIBS) $(GMODULE_LIBS) $(XML_LIBS) @LIBS@ @LIBINTL@

bin_PROGRAMS=obconf

obconf_LDADD=-lobparser -L../../parser
obconf_LDFLAGS=-export-dynamic
obconf_SOURCES=main.c about.c plugins.c

noinst_HEADERS=obconf.h plugins.h

MAINTAINERCLEANFILES=Makefile.in

distclean-local:
	$(RM) *\~ *.orig *.rej .\#*
