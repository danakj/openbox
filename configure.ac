AC_PREREQ([2.50])
AC_INIT([openbox], [2.90.0cvs], [http://bugzilla.icculus.org])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([kernel/openbox.c])

AC_PREFIX_DEFAULT([/usr/local])
test "$prefix" = "NONE" && prefix=$ac_default_prefix

AC_PATH_PROG([SED], [sed])
test "$SED" || AC_MSG_ERROR([sed not found])

# Determine build target
OB_DEBUG
# Pick compiler specific/build target flags, and set $CVS
OB_COMPILER_FLAGS
AC_C_CONST
AC_C_INLINE

if test "$CVS" = "yes"; then
    AC_PATH_PROG([FLEX], [flex])
    test "$FLEX" || AC_MSG_ERROR([flex not found])

    AC_PATH_PROG([BISON], [bison])
    test "BISON" || AC_MSG_ERROR([bison not found])
fi

AC_DISABLE_STATIC
AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL
AC_SUBST(LIBTOOL_DEPS)
LIBTOOL="$LIBTOOL --silent"

AC_PROG_INSTALL

ALL_LINGUAS=""
AM_GNU_GETTEXT_VERSION(0.11.5)
AM_GNU_GETTEXT([external])

AC_CHECK_HEADERS(ctype.h fcntl.h locale.h signal.h string.h stdio.h stdlib.h)
AC_CHECK_HEADERS(unistd.h sys/stat.h sys/select.h sys/time.h sys/wait.h)
# AC_HEADER_TIME
# AC_TYPE_SIGNAL

PKG_CHECK_MODULES([GLIB], [glib-2.0])
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)
	
PKG_CHECK_MODULES([GMODULE], [gmodule-2.0])
AC_SUBST(GMODULE_CFLAGS)
AC_SUBST(GMODULE_LIBS)

PKG_CHECK_MODULES(XFT, [xft])
AC_SUBST(XFT_CFLAGS)
AC_SUBST(XFT_LIBS)

PKG_CHECK_MODULES(XML, [libxml-2.0])
AC_SUBST(XML_CFLAGS)
AC_SUBST(XML_LIBS)

PKG_CHECK_MODULES(LIBSN, [libstartup-notification-1.0],
  [
    AC_DEFINE(USE_LIBSN)
    AC_SUBST(LIBSN_CFLAGS)
    AC_SUBST(LIBSN_LIBS)
  ],
  [
    no_such_luck=yes
  ]
)

PKG_CHECK_MODULES(GTK, [gtk+-2.0],
  [
    AC_SUBST(GTK_CFLAGS)
    AC_SUBST(GTK_LIBS)
    use_gtk="yes"

    PKG_CHECK_MODULES(GLADE, [libglade-2.0],
      [
        AC_SUBST(GLADE_CFLAGS)
        AC_SUBST(GLADE_LIBS)
        use_glade="yes"
      ],
      [
        use_glade="no"
        AC_MSG_WARN([disabling build of the configuration tool])
      ]
    )
  ],
  [
    use_gtk="no"
    AC_MSG_WARN([disabling build of the configuration tool])
  ]
)
AM_CONDITIONAL(OBCONF, [test "$use_gtk" = "yes" && test "$use_glade" = "yes"])

# Check for X11 extensions
X11_EXT_XKB
X11_EXT_XRANDR
X11_EXT_VIDMODE
X11_EXT_SHAPE
X11_EXT_XINERAMA

GL_OPTION

AC_CONFIG_FILES([Makefile
                 po/Makefile.in
                 themes/Makefile
		 data/Makefile
                 render/Makefile
		 parser/Makefile
                 kernel/Makefile
		 plugins/Makefile
		 plugins/resistance/Makefile
		 plugins/placement/Makefile
		 plugins/mouse/Makefile
		 plugins/keyboard/Makefile
		 plugins/menu/Makefile
		 tools/Makefile
		 tools/obconf/Makefile])
AC_OUTPUT

AC_MSG_RESULT
if test "$DEBUG" = "yes"; then
  AC_MSG_RESULT([Creating a DEBUG build.])
else
  AC_MSG_RESULT([Creating a RELEASE build.])
fi
AC_MSG_RESULT
AC_MSG_RESULT([configure complete, now type \"make\"])
