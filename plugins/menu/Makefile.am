plugindir=$(libdir)/openbox/plugins

CPPFLAGS=$(XFT_CFLAGS) $(GLIB_CFLAGS) $(LIBSN_CFLAGS) @CPPFLAGS@ \
         -DG_LOG_DOMAIN=\"Plugin-Timed-Menu\"

INCLUDES=-I../..

plugin_LTLIBRARIES=timed_menu.la

timed_menu_la_LDFLAGS=-module -avoid-version
timed_menu_la_SOURCES=timed_menu.c

noinst_HEADERS=timed_menu.h

MAINTAINERCLEANFILES=Makefile.in

distclean-local:
	$(RM) *\~ *.orig *.rej .\#*
