localedir=$(datadir)/locale
rcdir=$(datadir)/openbox
plugindir=$(libdir)/openbox/plugins
desktopfilesdir=$(datadir)/gnome/wm-properties

binary=openbox
url=http://openbox.org/

AM_CPPFLAGS=$(X_CFLAGS) $(SM_CFLAGS) $(XFT_CFLAGS) $(GLIB_CFLAGS) \
            $(GMODULE_CFLAGS) $(LIBSN_CFLAGS) $(GL_CFLAGS) $(XML_CFLAGS) \
            -DLOCALEDIR=\"$(localedir)\" \
            -DPLUGINDIR=\"$(plugindir)\" \
            -DRCDIR=\"$(rcdir)\" \
            -DG_LOG_DOMAIN=\"Openbox\" \
            -DBINARY=\"$(binary)\"

INCLUDES=-I..
LIBS=$(X_LIBS) $(SM_LIBS) $(XFT_LIBS) $(XINERAMA_LIBS) $(XKB_LIBS) \
	$(XRANDR_LIBS) $(VIDMODE_LIBS) $(XSHAPE_LIBS) $(GLIB_LIBS) \
	$(GMODULE_LIBS) $(LIBSN_LIBS) $(XML_LIBS) $(EFENCE_LIBS) \
	@LIBS@ @LIBINTL@

bin_PROGRAMS=$(binary)

openbox_LDADD=-lobrender -L../render -lobparser -L../parser
openbox_LDFLAGS=-export-dynamic
openbox_SOURCES=\
  action.h \
  action.c \
  client.h \
  client.c \
  config.h \
  config.c \
  extensions.h \
  extensions.c \
  focus.h \
  focus.c \
  frame.h \
  frame.c \
  grab.h \
  grab.c \
  menu.h \
  menu.c \
  menu_render.c \
  openbox.h \
  openbox.c \
  framerender.h \
  framerender.c \
  plugin.h \
  plugin.c \
  prop.h \
  prop.c \
  screen.h \
  screen.c \
  stacking.h \
  stacking.c \
  dispatch.h \
  dispatch.c \
  event.h \
  event.c \
  group.h \
  group.c \
  timer.h \
  timer.c \
  xerror.h \
  xerror.c \
  moveresize.h \
  moveresize.c \
  startup.h \
  startup.c \
  popup.h \
  popup.c \
  dock.h \
  dock.c \
  window.h \
  window.c \
  gettext.h \
  geom.h

desktopfiles_DATA=openbox.desktop

EXTRA_DIST=$(desktopfiles_DATA)

MAINTAINERCLEANFILES=Makefile.in

distclean-local:
	$(RM) *\~ *.orig *.rej .\#*
