SUBDIRS = m4 po

themedir        = $(datadir)/openbox/themes
localedir       = $(datadir)/locale
rcdir           = $(datadir)/openbox
plugindir       = $(libdir)/openbox/plugins
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
	kernel/openbox \
	tools/kdetrayproxy/kdetrayproxy

plugin_LTLIBRARIES = \
	plugins/resistance/resistance.la \
	plugins/placement/placement.la \
	plugins/mouse/mouse.la \
	plugins/keyboard/keyboard.la \
	plugins/menu/timed_menu.la \
	plugins/menu/fifo_menu.la \
	plugins/menu/client_menu.la \
	plugins/menu/include_menu.la

if OBCONF
bin_PROGRAMS += \
	tools/obconf/obconf

#plugin_LTLIBRARIES += \
#	plugins/resistance/resistance-config.la
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

## kernel ##

kernel_openbox_CPPFLAGS = \
	$(X_CFLAGS) \
	$(SM_CFLAGS) \
	$(XFT_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(GMODULE_CFLAGS) \
	$(LIBSN_CFLAGS) \
	$(XML_CFLAGS) \
	-DLOCALEDIR=\"$(localedir)\" \
	-DPLUGINDIR=\"$(plugindir)\" \
	-DRCDIR=\"$(rcdir)\" \
	-DG_LOG_DOMAIN=\"Openbox\"
kernel_openbox_LDADD = \
	$(SM_LIBS) \
	$(XINERAMA_LIBS) \
	$(XKB_LIBS) \
	$(XRANDR_LIBS) \
	$(VIDMODE_LIBS) \
	$(XSHAPE_LIBS) \
	$(GLIB_LIBS) \
	$(GMODULE_LIBS) \
	$(LIBSN_LIBS) \
	$(XML_LIBS) \
	$(EFENCE_LIBS) \
	$(LIBINTL) \
	render/libobrender.la \
	parser/libobparser.la
kernel_openbox_LDFLAGS = -export-dynamic
kernel_openbox_SOURCES = \
	gettext.h \
	kernel/action.c \
	kernel/action.h \
	kernel/client.c \
	kernel/client.h \
	kernel/config.c \
	kernel/config.h \
	kernel/debug.c \
	kernel/debug.h \
	kernel/dispatch.c \
	kernel/dispatch.h \
	kernel/dock.c \
	kernel/dock.h \
	kernel/event.c \
	kernel/event.h \
	kernel/extensions.c \
	kernel/extensions.h \
	kernel/focus.c \
	kernel/focus.h \
	kernel/frame.c \
	kernel/frame.h \
	kernel/framerender.c \
	kernel/framerender.h \
	kernel/geom.h \
	kernel/grab.c \
	kernel/grab.h \
	kernel/group.c \
	kernel/group.h \
	kernel/menu.c \
	kernel/menu.h \
	kernel/menu_render.c \
	kernel/misc.h \
	kernel/moveresize.c \
	kernel/moveresize.h \
	kernel/mwm.h \
	kernel/openbox.c \
	kernel/openbox.h \
	kernel/plugin.c \
	kernel/plugin.h \
	kernel/popup.c \
	kernel/popup.h \
	kernel/prop.c \
	kernel/prop.h \
	kernel/screen.c \
	kernel/screen.h \
	kernel/session.c \
	kernel/session.h \
	kernel/stacking.c \
	kernel/stacking.h \
	kernel/startup.c \
	kernel/startup.h \
	kernel/timer.c \
	kernel/timer.h \
	kernel/window.c \
	kernel/window.h \
	kernel/xerror.c \
	kernel/xerror.h \
	plugins/interface.h

## plugins/resistance ##

plugins_resistance_resistance_la_CPPFLAGS = \
	$(XFT_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(LIBSN_CFLAGS) \
	$(XML_CFLAGS) \
	-DPLUGINDIR=\"$(plugindir)\" \
	-DG_LOG_DOMAIN=\"Plugin-Resistance\"
plugins_resistance_resistance_la_LDFLAGS = \
	-module \
	-avoid-version
plugins_resistance_resistance_la_SOURCES = \
	plugins/resistance/resistance.c \
	plugins/resistance/resistance.h

if OBCONF
plugins_resistance_resistance_config_la_CPPFLAGS = \
	$(XFT_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(LIBSN_CFLAGS) \
	$(XML_CFLAGS) \
	-DPLUGINDIR=\"$(plugindir)\" \
	-DG_LOG_DOMAIN=\"Plugin-Resistance\" \
	$(GTK_CFLAGS) \
	$(GLADE_CFLAGS)
plugins_resistance_resistance_config_la_LDFLAGS = \
	-module \
	-avoid-version
plugins_resistance_resistance_config_la_SOURCES = \
	plugins/resistance/resistance_config.c
endif

## plugins/placement ##

plugins_placement_placement_la_CPPFLAGS = \
	$(XFT_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(LIBSN_CFLAGS) \
	$(XML_CFLAGS) \
	-DPLUGINDIR=\"$(plugindir)\" \
	-DG_LOG_DOMAIN=\"Plugin-Placement\"
plugins_placement_placement_la_LDFLAGS = \
	-module \
	-avoid-version
plugins_placement_placement_la_SOURCES = \
	plugins/placement/placement.c \
	plugins/placement/history.c \
	plugins/placement/history.h

## plugins/mouse ##

plugins_mouse_mouse_la_CPPFLAGS = \
	$(XFT_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(LIBSN_CFLAGS) \
	$(XML_CFLAGS) \
	-DPLUGINDIR=\"$(plugindir)\" \
	-DG_LOG_DOMAIN=\"Plugin-Mouse\"
plugins_mouse_mouse_la_LDFLAGS = \
	-module \
	-avoid-version
plugins_mouse_mouse_la_SOURCES = \
	plugins/mouse/mouse.c \
	plugins/mouse/mouse.h \
	plugins/mouse/translate.c \
	plugins/mouse/translate.h

## plugins/mouse ##

plugins_keyboard_keyboard_la_CPPFLAGS = \
	$(XFT_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(LIBSN_CFLAGS) \
	$(XML_CFLAGS) \
	-DPLUGINDIR=\"$(plugindir)\" \
	-DG_LOG_DOMAIN=\"Plugin-Keyboard\"
plugins_keyboard_keyboard_la_LDFLAGS = \
	-module \
	-avoid-version
plugins_keyboard_keyboard_la_SOURCES = \
	plugins/keyboard/keyboard.c \
	plugins/keyboard/keyboard.h \
	plugins/keyboard/tree.c \
	plugins/keyboard/tree.h \
	plugins/keyboard/translate.c \
	plugins/keyboard/translate.h

## plugins/menu ##

plugins_menu_timed_menu_la_CPPFLAGS = \
	$(XFT_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(LIBSN_CFLAGS) \
	$(XML_CFLAGS) \
	-DPLUGINDIR=\"$(plugindir)\" \
	-DG_LOG_DOMAIN=\"Plugin-Timed-Menu\"
plugins_menu_timed_menu_la_LDFLAGS = \
	-module \
	-avoid-version
plugins_menu_timed_menu_la_SOURCES = \
	plugins/menu/timed_menu.c \
	plugins/menu/timed_menu.h

plugins_menu_fifo_menu_la_CPPFLAGS = \
	$(XFT_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(LIBSN_CFLAGS) \
	$(XML_CFLAGS) \
	-DPLUGINDIR=\"$(plugindir)\" \
	-DG_LOG_DOMAIN=\"Plugin-Fifo-Menu\"
plugins_menu_fifo_menu_la_LDFLAGS = \
	-module \
	-avoid-version
plugins_menu_fifo_menu_la_SOURCES = \
	plugins/menu/fifo_menu.c \
	plugins/menu/fifo_menu.h

plugins_menu_client_menu_la_CPPFLAGS = \
	$(XFT_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(LIBSN_CFLAGS) \
	$(XML_CFLAGS) \
	-DPLUGINDIR=\"$(plugindir)\" \
	-DG_LOG_DOMAIN=\"Plugin-Client-Menu\"
plugins_menu_client_menu_la_LDFLAGS = \
	-module \
	-avoid-version
plugins_menu_client_menu_la_SOURCES = \
	plugins/menu/client_menu.c

plugins_menu_include_menu_la_CPPFLAGS = \
	$(XFT_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(LIBSN_CFLAGS) \
	$(XML_CFLAGS) \
	-DPLUGINDIR=\"$(plugindir)\" \
	-DG_LOG_DOMAIN=\"Plugin-Include-Menu\"
plugins_menu_include_menu_la_LDFLAGS = \
	-module \
	-avoid-version
plugins_menu_include_menu_la_SOURCES = \
	plugins/menu/include_menu.c


## obconf ##

tools_obconf_obconf_CPPFLAGS = \
	$(GTK_CFLAGS) \
	$(GDK_PIXBUF_CFLAGS) \
	$(XML_CFLAGS) \
	-DLOCALEDIR=\"$(localedir)\" \
	-DPLUGINDIR=\"$(plugindir)\" \
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

thebear_themedir = $(themedir)/thebear
dist_thebear_theme_DATA = \
	themes/thebear/themerc

planton_themedir = $(themedir)/planton
dist_planton_theme_DATA = \
	themes/planton/themerc \
	themes/planton/max.xbm \
	themes/planton/max_toggled.xbm \
	themes/planton/stick.xbm \
	themes/planton/stick_toggled.xbm \
	themes/planton/shade.xbm \
	themes/planton/shade_toggled.xbm \
	themes/planton/iconify.xbm \
	themes/planton/close.xbm

lightindustry_themedir = $(themedir)/Light-Industry
dist_lightindustry_theme_DATA = \
	themes/Light-Industry/themerc \
	themes/Light-Industry/max.xbm \
	themes/Light-Industry/iconify.xbm \
	themes/Light-Industry/close.xbm

lightindustrybig_themedir = $(themedir)/Light-Industry-big
dist_lightindustrybig_theme_DATA = \
	themes/Light-Industry-big/themerc \
	themes/Light-Industry-big/max.xbm \
	themes/Light-Industry-big/iconify.xbm \
	themes/Light-Industry-big/close.xbm

imerry_themedir = $(themedir)/I-merry
dist_imerry_theme_DATA = \
	themes/I-merry/themerc \
	themes/I-merry/max.xbm \
	themes/I-merry/iconify.xbm \
	themes/I-merry/close.xbm

greenz_themedir = $(themedir)/greenz
dist_greenz_theme_DATA = \
	themes/greenz/themerc

bbstyle_themedir = $(themedir)/BBstylE
dist_bbstyle_theme_DATA = \
	themes/BBstylE/themerc \
	themes/BBstylE/max.xbm \
	themes/BBstylE/max_toggled.xbm \
	themes/BBstylE/stick.xbm \
	themes/BBstylE/stick_toggled.xbm \
	themes/BBstylE/shade.xbm \
	themes/BBstylE/shade_toggled.xbm \
	themes/BBstylE/iconify.xbm \
	themes/BBstylE/close.xbm


## data ##

dist_rc_DATA = \
	data/rc3

dist_desktopfiles_DATA = \
	data/openbox.desktop

EXTRA_DIST = \
	config.rpath \
	mkinstalldirs \
	README \
	COPYING \
	AUTHORS \
	plugins/obconf_interface.h

#doc:
#       $(MAKE) -$(MAKEFLAGS) -C doc/doxygen doc

distclean-local:
	for d in . m4 po render; do \
		for p in core core.* gmon.out *\~ *.orig *.rej .\#*; do \
			rm -f "$$d/$$p"; \
		done \
	done

.PHONY: doc
