# Makefile.am for Openbox

SUBDIRS = intl po m4 data doc otk src
MAINTAINERCLEANFILES = aclocal.m4 config.h.in configure Makefile.in stamp-h.in

uninstall-local:
	-rmdir $(pkgdatadir)

distclean-local:
	rm -f *\~ gmon.out .\#*

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = config.rpath mkinstalldirs ABOUT-NLS INSTALL README \
             COPYING AUTHORS CHANGELOG CHANGELOG.Blackbox CHANGELOG.bsetbg \
             LICENSE README.bbtools README.bsetbg
