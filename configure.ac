AC_PREREQ([2.54])
AC_INIT([openbox], [3.0-alpha4], [http://bugzilla.icculus.org])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([kernel/openbox.c])

AC_PREFIX_DEFAULT([/usr/local])
test "$prefix" = "NONE" && prefix=$ac_default_prefix

AC_PATH_PROG([SED], [sed])
test "$SED" || AC_MSG_ERROR([sed not found])

# Determine build target
OB_DEBUG
# Pick compiler specific/build target flags, and set $CVS
AM_PROG_CC_C_O
OB_COMPILER_FLAGS
AC_C_CONST
AC_C_INLINE

AC_DISABLE_STATIC
AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL
AC_SUBST(LIBTOOL_DEPS)
LIBTOOL="$LIBTOOL --silent"

AC_PROG_INSTALL

ALL_LINGUAS=""
AM_GNU_GETTEXT_VERSION(0.12.1)
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
    AC_DEFINE(USE_LIBSN, [1], [Use startup-notification])
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

    PKG_CHECK_MODULES(GDK_PIXBUF, [gdk-pixbuf-2.0],
      [
        AC_SUBST(GDK_PIXBUF_CFLAGS)
        AC_SUBST(GDK_PIXBUF_LIBS)
        use_gdk_pixbuf="yes"
      ],
      [
        use_gdk_pixbuf="no"
        AC_MSG_WARN([disabling build of the configuration tool])
      ]
    )
  ],
  [
    use_gtk="no"
    AC_MSG_WARN([disabling build of the configuration tool])
  ]
)
AM_CONDITIONAL(OBCONF,
               [test "$use_gtk" = "yes" && test "$use_gdk_pixbuf" = "yes"])

# Check for session management
X11_SM

#EFENCE_LIBS=-lefence
EFENCE_LIBS=""
AC_SUBST(EFENCE_LIBS)

# Check for X11 extensions
X11_EXT_XKB
X11_EXT_XRANDR
X11_EXT_VIDMODE
X11_EXT_SHAPE
X11_EXT_XINERAMA

AC_CONFIG_FILES([
  Makefile
  m4/Makefile
  po/Makefile.in
])
AC_OUTPUT

AC_MSG_RESULT
AC_MSG_RESULT([configure complete, now type \"make\"])
