#SUBDIRS = styles buttons
CLEANFILES = menu
MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST = README.menu menu.in

pkgdata_DATA = menu

distclean-local:
	$(RM) *\~ .\#*

menu: menu.in
	@regex_cmd@ -e "s,@pkgdatadir@,$(pkgdatadir)," @srcdir@/menu.in > menu

uninstall-am:
	$(RM) $(DESTDIR)$(pkgdatadir)/menu
