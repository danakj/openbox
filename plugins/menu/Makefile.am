plugindir=$(libdir)/openbox/plugins

CPPFLAGS=$(XFT_CFLAGS) $(GLIB_CFLAGS) $(LIBSN_CFLAGS) $(XML_CFLAGS)
         \@CPPFLAGS@ \
         -DG_LOG_DOMAIN=\"Plugin-Timed-Menu\"

INCLUDES=-I../..

plugin_LTLIBRARIES=timed_menu.la fifo_menu.la client_menu.la

timed_menu_la_LDFLAGS=-module -avoid-version
timed_menu_la_SOURCES=timed_menu.c

fifo_menu_la_LDFLAGS=-module -avoid-version
fifo_menu_la_SOURCES=fifo_menu.c

client_menu_la_LDFLAGS=-module -avoid-version
client_menu_la_SOURCES=client_menu.c

noinst_HEADERS=timed_menu.h fifo_menu.h

MAINTAINERCLEANFILES=Makefile.in

distclean-local:
	$(RM) *\~ *.orig *.rej .\#*
