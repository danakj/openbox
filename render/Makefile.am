CPPFLAGS=$(XFT_CFLAGS) $(PYTHON_CFLAGS) $(GLIB_CFLAGS) @CPPFLAGS@ \
-DG_LOG_DOMAIN=\"Render\"

LIBS=$(XFT_LIBS) $(PYTHON_LIBS) $(GLIB_LIBS) @LIBS@

noinst_PROGRAMS=rendertest
rendertest_SOURCES=$(librender_a_SOURCES)

noinst_LIBRARIES=librender.a
librender_a_SOURCES=render.c test.c gradient.c color.c font.c

noinst_HEADERS=render.h gradient.h color.h font.h

MAINTAINERCLEANFILES= Makefile.in

distclean-local:
	$(RM) *\~ *.orig *.rej .\#*
