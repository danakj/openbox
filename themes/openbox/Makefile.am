themedir=$(datadir)/openbox/themes/openbox
eboxbuttondir=$(themedir)/ebox_buttons
fieronbuttondir=$(themedir)/fieron_buttons
fieron2buttondir=$(themedir)/fieron2_buttons

theme_DATA = artwiz bbs bluebox cthulhain deep fieron fieron2 flux frobozz \
	frobust mbdtex miklos nyz nyzclone ob20 operation outcomes paper \
	purplehaaze shade steelblue steelblue2 the_orange trisb twice \
	warp-xp ebox

eboxbutton_DATA = ebox_buttons/max.xbm ebox_buttons/icon.xbm \
		  ebox_buttons/close.xbm
fieronbutton_DATA = fieron_buttons/max.xbm fieron_buttons/stick.xbm \
		    fieron_buttons/close.xbm fieron_buttons/icon.xbm
fieron2button_DATA = fieron2_buttons/max.xbm fieron2_buttons/stick.xbm \
		     fieron2_buttons/close.xbm fieron2_buttons/icon.xbm

EXTRA_DIST = $(theme_DATA)

MAINTAINERCLEANFILES = Makefile.in

distclean-local:
	$(RM) *\~ .\#*
uninstall-am:
	-rmdir -p $(DESTDIR)$(themedir)
