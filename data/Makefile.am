# data/Makefile.am for Openbox

SUBDIRS = styles
CLEANFILES = menu
MAINTAINERCLEANFILES = Makefile.in

all-local: menu

distclean-local:
	rm -f *\~

menu: menu.in
	@regex_cmd@ -e "s,@pkgdatadir@,$(pkgdatadir)," @srcdir@/menu.in > menu

install-data-local: menu
	test -f $(DESTDIR)$(pkgdatadir)/menu || \
	$(INSTALL_DATA) menu $(DESTDIR)$(pkgdatadir)

uninstall-am:
	rm -f $(DESTDIR)$(pkgdatadir)/menu

