plugindir=$(libdir)/openbox/plugins

CPPFLAGS=$(XFT_CFLAGS) $(GLIB_CFLAGS) @CPPFLAGS@ \
-DPLUGINDIR=\"$(plugindir)\" \
-DG_LOG_DOMAIN=\"Openbox-Plugin\"

plugin_LTLIBRARIES=focus.la

focus_la_LDFLAGS=-module -avoid-version
focus_la_SOURCES=focus.c

noinst_HEADERS=

MAINTAINERCLEANFILES= Makefile.in

distclean-local:
	$(RM) *\~ *.orig *.rej .\#*
