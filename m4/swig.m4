# SWIG_PROG([required-version])
#
# Checks for the SWIG program.  If found you can (and should) call SWIG via $(SWIG).
# You can use the optional first argument to check if the version of the available SWIG
# is greater or equal to the value of the argument.  It should have the format:
# N[.N[.N]] (N is a number between 0 and 999.  Only the first N is mandatory.)
AC_DEFUN([SWIG_PROG],[
	AC_REQUIRE([AC_PROG_MAKE_SET])
	AC_CHECK_PROG(SWIG,swig,[`which swig`])
	if test -z "$SWIG" ; then
		AC_MSG_WARN([cannot find 'swig' program])
		SWIG=false
	elif test -n "$1" ; then
		AC_MSG_CHECKING([for SWIG version])
		swig_version=`$SWIG -version 2>&1 | \
			awk '/^SWIG Version [[0-9]+\.[0-9]+\.[0-9]]+.*$/ { split($[3],a,"[[^.0-9]]"); print a[[1]] }'`
		AC_MSG_RESULT([$swig_version])
		if test -n "$swig_version" ; then
			swig_version=`echo $swig_version | \
				awk '{ split($[1],a,"\."); print [a[1]*1000000+a[2]*1000+a[3]] }' 2>/dev/null`
			swig_required_version=`echo $1 | \
				awk '{ split($[1],a,"\."); print [a[1]*1000000+a[2]*1000+a[3]] }' 2>/dev/null`
			if test $swig_required_version -gt $swig_version ; then
				AC_MSG_WARN([SWIG version $1 required])
			fi
		else
			AC_MSG_WARN([cannot determine SWIG version])
		fi
	fi
])

# SWIG_ENABLE_CXX()
#
# Enable swig C++ support.  This effects all invocations of $(SWIG).
AC_DEFUN([SWIG_ENABLE_CXX],[
	AC_REQUIRE([SWIG_PROG])
	AC_REQUIRE([AC_PROG_CXX])
	if test "$SWIG" != "false" ; then
		SWIG="$SWIG -c++"
	fi
])

# SWIG_MULTI_MODULE_SUPPORT()
#
# Enable support for multiple modules.  This effects all invocations of $(SWIG).
# You have to link all generated modules against the appropriate SWIG library.
# If you want to build Python modules for example, use the SWIG_PYTHON() macro
# and link the modules against $(SWIG_PYTHON_LIB).  The $(SWIG_LDFLAGS) variable
# can be used to help the linker to find this library.
AC_DEFUN([SWIG_MULTI_MODULE_SUPPORT],[
	AC_REQUIRE([SWIG_PROG])
	if test "$SWIG" != "false" ; then
		SWIG="$SWIG -c"
        
		# Check for SWIG library path
		AC_MSG_CHECKING([for SWIG library path])
		swig_path=${SWIG%/bin*}/lib
		swig_path=`find $swig_path -type f -name libswig*.a -o -name libswig*.so -print`
		for i in $swig_path ; do
			swig_path=${i%/libswig*}
			break
		done
		AC_MSG_RESULT([$swig_path])
		if test -n "$swig_path" ; then
			AC_SUBST(SWIG_LDFLAGS,[-L$swig_path])
		else
			AC_MSG_WARN([cannot find SWIG library path])
		fi
	fi
])

# SWIG_PYTHON([use-shadow-classes])
#
# Checks for Python and provides the $(SWIG_PYTHON_CFLAGS), $(SWIG_PYTHON_LIB) and
# $(SWIG_PYTHON_OPT) output variables.  $(SWIG_PYTHON_OPT) contains all necessary swig
# options to generate code for Python.  Shadow classes are enabled unless the
# value of the optional first argument is exactly 'no'.  If you need multi module
# support use $(SWIG_PYTHON_LIB) (provided by the SWIG_MULTI_MODULE_SUPPORT() macro)
# to link against the appropriate library.  It contains the SWIG Python runtime library
# that is needed by the type check system for example.
AC_DEFUN([SWIG_PYTHON],[
	AC_REQUIRE([SWIG_PROG])
	AC_REQUIRE([PYTHON_DEVEL])
	if test "$SWIG" != "false" ; then
		AC_SUBST(SWIG_PYTHON_LIB,[`$SWIG -python -ldflags`])
		test ! "x$1" = "xno" && swig_shadow=" -shadow" || swig_shadow=""
		AC_SUBST(SWIG_PYTHON_OPT,[-python$swig_shadow])
	fi
	AC_SUBST(SWIG_PYTHON_CFLAGS,[$PYTHON_CFLAGS])
])
