# util/epist/Makefile.am for Blackbox - an X11 Window manager

DEFAULT_RC=$(pkgdatadir)/epistrc

CLEANFILES = epist.1 epistrc.5

CPPFLAGS= @CPPFLAGS@ @DEBUG@ -DDEFAULTRC=\"$(DEFAULT_RC)\"

#EXTRA_PROGRAMS =		epist
bin_PROGRAMS =		epist

man_MANS = epist.1 epistrc.5

epist_SOURCES =		epist.cc window.cc screen.cc main.cc actions.cc yacc_parser.cc parser.cc keytree.cc lex.yy.c config.cc
epist_LDADD =		../../src/XAtom.o ../../src/BaseDisplay.o \
					../../src/Util.o ../../src/i18n.o ../../src/Timer.o \
					../../src/GCCache.o ../../src/Color.o
#					../../src/Texture.o \
#					../../src/Image.o ../../src/ImageControl.o

MAINTAINERCLEANFILES =	Makefile.in

distclean-local:
	rm -f *\~ .\#*

epist.1: epist.1.in
	@regex_cmd@ -e "s,@pkgdatadir@,$(pkgdatadir)," \
		epist.1.in > epist.1

epistrc.5: epistrc.5.in
	@regex_cmd@ -e "s,@pkgdatadir@,$(pkgdatadir)," \
		epistrc.5.in > epistrc.5

install-data-local: epistrc
	test -f $(DESTDIR)$(pkgdatadir)/epistrc || \
	$(INSTALL_DATA) epistrc $(DESTDIR)$(pkgdatadir)

uninstall-am:
	rm -f $(DESTDIR)$(pkgdatadir)/epistrc

# local dependencies

actions.o: actions.cc actions.hh
epist.o: epist.cc actions.hh epist.hh window.hh ../../src/Util.hh \
  keytree.hh screen.hh ../../src/BaseDisplay.hh ../../src/Timer.hh \
  parser.hh ../../src/XAtom.hh
keytree.o: keytree.cc keytree.hh actions.hh screen.hh window.hh \
  ../../src/Util.hh
main.o: main.cc epist.hh actions.hh window.hh ../../src/Util.hh \
  keytree.hh screen.hh ../../src/BaseDisplay.hh ../../src/Timer.hh \
  ../../src/i18n.hh ../../nls/blackbox-nls.hh
parser.o: parser.cc parser.hh actions.hh keytree.hh screen.hh window.hh \
  ../../src/Util.hh
screen.o: screen.cc ../../src/BaseDisplay.hh ../../src/Timer.hh \
  ../../src/Util.hh ../../src/XAtom.hh screen.hh window.hh epist.hh \
  actions.hh keytree.hh
window.o: window.cc epist.hh actions.hh window.hh ../../src/Util.hh \
  keytree.hh screen.hh ../../src/BaseDisplay.hh ../../src/Timer.hh \
  ../../src/XAtom.hh
config.o: config.cc config.hh ../../src/Util.hh \
  keytree.hh screen.hh ../../src/BaseDisplay.hh ../../src/Timer.hh \
  ../../src/XAtom.hh
yacc_parser.cc: epist.y
	bison -d epist.y -o yacc_parser.cc
lex.yy.c: epist.l
	lex epist.l
