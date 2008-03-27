AC_PREREQ([2.54])
AC_INIT([openbox], [3.4.7-pre3], [http://bugzilla.icculus.org])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([openbox/openbox.c])

dnl Making releases:
dnl   OB_MICRO_VERSION += 1;
dnl   OB_INTERFACE_AGE += 1;
dnl   OB_BINARY_AGE += 1;
dnl if any functions have been added, set OB_INTERFACE_AGE to 0.
dnl if backwards compatibility has been broken,
dnl set OB_BINARY_AGE and OB_INTERFACE_AGE to 0.
dnl
dnl if MAJOR or MINOR version changes, be sure to change AC_INIT above to match
dnl
OB_MAJOR_VERSION=3
OB_MINOR_VERSION=4
OB_MICRO_VERSION=21
OB_INTERFACE_AGE=0
OB_BINARY_AGE=0
OB_VERSION=$OB_MAJOR_VERSION.$OB_MINOR_VERSION

AC_SUBST(OB_MAJOR_VERSION)
AC_SUBST(OB_MINOR_VERSION)
AC_SUBST(OB_MICRO_VERSION)
AC_SUBST(OB_INTERFACE_AGE)
AC_SUBST(OB_BINARY_AGE)
AC_SUBST(OB_VERSION)

dnl Libtool versioning
LT_RELEASE=$OB_MAJOR_VERSION.$OB_MINOR_VERSION
LT_CURRENT=`expr $OB_MICRO_VERSION - $OB_INTERFACE_AGE`
LT_REVISION=$OB_INTERFACE_AGE
LT_AGE=`expr $OB_BINARY_AGE - $OB_INTERFACE_AGE`
LT_CURRENT_MINUS_AGE=`expr $LT_CURRENT - $LT_AGE`

AC_SUBST(LT_RELEASE)
AC_SUBST(LT_CURRENT)
AC_SUBST(LT_REVISION)
AC_SUBST(LT_AGE)
AC_SUBST(LT_CURRENT_MINUS_AGE)

AC_PREFIX_DEFAULT([/usr/local])
test "$prefix" = "NONE" && prefix=$ac_default_prefix

dnl Determine build target
OB_DEBUG
dnl Pick compiler specific/build target flags, and set $CVS
AM_PROG_CC_C_O
OB_COMPILER_FLAGS
AC_C_CONST
AC_C_INLINE

AC_PROG_LIBTOOL
AC_SUBST(LIBTOOL_DEPS)
LIBTOOL="$LIBTOOL --silent"

AC_PROG_INSTALL

AM_GNU_GETTEXT_VERSION(0.15)
AM_GNU_GETTEXT([external])

AC_CHECK_HEADERS(ctype.h fcntl.h locale.h signal.h string.h stdio.h stdlib.h)
AC_CHECK_HEADERS(unistd.h sys/stat.h sys/select.h sys/time.h sys/wait.h)
# AC_HEADER_TIME
# AC_TYPE_SIGNAL

AC_PATH_PROG([SED], [sed], [no])
if test "$SED" = "no"; then
  AC_MSG_ERROR([The program "sed" is not available. This program is required to build Openbox.])
fi

PKG_CHECK_MODULES([GLIB], [glib-2.0 >= 2.6.0])
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

PKG_CHECK_MODULES(PANGO, [pango >= 1.8.0 pangoxft >= 1.8.0])
AC_SUBST(PANGO_CFLAGS)
AC_SUBST(PANGO_LIBS)

PKG_CHECK_MODULES(XFT, [xft])
AC_SUBST(XFT_CFLAGS)
AC_SUBST(XFT_LIBS)

PKG_CHECK_MODULES(XML, [libxml-2.0 >= 2.6.0])
AC_SUBST(XML_CFLAGS)
AC_SUBST(XML_LIBS)

AC_ARG_ENABLE(startup-notification,
  AC_HELP_STRING(
    [--disable-startup-notification],
    [disable the startup notification library. [[default=enabled]]]
  ),
  [enable_sn=$enableval],
  [enable_sn=yes]
)

if test "$enable_sn" = yes; then
PKG_CHECK_MODULES(LIBSN, [libstartup-notification-1.0 >= 0.8],
  [
    AC_DEFINE(USE_LIBSN, [1], [Use startup-notification])
    AC_SUBST(LIBSN_CFLAGS)
    AC_SUBST(LIBSN_LIBS)
    sn_found=yes
  ],
  [
    sn_found=no
  ]
)
else
  sn_found=no
fi

AC_ARG_ENABLE(xcursor,
  AC_HELP_STRING(
    [--disable-xcursor],
    [disable use of the X Cursor library. [[default=enabled]]]
  ),
  [enable_xcursor=$enableval],
  [enable_xcursor=yes]
)

if test "$enable_xcursor" = yes; then
PKG_CHECK_MODULES(XCURSOR, [xcursor],
  [
    AC_DEFINE(USE_XCURSOR, [1], [Use X Cursor library])
    AC_SUBST(XCURSOR_CFLAGS)
    AC_SUBST(XCURSOR_LIBS)
    xcursor_found=yes
  ],
  [
    xcursor_found=no
  ]
)
else
  xcursor_found=no
fi

dnl Check for session management
X11_SM

#EFENCE_LIBS=-lefence
EFENCE_LIBS=""
AC_SUBST(EFENCE_LIBS)

dnl Check for X11 extensions
X11_EXT_XKB
X11_EXT_XRANDR
X11_EXT_SHAPE
X11_EXT_XINERAMA
X11_EXT_SYNC

AC_CONFIG_FILES([
  Makefile
  m4/Makefile
  po/Makefile.in
  render/obrender-3.0.pc
  parser/obparser-3.0.pc
  version.h
])
AC_CONFIG_COMMANDS([doc],
                   [test -d doc || mkdir doc])
AC_CONFIG_COMMANDS([data],
                   [test -d data || mkdir data])
AC_CONFIG_COMMANDS([data/xsession],
                   [test -d data/xsession || mkdir data/xsession])
AC_OUTPUT

AC_MSG_RESULT
AC_MSG_RESULT([Compiling with these options:
               Startup Notification... $sn_found
               X Cursor Library... $xcursor_found
               Session Management... $SM
               ])
AC_MSG_RESULT([configure complete, now type "make"])
