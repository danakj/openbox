bin_PROGRAMS=obsetroot

obsetroot_LDADD=$(XFT_LIBS) -L../otk -lotk @LIBINTL@
obsetroot_CPPFLAGS=$(XFT_CFLAGS) -I..
obsetroot_SOURCES=obsetroot.cc

MAINTAINERCLEANFILES=Makefile.in

distclean-local:
	$(RM) *\~ *.orig *.rej .\#*
