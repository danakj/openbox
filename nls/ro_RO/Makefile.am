# Makefile.am for Blackbox - an X11 Window Manager

NLSTEST = @NLS@
THE_LANG = ro_RO
THE_NLS_PATH = $(DESTDIR)$(pkgdatadir)/nls/$(THE_LANG)
CLEANFILES = blackbox.cat
MAINTAINERCLEANFILES = Makefile.in Translation.m
DISTCLEANFILES = Translation.m
MFILES = @srcdir@/BaseDisplay.m @srcdir@/Basemenu.m @srcdir@/Configmenu.m @srcdir@/Icon.m @srcdir@/Image.m @srcdir@/Screen.m @srcdir@/Slit.m @srcdir@/Toolbar.m @srcdir@/Window.m @srcdir@/Windowmenu.m @srcdir@/Workspace.m @srcdir@/Workspacemenu.m @srcdir@/blackbox.m @srcdir@/common.m @srcdir@/main.m @srcdir@/bsetroot.m

all-local: blackbox.cat
install-data-local: blackbox.cat
	@if test x$(NLSTEST) = "x-DNLS"; then \
		echo "Installing catalog in $(THE_NLS_PATH)"; \
		$(mkinstalldirs) $(THE_NLS_PATH); \
		$(INSTALL_DATA) blackbox.cat $(THE_NLS_PATH); \
	fi
uninstall-local:
	@if test x$(NLSTEST) = "x-DNLS"; then \
		rm -f $(THE_NLS_PATH)/blackbox.cat; \
		rmdir $(THE_NLS_PATH); \
	fi

Translation.m: $(MFILES)
	@if test x$(NLSTEST) = "x-DNLS"; then \
		awk -f @srcdir@/../convert.awk output=Translation.m $(MFILES); \
	fi

blackbox.cat: Translation.m
	@if test x$(NLSTEST) = "x-DNLS"; then \
		$(gencat_cmd) blackbox.cat Translation.m; \
	fi

distclean-local:
	rm -f *\~ .\#*
