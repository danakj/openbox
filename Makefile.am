SUBDIRS = m4 po

docxbmdir       = $(docdir)/xbm
themedir        = $(datadir)/themes
localedir       = $(datadir)/locale
configdir       = $(sysconfdir)/xdg
rcdir           = $(configdir)/openbox
xsessionsdir    = $(datadir)/xsessions
gnomewmfilesdir = $(datadir)/gnome/wm-properties
pkgconfigdir    = $(libdir)/pkgconfig
obtpubincludedir= $(includedir)/openbox/@OBT_VERSION@/obt
rrpubincludedir = $(includedir)/openbox/@RR_VERSION@/render
pixmapdir       = $(datadir)/pixmaps
xsddir          = $(datadir)/openbox

theme = Clearlooks

AUTOMAKE_OPTIONS = subdir-objects foreign

ACLOCAL_AMFLAGS = -I m4

INCLUDES = -I.

check_PROGRAMS = \
	render/rendertest

lib_LTLIBRARIES = \
	render/libobrender.la \
	obt/libobt.la \
	engines/libconcept.la \
	engines/libdefault.la
#	engines/libconcept2.la \
#	engines/libminimal.la

bin_PROGRAMS = \
	openbox/openbox \
	tools/gnome-panel-control/gnome-panel-control

bin_SCRIPTS = \
	tools/xdg-autostart/xdg-autostart

nodist_bin_SCRIPTS = \
	data/xsession/openbox-session \
	data/xsession/openbox-gnome-session \
	data/xsession/openbox-kde-session

## render ##

render_rendertest_CPPFLAGS = \
	$(PANGO_CFLAGS) \
	$(XFT_CFLAGS) \
	$(GLIB_CFLAGS) \
	-DG_LOG_DOMAIN=\"RenderTest\"
render_rendertest_LDADD = \
	obt/libobt.la \
	render/libobrender.la \
	$(GLIB_LIBS) \
	$(PANGO_LIBS) \
	$(XFT_LIBS) \
	$(XML_LIBS) \
	$(X_LIBS)
render_rendertest_SOURCES = render/test.c

