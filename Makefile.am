SUBDIRS = m4 po

themedir        = $(datadir)/openbox/themes
localedir       = $(datadir)/locale
rcdir           = $(datadir)/openbox
desktopfilesdir = $(datadir)/gnome/wm-properties
pixmapdir       = $(datadir)/pixmaps

theme = thebear

AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I m4

INCLUDES = -I.

noinst_PROGRAMS = \
	render/rendertest

lib_LTLIBRARIES = \
	render/libobrender.la \
	parser/libobparser.la

bin_PROGRAMS = \
	openbox/openbox \
	tools/kdetrayproxy/kdetrayproxy

if OBCONF
bin_PROGRAMS += \
	tools/obconf/obconf
endif

## render ##

render_rendertest_CPPFLAGS = \
	$(XFT_CFLAGS) \
	$(GLIB_CFLAGS) \
	-DG_LOG_DOMAIN=\"RenderTest\"
render_rendertest_LDADD = render/libobrender.la
render_rendertest_SOURCES = render/test.c

render_libobrender_la_CPPFLAGS = \
	$(X_CFLAGS) \
	$(XFT_CFLAGS) \
	$(GLIB_CFLAGS) \
	-DG_LOG_DOMAIN=\"ObRender\" \
	-DDEFAULT_THEME=\"$(theme)\" \
	-DTHEMEDIR=\"$(themedir)\"
render_libobrender_la_LIBADD = \
	$(X_LIBS) \
	$(XFT_LIBS) \
	$(GLIB_LIBS)
render_libobrender_la_SOURCES = \
	gettext.h \
	render/color.h \
	render/color.c \
	render/font.h \
	render/font.c \
	render/geom.h \
	render/gradient.h \
	render/gradient.c \
	render/image.h \
	render/image.c \
	render/instance.h \
	render/instance.c \
	render/mask.h \
	render/mask.c \
	render/render.h \
	render/render.c \
	render/theme.h \
	render/theme.c

## parser ##

parser_libobparser_la_CPPFLAGS = \
	$(GLIB_CFLAGS) \
	$(XML_CFLAGS) \
	-DG_LOG_DOMAIN=\"ObParser\" \
	-DLOCALEDIR=\"$(localedir)\" \
	-DRCDIR=\"$(rcdir)\"
parser_libobparser_la_LIBADD = \
	$(GLIB_LIBS) \
	$(XML_LIBS)
parser_libobparser_la_SOURCES = \
	parser/parse.h \
	parser/parse.c

## openbox ##

