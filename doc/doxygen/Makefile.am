all:

doc: doxygen.conf
	@doxygen doxygen.conf
	@echo
	@echo Documentation generated in $(shell pwd)/html
	@echo

doxygen.conf: doxygen.conf.in
	@regex_cmd@ -e "s,\@version\@,$(VERSION)," \
	  -e "s,\@basedir\@,`cd $(top_builddir) && pwd`," \
	  doxygen.conf.in > doxygen.conf
