# util/epist/Makefile.am for Blackbox - an X11 Window manager

DEFAULT_RC=$(pkgdatadir)/epistrc

CLEANFILES = epist.1 epistrc.5

CPPFLAGS= @CPPFLAGS@ -DDEFAULTRC=\"$(DEFAULT_RC)\"

#EXTRA_PROGRAMS =		epist
bin_PROGRAMS =		epist

man_MANS = epist.1 epistrc.5

epist_SOURCES =		epist.cc window.cc screen.cc main.cc actions.cc \
			yacc_parser.cc parser.cc keytree.cc lex.yy.c config.cc

epist_LDADD =		../../src/xatom.o ../../src/basedisplay.o \
			../../src/util.o ../../src/timer.o

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
config.o: config.cc config.hh
epist.o: epist.cc actions.hh epist.hh window.hh ../../src/util.hh \
 keytree.hh ../../src/timer.hh screen.hh config.hh \
 ../../src/basedisplay.hh parser.hh ../../src/xatom.hh
keytree.o: keytree.cc keytree.hh ../../src/timer.hh actions.hh \
 screen.hh window.hh ../../src/util.hh config.hh epist.hh \
 ../../src/basedisplay.hh
main.o: main.cc ../../version.h epist.hh actions.hh window.hh \
 ../../src/util.hh keytree.hh ../../src/timer.hh screen.hh config.hh \
 ../../src/basedisplay.hh
parser.o: parser.cc parser.hh actions.hh keytree.hh ../../src/timer.hh \
 screen.hh window.hh ../../src/util.hh config.hh
screen.o: screen.cc ../../src/basedisplay.hh ../../src/timer.hh \
 ../../src/util.hh ../../src/xatom.hh screen.hh window.hh config.hh \
 epist.hh actions.hh keytree.hh
window.o: window.cc epist.hh actions.hh window.hh ../../src/util.hh \
 keytree.hh ../../src/timer.hh screen.hh config.hh \
 ../../src/basedisplay.hh ../../src/xatom.hh
yacc_parser.o: yacc_parser.cc parser.hh actions.hh keytree.hh \
 ../../src/timer.hh screen.hh window.hh ../../src/util.hh config.hh
