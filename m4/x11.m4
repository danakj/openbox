# X11_DEVEL()
#
# Check for the presence of the X Window System headers and libraries.
# Sets the CXXFLAGS and LIBS variables as appropriate.
AC_DEFUN([X11_DEVEL],
[
  AC_PATH_X
  AC_PATH_XTRA
  test "$no_x" = "yes" && \
    AC_MSG_ERROR([The X Window System could not be found.])
     
  CXXFLAGS="$CXXFLAGS $X_CFLAGS"
  LIBS="$LIBS $X_PRE_LIBS $X_LIBS $X_EXTRA_LIBS"

  # Check for required functions in -lX11
  AC_CHECK_LIB(
    [X11], [XOpenDisplay],
    ,
    AC_MSG_ERROR([Could not find XOpenDisplay in -lX11.])
  )
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
    XFT_MIN=$1
    XFT_MIN_MAJOR=${XFT_MIN%.*.*}
    XFT_MIN_MINOR=${XFT_MIN%.*}
    XFT_MIN_MINOR=${XFT_MIN_MINOR#*.}
    XFT_MIN_REVISION=${XFT_MIN#*.*.}
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
  OLDCXXFLAGS=$CXXFLAGS

  XFT_CFLAGS="`pkg-config --cflags xft`"
  XFT_LIBS="`pkg-config --libs xft`"

  # Set these for checking with the tests below. They'll be restored after
  LIBS="$LIBS $XFT_LIBS"
  CXXFLAGS="$XFT_CFLAGS $CXXFLAGS"

  AC_CHECK_LIB([Xft], [XftFontOpenName],
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

# Restore the old values. Use XFT_CFLAGS and XFT_LIBS in the Makefile.am's
  LIBS=$OLDLIBS
  CXXFLAGS=$OLDCXXFLAGS

  AC_SUBST([XFT_CFLAGS])
  AC_SUBST([XFT_LIBS])
])
