# util/Makefile.am for Openbox

CPPFLAGS= @CPPFLAGS@ @DEBUG@ @NLS@ @TIMEDCACHE@ @NEWWMSPEC@ @INTERLACE@ \
@ORDEREDPSEUDO@ @CLOBBER@

bin_SCRIPTS = 		bsetbg
bin_PROGRAMS =		bsetroot

bsetroot_SOURCES =	bsetroot.cc
bsetroot_LDADD =	../src/BaseDisplay.o ../src/LinkedList.o ../src/Timer.o ../src/Image.o ../src/i18n.o ../src/Geometry.o

MAINTAINERCLEANFILES =	Makefile.in

distclean-local:
	rm -f *\~

bsetroot.o: bsetroot.cc ../config.h bsetroot.h ../src/BaseDisplay.h \
 ../src/LinkedList.h ../src/Timer.h ../src/Image.h ../src/Geometry.h
