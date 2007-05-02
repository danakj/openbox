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

theme = Clearlooks

AUTOMAKE_OPTIONS = subdir-objects foreign

ACLOCAL_AMFLAGS = -I m4

INCLUDES = -I.

check_PROGRAMS = \
	render/rendertest

lib_LTLIBRARIES = \
	parser/libobparser.la \
	render/libobrender.la

bin_PROGRAMS = \
	openbox/openbox \
	tools/gnome-panel-control/gnome-panel-control

noinst_PROGRAMS = \
	tools/themetoxml/themetoxml

## render ##

render_rendertest_CPPFLAGS = \
	$(PANGO_CFLAGS) \
	$(GLIB_CFLAGS) \
	-DG_LOG_DOMAIN=\"RenderTest\"
render_rendertest_LDADD = \
	parser/libobparser.la \
	render/libobrender.la \
	$(GLIB_LIBS) \
	$(PANGO_LIBS) \
	$(XML_LIBS) \
	$(X_LIBS)
render_rendertest_SOURCES = render/test.c

render_libobrender_la_CPPFLAGS = \
	$(X_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(XML_CFLAGS) \
	$(PANGO_CFLAGS) \
	-DG_LOG_DOMAIN=\"ObRender\" \
	-DDEFAULT_THEME=\"$(theme)\"
render_libobrender_la_LDFLAGS = \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)
render_libobrender_la_LIBADD = \
	parser/libobparser.la \
	$(X_LIBS) \
	$(PANGO_LIBS) \
	$(GLIB_LIBS) \
	$(XML_LIBS)
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
parser_libobparser_la_LDFLAGS = \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)
parser_libobparser_la_LIBADD = \
	$(GLIB_LIBS) \
	$(XML_LIBS) 
parser_libobparser_la_SOURCES = \
	parser/parse.h \
	parser/parse.c

## openbox ##

openbox_openbox_CPPFLAGS = \
	$(SM_CFLAGS) \
	$(XINERAMA_CFLAGS) \
	$(XKB_CFLAGS) \
	$(XRANDR_CFLAGS) \
	$(XSHAPE_CFLAGS) \
	$(XSYNC_CFLAGS) \
	$(X_CFLAGS) \
	$(XCURSOR_CFLAGS) \
	$(SM_CFLAGS) \
	$(PANGO_CFLAGS) \
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
	$(XSHAPE_LIBS) \
	$(XSYNC_LIBS) \
	$(GLIB_LIBS) \
	$(X_LIBS) \
	$(XCURSOR_LIBS) \
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
	openbox/client_list_combined_menu.c \
	openbox/client_list_combined_menu.h \
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


## themetoxml ##

