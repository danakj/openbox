AC_PREREQ([2.50])
AC_INIT([src/main.cc])
AC_CONFIG_HEADERS(config.h)
AC_LANG([C++])
AC_ENABLE_STATIC([no])
AC_ENABLE_SHARED([yes])

AC_PREFIX_DEFAULT([/usr/local])
if test "$prefix" = "NONE"; then
  prefix=$ac_default_prefix
  ac_configure_args="$ac_configure_args --prefix $prefix"
fi

AM_INIT_AUTOMAKE([openbox], [2.90.0cvs])

AC_PATH_PROG([regex_cmd], [sed])
test "$regex_cmd" || AC_MSG_ERROR([sed not found])

AM_MAINTAINER_MODE
# Determine build target
OB_DEBUG
# Pick compiler specific/build target flags
OB_COMPILER_FLAGS
AC_C_BIGENDIAN

AC_PROG_LIBTOOL
LIBTOOL="$LIBTOOL --silent"
AC_PROG_INSTALL

ALL_LINGUAS=""
AM_GNU_GETTEXT_VERSION(0.11.5)
AM_GNU_GETTEXT([external])

PYTHON_DEVEL
		    
AC_CHECK_HEADERS(ctype.h dirent.h fcntl.h libgen.h locale.h nl_types.h process.h signal.h stdarg.h stdint.h stdio.h stdlib.h string.h time.h unistd.h sys/param.h sys/select.h sys/signal.h sys/stat.h sys/time.h sys/types.h sys/wait.h)
AC_HEADER_TIME
# AC_TYPE_SIGNAL

# Check for Xft2
XFT_DEVEL(2.0.0)

# Check for X11 extensions
X11_EXT_XKB
X11_EXT_SHAPE
X11_EXT_XINERAMA

AC_CONFIG_FILES([Makefile po/Makefile.in
		otk/Makefile
		src/Makefile
		scripts/Makefile
		doc/Makefile
		doc/doxygen/Makefile
		data/Makefile
		data/buttons/Makefile
		data/styles/Makefile
		])
AC_OUTPUT

AC_MSG_RESULT
AC_MSG_RESULT([$PACKAGE version $VERSION configured successfully.])

AC_MSG_RESULT
if test "$DEBUG" = "yes"; then
  AC_MSG_RESULT([Creating a DEBUG build.])
else
  AC_MSG_RESULT([Creating a RELEASE build.])
fi
AC_MSG_RESULT([Using '$prefix' for installation.])
AC_MSG_RESULT([Using '$CXX' for C++ compiler.])
AC_MSG_RESULT([Building with '$CXXFLAGS' for C++ compiler flags.])
AC_MSG_RESULT([Building with '$LIBS' for linker flags.])
AC_MSG_RESULT
AC_MSG_RESULT([configure complete, now type \"make\"])
