localedir=$(datadir)/locale
scriptdir = $(libdir)/openbox/python

CPPFLAGS=$(PYTHON_CFLAGS) $(GLIB_CFLAGS) @CPPFLAGS@ \
-DLOCALEDIR=\"$(localedir)\" \
-DSCRIPTDIR=\"$(scriptdir)\" \
-DG_LOG_DOMAIN=\"Openbox\"

LIBS=$(PYTHON_LIBS) $(GLIB_LIBS) @LIBS@

bin_PROGRAMS= ob3

ob3_LDADD=@LIBINTL@
ob3_LDFLAGS=-export-dynamic
ob3_SOURCES=client.c event.c extensions.c focus.c frame.c openbox.c prop.c \
	python.c screen.c stacking.c xerror.c hooks.c eventdata.c obexport.c \
	clientwrap.c screenwrap.c kbind.c mbind.c

noinst_HEADERS=client.h event.h extensions.h focus.h frame.h geom.h gettext.h \
	openbox.h prop.h python.h screen.h stacking.h xerror.h hooks.h \
	eventdata.h clientwrap.h obexport.h screenwrap.h kbind.h mbind.h

MAINTAINERCLEANFILES= Makefile.in

#if CVS
#ob3.i: $(wildcard *.h)
#	@touch $@

#ob3_wrap.c: ob3.i
#	$(SWIG) $(SWIG_PYTHON_OPT) $(filter -I%,$(CPPFLAGS)) -o $@ $<
#endif

distclean-local:
	$(RM) *\~ *.orig *.rej .\#*
