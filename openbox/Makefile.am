localedir=$(datadir)/locale
plugindir=$(libdir)/openbox/plugins
rcdir=$(datadir)/openbox

binary=openbox3
url=http://icculus.org/openbox

CPPFLAGS=$(X_CFLAGS) $(XFT_CFLAGS) $(GLIB_CFLAGS) $(GMODULE_CFLAGS) \
          @CPPFLAGS@ \
         -DLOCALEDIR=\"$(localedir)\" \
         -DRCDIR=\"$(rcdir)\" \
         -DPLUGINDIR=\"$(plugindir)\" \
         -DG_LOG_DOMAIN=\"Openbox\" \
         -DBINARY=\"$(binary)\"

INCLUDES=-I..
LIBS=$(X_LIBS) $(XFT_LIBS) $(XINERAMA_LIBS) $(GLIB_LIBS) $(GMODULE_LIBS) @LIBS@

bin_PROGRAMS=$(binary)

openbox3_LDADD=@LIBINTL@ -lobrender -L../render
openbox3_LDFLAGS=-export-dynamic
openbox3_SOURCES=parse.tab.c parse.lex.c action.c client.c config.c \
                 extensions.c focus.c frame.c grab.c menu.c menu_render.c \
                 openbox.c framerender.c parse.c plugin.c prop.c screen.c \
                 stacking.c dispatch.c event.c group.c timer.c xerror.c \
                 moveresize.c startup.c

noinst_HEADERS=action.h client.h config.h dispatch.h event.h extensions.h \
               focus.h frame.h framerender.h geom.h gettext.h grab.h group.h \
               menu.h openbox.h parse.h parse.tab.h plugin.h prop.h screen.h \
               stacking.h timer.h xerror.h moveresize.h startup.h

# kill the implicit .c.y rule
%.c: %.y
	@

%.lex.c: %.l
	$(FLEX) -o$@ $<

%.tab.c: %.y
	$(BISON) -d -o $@ $<

MAINTAINERCLEANFILES=Makefile.in

clean-local:
	$(RM) parse.lex.c parse.tab.c parse.tab.h

distclean-local:
	$(RM) *\~ *.orig *.rej .\#*
