#SUBDIRS = styles buttons
CLEANFILES = menu
MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST = README.menu menu.in

pkgdata_DATA = menu

distclean-local:
	$(RM) *\~ .\#*

menu: menu.in
	$(SED) -e "s!@pkgdatadir@!$(pkgdatadir)!" $< > $@

uninstall-am:
	$(RM) $(DESTDIR)$(pkgdatadir)/menu
