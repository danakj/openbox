plugindir=$(libdir)/openbox/plugins

CPPFLAGS=$(XFT_CFLAGS) $(GLIB_CFLAGS) $(LIBSN_CFLAGS) $(GL_CFLAGS) \
         $(XML_CFLAGS)  @CPPFLAGS@ \
         -DPLUGINDIR=\"$(plugindir)\"

INCLUDES=-I../.. -I../../tools

plugin_LTLIBRARIES=resistance.la
if OBCONF
plugin_LTLIBRARIES+=resistance-config.la
endif

resistance_la_CPPFLAGS=-DG_LOG_DOMAIN=\"Plugin-Resistance\"
resistance_la_LDFLAGS=-module -avoid-version
resistance_la_SOURCES=resistance.c

if OBCONF
resistance_config_la_CPPFLAGS=-DG_LOG_DOMAIN=\"Plugin-Resistance\" \
                              $(GTK_CFLAGS) $(GLADE_CFLAGS)
resistance_config_la_LDFLAGS=-module -avoid-version
resistance_config_la_SOURCES=resistance_config.c
endif

noinst_HEADERS=resistance.h

noinst_DATA=resistance.glade resistance.gladep

MAINTAINERCLEANFILES= Makefile.in

distclean-local:
	$(RM) *\~ *.orig *.rej .\#*
