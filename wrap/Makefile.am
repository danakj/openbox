oblibdir = $(libdir)/openbox

#pythondir = $(oblibdir)/python
obpythondir = $(oblibdir)/python

CPPFLAGS = -I$(srcdir)/../otk -I$(srcdir)/../src $(XFT_CFLAGS) $(SWIG_PYTHON_CFLAGS) @CPPFLAGS@
CXXFLAGS = $(filter-out -W -Wall,@CXXFLAGS@)
LDFLAGS  = -module -avoid-version

obpython_LTLIBRARIES = otkpy.la obpy.la
obpython_PYTHON = otk.py ob.py

otkpy_la_SOURCES  = wrap_otk.cc
#otkpy_la_LIBADD   = ../otk/libotk.la   # do this for the global version of otk

obpy_la_SOURCES  = wrap_ob.cc

EXTRA_DIST = otk.i ob.i callback.i

CLEANFILES = wrap_* *.py
MAINTAINERCLEANFILES = Makefile.in

install-exec-hook:
	$(mkinstalldirs) "$(DESTDIR)$(obpythondir)"
	cd "$(DESTDIR)$(obpythondir)" && \
		$(RM) -f _otk.so && $(LN_S) otkpy.so _otk.so
	$(mkinstalldirs) "$(DESTDIR)$(obpythondir)"
	cd "$(DESTDIR)$(obpythondir)" && \
		$(RM) -f _ob.so && $(LN_S) obpy.so _ob.so

uninstall-am:
	$(RM) "$(DESTDIR)$(obpythondir)/_otk.so"
	$(RM) "$(DESTDIR)$(obpythondir)/_ob.so"
	-rmdir -p $(obpythondir)

%.py: wrap_%.cc

otk.i: $(wildcard ../otk/*.hh)
	@touch $@

ob.i: $(addprefix ../src/,openbox.hh screen.hh client.hh python.hh frame.hh)
	@touch $@

wrap_otk.cc: otk.i
	$(SWIG) $(SWIG_PYTHON_OPT) $(filter -I%,$(CPPFLAGS)) -nodefault -o $@ $<

wrap_ob.cc: ob.i callback.i ob_client.i ob_screen.i ob_openbox.i ob_frame.i ob_python.i
	$(SWIG) $(SWIG_PYTHON_OPT) $(filter -I%,$(CPPFLAGS)) -nodefault -o $@ $<

