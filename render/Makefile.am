themedir=$(datadir)/openbox/themes

theme=operation

CPPFLAGS=$(XFT_CFLAGS) $(GLIB_CFLAGS) $(LIBSN_CFLAGS) @CPPFLAGS@ \
         -DG_LOG_DOMAIN=\"Render\" \
         -DDEFAULT_THEME=\"$(theme)\" \
         -DTHEMEDIR=\"$(themedir)\"

INCLUDES=-I..
LIBS=$(XFT_LIBS) $(GLIB_LIBS) @LIBS@

noinst_PROGRAMS=rendertest
rendertest_LDFLAGS=-lobrender -L.
rendertest_SOURCES=test.c

lib_LTLIBRARIES=libobrender.la
libobrender_la_SOURCES=color.c font.c gradient.c image.c mask.c render.c \
                       theme.c


noinst_HEADERS=render.h gradient.h color.h font.h mask.h image.h

MAINTAINERCLEANFILES=Makefile.in

distclean-local:
	$(RM) *\~ *.orig *.rej .\#*
