plugindir=$(libdir)/openbox/plugins
rcdir=$(datadir)/openbox

CPPFLAGS=$(XFT_CFLAGS) $(GLIB_CFLAGS) @CPPFLAGS@ \
-DPLUGINDIR=\"$(plugindir)\" \
-DRCDIR=\"$(rcdir)\" \
-DG_LOG_DOMAIN=\"Plugin-Keyboard\"

plugin_LTLIBRARIES=keyboard.la

keyboard_la_LDFLAGS=-module -avoid-version
keyboard_la_SOURCES=keyboard.c tree.c translate.c y.tab.c lex.kparse.c

lex.kparse.c: keysrc.l y.tab.c
	$(FLEX) -Pkparse $<

y.tab.c: keysrc.yacc
	$(YACC) -d -p kparse $<

rc_DATA=keysrc

noinst_HEADERS=keyboard.h tree.h translate.h keysrc.h

MAINTAINERCLEANFILES= Makefile.in

clean-local:
	$(RM) y.tab.c lex.kparse.c

distclean-local:
	$(RM) *\~ *.orig *.rej .\#*
