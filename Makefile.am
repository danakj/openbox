# Makefile.am for Openbox

SUBDIRS = data doc nls src util
MAINTAINERCLEANFILES = aclocal.m4 config.h.in configure Makefile.in stamp-h.in

EXTRA_DIST=CHANGELOG CHANGELOG.Blackbox CHANGELOG.bsetbg INSTALL LICENSE \
           README.bbtools README.bsetbg stamp-h.in version.h.in

uninstall-local:
	-rmdir $(pkgdatadir)

distclean-local:
	rm -f *\~ gmon.out .\#*
