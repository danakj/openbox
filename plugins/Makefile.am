XFplugindir=$(libdir)/openbox/plugins

SUBDIRS = keyboard mouse placement menu

CPPFLAGS=$(XFT_CFLAGS) $(GLIB_CFLAGS) @CPPFLAGS@ \
-DPLUGINDIR=\"$(plugindir)\"

INCLUDES=-I..

plugin_LTLIBRARIES=resistance.la

resistance_la_CPPFLAGS=-DG_LOG_DOMAIN=\"Plugin-Resistance\"
resistance_la_LDFLAGS=-module -avoid-version
resistance_la_SOURCES=resistance.c

noinst_HEADERS=

MAINTAINERCLEANFILES= Makefile.in

distclean-local:
	$(RM) *\~ *.orig *.rej .\#*
