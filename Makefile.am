SUBDIRS = po data themes doc render kernel engines plugins
MAINTAINERCLEANFILES = aclocal.m4 config.h.in configure Makefile.in stamp-h.in

.PHONY: doc

doc:
	$(MAKE) -$(MAKEFLAGS) -C doc/doxygen doc

distclean-local:
	rm -f *\~ gmon.out .\#*

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = config.rpath mkinstalldirs INSTALL README \
             COPYING AUTHORS CHANGELOG
