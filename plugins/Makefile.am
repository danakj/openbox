plugindir=$(libdir)/openbox/plugins

CPPFLAGS=$(XFT_CFLAGS) $(GLIB_CFLAGS) @CPPFLAGS@ \
-DPLUGINDIR=\"$(plugindir)\" \
-DG_LOG_DOMAIN=\"Openbox-Plugin\"

#engine_LTLIBRARIES=openbox.la

#openbox_la_LDFLAGS=-module -avoid-version
#openbox_la_SOURCES=openbox.c theme.c

noinst_HEADERS=

MAINTAINERCLEANFILES= Makefile.in

distclean-local:
	$(RM) *\~ *.orig *.rej .\#*
