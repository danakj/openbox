localedir=$(datadir)/locale
plugindir=$(libdir)/openbox/plugins
rcdir=$(datadir)/openbox

binary=openbox3
url=http://icculus.org/openbox

CPPFLAGS=$(X_CFLAGS) $(XFT_CFLAGS) $(GLIB_CFLAGS) $(GMODULE_CFLAGS) \
         $(LIBSN_CFLAGS) $(GL_CFLAGS) $(XML_CFLAGS) @CPPFLAGS@ \
         -DLOCALEDIR=\"$(localedir)\" \
         -DRCDIR=\"$(rcdir)\" \
         -DPLUGINDIR=\"$(plugindir)\" \
         -DG_LOG_DOMAIN=\"Openbox\" \
         -DBINARY=\"$(binary)\"

INCLUDES=-I..
LIBS=$(X_LIBS) $(XFT_LIBS) $(XINERAMA_LIBS) $(XKB_LIBS) $(XRANDR_LIBS) \
	$(VIDMODE_LIBS) $(XSHAPE_LIBS) $(GLIB_LIBS) $(GMODULE_LIBS) \
	$(LIBSN_LIBS) $(XML_LIBS) @LIBS@ @LIBINTL@

bin_PROGRAMS=$(binary)

openbox3_LDADD=-lobrender -L../render
openbox3_LDFLAGS=-export-dynamic
openbox3_SOURCES=action.c client.c config.c parse.c \
                 extensions.c focus.c frame.c grab.c menu.c menu_render.c \
                 openbox.c framerender.c plugin.c prop.c screen.c \
                 stacking.c dispatch.c event.c group.c timer.c xerror.c \
                 moveresize.c startup.c popup.c dock.c window.c

noinst_HEADERS=action.h client.h config.h dispatch.h event.h extensions.h \
               focus.h frame.h framerender.h geom.h gettext.h grab.h group.h \
               menu.h openbox.h plugin.h prop.h screen.h \
               stacking.h timer.h xerror.h moveresize.h startup.h popup.h \
               dock.h window.h parse.h

MAINTAINERCLEANFILES=Makefile.in

distclean-local:
	$(RM) *\~ *.orig *.rej .\#*
