AC_PREREQ([2.54])
AC_INIT([openbox], [3.999.0], [http://bugzilla.icculus.org])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([openbox/openbox.c])

dnl Making releases:
dnl   RR_MICRO_VERSION += 1;
dnl   RR_INTERFACE_AGE += 1;
dnl   R_BINARY_AGE += 1;
dnl if any functions have been added, set RR_INTERFACE_AGE to 0.
dnl if backwards compatibility has been broken,
dnl set RR_BINARY_AGE and RR_INTERFACE_AGE to 0.
dnl
dnl if MAJOR or MINOR version changes, be sure to change AC_INIT above to match
dnl
RR_MAJOR_VERSION=4
RR_MINOR_VERSION=0
RR_MICRO_VERSION=17
RR_INTERFACE_AGE=0
RR_BINARY_AGE=0
RR_VERSION=$RR_MAJOR_VERSION.$RR_MINOR_VERSION

OBT_MAJOR_VERSION=4
OBT_MINOR_VERSION=0
OBT_MICRO_VERSION=0
OBT_INTERFACE_AGE=0
OBT_BINARY_AGE=0
OBT_VERSION=$OBT_MAJOR_VERSION.$OBT_MINOR_VERSION

LOCO_MAJOR_VERSION=4
LOCO_MINOR_VERSION=0
LOCO_MICRO_VERSION=0
LOCO_INTERFACE_AGE=0
LOCO_BINARY_AGE=0
LOCO_VERSION=$OB_MAJOR_VERSION.$OB_MINOR_VERSION

AC_SUBST(RR_MAJOR_VERSION)
AC_SUBST(RR_MINOR_VERSION)
AC_SUBST(RR_MICRO_VERSION)
AC_SUBST(RR_INTERFACE_AGE)
AC_SUBST(RR_BINARY_AGE)
AC_SUBST(RR_VERSION)
AC_SUBST(OBT_MAJOR_VERSION)
AC_SUBST(OBT_MINOR_VERSION)
AC_SUBST(OBT_MICRO_VERSION)
AC_SUBST(OBT_INTERFACE_AGE)
AC_SUBST(OBT_BINARY_AGE)
AC_SUBST(OBT_VERSION)
AC_SUBST(LOCO_MAJOR_VERSION)
AC_SUBST(LOCO_MINOR_VERSION)
AC_SUBST(LOCO_MICRO_VERSION)
AC_SUBST(LOCO_INTERFACE_AGE)
AC_SUBST(LOCO_BINARY_AGE)
AC_SUBST(LOCO_VERSION)

dnl Libtool versioning
RR_RELEASE=$RR_MAJOR_VERSION.$RR_MINOR_VERSION
RR_CURRENT=`expr $RR_MICRO_VERSION - $RR_INTERFACE_AGE`
RR_REVISION=$RR_INTERFACE_AGE
RR_AGE=`expr $RR_BINARY_AGE - $RR_INTERFACE_AGE`
RR_CURRENT_MINUS_AGE=`expr $RR_CURRENT - $RR_AGE`

OBT_RELEASE=$OBT_MAJOR_VERSION.$OBT_MINOR_VERSION
OBT_CURRENT=`expr $OBT_MICRO_VERSION - $OBT_INTERFACE_AGE`
OBT_REVISION=$OBT_INTERFACE_AGE
OBT_AGE=`expr $OBT_BINARY_AGE - $OBT_INTERFACE_AGE`
OBT_CURRENT_MINUS_AGE=`expr $OBT_CURRENT - $OBT_AGE`

LOCO_RELEASE=$LOCO_MAJOR_VERSION.$LOCO_MINOR_VERSION
LOCO_CURRENT=`expr $LOCO_MICRO_VERSION - $LOCO_INTERFACE_AGE`
LOCO_REVISION=$LOCO_INTERFACE_AGE
LOCO_AGE=`expr $LOCO_BINARY_AGE - $LOCO_INTERFACE_AGE`
LOCO_CURRENT_MINUS_AGE=`expr $LOCO_CURRENT - $LOCO_AGE`

AC_SUBST(RR_RELEASE)
AC_SUBST(RR_CURRENT)
AC_SUBST(RR_REVISION)
AC_SUBST(RR_AGE)
AC_SUBST(RR_CURRENT_MINUS_AGE)
AC_SUBST(OBT_RELEASE)
AC_SUBST(OBT_CURRENT)
AC_SUBST(OBT_REVISION)
AC_SUBST(OBT_AGE)
AC_SUBST(OBT_CURRENT_MINUS_AGE)
AC_SUBST(LOCO_RELEASE)
AC_SUBST(LOCO_CURRENT)
AC_SUBST(LOCO_REVISION)
AC_SUBST(LOCO_AGE)
AC_SUBST(LOCO_CURRENT_MINUS_AGE)

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

AC_ARG_ENABLE(xcomposite,
  AC_HELP_STRING(
    [--disable-xcomposite],
    [disable use of the X Composite library. [[default=enabled]]]
  ),
  [enable_xcomposite=$enableval],
  [enable_xcomposite=yes]
)

if test "$enable_xcomposite" = yes; then
PKG_CHECK_MODULES(XRENDER, [xrender],
  [
    AC_DEFINE(USE_XRENDER, [1], [Use X Render library])
    AC_SUBST(XRENDER_CFLAGS)
    AC_SUBST(XRENDER_LIBS)
    PKG_CHECK_MODULES(XDAMAGE, [xdamage],
      [
        AC_DEFINE(USE_XDAMAGE, [1], [Use X Damage library])
        AC_SUBST(XDAMAGE_CFLAGS)
        AC_SUBST(XDAMAGE_LIBS)
        PKG_CHECK_MODULES(XCOMPOSITE, [xcomposite],
          [
            AC_DEFINE(USE_XCOMPOSITE, [1], [Use X Composite library])
            AC_SUBST(XCOMPOSITE_CFLAGS)
            AC_SUBST(XCOMPOSITE_LIBS)
            xcomposite_found=yes
          ],
          [
            xcomposite_found=no
          ]
        )
      ],
      [
        xcomposite_found=no
      ]
    )
  ],
  [
    xcomposite_found=no
  ]
)
else
  xcomposite_found=no
fi

AC_MSG_CHECKING(for GL_CFLAGS)
AC_ARG_WITH(gl-cflags, [  --with-gl-cflags=CFLAGS ],
                       [GL_CFLAGS="$withval"],
                       [GL_CFLAGS=""])

AC_MSG_RESULT($GL_CFLAGS)
AC_MSG_CHECKING(for GL_LIBS)
AC_ARG_WITH(gl-libs, [  --with-gl-libs=LIBS ],
                     [GL_LIBS="$withval"],
                     [GL_LIBS="-lGL"])
AC_MSG_RESULT($GL_LIBS)

AC_SUBST(GL_CFLAGS)
AC_SUBST(GL_LIBS)

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
  render/obrender-4.0.pc
  obt/obt-4.0.pc
  loco/loco-4.0.pc
  render/version.h
  obt/version.h
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
               X Composite Library... $xcomposite_found
               Session Management... $SM
               ])
AC_MSG_RESULT([configure complete, now type "make"])
