AC_INIT([src/main.cc])
AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE([openbox], [2.90.0cvs])

AC_PREREQ([2.50])

AC_PATH_PROG([regex_cmd], [sed])
test "$regex_cmd" || AC_MSG_ERROR([sed not found])

AC_PROG_CC
AC_PROG_CXX
AC_PROG_LIBTOOL
LIBTOOL="$LIBTOOL --silent"
AC_PROG_INSTALL

ALL_LINGUAS=""
AM_GNU_GETTEXT

dnl AC_LANG(C++)
	
dnl Check what compiler we are using
AC_MSG_CHECKING([for GCC])
if test "$GCC" = "yes"; then
  AC_MSG_RESULT([yes])
  CFLAGS="$CFLAGS -Wall -W -pedantic"
else
  AC_MSG_RESULT([no, trying other compilers])
  AC_MSG_CHECKING(for MIPSpro)
  mips_pro_ver=`$CXX -version 2>&1 | grep -i mipspro | cut -f4 -d ' '`
  if test -z "$mips_pro_ver"; then
    AC_MSG_RESULT([no])
  else
    AC_MSG_RESULT([yes, version $mips_pro_ver.])
    AC_MSG_CHECKING(for -LANG:std in CFLAGS)
    lang_std_not_set=`echo $CFLAGS | grep "\-LANG:std"`
    if test "x$lang_std_not_set" = "x"; then
      AC_MSG_RESULT([not set, setting.])
      CFLAGS="${CFLAGS} -LANG:std"
    else
      AC_MSG_RESULT([already set.])
    fi
  fi
fi

dnl Determine if maintainer portions of the Makefiles should be included.
AM_MAINTAINER_MODE

dnl Check for system header files
AC_CHECK_HEADERS(ctype.h dirent.h fcntl.h libgen.h locale.h nl_types.h process.h signal.h stdarg.h stdio.h stdlib.h string.h time.h unistd.h sys/param.h sys/select.h sys/signal.h sys/stat.h sys/time.h sys/types.h sys/wait.h)
AC_HEADER_TIME

dnl Determine the return type of signal handlers
AC_TYPE_SIGNAL

dnl Check whether to include debugging code
DEBUG=""
AC_MSG_CHECKING([whether to include verbose debugging code])
AC_ARG_ENABLE([debug],
  [  --enable-debug          include verbose debugging code @<:@default=no@:>@],
  if test "$enableval" = "yes"; then
    AC_MSG_RESULT([yes])
    if test "$GCC" = "yes"; then
       DEBUG="-DDEBUG -fno-inline -g"
    else
       DEBUG="-DDEBUG"
    fi
  else
    AC_MSG_RESULT([no])
    DEBUG="-DNDEBUG"
  fi,
  AC_MSG_RESULT([no])
  DEBUG="-DNDEBUG"
)
CFLAGS="$CFLAGS $DEBUG"

dnl Check for Python
AC_CHECK_HEADER([python2.2/Python.h],
  PYTHON_CFLAGS="-Ipython2.2",
  AC_MSG_ERROR([Openbox requires the use of Python 2.2. This is its secret special formula for extreme sexiness.
See http://www.python.org
]))
AC_CHECK_LIB([python2.2], [Py_Initialize],
  PYTHON_LDFLAGS="-lpython2.2 -Xlinker -export-dynamic",
  AC_MSG_ERROR([Openbox requires the use of Python 2.2. This is its secret special formula for extreme sexiness.
See http://www.python.org
]))
AC_SUBST([PYTHON_CFLAGS])
AC_SUBST([PYTHON_LDFLAGS])

dnl Check for X headers and libraries
AC_PATH_X
AC_PATH_XTRA
test "$no_x" = "yes" && AC_MSG_ERROR([No Xlibs found.])
test -z "$x_includes" && x_includes="/usr/include"
test -z "$x_libraries" && x_libraries="/usr/lib"
     
CFLAGS="$CFLAGS $X_CFLAGS"
LIBS="$LIBS $X_LIBS $X_EXTRA_LIBS"
LDFLAGS="$LDFLAGS $X_PRE_LIBS $LIBS $X_EXTRA_LIBS"

dnl Check for required functions in -lX11
AC_CHECK_LIB([X11], [XOpenDisplay],
  ,
  AC_MSG_ERROR([Could not find XOpenDisplay in -lX11.])
)


