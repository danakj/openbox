plugindir=$(libdir)/openbox/plugins
rcdir=$(datadir)/openbox

CPPFLAGS=$(XFT_CFLAGS) $(GLIB_CFLAGS) @CPPFLAGS@ \
-DPLUGINDIR=\"$(plugindir)\" \
-DRCDIR=\"$(rcdir)\" \
-DG_LOG_DOMAIN=\"Plugin-Mouse\"

plugin_LTLIBRARIES=mouse.la

mouse_la_LDFLAGS=-module -avoid-version
mouse_la_SOURCES=mouse.c translate.c lex.mparse.c

lex.mparse.c: mouserc_parse.l
	$(FLEX) -Pmparse $^

rc_DATA=mouserc

noinst_HEADERS=mouse.h translate.h mouserc_parse.h

MAINTAINERCLEANFILES= Makefile.in

clean-local:
	$(RM) lex.mparse.c

distclean-local:
	$(RM) *\~ *.orig *.rej .\#*
