themedir=$(datadir)/openbox/themes

theme=operation

CPPFLAGS=$(FC_CFLAGS) $(GLIB_CFLAGS) $(GL_CFLAGS) @CPPFLAGS@ \
         -DG_LOG_DOMAIN=\"Render\" \
         -DDEFAULT_THEME=\"$(theme)\" \
         -DTHEMEDIR=\"$(themedir)\"

INCLUDES=-I..
LIBS=$(GL_LIBS) @LIBS@

noinst_PROGRAMS=rendertest
rendertest_LDFLAGS=-lobrender2 -L.
rendertest_SOURCES=test.c

lib_LTLIBRARIES=libobrender2.la
libobrender2_la_SOURCES=\
	init.c \
	instance.c \
	color.c \
	debug.c \
	font.c \
	surface.c \
	texture.c \
	paint.c

noinst_HEADERS=\
	render.h \
	instance.h \
	debug.h \
	surface.h \
	texture.h

MAINTAINERCLEANFILES=Makefile.in

distclean-local:
	$(RM) *\~ *.orig *.rej .\#*
