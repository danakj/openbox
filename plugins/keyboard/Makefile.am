plugindir=$(libdir)/openbox/plugins

CPPFLAGS=$(XFT_CFLAGS) $(GLIB_CFLAGS) @CPPFLAGS@ \
         -DG_LOG_DOMAIN=\"Plugin-Keyboard\"

INCLUDES=-I../..

plugin_LTLIBRARIES=keyboard.la

keyboard_la_LDFLAGS=-module -avoid-version
keyboard_la_SOURCES=keyboard.c keyparse.c translate.c tree.c

noinst_HEADERS=keyboard.h keyparse.h translate.h tree.h

MAINTAINERCLEANFILES=Makefile.in

distclean-local:
	$(RM) *\~ *.orig *.rej .\#*
