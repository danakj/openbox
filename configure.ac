AC_PREREQ([2.50])
AC_INIT([kernel/openbox.c])
# AC_CONFIG_HEADERS(config.h)
AC_LANG([C])
AC_ENABLE_STATIC([no])
AC_ENABLE_SHARED([yes])

AC_PREFIX_DEFAULT([/usr/local])
if test "$prefix" = "NONE"; then
  prefix=$ac_default_prefix
  ac_configure_args="$ac_configure_args --prefix $prefix"
fi

AM_INIT_AUTOMAKE([openbox], [2.90.0cvs])

AC_CHECK_PROG([SED], [sed])
test "$SED" || AC_MSG_ERROR([sed not found])

AM_MAINTAINER_MODE
# Determine build target
OB_DEBUG
# Pick compiler specific/build target flags, and set $CVS
OB_COMPILER_FLAGS

PKG_CHECK_MODULES(GLIB, glib-2.0)
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)
	
PKG_CHECK_MODULES(GMODULE, gmodule-2.0)
AC_SUBST(GMODULE_CFLAGS)
AC_SUBST(GMODULE_LIBS)
	
AC_PROG_LIBTOOL
LIBTOOL="$LIBTOOL --silent"
AC_PROG_INSTALL

ALL_LINGUAS=""
AM_GNU_GETTEXT_VERSION(0.11.5)
AM_GNU_GETTEXT([external])

AC_CHECK_HEADERS(ctype.h fcntl.h locale.h signal.h string.h stdlib.h)
AC_CHECK_HEADERS(unistd.h sys/select.h sys/time.h sys/wait.h)
# AC_HEADER_TIME
# AC_TYPE_SIGNAL

PKG_CHECK_MODULES(XFT, xft)
AC_SUBST(XFT_CFLAGS)
AC_SUBST(XFT_LIBS)

# Check for X11 extensions
X11_EXT_XKB
X11_EXT_SHAPE
X11_EXT_XINERAMA

AC_CONFIG_FILES([Makefile po/Makefile.in
		kernel/Makefile
		render/Makefile
		engines/Makefile
		engines/openbox/Makefile
		plugins/Makefile
		plugins/keyboard/Makefile
		plugins/mouse/Makefile
		plugins/placement/Makefile
		doc/Makefile
		doc/doxygen/Makefile
		data/Makefile
		data/buttons/Makefile
		themes/Makefile
		themes/openbox/Makefile])
AC_OUTPUT

AC_MSG_RESULT
AC_MSG_RESULT([$PACKAGE version $VERSION configured successfully.])

AC_MSG_RESULT
if test "$DEBUG" = "yes"; then
  AC_MSG_RESULT([Creating a DEBUG build.])
else
  AC_MSG_RESULT([Creating a RELEASE build.])
fi
AC_MSG_RESULT
AC_MSG_RESULT([configure complete, now type \"make\"])
