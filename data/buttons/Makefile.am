# data/styles/Makefile.am for Openbox

buttonsdir = $(pkgdatadir)/buttons
MAINTAINERCLEANFILES = Makefile.in
buttons_DATA = fieron-bullet.xbm fieron-bulletl.xbm fieron-close.xbm \
               fieron-icon.xbm fieron-max.xbm fieron-stick.xbm \
               max.xbm stick.xbm icon.xbm close.xbm

distclean-local:
	rm -f *\~ .\#*
uninstall-am:
	rmdir $(buttonsdir)
