themedir=$(datadir)/openbox/themes/openbox
fieron2buttondir=$(themedir)/fieron2_buttons

theme_DATA = artwiz bbs bluebox cthulhain deep fieron fieron2 flux frobozz \
	frobust mbdtex miklos nyz nyzclone ob20 operation outcomes paper \
	purplehaaze shade steelblue steelblue2 the_orange trisb twice \
	warp-xp

fieron2button_DATA = fieron2_buttons/max.xbm fieron2_buttons/stick.xbm \
		     fieron2_buttons/close.xbm fieron2_buttons/icon.xbm

EXTRA_DIST = $(theme_DATA)

MAINTAINERCLEANFILES = Makefile.in

distclean-local:
	$(RM) *\~ .\#*
uninstall-am:
	-rmdir -p $(DESTDIR)$(themedir)