dnl Check for Xft >= 2
XFT_MIN_MAJOR=2
XFT_MIN_MINOR=0
XFT_MIN_REVISION=0
XFT_MIN=$XFT_MIN_MAJOR.$XFT_MIN_MINOR.$XFT_MIN_REVISION
AC_MSG_CHECKING([for Xft version >= $XFT_MIN])
if ! pkg-config --atleast-version $XFT_MIN xft; then
  AC_MSG_RESULT([no])
  AC_MSG_ERROR([Openbox requires the Xft version >= $XFT_MIN font library.
See http://www.fontconfig.org/
])
fi

AC_MSG_RESULT([yes])

dnl Store these
OLDLIBS=$LIBS
OLDCFLAGS=$CFLAGS

XFT_CFLAGS="`pkg-config --cflags xft`"
XFT_LIBS="`pkg-config --libs xft`"

dnl Set these for checking with the tests below. They'll be restored after
LIBS="$LIBS $XFT_LIBS"
CFLAGS="$XFT_CFLAGS $CFLAGS"

AC_CHECK_LIB([Xft], [XftFontOpenName],
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
      AC_MSG_ERROR([Openbox requires the Xft version >= $XFT_MIN font library.
See http://www.fontconfig.org/
])
    ]
  )

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
      AC_MSG_ERROR([Unable to compile with the Xft library.
])
    ]
  )
)
dnl Restore the old values. Use XFT_CFLAGS and XFT_LIBS in the Makefile.am's
LIBS=$OLDLIBS
CFLAGS=$OLDCFLAGS

AC_SUBST([XFT_CFLAGS])
AC_SUBST([XFT_LIBS])

dnl Check for XShape extension support
AC_CHECK_LIB([Xext], [XShapeCombineShape],
  AC_MSG_CHECKING([for X11/extensions/shape.h])
  AC_TRY_LINK(
    [
      #include <X11/Xlib.h>
      #include <X11/Xutil.h>
      #include <X11/extensions/shape.h>
    ],
    [
      long foo = ShapeSet
    ],
    [
      AC_MSG_RESULT([yes])
      SHAPE="yes"
      AC_DEFINE([SHAPE], [1], [Found the XShape extension])
      LIBS="$LIBS -lXext"
    ],
    [ 
      AC_MSG_RESULT([no])
      SHAPE="no"
    ]
  )
)
AC_MSG_CHECKING([for the XShape extension])
if test "$SHAPE" = "yes"; then
  AC_MSG_RESULT([yes])
else
  AC_MSG_RESULT([no])
fi

dnl Check for Xinerama extension support
AC_MSG_CHECKING([whether to build support for the Xinerama extension])
XINERAMA="no"
AC_ARG_ENABLE([xinerama],
[  --enable-xinerama       enable support of the Xinerama extension @<:@default=no@:>@],
  if test "$enableval" = "yes"; then
	AC_MSG_RESULT([yes])

	AC_CHECK_LIB([Xinerama], [XineramaQueryExtension],
	  [
	    AC_MSG_CHECKING([for X11/extensions/Xinerama.h])
	    AC_TRY_LINK(
	      [
                #include <X11/Xlib.h>
                #include <X11/extensions/Xinerama.h>
	      ],
	      [
                XineramaScreenInfo foo
	      ],
	      [
	        AC_MSG_RESULT([yes])
	        XINERAMA="yes"
	        AC_DEFINE([XINERAMA], [1],
	                  [Enable support of the Xinerama extension])
	        LIBS="$LIBS -lXinerama"
	      ],
	      [
	        AC_MSG_RESULT([no])
	      ]
	    )
	  ]
	)

    AC_MSG_CHECKING([for the Xinerama extension])
    if test "$XINRERAMA" = "yes"; then
      AC_MSG_RESULT([yes])
    else
      AC_MSG_RESULT([no])
    fi
  else
	AC_MSG_RESULT([no])
  fi,
  AC_MSG_RESULT([no])
)



AC_CONFIG_FILES([Makefile
		m4/Makefile 
		po/Makefile.in
		intl/Makefile
		otk/Makefile
		src/Makefile
		util/Makefile
		util/epist/Makefile
		doc/Makefile
		doc/doxygen/Makefile
		data/Makefile
		data/styles/Makefile
		version.h
		])
AC_OUTPUT

AC_MSG_RESULT
AC_MSG_RESULT([$PACKAGE version $VERSION configured successfully.])

AC_MSG_RESULT
AC_MSG_RESULT([Using '$prefix' for installation.])
AC_MSG_RESULT([Using '$CXX' for C++ compiler.])
AC_MSG_RESULT([Building with '$CFLAGS' for C++ compiler flags.])
AC_MSG_RESULT([Building with '$LIBS' for linker flags.])
AC_MSG_RESULT
AC_MSG_RESULT([configure complete, now type \"make\"])
AC_MSG_RESULT
