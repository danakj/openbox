SUBDIRS = styles buttons
CLEANFILES = menu
MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST = README.menu menu.in

all-local: menu

distclean-local:
	$(RM) *\~ .\#*

menu: menu.in
	@regex_cmd@ -e "s,@pkgdatadir@,$(pkgdatadir)," @srcdir@/menu.in > menu

install-data-local: menu
	test -f $(DESTDIR)$(pkgdatadir)/menu || \
	$(INSTALL_DATA) menu $(DESTDIR)$(pkgdatadir)

uninstall-am:
	$(RM) $(DESTDIR)$(pkgdatadir)/menu
