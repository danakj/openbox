scriptdir = $(libdir)/openbox/python
MAINTAINERCLEANFILES = Makefile.in
script_PYTHON = config.py defaults.py focus.py callbacks.py \
                focusmodel.py windowplacement.py behavior.py motion.py \
                historyplacement.py cycle.py

distclean-local:
	$(RM) *\~ .\#*
uninstall-am:
	rmdir -p $(scriptdir)
