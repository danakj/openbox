themedir=$(datadir)/openbox/themes

theme=operation

CPPFLAGS=$(XFT_CFLAGS) $(GLIB_CFLAGS) $(LIBSN_CFLAGS) $(GL_CFLAGS) @CPPFLAGS@ \
         -DG_LOG_DOMAIN=\"Render\" \
         -DDEFAULT_THEME=\"$(theme)\" \
         -DTHEMEDIR=\"$(themedir)\"

INCLUDES=-I..
LIBS=$(XFT_LIBS) $(GLIB_LIBS) $(GL_LIBS) @LIBS@

noinst_PROGRAMS=rendertest
rendertest_LDFLAGS=-lobrender -L.
rendertest_SOURCES=test.c

lib_LTLIBRARIES=libobrender.la
libobrender_la_SOURCES=\
  color.h \
  color.c \
  font.h \
  font.c \
  gradient.h \
  gradient.c \
  image.h \
  image.c \
  mask.h \
  mask.c \
  render.h \
  render.c \
  theme.c \
  instance.h \
  instance.c


noinst_HEADERS=render.h gradient.h color.h font.h mask.h image.h

MAINTAINERCLEANFILES=Makefile.in

distclean-local:
	$(RM) *\~ *.orig *.rej .\#*
