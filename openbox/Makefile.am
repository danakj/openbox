localedir=$(datadir)/locale
themercdir=$(datadir)/openbox
scriptdir=$(libdir)/openbox/python
enginedir=$(libdir)/openbox/engines

CPPFLAGS=$(XFT_CFLAGS) $(PYTHON_CFLAGS) $(GLIB_CFLAGS) $(GMODULE_CFLAGS) \
@CPPFLAGS@ \
-DLOCALEDIR=\"$(localedir)\" \
-DTHEMERCDIR=\"$(themercdir)\" \
-DSCRIPTDIR=\"$(scriptdir)\" \
-DENGINEDIR=\"$(enginedir)\" \
-DDEFAULT_ENGINE=\"openbox\" \
-DG_LOG_DOMAIN=\"Openbox\"

LIBS=$(XFT_LIBS) $(PYTHON_LIBS) $(GLIB_LIBS) $(GMODULE_LIBS) @LIBS@

bin_PROGRAMS= ob3

ob3_LDADD=@LIBINTL@ ../render/librender.a
ob3_LDFLAGS=-export-dynamic
ob3_SOURCES=client.c event.c extensions.c focus.c frame.c openbox.c prop.c \
	python.c screen.c stacking.c xerror.c hooks.c themerc.c timer.c \
	clientwrap.c openboxwrap.c pointer.c keyboard.c engine.c

noinst_HEADERS=client.h event.h extensions.h focus.h frame.h geom.h gettext.h \
	openbox.h prop.h python.h screen.h stacking.h xerror.h themerc.h \
	timer.h hooks.h clientwrap.h openboxwrap.h pointer.h keyboard.h \
	engine.h

MAINTAINERCLEANFILES= Makefile.in

distclean-local:
	$(RM) *\~ *.orig *.rej .\#*