openbox_openbox_CPPFLAGS = \
	$(X_CFLAGS) \
	$(SM_CFLAGS) \
	$(XFT_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(LIBSN_CFLAGS) \
	$(XML_CFLAGS) \
	-DLOCALEDIR=\"$(localedir)\" \
	-DRCDIR=\"$(rcdir)\" \
	-DG_LOG_DOMAIN=\"Openbox\"
openbox_openbox_LDADD = \
	$(SM_LIBS) \
	$(XINERAMA_LIBS) \
	$(XKB_LIBS) \
	$(XRANDR_LIBS) \
	$(VIDMODE_LIBS) \
	$(XSHAPE_LIBS) \
	$(GLIB_LIBS) \
	$(LIBSN_LIBS) \
	$(XML_LIBS) \
	$(EFENCE_LIBS) \
	$(LIBINTL) \
	render/libobrender.la \
	parser/libobparser.la
openbox_openbox_LDFLAGS = -export-dynamic
openbox_openbox_SOURCES = \
	gettext.h \
	openbox/action.c \
	openbox/action.h \
	openbox/client.c \
	openbox/client.h \
	openbox/client_list_menu.c \
	openbox/client_list_menu.h \
	openbox/client_menu.c \
	openbox/client_menu.h \
	openbox/config.c \
	openbox/config.h \
	openbox/debug.c \
	openbox/debug.h \
	openbox/dock.c \
	openbox/dock.h \
	openbox/event.c \
	openbox/event.h \
	openbox/extensions.c \
	openbox/extensions.h \
	openbox/focus.c \
	openbox/focus.h \
	openbox/frame.c \
	openbox/frame.h \
	openbox/framerender.c \
	openbox/framerender.h \
	openbox/geom.h \
	openbox/grab.c \
	openbox/grab.h \
	openbox/group.c \
	openbox/group.h \
	openbox/keyboard.c \
	openbox/keyboard.h \
	openbox/keytree.c \
	openbox/keytree.h \
	openbox/mainloop.c \
	openbox/mainloop.h \
	openbox/menuframe.c \
	openbox/menuframe.h \
	openbox/menu.c \
	openbox/menu.h \
	openbox/misc.h \
	openbox/mouse.c \
	openbox/mouse.h \
	openbox/moveresize.c \
	openbox/moveresize.h \
	openbox/mwm.h \
	openbox/openbox.c \
	openbox/openbox.h \
	openbox/place.c \
	openbox/place.h \
	openbox/popup.c \
	openbox/popup.h \
	openbox/prop.c \
	openbox/prop.h \
	openbox/resist.c \
	openbox/resist.h \
	openbox/screen.c \
	openbox/screen.h \
	openbox/session.c \
	openbox/session.h \
	openbox/stacking.c \
	openbox/stacking.h \
	openbox/startup.c \
	openbox/startup.h \
	openbox/translate.c \
	openbox/translate.h \
	openbox/window.c \
	openbox/window.h \
	openbox/xerror.c \
	openbox/xerror.h

## obconf ##

tools_obconf_obconf_CPPFLAGS = \
	$(GTK_CFLAGS) \
	$(GDK_PIXBUF_CFLAGS) \
	$(XML_CFLAGS) \
	-DLOCALEDIR=\"$(localedir)\" \
	-DRCDIR=\"$(rcdir)\" \
	-DPIXMAPDIR=\"$(pixmapdir)\" \
	-DG_LOG_DOMAIN=\"Obconf\"
tools_obconf_obconf_LDADD = \
	$(GTK_LIBS) \
	$(GDK_PIXBUF_LIBS) \
	$(LIBINTL) \
	render/libobrender.la \
	parser/libobparser.la
tools_obconf_obconf_SOURCES = \
	gettext.h \
	tools/obconf/main.c


## kdetrayproxy ##

tools_kdetrayproxy_kdetrayproxy_CPPFLAGS = \
	$(X_CFLAGS)
tools_kdetrayproxy_kdetrayproxy_LDADD = \
	$(X_LIBS)
tools_kdetrayproxy_kdetrayproxy_SOURCES = \
	tools/kdetrayproxy/kdetrayproxy.c


## themes ##

allegro_themedir = $(themedir)/allegro
dist_allegro_theme_DATA = \
	themes/allegro/themerc \
	themes/allegro/bullet.xbm

artwiz_themedir = $(themedir)/artwiz
dist_artwiz_theme_DATA = \
	themes/artwiz/themerc

blah41_themedir = $(themedir)/blah41
dist_blah41_theme_DATA = \
	themes/blah41/themerc

om4ob_themedir = $(themedir)/om4ob
dist_om4ob_theme_DATA = \
	themes/om4ob/themerc \
	themes/om4ob/close_hover.xbm \
	themes/om4ob/close.xbm \
	themes/om4ob/desk_hover.xbm \
	themes/om4ob/desk_toggled.xbm \
	themes/om4ob/desk.xbm \
	themes/om4ob/iconify_hover.xbm \
	themes/om4ob/iconify_pressed.xbm \
	themes/om4ob/iconify.xbm \
	themes/om4ob/max_disabled.xbm \
	themes/om4ob/max_hover.xbm \
	themes/om4ob/max_pressed.xbm \
	themes/om4ob/max_toggled.xbm \
	themes/om4ob/max.xbm \
	themes/om4ob/shade_disabled.xbm \
	themes/om4ob/shade_hover.xbm \
	themes/om4ob/shade_toggled.xbm \
	themes/om4ob/shade.xbm

thebear_themedir = $(themedir)/thebear
dist_thebear_theme_DATA = \
	themes/thebear/themerc


## data ##

dist_rc_DATA = \
	data/rc.xml \
	data/menu.xml

dist_desktopfiles_DATA = \
	data/openbox.desktop

EXTRA_DIST = \
	config.rpath \
	mkinstalldirs \
	README \
	COPYING \
	AUTHORS

#doc:
#       $(MAKE) -$(MAKEFLAGS) -C doc/doxygen doc

distclean-local:
	for d in . m4 po render; do \
		for p in core core.* gmon.out *\~ *.orig *.rej .\#*; do \
			rm -f "$$d/$$p"; \
		done \
	done

.PHONY: doc
