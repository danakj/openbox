# doc/Makefile.am for Openbox

if XFT
if XFT2
else
XFTMAN =		xftlsfonts.1
endif
endif

CLEANFILES = openbox.1
MAINTAINERCLEANFILES = Makefile.in
man_MANS = openbox.1 bsetroot.1 bsetbg.1 $(XFTMAN)
SUBDIRS = 

DEFAULT_MENU=$(pkgdatadir)/menu

EXTRA_DIST = openbox.1.in bsetroot.1 bsetbg.1 xftlsfonts.1

distclean-local:
	rm -f *\~ .\#*

openbox.1: openbox.1.in
	@regex_cmd@ -e "s,@defaultmenu@,$(DEFAULT_MENU)," \
		-e "s,@pkgdatadir@,$(pkgdatadir)," \
		-e "s,@version@,$(VERSION)," \
		 @srcdir@/openbox.1.in > openbox.1
