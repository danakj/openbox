# nls/Makefile.am for Openbox

NLSTEST = @NLS@
SUBDIRS = C da_DK de_DE es_ES et_EE fr_FR it_IT ja_JP nl_NL pt_BR ru_RU \
	sl_SI sv_SE tr_TR zh_CN
MAINTAINERCLEANFILES = Makefile.in openbox-nls.h
DISTCLEANFILES = openbox-nls.h

uninstall-local:
	@if test x$(NLSTEST) = "x-DNLS"; then \
		rmdir $(DESTDIR)$(pkgdatadir)/nls; \
	fi
distclean-local:
	rm -f *\~
