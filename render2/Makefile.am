themedir=$(datadir)/openbox/themes

theme=operation

CPPFLAGS=$(FC_CFLAGS) $(GLIB_CFLAGS) $(GL_CFLAGS) @CPPFLAGS@ \
         -DG_LOG_DOMAIN=\"Render\" \
         -DDEFAULT_THEME=\"$(theme)\" \
         -DTHEMEDIR=\"$(themedir)\"

INCLUDES=-I..
LIBS=$(GL_LIBS) $(GLIB_LIBS) $(X_LIBS) $(XSHAPE_LIBS) @LIBS@

noinst_PROGRAMS=rendertest
rendertest_LDFLAGS=-lobrender2 -L.
rendertest_SOURCES=test.c

lib_LTLIBRARIES=libobrender2.la
libobrender2_la_LIBADD=-lglft -L../glft
libobrender2_la_SOURCES=\
	instance.c \
	color.c \
	debug.c \
	font.c \
	surface.c \
	planar.c \
	texture.c \
	paint.c \
	theme.c

noinst_HEADERS=\
	render.h \
	instance.h \
	debug.h \
	surface.h \
	planar.h \
	texture.h \
	color.h \
	theme.h

MAINTAINERCLEANFILES=Makefile.in

distclean-local:
	$(RM) *\~ *.orig *.rej .\#*
