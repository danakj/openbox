enginedir=$(libdir)/openbox/engines
themedir=$(datadir)/openbox/themes/openbox

CPPFLAGS=$(XFT_CFLAGS) $(GLIB_CFLAGS) @CPPFLAGS@ \
-DENGINEDIR=\"$(enginedir)\" \
-DTHEMEDIR=\"$(themedir)\" \
-DDEFAULT_THEME=\"nyz\" \
-DG_LOG_DOMAIN=\"Openbox-Engine\"

engine_LTLIBRARIES=openbox.la

openbox_la_LDFLAGS=-module -avoid-version
openbox_la_SOURCES=openbox.c theme.c

noinst_HEADERS=openbox.h theme.h

MAINTAINERCLEANFILES= Makefile.in

distclean-local:
	$(RM) *\~ *.orig *.rej .\#*