render_libobrender_la_CPPFLAGS = \
	$(X_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(XML_CFLAGS) \
	$(PANGO_CFLAGS) \
	$(XFT_CFLAGS) \
	-DG_LOG_DOMAIN=\"ObRender\" \
	-DDEFAULT_THEME=\"$(theme)\"
render_libobrender_la_LDFLAGS = \
	-version-info $(RR_CURRENT):$(RR_REVISION):$(RR_AGE)
render_libobrender_la_LIBADD = \
	obt/libobt.la \
	$(X_LIBS) \
	$(PANGO_LIBS) \
	$(XFT_LIBS) \
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
## frame_default ##
engines_libdefault_la_CPPFLAGS = \
	$(X_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(XML_CFLAGS) \
	$(PANGO_CFLAGS) \
	$(XFT_CFLAGS) \
	-DG_LOG_DOMAIN=\"ObRender\" \
	-DDEFAULT_THEME=\"$(theme)\"
engines_libdefault_la_LDFLAGS = \
	-version-info $(RR_CURRENT):$(RR_REVISION):$(RR_AGE)
engines_libdefault_la_LIBADD = \
	render/libobrender.la \
	$(X_LIBS) \
	$(PANGO_LIBS) \
	$(XFT_LIBS) \
	$(GLIB_LIBS) \
	$(XML_LIBS)
engines_libdefault_la_SOURCES = \
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
	openbox/engine_interface.h \
	openbox/engine_interface.c \
	engines/default/config.h \
	engines/default/config.c \
	engines/default/render.h \
	engines/default/render.c \
	engines/default/plugin.h \
	engines/default/plugin.c

## frame_minimal ##
#engines_libminimal_la_CPPFLAGS = \
#    $(X_CFLAGS) \
#    $(GLIB_CFLAGS) \
#    $(XML_CFLAGS) \
#    $(PANGO_CFLAGS) \
#    $(XFT_CFLAGS) \
#    -DG_LOG_DOMAIN=\"ObRender\" \
#    -DDEFAULT_THEME=\"$(theme)\"
#engines_libminimal_la_LDFLAGS = \
#    -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)
#engines_libminimal_la_LIBADD = \
#    parser/libobparser.la \
#    $(X_LIBS) \
#    $(PANGO_LIBS) \
#    $(XFT_LIBS) \
#    $(GLIB_LIBS) \
#    $(XML_LIBS)
#engines_libminimal_la_SOURCES = \
#    gettext.h \
#    render/color.h \
#    render/color.c \
#    render/font.h \
#    render/font.c \
#    render/geom.h \
#    render/gradient.h \
#    render/gradient.c \
#    render/icon.h \
#    render/image.h \
#    render/image.c \
#    render/instance.h \
#    render/instance.c \
#    render/mask.h \
#    render/mask.c \
#    render/render.h \
#    render/render.c \
#    openbox/engine_interface.h \
#    openbox/engine_interface.c \
#    engines/minimal/frame_minimal_config.h \
#    engines/minimal/frame_minimal_config.c \
#    engines/minimal/frame_minimal_render.h \
#    engines/minimal/frame_minimal_render.c \
#    engines/minimal/frame_minimal_plugin.h \
#    engines/minimal/frame_minimal_plugin.c

## frame_concept ##
engines_libconcept_la_CPPFLAGS = \
	$(X_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(XML_CFLAGS) \
	$(PANGO_CFLAGS) \
	$(XFT_CFLAGS) \
	$(IMLIB2_CFLAGS) \
	$(GDK_PIXBUF_CFLAGS) \
	$(GDK_PIXBUF_XLIB_CFLAGS) \
	-DG_LOG_DOMAIN=\"ObRender\" \
	-DDEFAULT_THEME=\"$(theme)\"
engines_libconcept_la_LDFLAGS = \
	-version-info $(RR_CURRENT):$(RR_REVISION):$(RR_AGE)
engines_libconcept_la_LIBADD = \
	render/libobrender.la \
	$(X_LIBS) \
	$(PANGO_LIBS) \
	$(XFT_LIBS) \
	$(GLIB_LIBS) \
	$(IMLIB2_LIBS) \
	$(GDK_PIXBUF_LIBS) \
	$(GDK_PIXBUF_XLIB_LIBS) \
	$(XML_LIBS)
engines_libconcept_la_SOURCES = \
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
	openbox/engine_interface.h \
	openbox/engine_interface.c \
	engines/concept/config.h \
	engines/concept/config.c \
	engines/concept/plugin.h \
	engines/concept/plugin.c

## obt ##

obt_libobt_la_CPPFLAGS = \
	$(XINERAMA_CFLAGS) \
	$(XKB_CFLAGS) \
	$(XRANDR_CFLAGS) \
	$(XSHAPE_CFLAGS) \
	$(XSYNC_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(XML_CFLAGS) \
	-DG_LOG_DOMAIN=\"Obt\" \
	-DLOCALEDIR=\"$(localedir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DCONFIGDIR=\"$(configdir)\"
obt_libobt_la_LDFLAGS = \
	-version-info $(OBT_CURRENT):$(OBT_REVISION):$(OBT_AGE)
obt_libobt_la_LIBADD = \
	$(XINERAMA_LIBS) \
	$(XKB_LIBS) \
	$(XRANDR_LIBS) \
	$(XSHAPE_LIBS) \
	$(XSYNC_LIBS) \
	$(GLIB_LIBS) \
	$(XML_LIBS)
obt_libobt_la_SOURCES = \
	obt/display.h \
	obt/display.c \
	obt/internal.h \
	obt/keyboard.h \
	obt/keyboard.c \
	obt/mainloop.h \
	obt/mainloop.c \
	obt/parse.h \
	obt/parse.c \
	obt/paths.h \
	obt/paths.c \
	obt/prop.h \
	obt/prop.c \
	obt/util.h \
	obt/xevent.h \
	obt/xevent.c

## openbox ##

openbox_openbox_CPPFLAGS = \
	$(SM_CFLAGS) \
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
	$(GLIB_LIBS) \
	$(X_LIBS) \
	$(XCURSOR_LIBS) \
	$(LIBSN_LIBS) \
	$(XML_LIBS) \
	$(EFENCE_LIBS) \
	$(LIBINTL) \
	render/libobrender.la \
	obt/libobt.la
openbox_openbox_LDFLAGS = -export-dynamic
openbox_openbox_SOURCES = \
	gettext.h \
	openbox/actions/all.c \
	openbox/actions/all.h \
	openbox/actions/addremovedesktop.c \
	openbox/actions/breakchroot.c \
	openbox/actions/close.c \
	openbox/actions/cyclewindows.c \
	openbox/actions/debug.c \
	openbox/actions/decorations.c \
	openbox/actions/desktop.c \
	openbox/actions/dockautohide.c \
	openbox/actions/directionalwindows.c \
	openbox/actions/execute.c \
	openbox/actions/exit.c \
	openbox/actions/focus.c \
	openbox/actions/focustobottom.c \
	openbox/actions/fullscreen.c \
	openbox/actions/growtoedge.c \
	openbox/actions/iconify.c \
	openbox/actions/if.c \
	openbox/actions/kill.c \
	openbox/actions/layer.c \
	openbox/actions/lower.c \
	openbox/actions/maximize.c \
	openbox/actions/move.c \
	openbox/actions/moverelative.c \
	openbox/actions/moveresizeto.c \
	openbox/actions/movetoedge.c \
	openbox/actions/omnipresent.c \
	openbox/actions/raise.c \
	openbox/actions/raiselower.c \
	openbox/actions/reconfigure.c \
	openbox/actions/resize.c \
	openbox/actions/resizerelative.c \
	openbox/actions/restart.c \
	openbox/actions/shade.c \
	openbox/actions/showdesktop.c \
	openbox/actions/showmenu.c \
	openbox/actions/unfocus.c \
	openbox/actions.c \
	openbox/actions.h \
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
	openbox/focus.c \
	openbox/focus.h \
	openbox/focus_cycle.c \
	openbox/focus_cycle.h \
	openbox/focus_cycle_indicator.c \
	openbox/focus_cycle_indicator.h \
	openbox/focus_cycle_popup.c \
	openbox/focus_cycle_popup.h \
	openbox/engine_interface.h \
	openbox/engine_interface.c \
	openbox/geom.h \
	openbox/grab.c \
	openbox/grab.h \
	openbox/group.c \
	openbox/group.h \
	openbox/keyboard.c \
	openbox/keyboard.h \
	openbox/keytree.c \
	openbox/keytree.h \
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
	openbox/ping.c \
	openbox/ping.h \
	openbox/place.c \
	openbox/place.h \
	openbox/prompt.c \
	openbox/prompt.h \
	openbox/popup.c \
	openbox/popup.h \
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
	openbox/window.h

## gnome-panel-control ##

tools_gnome_panel_control_gnome_panel_control_CPPFLAGS = \
	$(X_CFLAGS)
tools_gnome_panel_control_gnome_panel_control_LDADD = \
	$(X_LIBS)
tools_gnome_panel_control_gnome_panel_control_SOURCES = \
	tools/gnome-panel-control/gnome-panel-control.c


## default button masks ##
dist_docxbm_DATA = \
	data/xbm/bullet.xbm \
	data/xbm/close.xbm \
	data/xbm/desk_toggled.xbm \
	data/xbm/desk.xbm \
	data/xbm/iconify.xbm \
	data/xbm/max_toggled.xbm \
	data/xbm/max.xbm \
	data/xbm/shade_toggled.xbm \
	data/xbm/shade.xbm


## themes ##

clearlooks_themedir = $(themedir)/Clearlooks/openbox-3
dist_clearlooks_theme_DATA= \
	themes/Clearlooks/openbox-3/themerc

clearlooks34_themedir = $(themedir)/Clearlooks/openbox-3
dist_clearlooks34_theme_DATA= \
	themes/Clearlooks-3.4/openbox-3/themerc

clearlooksolive_themedir = $(themedir)/Clearlooks-Olive/openbox-3
dist_clearlooksolive_theme_DATA= \
	themes/Clearlooks-Olive/openbox-3/themerc

mikachu_themedir = $(themedir)/Mikachu/openbox-3
dist_mikachu_theme_DATA= \
	themes/Mikachu/openbox-3/bullet.xbm \
	themes/Mikachu/openbox-3/close.xbm \
	themes/Mikachu/openbox-3/desk.xbm \
	themes/Mikachu/openbox-3/iconify.xbm \
	themes/Mikachu/openbox-3/max.xbm \
	themes/Mikachu/openbox-3/themerc


natura_themedir = $(themedir)/Natura/openbox-3
dist_natura_theme_DATA= \
	themes/Natura/openbox-3/close_hover.xbm \
	themes/Natura/openbox-3/close.xbm \
	themes/Natura/openbox-3/desk_toggled.xbm \
	themes/Natura/openbox-3/desk_hover.xbm \
	themes/Natura/openbox-3/desk.xbm \
	themes/Natura/openbox-3/iconify_hover.xbm \
	themes/Natura/openbox-3/iconify.xbm \
	themes/Natura/openbox-3/max_hover.xbm \
	themes/Natura/openbox-3/max_toggled.xbm \
	themes/Natura/openbox-3/max.xbm \
	themes/Natura/openbox-3/shade.xbm \
	themes/Natura/openbox-3/shade_hover.xbm \
	themes/Natura/openbox-3/themerc

artwizboxed_themedir = $(themedir)/Artwiz-boxed/openbox-3
dist_artwizboxed_theme_DATA= \
	themes/Artwiz-boxed/openbox-3/themerc

bear2_themedir = $(themedir)/Bear2/openbox-3
dist_bear2_theme_DATA= \
	themes/Bear2/openbox-3/close_pressed.xbm \
	themes/Bear2/openbox-3/close.xbm \
	themes/Bear2/openbox-3/desk_toggled.xbm \
	themes/Bear2/openbox-3/desk.xbm \
	themes/Bear2/openbox-3/iconify_pressed.xbm \
	themes/Bear2/openbox-3/iconify.xbm \
	themes/Bear2/openbox-3/max_pressed.xbm \
	themes/Bear2/openbox-3/max_toggled.xbm \
	themes/Bear2/openbox-3/max.xbm \
	themes/Bear2/openbox-3/shade_pressed.xbm \
	themes/Bear2/openbox-3/shade.xbm \
	themes/Bear2/openbox-3/themerc

orang_themedir = $(themedir)/Orang/openbox-3
dist_orang_theme_DATA= \
	themes/Orang/openbox-3/themerc

onyx_themedir = $(themedir)/Onyx/openbox-3
dist_onyx_theme_DATA= \
	themes/Onyx/openbox-3/themerc

onyxcitrus_themedir = $(themedir)/Onyx-Citrus/openbox-3
dist_onyxcitrus_theme_DATA= \
	themes/Onyx-Citrus/openbox-3/themerc

syscrash_themedir = $(themedir)/Syscrash/openbox-3
dist_syscrash_theme_DATA= \
	themes/Syscrash/openbox-3/max_disabled.xbm \
	themes/Syscrash/openbox-3/max_pressed.xbm \
	themes/Syscrash/openbox-3/max_toggled.xbm \
	themes/Syscrash/openbox-3/max.xbm \
	themes/Syscrash/openbox-3/themerc

## public headers ##

rrpubinclude_HEADERS = \
	render/color.h \
	render/font.h \
	render/geom.h \
	render/gradient.h \
	render/image.h \
	render/instance.h \
	render/mask.h \
	render/render.h \
	render/theme.h \
	render/version.h

obtpubinclude_HEADERS = \
	obt/display.h \
	obt/keyboard.h \
	obt/mainloop.h \
	obt/parse.h \
	obt/paths.h \
	obt/prop.h \
	obt/util.h \
	obt/version.h \
	obt/xevent.h

nodist_pkgconfig_DATA = \
	render/obrender-4.0.pc \
	obt/obt-4.0.pc

## data ##

dist_pixmap_DATA = \
	data/openbox.png

dist_rc_DATA = \
	data/autostart.sh \
	data/rc.xml \
	data/menu.xml

edit = $(SED) \
	-e 's!@version\@!$(VERSION)!' \
	-e 's!@configdir\@!$(configdir)!' \
	-e 's!@bindir\@!$(bindir)!'

%.desktop: %.desktop.in Makefile
	@echo make: creating $@
	@$(edit) $< >$@	

%-session: %-session.in Makefile
	@echo make: creating $@
	@$(edit) $< >$@	

%.1.in: %.1.sgml
	@echo make: creating $@
	@docbook-to-man $< >$@

%.1: %.1.in Makefile
	@echo make: creating $@
	@$(edit) $< >$@	

dist_gnomewmfiles_DATA = \
	data/gnome-wm-properties/openbox.desktop

nodist_xsessions_DATA = \
	data/xsession/openbox.desktop \
	data/xsession/openbox-gnome.desktop \
	data/xsession/openbox-kde.desktop

dist_noinst_DATA = \
	data/rc.xsd \
	data/menu.xsd \
	data/xsession/openbox.desktop.in \
	data/xsession/openbox-gnome.desktop.in \
	data/xsession/openbox-kde.desktop.in \
	data/xsession/openbox-session.in \
	data/xsession/openbox-gnome-session.in \
	data/xsession/openbox-kde-session.in \
	doc/openbox.1.sgml \
	doc/openbox.1.in \
	doc/openbox-session.1.sgml \
	doc/openbox-session.1.in \
	doc/openbox-gnome-session.1.sgml \
	doc/openbox-gnome-session.1.in \
	doc/openbox-kde-session.1.sgml \
	doc/openbox-kde-session.1.in \
	render/version.h.in \
	render/obrender-4.0.pc.in \
	obt/obt-4.0.pc.in \
	obt/version.h.in \
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

dist_doc_DATA = \
	COMPLIANCE \
	README \
	AUTHORS \
	CHANGELOG \
	COPYING \
	data/rc.xsd \
	data/menu.xsd \
	doc/rc-mouse-focus.xml

nodist_man_MANS = \
	doc/openbox.1 \
	doc/openbox-session.1 \
	doc/openbox-gnome-session.1 \
	doc/openbox-kde-session.1

EXTRA_DIST = \
	config.rpath

# make clean doesn't delete these for some reason, even though they are
# built by make
CLEANFILES = \
	doc/openbox.1 \
	doc/openbox-session.1 \
	doc/openbox-gnome-session.1 \
	doc/openbox-kde-session.1 \
	data/xsession/openbox-session \
	data/xsession/openbox-gnome-session \
	data/xsession/openbox-kde-session \
	data/xsession/openbox.desktop \
	data/xsession/openbox-gnome.desktop \
	data/xsession/openbox-kde.desktop

#doc:
#       $(MAKE) -$(MAKEFLAGS) -C doc/doxygen doc

distclean-local:
	for d in . m4 po render parser obt openbox; do \
		for p in core core.* gmon.out *\~ *.orig *.rej .\#*; do \
			rm -f "$$d/$$p"; \
		done \
	done

.PHONY: doc
