plugindir=$(libdir)/openbox/plugins

CPPFLAGS=$(XFT_CFLAGS) $(GLIB_CFLAGS) @CPPFLAGS@ \
-DPLUGINDIR=\"$(plugindir)\" \
-DG_LOG_DOMAIN=\"Plugin-Placement\"

plugin_LTLIBRARIES=placement.la

placement_la_LDFLAGS=-module -avoid-version
placement_la_SOURCES=placement.c history.c

noinst_HEADERS=history.h

MAINTAINERCLEANFILES=Makefile.in

distclean-local:
	$(RM) *\~ *.orig *.rej .\#*
