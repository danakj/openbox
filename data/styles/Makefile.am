# data/styles/Makefile.am for Openbox

styledir = $(pkgdatadir)/styles
MAINTAINERCLEANFILES = Makefile.in
style_DATA = artwiz bbs bluebox cthulhain deep fieron flux frobozz frobust \
	mbdtex miklos nyz nyzclone ob20 operation outcomes paper purplehaaze
	shade steelblue steelblue2 the_orange trisb twice warp-xp
EXTRA_DIST = $(style_DATA)

distclean-local:
	rm -f *\~ .\#*
uninstall-am:
	rmdir $(styledir)
