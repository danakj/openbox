plugindir=$(libdir)/openbox/plugins

AM_CPPFLAGS=$(XFT_CFLAGS) $(GLIB_CFLAGS) $(LIBSN_CFLAGS) $(GL_CFLAGS) \
            $(XML_CFLAGS)  \
            -DG_LOG_DOMAIN=\"Plugin-Mouse\"

INCLUDES=-I../..

plugin_LTLIBRARIES=mouse.la

mouse_la_LDFLAGS=-module -avoid-version
mouse_la_SOURCES=\
  mouse.h \
  mouse.c \
  translate.h \
  translate.c

noinst_HEADERS=mouse.h translate.h

MAINTAINERCLEANFILES=Makefile.in

distclean-local:
	$(RM) *\~ *.orig *.rej .\#*
