plugindir=$(libdir)/openbox/plugins

CPPFLAGS=$(XFT_CFLAGS) $(GLIB_CFLAGS) @CPPFLAGS@ \
-DPLUGINDIR=\"$(plugindir)\" \
-DG_LOG_DOMAIN=\"Plugin-Keyboard\"

plugin_LTLIBRARIES=keyboard.la

keyboard_la_LDFLAGS=-module -avoid-version
keyboard_la_SOURCES=keyboard.c tree.c translate.c keyaction.c

noinst_HEADERS=keyboard.h tree.h translate.h keyaction.h

MAINTAINERCLEANFILES= Makefile.in

distclean-local:
	$(RM) *\~ *.orig *.rej .\#*
