# nls/tr_TR/Makefile.am for Openbox
# 
# -- Türkce tercüme dizini için bir yapim-dosya-tabaný
# 	kabalak -- kabalak@gmx.net
#

NLSTEST = @NLS@
CLEANFILES = openbox.cat
MAINTAINERCLEANFILES = Makefile.in Translation.m
DISTCLEANFILES = Translation.m
MFILES = @srcdir@/BaseDisplay.m @srcdir@/Basemenu.m @srcdir@/Configmenu.m @srcdir@/Icon.m @srcdir@/Image.m @srcdir@/Screen.m @srcdir@/Slit.m @srcdir@/Toolbar.m @srcdir@/Window.m @srcdir@/Windowmenu.m @srcdir@/Workspace.m @srcdir@/Workspacemenu.m @srcdir@/openbox.m @srcdir@/common.m @srcdir@/main.m @srcdir@/bsetroot.m

all-local: openbox.cat
install-data-local: openbox.cat
	@if test x$(NLSTEST) = "x-DNLS"; then \
		echo "Installing catalog in $(DESTDIR)$(pkgdatadir)/nls/tr_TR"; \
		$(mkinstalldirs) $(DESTDIR)$(pkgdatadir)/nls/tr_TR; \
		$(INSTALL_DATA) openbox.cat $(DESTDIR)$(pkgdatadir)/nls/tr_TR; \
	fi
uninstall-local:
	@if test x$(NLSTEST) = "x-DNLS"; then \
		rm -f $(DESTDIR)$(pkgdatadir)/nls/tr_TR/openbox.cat; \
		rmdir $(DESTDIR)$(pkgdatadir)/nls/tr_TR; \
	fi

Translation.m: $(MFILES)
	@if test x$(NLSTEST) = "x-DNLS"; then \
		awk -f @srcdir@/../convert.awk output=Translation.m $(MFILES); \
	fi

openbox.cat: Translation.m
	@if test x$(NLSTEST) = "x-DNLS"; then \
		$(gencat_cmd) openbox.cat Translation.m; \
	fi

distclean-local:
	rm -f *\~
