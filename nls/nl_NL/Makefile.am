# Makefile.am for Blackbox - an X11 Window Manager

NLSTEST = @NLS@
THE_LANG = nl_NL
THE_NLS_PATH = $(DESTDIR)$(pkgdatadir)/nls/$(THE_LANG)
CLEANFILES = openbox.cat
MAINTAINERCLEANFILES = Makefile.in Translation.m
DISTCLEANFILES = Translation.m
MFILES = @srcdir@/BaseDisplay.m @srcdir@/Basemenu.m @srcdir@/Configmenu.m @srcdir@/Icon.m @srcdir@/Image.m @srcdir@/Screen.m @srcdir@/Slit.m @srcdir@/Toolbar.m @srcdir@/Window.m @srcdir@/Windowmenu.m @srcdir@/Workspace.m @srcdir@/Workspacemenu.m @srcdir@/blackbox.m @srcdir@/common.m @srcdir@/main.m @srcdir@/bsetroot.m
EXTRA_DIST= BaseDisplay.m Basemenu.m Configmenu.m Icon.m Image.m Screen.m Slit.m Toolbar.m Window.m Windowmenu.m Workspace.m Workspacemenu.m blackbox.m bsetroot.m common.m main.m

all-local: openbox.cat
install-data-local: openbox.cat
	@if test x$(NLSTEST) = "xyes"; then \
		echo "Installing catalog in $(THE_NLS_PATH)"; \
		$(mkinstalldirs) $(THE_NLS_PATH); \
		$(INSTALL_DATA) openbox.cat $(THE_NLS_PATH); \
                rm -f $(DESTDIR)$(pkgdatadir)/nls/nl; \
                ln -f -s nl_NL $(DESTDIR)$(pkgdatadir)/nls/nl; \
fi
uninstall-local:
	@if test x$(NLSTEST) = "xyes"; then \
		rm -f $(THE_NLS_PATH)/openbox.cat; \
		rm -f $(DESTDIR)$(pkgdatadir)/nls/nl; \
		rmdir $(THE_NLS_PATH); \
	fi

Translation.m: $(MFILES)
	@if test x$(NLSTEST) = "xyes"; then \
		awk -f @srcdir@/../convert.awk output=Translation.m $(MFILES); \
	fi

openbox.cat: Translation.m
	@if test x$(NLSTEST) = "xyes"; then \
		$(gencat_cmd) openbox.cat Translation.m; \
	fi

distclean-local:
	rm -f *\~ .\#*
