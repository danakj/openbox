scriptdir = $(libdir)/openbox/python
MAINTAINERCLEANFILES = Makefile.in
script_DATA = clientmotion.py clicks.py globals.py config.py
EXTRA_DIST = $(script_DATA)

distclean-local:
	rm -f *\~ .\#*
uninstall-am:
	rmdir -p $(scriptdir)
