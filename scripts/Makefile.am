scriptdir = $(libdir)/openbox/python
MAINTAINERCLEANFILES = Makefile.in
script_DATA = config.py defaults.py focus.py callbacks.py \
              focusmodel.py windowplacement.py behavior.py motion.py \
              historyplacement.py stackedcycle.py
EXTRA_DIST = $(script_DATA)

distclean-local:
	rm -f *\~ .\#*
uninstall-am:
	rmdir -p $(scriptdir)
