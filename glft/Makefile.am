themedir=$(datadir)/openbox/themes

theme=operation

CPPFLAGS=$(FC_CFLAGS) $(GLIB_CFLAGS) $(GL_CFLAGS) @CPPFLAGS@ \
         -DG_LOG_DOMAIN=\"GlFt\" \
         -DDEFAULT_THEME=\"$(theme)\" \
         -DTHEMEDIR=\"$(themedir)\"

INCLUDES=-I..
LIBS=$(FC_LIBS) $(GLIB_LIBS) $(X_LIBS) $(GL_LIBS) @LIBS@

noinst_PROGRAMS=glfttest
glfttest_LDFLAGS=-lglft -L.
glfttest_SOURCES=test.c

lib_LTLIBRARIES=libglft.la
libglft_la_SOURCES=init.c


noinst_HEADERS=glft.h

MAINTAINERCLEANFILES=Makefile.in

distclean-local:
	$(RM) *\~ *.orig *.rej .\#*
