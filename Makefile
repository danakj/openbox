include build/Makefile.incl

depdir:=.deps

all: alltargets

include build/Makefile.render
include build/Makefile.kernel
include build/Makefile.themes
include build/Makefile.plugins
include build/Makefile.engines

alltargets: $(kernel_target) $(plugins_targets) $(engines_targets)

install: all render-install kernel-install themes-install plugins-install engines-install

uninstall: render-uninstall kernel-uninstall themes-uninstall plugins-uninstall engines-uninstall

clean: render-clean kernel-clean plugins-clean engines-clean
	$(RM) *\~

distclean: clean
	$(RM) configure Makefile.incl
	$(RM) -r .deps/

$(depdir):
	@mkdir $@

.PHONY: all clean distclean