tools_themetoxml_themetoxml_CPPFLAGS = \
	$(X_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(XML_CFLAGS)
tools_themetoxml_themetoxml_LDADD = \
	$(X_LIBS) \
	$(GLIB_LIBS) \
	$(XML_LIBS)
tools_themetoxml_themetoxml_SOURCES = \
	tools/themetoxml/themetoxml.c \
	tools/themetoxml/rgb.h


## gnome-panel-control ##

tools_gnome_panel_control_gnome_panel_control_CPPFLAGS = \
	$(X_CFLAGS)
tools_gnome_panel_control_gnome_panel_control_LDADD = \
	$(X_LIBS)
tools_gnome_panel_control_gnome_panel_control_SOURCES = \
	tools/gnome-panel-control/gnome-panel-control.c


## themes ##

clearlooks_themedir = $(themedir)/Clearlooks/openbox-3
dist_clearlooks_theme_DATA= \
	themes/Clearlooks/openbox-3/bullet.xbm \
	themes/Clearlooks/openbox-3/close.xbm \
	themes/Clearlooks/openbox-3/desk_toggled.xbm \
	themes/Clearlooks/openbox-3/desk.xbm \
	themes/Clearlooks/openbox-3/iconify.xbm \
	themes/Clearlooks/openbox-3/max_toggled.xbm \
	themes/Clearlooks/openbox-3/max.xbm \
	themes/Clearlooks/openbox-3/themerc.xml

clearlooksolive_themedir = $(themedir)/Clearlooks-Olive/openbox-3
dist_clearlooksolive_theme_DATA= \
	themes/Clearlooks-Olive/openbox-3/bullet.xbm \
	themes/Clearlooks-Olive/openbox-3/close.xbm \
	themes/Clearlooks-Olive/openbox-3/desk_toggled.xbm \
	themes/Clearlooks-Olive/openbox-3/desk.xbm \
	themes/Clearlooks-Olive/openbox-3/iconify.xbm \
	themes/Clearlooks-Olive/openbox-3/max_toggled.xbm \
	themes/Clearlooks-Olive/openbox-3/max.xbm \
	themes/Clearlooks-Olive/openbox-3/themerc.xml

mikachu_themedir = $(themedir)/Mikachu/openbox-3
dist_mikachu_theme_DATA= \
	themes/Mikachu/openbox-3/bullet.xbm \
	themes/Mikachu/openbox-3/close.xbm \
	themes/Mikachu/openbox-3/desk.xbm \
	themes/Mikachu/openbox-3/iconify.xbm \
	themes/Mikachu/openbox-3/max.xbm \
	themes/Mikachu/openbox-3/themerc.xml


natura_themedir = $(themedir)/Natura/openbox-3
dist_natura_theme_DATA= \
	themes/Natura/openbox-3/close_hover.xbm \
	themes/Natura/openbox-3/close.xbm \
	themes/Natura/openbox-3/desk_toggled.xbm \
	themes/Natura/openbox-3/desk.xbm \
	themes/Natura/openbox-3/iconify_disabled.xbm \
	themes/Natura/openbox-3/iconify_hover.xbm \
	themes/Natura/openbox-3/iconify.xbm \
	themes/Natura/openbox-3/max_disabled.xbm \
	themes/Natura/openbox-3/max_hover.xbm \
	themes/Natura/openbox-3/max_toggled.xbm \
	themes/Natura/openbox-3/max.xbm \
	themes/Natura/openbox-3/shade.xbm \
	themes/Natura/openbox-3/themerc.xml

artwizboxed_themedir = $(themedir)/Artwiz-boxed/openbox-3
dist_artwizboxed_theme_DATA= \
	themes/Artwiz-boxed/openbox-3/bullet.xbm \
	themes/Artwiz-boxed/openbox-3/close.xbm \
	themes/Artwiz-boxed/openbox-3/desk_toggled.xbm \
	themes/Artwiz-boxed/openbox-3/desk.xbm \
	themes/Artwiz-boxed/openbox-3/iconify.xbm \
	themes/Artwiz-boxed/openbox-3/max_toggled.xbm \
	themes/Artwiz-boxed/openbox-3/max.xbm \
	themes/Artwiz-boxed/openbox-3/themerc.xml

bear2_themedir = $(themedir)/Bear2/openbox-3
dist_bear2_theme_DATA= \
	themes/Bear2/openbox-3/close_pressed.xbm \
	themes/Bear2/openbox-3/close.xbm \
	themes/Bear2/openbox-3/desk_toggled.xbm \
	themes/Bear2/openbox-3/desk.xbm \
	themes/Bear2/openbox-3/iconify_disabled.xbm \
	themes/Bear2/openbox-3/iconify_pressed.xbm \
	themes/Bear2/openbox-3/iconify.xbm \
	themes/Bear2/openbox-3/max_disabled.xbm \
	themes/Bear2/openbox-3/max_pressed.xbm \
	themes/Bear2/openbox-3/max_toggled.xbm \
	themes/Bear2/openbox-3/max.xbm \
	themes/Bear2/openbox-3/shade_pressed.xbm \
	themes/Bear2/openbox-3/shade.xbm \
	themes/Bear2/openbox-3/themerc.xml

orang_themedir = $(themedir)/Orang/openbox-3
dist_orang_theme_DATA= \
	themes/Orang/openbox-3/themerc.xml

syscrash_themedir = $(themedir)/Syscrash/openbox-3
dist_syscrash_theme_DATA= \
	themes/Syscrash/openbox-3/bullet.xbm \
	themes/Syscrash/openbox-3/close_disabled.xbm \
	themes/Syscrash/openbox-3/close.xbm \
	themes/Syscrash/openbox-3/desk_toggled.xbm \
	themes/Syscrash/openbox-3/desk.xbm \
	themes/Syscrash/openbox-3/iconify.xbm \
	themes/Syscrash/openbox-3/max_disabled.xbm \
	themes/Syscrash/openbox-3/max_pressed.xbm \
	themes/Syscrash/openbox-3/max_toggled.xbm \
	themes/Syscrash/openbox-3/max.xbm \
	themes/Syscrash/openbox-3/shade.xbm \
	themes/Syscrash/openbox-3/themerc.xml

## public headers ##

pubinclude_HEADERS = \
	render/color.h \
	render/font.h \
	render/geom.h \
	render/gradient.h \
	render/image.h \
	render/instance.h \
	render/mask.h \
	render/render.h \
	render/theme.h \
	parser/parse.h

nodist_pubinclude_HEADERS = \
	version.h

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

dist_desktopfiles_DATA = \
	data/openbox.desktop

dist_noinst_DATA = \
	version.h.in \
	data/rc.xml.in \
	data/menu.xml.in \
	render/obrender-3.0.pc.in \
	parser/obparser-3.0.pc.in \
	tools/themeupdate/themeupdate.py \
	tests/hideshow.py \
	tests/Makefile \
	tests/aspect.c \
	tests/fullscreen.c \
	tests/grav.c \
	tests/grouptran.c \
	tests/icons.c \
	tests/modal2.c \
	tests/modal3.c \
	tests/modal.c \
	tests/noresize.c \
	tests/override.c \
	tests/positioned.c \
	tests/strut.c \
	tests/title.c \
	tests/urgent.c

EXTRA_DIST = \
	config.rpath \
	mkinstalldirs \
	CHANGELOG \
	COMPLIANCE \
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
