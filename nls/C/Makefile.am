# nls/C/Makefile.am for Openbox

NLSTEST = @NLS@
CLEANFILES = openbox.cat
MAINTAINERCLEANFILES = Makefile.in Translation.m
DISTCLEANFILES = Translation.m
MFILES = @srcdir@/BaseDisplay.m @srcdir@/Basemenu.m @srcdir@/Configmenu.m @srcdir@/Icon.m @srcdir@/Image.m @srcdir@/Screen.m @srcdir@/Slit.m @srcdir@/Toolbar.m @srcdir@/Window.m @srcdir@/Windowmenu.m @srcdir@/Workspace.m @srcdir@/Workspacemenu.m @srcdir@/openbox.m @srcdir@/common.m @srcdir@/main.m @srcdir@/bsetroot.m

all-local: Translation.m openbox.cat
install-data-local: openbox.cat
	@if test x$(NLSTEST) = "x-DNLS"; then \
		echo "Installing catalog in $(DESTDIR)$(pkgdatadir)/nls/C"; \
		$(mkinstalldirs) $(DESTDIR)$(pkgdatadir)/nls/C; \
		$(INSTALL_DATA) openbox.cat $(DESTDIR)$(pkgdatadir)/nls/C; \
		rm -f $(DESTDIR)$(pkgdatadir)/nls/US_ASCII; \
		ln -f -ss C $(DESTDIR)$(pkgdatadir)/nls/US_ASCII; \
		rm -f $(DESTDIR)$(pkgdatadir)/nls/POSIX; \
		ln -f -s C $(DESTDIR)$(pkgdatadir)/nls/POSIX; \
		rm -f $(DESTDIR)$(pkgdatadir)/nls/en_US; \
		ln -f -s C $(DESTDIR)$(pkgdatadir)/nls/en_US; \
		rm -f $(DESTDIR)$(pkgdatadir)/nls/en; \
		ln -f -s C $(DESTDIR)$(pkgdatadir)/nls/en; \
	fi
uninstall-local:
	@if test x$(NLSTEST) = "x-DNLS"; then \
		rm -f $(DESTDIR)$(pkgdatadir)/nls/C/openbox.cat; \
		rm -f $(DESTDIR)$(pkgdatadir)/nls/US_ASCII; \
		rm -f $(DESTDIR)$(pkgdatadir)/nls/POSIX; \
		rm -f $(DESTDIR)$(pkgdatadir)/nls/en_US; \
		rm -f $(DESTDIR)$(pkgdatadir)/nls/en; \
		rmdir $(DESTDIR)$(pkgdatadir)/nls/C; \
	fi

# note... when adding new catalogs... you do not need the
# header=../openbox-nls.h which is passed to awk... we
# only need to generate it once.

Translation.m: $(MFILES)
	awk -f @srcdir@/../convert.awk header=../openbox-nls.h \
		output=Translation.m $(MFILES)

openbox.cat: Translation.m
	@if test x$(NLSTEST) = "x-DNLS"; then \
		$(gencat_cmd) openbox.cat Translation.m; \
	fi

distclean-local:
	rm -f *\~
