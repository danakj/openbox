all:

doc: doxygen.conf
	@doxygen doxygen.conf

doxygen.conf: doxygen.conf.in
	@regex_cmd@ -e "s,\@version\@,$(VERSION)," \
	  -e "s,\@basedir\@,`cd $(top_builddir) && pwd`," \
	  doxygen.conf.in > doxygen.conf
