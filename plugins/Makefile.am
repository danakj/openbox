plugindir=$(libdir)/openbox/plugins

SUBDIRS = keyboard mouse placement

CPPFLAGS=$(XFT_CFLAGS) $(GLIB_CFLAGS) @CPPFLAGS@ \
-DPLUGINDIR=\"$(plugindir)\"

plugin_LTLIBRARIES=focus.la resistance.la

focus_la_CPPFLAGS=-DG_LOG_DOMAIN=\"Plugin-Focus\"
focus_la_LDFLAGS=-module -avoid-version
focus_la_SOURCES=focus.c

resistance_la_CPPFLAGS=-DG_LOG_DOMAIN=\"Plugin-Resistance\"
resistance_la_LDFLAGS=-module -avoid-version
resistance_la_SOURCES=resistance.c

noinst_HEADERS=

MAINTAINERCLEANFILES= Makefile.in

distclean-local:
	$(RM) *\~ *.orig *.rej .\#*
