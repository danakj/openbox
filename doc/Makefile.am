# doc/Makefile.am for Openbox

CLEANFILES = openbox.1
MAINTAINERCLEANFILES = Makefile.in
man_MANS = openbox.1 bsetroot.1 bsetbg.1
SUBDIRS = 

DEFAULT_MENU=$(pkgdatadir)/menu

distclean-local:
	rm -f *\~

openbox.1: openbox.1.in
	@regex_cmd@ -e "s,@defaultmenu@,$(DEFAULT_MENU)," \
		-e "s,@pkgdatadir@,$(pkgdatadir)," \
		-e "s,@version@,$(VERSION)," \
		 @srcdir@/openbox.1.in > openbox.1
