plugindir=$(libdir)/openbox/plugins

AM_CPPFLAGS=$(XFT_CFLAGS) $(GLIB_CFLAGS) $(LIBSN_CFLAGS) $(GL_CFLAGS) \
            $(XML_CFLAGS) \
            -DPLUGINDIR=\"$(plugindir)\"

INCLUDES=-I../.. -I../../tools

plugin_LTLIBRARIES=resistance.la
if OBCONF
plugin_LTLIBRARIES+=resistance-config.la
endif

resistance_la_CPPFLAGS=$(AM_CPPFLAGS) -DG_LOG_DOMAIN=\"Plugin-Resistance\"
resistance_la_LDFLAGS=-module -avoid-version
resistance_la_SOURCES=resistance.c resistance.h

if OBCONF
resistance_config_la_CPPFLAGS=$(AM_CPPFLAGS) \
                              -DG_LOG_DOMAIN=\"Plugin-Resistance\" \
                              $(GTK_CFLAGS) $(GLADE_CFLAGS)
resistance_config_la_LDFLAGS=-module -avoid-version
resistance_config_la_SOURCES=resistance_config.c
endif

noinst_DATA=resistance.glade resistance.gladep

EXTRA_DIST=$(noinst_DATA)

MAINTAINERCLEANFILES= Makefile.in

distclean-local:
	$(RM) *\~ *.orig *.rej .\#*
