plugindir=$(libdir)/openbox/plugins

CPPFLAGS=$(XFT_CFLAGS) $(GLIB_CFLAGS) $(LIBSN_CFLAGS) $(GL_CFLAGS) @CPPFLAGS@ \
         -DG_LOG_DOMAIN=\"Plugin-Placement\"

INCLUDES=-I../..

plugin_LTLIBRARIES=placement.la

placement_la_LDFLAGS=-module -avoid-version
placement_la_SOURCES=history.c placement.c

noinst_HEADERS=history.h

MAINTAINERCLEANFILES=Makefile.in

distclean-local:
	$(RM) *\~ *.orig *.rej .\#*
