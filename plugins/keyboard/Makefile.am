plugindir=$(libdir)/openbox/plugins

AM_CPPFLAGS=$(XFT_CFLAGS) $(GLIB_CFLAGS) $(LIBSN_CFLAGS) $(GL_CFLAGS) \
            $(XML_CFLAGS) \
            -DG_LOG_DOMAIN=\"Plugin-Keyboard\"

INCLUDES=-I../..

plugin_LTLIBRARIES=keyboard.la

keyboard_la_LDFLAGS=-module -avoid-version
keyboard_la_SOURCES=\
  keyboard.h \
  keyboard.c \
  translate.h \
  translate.c \
  tree.h \
  tree.c

MAINTAINERCLEANFILES=Makefile.in

distclean-local:
	$(RM) *\~ *.orig *.rej .\#*
