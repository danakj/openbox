plugindir=$(libdir)/openbox/plugins

CPPFLAGS=$(XFT_CFLAGS) $(GLIB_CFLAGS) $(LIBSN_CFLAGS) $(GL_CFLAGS) @CPPFLAGS@ \
         -DG_LOG_DOMAIN=\"Plugin-Mouse\"

INCLUDES=-I../..

plugin_LTLIBRARIES=mouse.la

mouse_la_LDFLAGS=-module -avoid-version
mouse_la_SOURCES=mouse.c mouseparse.c translate.c

noinst_HEADERS=mouse.h mouseparse.h translate.h

MAINTAINERCLEANFILES=Makefile.in

distclean-local:
	$(RM) *\~ *.orig *.rej .\#*
