themedir=$(datadir)/openbox/themes/openbox

theme_DATA = artwiz bbs bluebox cthulhain deep fieron fieron2 flux frobozz \
	frobust mbdtex miklos nyz nyzclone ob20 operation outcomes paper \
	purplehaaze shade steelblue steelblue2 the_orange trisb twice \
	warp-xp

EXTRA_DIST = $(theme_DATA)

MAINTAINERCLEANFILES = Makefile.in

distclean-local:
	$(RM) *\~ .\#*
uninstall-am:
	-rmdir -p $(DESTDIR)$(themedir)
