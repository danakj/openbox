all install uninstall:
	@$(MAKE) -$(MAKEFLAGS) -f build/Makefile.render $@
	@$(MAKE) -$(MAKEFLAGS) -f build/Makefile.kernel $@
	@$(MAKE) -$(MAKEFLAGS) -f build/Makefile.plugins $@
#	@$(MAKE) -$(MAKEFLAGS) -f build/Makefile.engines $@
#	@$(MAKE) -$(MAKEFLAGS) -f build/Makefile.themes $@

clean:
	@$(MAKE) -$(MAKEFLAGS) -f build/Makefile.render $@
	@$(MAKE) -$(MAKEFLAGS) -f build/Makefile.kernel $@
	@$(MAKE) -$(MAKEFLAGS) -f build/Makefile.plugins $@
#	@$(MAKE) -$(MAKEFLAGS) -f build/Makefile.engines $@
#	@$(MAKE) -$(MAKEFLAGS) -f build/Makefile.themes $@
	$(RM) *\~

distclean: clean
	$(RM) configure Makefile.incl
	$(RM) -r .deps/

.PHONY: all clean distclean install uninstall
