#SUBDIRS = po themes doc render cwmcc obcl kernel plugins
SUBDIRS = po themes data render kernel plugins tools
MAINTAINERCLEANFILES = aclocal.m4 config.h.in configure Makefile.in stamp-h.in

doc:
	$(MAKE) -$(MAKEFLAGS) -C doc/doxygen doc

distclean-local:
	rm -f *\~ gmon.out .\#*

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = config.rpath mkinstalldirs INSTALL README \
             COPYING AUTHORS CHANGELOG

.PHONY: doc
