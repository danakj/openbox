rcdir=$(datadir)/openbox

INCLUDES=-I../..

CPPFLAGS=$(X_CFLAGS) $(XFT_CFLAGS) $(GLIB_CFLAGS) @CPPFLAGS@ \
         -DRCDIR=\"$(rcdir)\" \
         -DG_LOG_DOMAIN=\"ObSlit\"
LIBS=$(X_LIBS) $(XFT_LIBS) $(GLIB_LIBS) @LIBS@ @LIBINTL@

bin_PROGRAMS=obslit

obslit_LDADD=-lobrender -L../../render
obslit_SOURCES=slit.c
MAINTAINERCLEANFILES=Makefile.in

distclean-local:
	$(RM) *\~ *.orig *.rej .\#*
