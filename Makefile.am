# Makefile.am for Openbox

SUBDIRS = po data doc otk src scripts
MAINTAINERCLEANFILES = aclocal.m4 config.h.in configure Makefile.in stamp-h.in

.PHONY: doc

doc:
	$(MAKE) -$(MAKEFLAGS) -C doc/doxygen doc

uninstall-local:
	-rmdir $(pkgdatadir)

distclean-local:
	rm -f *\~ gmon.out .\#*

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = config.rpath mkinstalldirs ABOUT-NLS INSTALL README \
             COPYING AUTHORS CHANGELOG CHANGELOG.Blackbox CHANGELOG.bsetbg \
             LICENSE README.bbtools README.bsetbg
