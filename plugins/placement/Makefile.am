plugindir=$(libdir)/openbox/plugins

CPPFLAGS=$(XFT_CFLAGS) $(GLIB_CFLAGS) @CPPFLAGS@ \
-DPLUGINDIR=\"$(plugindir)\" \
-DG_LOG_DOMAIN=\"Plugin-Placement\"

plugin_LTLIBRARIES=placement.la

placement_la_LDFLAGS=-module -avoid-version
placement_la_SOURCES=placement.c

noinst_HEADERS=

MAINTAINERCLEANFILES=Makefile.in

distclean-local:
	$(RM) *\~ *.orig *.rej .\#*
