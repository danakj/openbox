SUBDIRS = m4 po

themedir        = $(datadir)/themes
localedir       = $(datadir)/locale
configdir       = $(sysconfdir)/xdg
rcdir           = $(configdir)/openbox
desktopfilesdir = $(datadir)/gnome/wm-properties
pkgconfigdir    = $(libdir)/pkgconfig
pubincludedir   = $(includedir)/openbox/@OB_VERSION@/openbox
pixmapdir       = $(datadir)/pixmaps
xsddir          = $(datadir)/openbox
userscriptdir   = $(datadir)/openbox

theme = TheBear

AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I m4

INCLUDES = -I.

noinst_PROGRAMS = \
	render/rendertest

lib_LTLIBRARIES = \
	parser/libobparser.la \
	render/libobrender.la

bin_PROGRAMS = \
	openbox/openbox \
	tools/kdetrayproxy/kdetrayproxy \
	tools/gnome-panel-control/gnome-panel-control

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
	$(XML_CFLAGS) \
	-DG_LOG_DOMAIN=\"ObRender\" \
	-DDEFAULT_THEME=\"$(theme)\"
render_libobrender_la_LIBADD = \
	$(X_LIBS) \
	$(XFT_LIBS) \
	$(GLIB_LIBS) \
	parser/libobparser.la
render_libobrender_la_LDFLAGS = \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)
render_libobrender_la_SOURCES = \
	gettext.h \
	render/color.h \
	render/color.c \
	render/font.h \
	render/font.c \
	render/geom.h \
	render/gradient.h \
	render/gradient.c \
	render/icon.h \
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
	-DDATADIR=\"$(datadir)\" \
	-DCONFIGDIR=\"$(configdir)\"
parser_libobparser_la_LIBADD = \
	$(GLIB_LIBS) \
	$(XML_LIBS)
parser_libobparser_la_LDFLAGS = \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)
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
	-DDATADIR=\"$(datadir)\" \
	-DCONFIGDIR=\"$(configdir)\" \
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
	openbox/startupnotify.c \
	openbox/startupnotify.h \
	openbox/translate.c \
	openbox/translate.h \
	openbox/window.c \
	openbox/window.h \
	openbox/xerror.c \
	openbox/xerror.h


## kdetrayproxy ##

tools_kdetrayproxy_kdetrayproxy_CPPFLAGS = \
	$(X_CFLAGS)
tools_kdetrayproxy_kdetrayproxy_LDADD = \
	$(X_LIBS)
tools_kdetrayproxy_kdetrayproxy_SOURCES = \
	tools/kdetrayproxy/kdetrayproxy.c


## gnome-panel-control ##

tools_gnome_panel_control_gnome_panel_control_CPPFLAGS = \
	$(X_CFLAGS)
tools_gnome_panel_control_gnome_panel_control_LDADD = \
	$(X_LIBS)
tools_gnome_panel_control_gnome_panel_control_SOURCES = \
	tools/gnome-panel-control/gnome-panel-control.c


## themes ##

allegro_themedir = $(themedir)/Allegro/openbox-3
dist_allegro_theme_DATA = \
	themes/Allegro/openbox-3/themerc \
	themes/Allegro/openbox-3/bullet.xbm

artwiz_themedir = $(themedir)/Artwiz/openbox-3
dist_artwiz_theme_DATA = \
	themes/Artwiz/openbox-3/themerc

blah41_themedir = $(themedir)/Blah41/openbox-3
dist_blah41_theme_DATA = \
	themes/Blah41/openbox-3/themerc

om4ob_themedir = $(themedir)/Om4Ob/openbox-3
dist_om4ob_theme_DATA = \
	themes/Om4Ob/openbox-3/themerc \
	themes/Om4Ob/openbox-3/close_hover.xbm \
	themes/Om4Ob/openbox-3/close.xbm \
	themes/Om4Ob/openbox-3/desk_hover.xbm \
	themes/Om4Ob/openbox-3/desk_toggled.xbm \
	themes/Om4Ob/openbox-3/desk.xbm \
	themes/Om4Ob/openbox-3/iconify_hover.xbm \
	themes/Om4Ob/openbox-3/iconify_pressed.xbm \
	themes/Om4Ob/openbox-3/iconify.xbm \
	themes/Om4Ob/openbox-3/max_disabled.xbm \
	themes/Om4Ob/openbox-3/max_hover.xbm \
	themes/Om4Ob/openbox-3/max_pressed.xbm \
	themes/Om4Ob/openbox-3/max_toggled.xbm \
	themes/Om4Ob/openbox-3/max.xbm \
	themes/Om4Ob/openbox-3/shade_disabled.xbm \
	themes/Om4Ob/openbox-3/shade_hover.xbm \
	themes/Om4Ob/openbox-3/shade_toggled.xbm \
	themes/Om4Ob/openbox-3/shade.xbm

thebear_themedir = $(themedir)/TheBear/openbox-3
dist_thebear_theme_DATA = \
	themes/TheBear/openbox-3/themerc


## public headers ##

pubinclude_HEADERS = \
	render/render.h \
	render/theme.h \
	parser/parse.h

nodist_pkgconfig_DATA = \
	render/obrender-3.0.pc \
	parser/obparser-3.0.pc

## data ##

dist_pixmap_DATA = \
	data/openbox.png

nodist_rc_DATA = \
	data/rc.xml \
	data/menu.xml

dist_xsd_DATA = \
	data/rc.xsd \
	data/menu.xsd

edit = $(SED) \
	-e 's!@version\@!$(VERSION)!' \
	-e 's!@xsddir\@!$(xsddir)!'

data/rc.xml: Makefile $(srcdir)/data/rc.xml.in data
	@echo make: creating $@
	@rm -f $@
	@mkdir data 2>/dev/null || true
	@$(edit) $(srcdir)/data/rc.xml.in >$@

data/menu.xml: Makefile $(srcdir)/data/menu.xml.in data
	@echo make: creating $@
	@rm -f $@
	@mkdir data 2>/dev/null || true
	@$(edit) $(srcdir)/data/menu.xml.in >$@

dist_userscript_DATA = \
	tools/themeupdate/themeupdate.py

install-data-hook:
	chmod +x $(DESTDIR)$(userscriptdir)/themeupdate.py

dist_desktopfiles_DATA = \
	data/openbox.desktop

dist_noinst_DATA = \
	data/rc.xml.in \
	data/menu.xml.in \
	render/obrender-3.0.pc.in \
	parser/obparser-3.0.pc.in

EXTRA_DIST = \
	config.rpath \
	mkinstalldirs \
	README \
	COPYING \
	AUTHORS

CLEANFILES = \
	data/rc.xml \
	data/menu.xml

#doc:
#       $(MAKE) -$(MAKEFLAGS) -C doc/doxygen doc

distclean-local:
	for d in . m4 po render; do \
		for p in core core.* gmon.out *\~ *.orig *.rej .\#*; do \
			rm -f "$$d/$$p"; \
		done \
	done

.PHONY: doc
