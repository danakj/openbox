enginedir=$(libdir)/openbox/engines
themedir=$(datadir)/openbox/themes/openbox

CPPFLAGS=$(XFT_CFLAGS) $(GLIB_CFLAGS) @CPPFLAGS@ \
-DENGINEDIR=\"$(enginedir)\" \
-DTHEMEDIR=\"$(themedir)\" \
-DDEFAULT_THEME=\"nyz\" \
-DDEFAULT_FONT=\"Sans-7\" \
-DG_LOG_DOMAIN=\"Openbox-Engine\"

engine_LTLIBRARIES=openbox.la

openbox_la_LDFLAGS=-module -avoid-version
openbox_la_SOURCES=obengine.c obtheme.c obrender.c

noinst_HEADERS=obengine.h obtheme.h obrender.h

MAINTAINERCLEANFILES= Makefile.in

distclean-local:
	$(RM) *\~ *.orig *.rej .\#*
