localedir=$(datadir)/locale
themercdir=$(datadir)/openbox
enginedir=$(libdir)/openbox/engines
plugindir=$(libdir)/openbox/plugins

binary=openbox3
url=http://icculus.org/openbox
bugurl=http://bugzilla.icculus.org

CPPFLAGS=$(XFT_CFLAGS) $(GLIB_CFLAGS) $(GMODULE_CFLAGS) \
@CPPFLAGS@ \
-DLOCALEDIR=\"$(localedir)\" \
-DTHEMERCDIR=\"$(themercdir)\" \
-DPLUGINDIR=\"$(plugindir)\" \
-DENGINEDIR=\"$(enginedir)\" \
-DDEFAULT_ENGINE=\"openbox\" \
-DG_LOG_DOMAIN=\"Openbox\" \
-DBINARY=\"$(binary)\" \
-DURL=\"$(url)\" \
-DBUGURL=\"$(bugurl)\"

LIBS=$(XFT_LIBS) $(GLIB_LIBS) $(GMODULE_LIBS) @LIBS@

bin_PROGRAMS=$(binary)

ob3_LDADD=@LIBINTL@ ../render/librender.a
ob3_LDFLAGS=-export-dynamic
ob3_SOURCES=client.c event.c extensions.c focus.c frame.c openbox.c prop.c \
	screen.c stacking.c xerror.c themerc.c timer.c dispatch.c \
	engine.c plugin.c action.c grab.c

noinst_HEADERS=client.h event.h extensions.h focus.h frame.h geom.h gettext.h \
	openbox.h prop.h screen.h stacking.h xerror.h themerc.h dispatch.h \
	timer.h engine.h plugin.h action.h grab.h

MAINTAINERCLEANFILES= Makefile.in

distclean-local:
	$(RM) *\~ *.orig *.rej .\#*
