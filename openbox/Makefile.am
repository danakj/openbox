localedir=$(datadir)/locale
enginedir=$(libdir)/openbox/engines
plugindir=$(libdir)/openbox/plugins
rcdir=$(datadir)/openbox

binary=openbox3
url=http://icculus.org/openbox
bugurl=http://bugzilla.icculus.org

CPPFLAGS=$(XFT_CFLAGS) $(GLIB_CFLAGS) $(GMODULE_CFLAGS) \
@CPPFLAGS@ \
-DLOCALEDIR=\"$(localedir)\" \
-DRCDIR=\"$(rcdir)\" \
-DPLUGINDIR=\"$(plugindir)\" \
-DENGINEDIR=\"$(enginedir)\" \
-DDEFAULT_ENGINE=\"openbox\" \
-DG_LOG_DOMAIN=\"Openbox\" \
-DBINARY=\"$(binary)\" \
-DURL=\"$(url)\" \
-DBUGURL=\"$(bugurl)\"

LIBS=$(XFT_LIBS) $(GLIB_LIBS) $(GMODULE_LIBS) @LIBS@

bin_PROGRAMS=$(binary)

openbox3_LDADD=@LIBINTL@ ../render/librender.a
openbox3_LDFLAGS=-export-dynamic
openbox3_SOURCES=client.c event.c extensions.c focus.c frame.c openbox.c \
	prop.c screen.c stacking.c xerror.c timer.c dispatch.c \
	engine.c plugin.c action.c grab.c lex.cparse.c config.c menu.c

noinst_HEADERS=client.h event.h extensions.h focus.h frame.h geom.h gettext.h \
	openbox.h prop.h screen.h stacking.h xerror.h dispatch.h \
	timer.h engine.h plugin.h action.h grab.h config.h menu.h

lex.cparse.c: cparse.l
	$(FLEX) -Pcparse $^

MAINTAINERCLEANFILES= Makefile.in

distclean-local:
	$(RM) *\~ *.orig *.rej .\#*
