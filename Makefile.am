SUBDIRS = m4 po

themedir        = $(datadir)/openbox/themes
localedir       = $(datadir)/locale
rcdir           = $(datadir)/openbox
plugindir       = $(libdir)/openbox/plugins
desktopfilesdir = $(datadir)/gnome/wm-properties

theme = operation

AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I m4

INCLUDES = -I.

noinst_PROGRAMS = \
	render/rendertest

lib_LTLIBRARIES = \
	render/libobrender.la \
	parser/libobparser.la

bin_PROGRAMS = \
	kernel/openbox

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
plugin_LTLIBRARIES += \
	plugins/resistance/resistance-config.la
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


## themes ##

dist_theme_DATA = \
	themes/artwiz \
	themes/bbs \
	themes/bluebox \
	themes/cthulhain \
	themes/deep \
	themes/ebox \
	themes/fieron \
	themes/fieron2 \
	themes/flux \
	themes/frobozz \
	themes/frobust \
	themes/mbdtex \
	themes/miklos \
	themes/nyz \
	themes/nyzclone \
	themes/ob20 \
	themes/operation \
	themes/outcomes \
	themes/paper \
	themes/purplehaaze \
	themes/shade \
	themes/steelblue \
	themes/steelblue2 \
	themes/the_orange \
	themes/trisb \
	themes/twice \
	themes/warp-xp

fieronthemedir = $(themedir)/fieron_data
dist_fierontheme_DATA = \
	themes/fieron_data/close.xbm \
	themes/fieron_data/iconify.xbm \
	themes/fieron_data/max.xbm \
	themes/fieron_data/stick.xbm

fieron2themedir = $(themedir)/fieron2_data
dist_fieron2theme_DATA = \
	themes/fieron2_data/close.xbm \
	themes/fieron2_data/iconify.xbm \
	themes/fieron2_data/max.xbm \
	themes/fieron2_data/stick.xbm

eboxthemedir = $(themedir)/ebox_data
dist_eboxtheme_DATA = \
	themes/ebox_data/close.xbm \
	themes/ebox_data/iconify.xbm \
	themes/ebox_data/max.xbm

## data ##

dist_rc_DATA = \
	data/rc3

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
			$(RM) "$$d/$$p"; \
		done \
	done

.PHONY: doc
