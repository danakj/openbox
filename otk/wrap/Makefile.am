# XXX - INSTALL THIS GLOBALLY!#&@(!!!
pythondir = $(libdir)/openbox/python

CXXFLAGS = $(XFT_CFLAGS) $(filter-out -W -Wall,@CXXFLAGS@)

INCLUDES = -I.. -I../..

python_LTLIBRARIES = otkpy.la

otkpy_la_CXXFLAGS = $(PYTHON_CFLAGS)
otkpy_la_LDFLAGS  = -module
otkpy_la_SOURCES  = wrap_otk.cc
otkpy_la_LIBADD   = ../libotk.la

CLEANFILES = wrap_* otk.py
MAINTAINERCLEANFILES = Makefile.in

python_PYTHON = otk.py

install-exec-hook:
	$(mkinstalldirs) "$(DESTDIR)$(pythondir)"
	cd "$(DESTDIR)$(pythondir)" && \
		$(RM) -f _otk.so && $(LN_S) otkpy.so _otk.so

uninstall-local:
	rm -f "$(DESTDIR)$(pythondir)/_otk.so"

%.py: wrap_%.cc

wrap_%.cc: %.i $(wildcard ../*.hh)
	swig $(INCLUDES) $(filter -I%,$(CXXFLAGS)) -python -shadow -c++ -nodefault -o $@ $<

