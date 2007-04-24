# X11_DEVEL()
#
# Check for the presence of the X Window System headers and libraries.
# Sets the CPPFLAGS and LIBS variables as appropriate.
AC_DEFUN([X11_DEVEL],
[
  AC_PATH_XTRA
  test "$no_x" = "yes" && \
    AC_MSG_ERROR([The X Window System could not be found.])

  # Store these
  OLDLIBS=$LIBS
  OLDCPPFLAGS=$CPPFLAGS
     
  CPPFLAGS="$CPPFLAGS $X_CFLAGS"
  X_LIBS="$X_PRE_LIBS $X_LIBS -lX11"
  LIBS="$LIBS $X_LIBS"

  # Check for required functions in -lX11
  AC_CHECK_LIB(
    [X11], [XOpenDisplay],
    ,
    AC_MSG_ERROR([Could not find XOpenDisplay in -lX11.])
  )

  # Restore the old values. Use X_CFLAGS and X_LIBS in
  # the Makefiles
  LIBS=$OLDLIBS
  CPPFLAGS=$OLDCPPFLAGS
])


AC_DEFUN([XFT_ERROR],
[
  if test "$XFT_MIN"; then
    AC_MSG_ERROR([$PACKAGE requires the Xft font library >= $XFT_MIN.
                  See http://www.fontconfig.org/
])
  else
    AC_MSG_ERROR([$PACKAGE requires the Xft font library.
                  See http://www.fontconfig.org/
])
  fi
])

