plugindir=$(libdir)/openbox/plugins

CPPFLAGS=$(XFT_CFLAGS) $(GLIB_CFLAGS) @CPPFLAGS@ \
-DPLUGINDIR=\"$(plugindir)\" \
-DG_LOG_DOMAIN=\"Plugin-Mouse\"

plugin_LTLIBRARIES=mouse.la

mouse_la_LDFLAGS=-module -avoid-version
mouse_la_SOURCES=mouse.c translate.c

noinst_HEADERS=mouse.h translate.h

MAINTAINERCLEANFILES= Makefile.in

distclean-local:
	$(RM) *\~ *.orig *.rej .\#*
