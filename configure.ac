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
AM_GNU_GETTEXT_VERSION(0.11.5)
AM_GNU_GETTEXT([external])

AC_LANG(C++)
	
# Check what compiler we are using
AC_MSG_CHECKING([for GCC])
if test "$GCC" = "yes"; then
  AC_MSG_RESULT([yes])
  CXXFLAGS="$CXXFLAGS -Wall -W"
# -pedantic"
else
  AC_MSG_RESULT([no, trying other compilers])
  AC_MSG_CHECKING(for MIPSpro)
  mips_pro_ver=`$CXX -version 2>&1 | grep -i mipspro | cut -f4 -d ' '`
  if test -z "$mips_pro_ver"; then
    AC_MSG_RESULT([no])
  else
    AC_MSG_RESULT([yes, version $mips_pro_ver.])
    AC_MSG_CHECKING(for -LANG:std in CXXFLAGS)
    lang_std_not_set=`echo $CXXFLAGS | grep "\-LANG:std"`
    if test "x$lang_std_not_set" = "x"; then
      AC_MSG_RESULT([not set, setting.])
      CXXFLAGS="${CXXFLAGS} -LANG:std"
    else
      AC_MSG_RESULT([already set.])
    fi
  fi
fi

# Determine if maintainer portions of the Makefiles should be included.
AM_MAINTAINER_MODE

# Check for system header files
AC_CHECK_HEADERS(ctype.h dirent.h fcntl.h libgen.h locale.h nl_types.h process.h signal.h stdarg.h stdio.h stdlib.h string.h time.h unistd.h sys/param.h sys/select.h sys/signal.h sys/stat.h sys/time.h sys/types.h sys/wait.h)
AC_HEADER_TIME

# Determine the return type of signal handlers
# AC_TYPE_SIGNAL

# Check whether to include debugging code
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
CXXFLAGS="$CXXFLAGS $DEBUG"

# Check for Python
AC_ARG_WITH([python],
	[  --with-python=path      Set location of Python executable],
	[PYTHON="$withval"],
	[PYTHON=])

# Find the python bin
if ! test "$PYTHON"; then
  AC_CHECK_PROGS(PYTHON, $prefix/bin/python python python2.4 python2.3 python2.2 python2.1 python2.0 python1.6 python1.5 python1.4)
fi
if ! test "$PYTHON"; then
   AC_MSG_ERROR([Openbox requires the use of Python. This is its secret special formula for extreme sexiness.
See http://www.python.org
])
fi

# Find our python paths
AC_MSG_CHECKING([for Python version])
# Need to do this hack since autoconf replaces __file__ with the name of the configure file
filehack="file__"
PYVERSION=`($PYTHON -c "import string,operator; print operator.getitem(string.split(string.__$filehack,'/'),-2)")`
AC_MSG_RESULT([$PYVERSION])

AC_MSG_CHECKING([for Python prefix])
PYPREFIX=`($PYTHON -c "import sys; print sys.prefix") 2>/dev/null`
AC_MSG_RESULT([$PYPREFIX])
AC_MSG_CHECKING([for Python exec-prefix])
PYEPREFIX=`($PYTHON -c "import sys; print sys.exec_prefix") 2>/dev/null`
AC_MSG_RESULT([$PYEPREFIX])

# Find the include directory
AC_MSG_CHECKING([for Python header files])
if test -r "$PYPREFIX/include/$PYVERSION/Python.h"; then
  PYTHON_CFLAGS="-I$PYPREFIX/include/$PYVERSION"
fi
if test -z "$PYTHON_CFLAGS"; then
  if test -r "$PYPREFIX/include/Py/Python.h"; then
    PYTHON_CFLAGS="-I$PYPREFIX/include/Py"
  fi
fi
if ! test "$PYTHON_CFLAGS"; then
  AC_MSG_ERROR([Not found])
else
  AC_MSG_RESULT([$PYTHON_CFLAGS])
fi

# Set the library directory blindly.   This probably won't work with older versions
AC_MSG_CHECKING([for Python library])
dirs="$PYVERSION/config $PYVERSION/lib python/lib"
for i in $dirs; do
  if test -d "$PYEPREFIX/lib/$i"; then
    PYLIB="$PYEPREFIX/lib/$i"
    break
  fi
done
if ! test "$PYLIB"; then
  AC_MSG_ERROR([Not found])
else
  AC_MSG_RESULT([$PYLIB])
fi

# Check for really old versions
if test -r "$PYLIB/libPython.a"; then
    PYTHON_LIBS="-L$PYLIB -lModules -lPython -lObjects -lParser"
else
    PYTHON_LIBS="-L$PYLIB -l$PYVERSION"
fi

AC_SUBST([PYTHON_CFLAGS])
AC_SUBST([PYTHON_LIBS])

# Check for X headers and libraries
AC_PATH_X
AC_PATH_XTRA
test "$no_x" = "yes" && AC_MSG_ERROR([No Xlibs found.])
test -z "$x_includes" && x_includes="/usr/include"
test -z "$x_libraries" && x_libraries="/usr/lib"
     
CXXFLAGS="$CXXFLAGS $X_CFLAGS"
LIBS="$LIBS $X_LIBS $X_EXTRA_LIBS"
LDFLAGS="$LDFLAGS $X_PRE_LIBS $LIBS $X_EXTRA_LIBS"

# Check for required functions in -lX11
AC_CHECK_LIB([X11], [XOpenDisplay],
  ,
  AC_MSG_ERROR([Could not find XOpenDisplay in -lX11.])
)


# Check for Xft >= 2
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

# Store these
OLDLIBS=$LIBS
OLDCXXFLAGS=$CXXFLAGS

XFT_CFLAGS="`pkg-config --cflags xft`"
XFT_LIBS="`pkg-config --libs xft`"

# Set these for checking with the tests below. They'll be restored after
LIBS="$LIBS $XFT_LIBS"
CXXFLAGS="$XFT_CFLAGS $CXXFLAGS"

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
# Restore the old values. Use XFT_CFLAGS and XFT_LIBS in the Makefile.am's
LIBS=$OLDLIBS
CXXFLAGS=$OLDCXXFLAGS

AC_SUBST([XFT_CFLAGS])
AC_SUBST([XFT_LIBS])

# Check for XKB extension support
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
    ],
    [ 
      AC_MSG_RESULT([no])
      XKB="no"
    ]
  )
)
AC_MSG_CHECKING([for the XKB extension])
if test "$XKB" = "yes"; then
  AC_MSG_RESULT([yes])
else
  AC_MSG_RESULT([no])
fi

# Check for XShape extension support
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

# Check for Xinerama extension support
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

AC_CONFIG_FILES([Makefile po/Makefile.in
		otk/Makefile
		src/Makefile
		scripts/Makefile
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
AC_MSG_RESULT([Building with '$CXXFLAGS' for C++ compiler flags.])
AC_MSG_RESULT([Building with '$LIBS' for linker flags.])
AC_MSG_RESULT
AC_MSG_RESULT([configure complete, now type \"make\"])
AC_MSG_RESULT