# XFT_DEVEL([required-version])
#
# Check for the XFT development package.
# You can use the optional argument to check for a library of at least the
# given version.
# It provides the $(PYTHON_CFLAGS) $(PYTHON_LIBS) output variables.
AC_DEFUN([XFT_DEVEL],
[
  AC_REQUIRE([X11_DEVEL])
  
  if test "$1"; then
    XFT_MIN="$1"
    XFT_MIN_MAJOR=${XFT_MIN%.*.*}
    XFT_MIN_MINOR=${XFT_MIN%.*}
    XFT_MIN_MINOR=${XFT_MIN_MINOR#*.}
    XFT_MIN_REVISION=${XFT_MIN#*.*.}
    XFT_MIN="$XFT_MIN_MAJOR.$XFT_MIN_MINOR.$XFT_MIN_REVISION"
  else
    XFT_MIN=""
  fi

  if test -z "$XFT_MIN"; then
    AC_MSG_CHECKING([for Xft])
    if ! pkg-config xft; then
      AC_MSG_RESULT([no])
      XFT_ERROR
    fi
  else
    AC_MSG_CHECKING([for Xft version >= $XFT_MIN])
    if ! pkg-config --atleast-version $XFT_MIN xft; then
      AC_MSG_RESULT([no])
      XFT_ERROR
    fi
  fi
  AC_MSG_RESULT([yes])


  # Store these
  OLDLIBS=$LIBS
  OLDCPPFLAGS=$CPPFLAGS

  XFT_CFLAGS="`pkg-config --cflags xft`"
  XFT_LIBS="`pkg-config --libs xft`"

  # Set these for checking with the tests below. They'll be restored after
  LIBS="$LIBS $XFT_LIBS"
  CPPFLAGS="$XFT_CFLAGS $CPPFLAGS"

  AC_CHECK_LIB([Xft], [XftGetVersion], # this was not defined in < 2.0
    if test "$XFT_MIN"; then
      AC_MSG_CHECKING([for X11/Xft/Xft.h for Xft >= $XFT_MIN])
      AC_TRY_COMPILE(
      [
        #include <X11/Xlib.h>
        #include <X11/Xft/Xft.h>
      ],
      [
        #if !defined(XFT_MAJOR)
        # error Xft.h is too old
        #endif
        #if XFT_MAJOR < $XFT_MIN_MAJOR
        # error Xft.h is too old
        #endif
        #if XFT_MAJOR == $XFT_MIN_MAJOR
        # if XFT_MINOR < $XFT_MIN_MINOR
        #  error Xft.h is too old
        # endif
        #endif
        #if XFT_MAJOR == $XFT_MIN_MAJOR
        # if XFT_MAJOR == $XFT_MIN_MINOR
        #  if XFT_REVISION < $XFT_MIN_REVISION
        #   error Xft.h is too old
        #  endif
        # endif
        #endif
      
        int i = XFT_MAJOR;
        XftFont foo;
      ],
      [
        AC_MSG_RESULT([yes])
      ],
      [
        AC_MSG_RESULT([no])
        XFT_ERROR
      ])
    else
      AC_MSG_CHECKING([for X11/Xft/Xft.h])
      AC_TRY_COMPILE(
      [
        #include <X11/Xlib.h>
        #include <X11/Xft/Xft.h>
      ],
      [
        int i = XFT_MAJOR; /* make sure were using Xft 2, not 1 */
        XftFont foo;
      ],
      [
        AC_MSG_RESULT([yes])
      ],
      [
        AC_MSG_RESULT([no])
        XFT_ERROR
      ])
    fi

    AC_MSG_CHECKING([if we can compile with Xft])
    AC_TRY_LINK(
    [
      #include <X11/Xlib.h>
      #include <X11/Xft/Xft.h>
    ],
    [
      int i = XFT_MAJOR;
      XftFont foo
    ],
    [
      AC_MSG_RESULT([yes])
    ],
    [ 
      AC_MSG_RESULT([no])
      AC_MSG_ERROR([Unable to compile with the Xft font library.
])
    ])
  )

  # Restore the old values. Use XFT_CFLAGS and XFT_LIBS in the Makefiles
  LIBS=$OLDLIBS
  CPPFLAGS=$OLDCPPFLAGS

  AC_SUBST([XFT_CFLAGS])
  AC_SUBST([XFT_LIBS])
])


# X11_EXT_XKB()
#
# Check for the presence of the "Xkb" X Window System extension.
# Defines "XKB" and sets the $(XKB) variable to "yes" if the extension is
# present.
AC_DEFUN([X11_EXT_XKB],
[
  AC_REQUIRE([X11_DEVEL])

  # Store these
  OLDLIBS=$LIBS
  OLDCPPFLAGS=$CPPFLAGS
     
  CPPFLAGS="$CPPFLAGS $X_CFLAGS"
  LIBS="$LIBS $X_LIBS"

  AC_CHECK_LIB([X11], [XkbBell],
    AC_MSG_CHECKING([for X11/XKBlib.h])
    AC_TRY_LINK(
    [
      #include <X11/Xlib.h>
      #include <X11/Xutil.h>
      #include <X11/XKBlib.h>
    ],
    [
      Display *d;
      Window w;
      XkbBell(d, w, 0, 0);
    ],
    [
      AC_MSG_RESULT([yes])
      XKB="yes"
      AC_DEFINE([XKB], [1], [Found the XKB extension])

      XKB_CFLAGS=""
      XKB_LIBS=""
      AC_SUBST(XKB_CFLAGS)
      AC_SUBST(XKB_LIBS)
    ],
    [ 
      AC_MSG_RESULT([no])
      XKB="no"
    ])
  )

  LIBS=$OLDLIBS
  CPPFLAGS=$OLDCPPFLAGS

  AC_MSG_CHECKING([for the Xkb extension])
  if test "$XKB" = "yes"; then
    AC_MSG_RESULT([yes])
  else
    AC_MSG_RESULT([no])
  fi
])

# X11_EXT_XRANDR()
#
# Check for the presence of the "XRandR" X Window System extension.
# Defines "XRANDR" and sets the $(XRANDR) variable to "yes" if the extension is
# present.
AC_DEFUN([X11_EXT_XRANDR],
[
  AC_REQUIRE([X11_DEVEL])

  # Store these
  OLDLIBS=$LIBS
  OLDCPPFLAGS=$CPPFLAGS
     
  CPPFLAGS="$CPPFLAGS $X_CFLAGS"
  LIBS="$LIBS $X_LIBS -lXext -lXrender -lXrandr"

  AC_CHECK_LIB([Xrandr], [XRRSelectInput],
    AC_MSG_CHECKING([for X11/extensions/Xrandr.h])
    AC_TRY_LINK(
    [
      #include <X11/Xlib.h>
      #include <X11/extensions/Xrandr.h>
    ],
    [
      Display *d;
      Drawable r;
      int i;
      XRRQueryExtension(d, &i, &i);
      XRRGetScreenInfo(d, r);
    ],
    [
      AC_MSG_RESULT([yes])
      XRANDR="yes"
      AC_DEFINE([XRANDR], [1], [Found the XRandR extension])

      XRANDR_CFLAGS=""
      XRANDR_LIBS="-lXext -lXrender -lXrandr"
      AC_SUBST(XRANDR_CFLAGS)
      AC_SUBST(XRANDR_LIBS)
    ],
    [ 
      AC_MSG_RESULT([no])
      XRANDR="no"
    ])
  )

  LIBS=$OLDLIBS
  CPPFLAGS=$OLDCPPFLAGS

  AC_MSG_CHECKING([for the XRandR extension])
  if test "$XRANDR" = "yes"; then
    AC_MSG_RESULT([yes])
  else
    AC_MSG_RESULT([no])
  fi
])

# X11_EXT_SHAPE()
#
# Check for the presence of the "Shape" X Window System extension.
# Defines "SHAPE", sets the $(SHAPE) variable to "yes", and sets the $(LIBS)
# appropriately if the extension is present.
AC_DEFUN([X11_EXT_SHAPE],
[
  AC_REQUIRE([X11_DEVEL])

  # Store these
  OLDLIBS=$LIBS
  OLDCPPFLAGS=$CPPFLAGS
     
  CPPFLAGS="$CPPFLAGS $X_CFLAGS"
  LIBS="$LIBS $X_LIBS"

  AC_CHECK_LIB([Xext], [XShapeCombineShape],
    AC_MSG_CHECKING([for X11/extensions/shape.h])
    AC_TRY_LINK(
    [
      #include <X11/Xlib.h>
      #include <X11/Xutil.h>
      #include <X11/extensions/shape.h>
    ],
    [
      long foo = ShapeSet;
    ],
    [
      AC_MSG_RESULT([yes])
      SHAPE="yes"
      AC_DEFINE([SHAPE], [1], [Found the XShape extension])

      XSHAPE_CFLAGS=""
      XSHAPE_LIBS="-lXext"
      AC_SUBST(XSHAPE_CFLAGS)
      AC_SUBST(XSHAPE_LIBS)
    ],
    [ 
      AC_MSG_RESULT([no])
      SHAPE="no"
    ])
  )

  LIBS=$OLDLIBS
  CPPFLAGS=$OLDCPPFLAGS
 
  AC_MSG_CHECKING([for the Shape extension])
  if test "$SHAPE" = "yes"; then
    AC_MSG_RESULT([yes])
  else
    AC_MSG_RESULT([no])
  fi
])


# X11_EXT_XINERAMA()
#
# Check for the presence of the "Xinerama" X Window System extension.
# Defines "XINERAMA", sets the $(XINERAMA) variable to "yes", and sets the
# $(LIBS) appropriately if the extension is present.
AC_DEFUN([X11_EXT_XINERAMA],
[
  AC_REQUIRE([X11_DEVEL])

  # Store these
  OLDLIBS=$LIBS
  OLDCPPFLAGS=$CPPFLAGS
     
  CPPFLAGS="$CPPFLAGS $X_CFLAGS"
  LIBS="$LIBS $X_LIBS -lXext"

  AC_CHECK_LIB([Xinerama], [XineramaQueryExtension],
  [
    AC_MSG_CHECKING([for X11/extensions/Xinerama.h])
    AC_TRY_LINK(
    [
      #include <X11/Xlib.h>
      #include <X11/extensions/Xinerama.h>
    ],
    [
      XineramaScreenInfo foo;
    ],
    [
      AC_MSG_RESULT([yes])
      XINERAMA="yes"
      AC_DEFINE([XINERAMA], [1], [Enable support of the Xinerama extension])
      XINERAMA_LIBS="-lXext -lXinerama"
      AC_SUBST(XINERAMA_LIBS)
    ],
    [
      AC_MSG_RESULT([no])
      XINERAMA="no"
    ])
  ])

  LIBS=$OLDLIBS
  CPPFLAGS=$OLDCPPFLAGS

  AC_MSG_CHECKING([for the Xinerama extension])
  if test "$XINERAMA" = "yes"; then
    AC_MSG_RESULT([yes])
  else
    AC_MSG_RESULT([no])
  fi
])

# X11_EXT_SYNC()
#
# Check for the presence of the "Sync" X Window System extension.
# Defines "SYNC", sets the $(SYNC) variable to "yes", and sets the $(LIBS)
# appropriately if the extension is present.
AC_DEFUN([X11_EXT_SYNC],
[
  AC_REQUIRE([X11_DEVEL])

  # Store these
  OLDLIBS=$LIBS
  OLDCPPFLAGS=$CPPFLAGS
     
  CPPFLAGS="$CPPFLAGS $X_CFLAGS"
  LIBS="$LIBS $X_LIBS"

  AC_CHECK_LIB([Xext], [XSyncInitialize],
    AC_MSG_CHECKING([for X11/extensions/sync.h])
    AC_TRY_LINK(
    [
      #include <X11/Xlib.h>
      #include <X11/Xutil.h>
      #include <X11/extensions/sync.h>
    ],
    [
      XSyncValueType foo;
    ],
    [
      AC_MSG_RESULT([yes])
      SYNC="yes"
      AC_DEFINE([SYNC], [1], [Found the XSync extension])

      XSYNC_CFLAGS=""
      XSYNC_LIBS="-lXext"
      AC_SUBST(XSYNC_CFLAGS)
      AC_SUBST(XSYNC_LIBS)
    ],
    [ 
      AC_MSG_RESULT([no])
      SYNC="no"
    ])
  )

  LIBS=$OLDLIBS
  CPPFLAGS=$OLDCPPFLAGS
 
  AC_MSG_CHECKING([for the Sync extension])
  if test "$SYNC" = "yes"; then
    AC_MSG_RESULT([yes])
  else
    AC_MSG_RESULT([no])
  fi
])

# X11_SM()
#
# Check for the presence of SMlib for session management.
# Defines "USE_SM" if SMlib is present.
AC_DEFUN([X11_SM],
[
  AC_REQUIRE([X11_DEVEL])

  AC_ARG_ENABLE([session-management],
  AC_HELP_STRING(
  [--disable-session-management], [build without support for session managers [[default=enabled]]]),
  [SM=$enableval], [SM="yes"])
  
  if test "$SM" = "yes"; then
    # Store these
    OLDLIBS=$LIBS
    OLDCPPFLAGS=$CPPFLAGS
     
    CPPFLAGS="$CPPFLAGS $X_CFLAGS"
    LIBS="$LIBS $X_LIBS"

    SM="no"

    AC_CHECK_LIB([SM], [SmcSaveYourselfDone], [
      AC_CHECK_HEADERS([X11/SM/SMlib.h], [
        SM_CFLAGS="$X_CFLAGS"
        SM_LIBS="-lSM -lICE"
        AC_DEFINE(USE_SM, 1, [Use session management])
        AC_SUBST(SM_CFLAGS)
        AC_SUBST(SM_LIBS)
        SM="yes"
      ])
    ])
  fi

  LIBS=$OLDLIBS
  CPPFLAGS=$OLDCPPFLAGS

  AC_MSG_CHECKING([for session management support])
  if test "$SM" = "yes"; then
    AC_MSG_RESULT([yes])
  else
    AC_MSG_RESULT([no])
  fi
])
