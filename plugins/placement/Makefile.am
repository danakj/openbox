plugindir=$(libdir)/openbox/plugins

AM_CPPFLAGS=$(XFT_CFLAGS) $(GLIB_CFLAGS) $(LIBSN_CFLAGS) $(GL_CFLAGS) \
            $(XML_CFLAGS) \
            -DG_LOG_DOMAIN=\"Plugin-Placement\"

INCLUDES=-I../..

plugin_LTLIBRARIES=placement.la

placement_la_LDFLAGS=-module -avoid-version
placement_la_SOURCES=history.c history.h placement.c

MAINTAINERCLEANFILES=Makefile.in

distclean-local:
	$(RM) *\~ *.orig *.rej .\#*
