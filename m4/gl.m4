# X11_DEVEL()
#
# Check for the presence of the X Window System headers and libraries.
# Sets the CPPFLAGS and LIBS variables as appropriate.
AC_DEFUN([GL_OPTION],
[
  AC_REQUIRE([X11_DEVEL])

  AC_ARG_ENABLE(gl, [  --enable-gl             enable support for OpenGL rendering default=no],
                ,[enable_gl="no"])

  # Store these
  OLDLIBS=$LIBS
  OLDCPPFLAGS=$CPPFLAGS

  if test "$enable_gl" = "yes"; then
    AC_CHECK_LIB([GL], [glXGetConfig],
                 ,
                 [
                   enable_gl="no"
                   AC_MSG_WARN([Disabling GL rendering support])
                 ])
  fi

  if test "$enable_gl" = "yes"; then
    CPPFLAGS="$CPPFLAGS $X_CFLAGS"
    LIBS="$LIBS $X_PRE_LIBS $X_LIBS $X_EXTRA_LIBS"

    AC_MSG_CHECKING([if we can compile with GL])
    AC_TRY_LINK(
    [
      #include <GL/gl.h>
    ],
    [
      GLfloat f = 0.0;
      glVertex3f(f, f, f);
    ],
    [
      AC_MSG_RESULT([yes])
    ],
    [
      AC_MSG_RESULT([no])
      AC_MSG_WARN([Disabling GL rendering support])
      enable_gl="no"
    ])

    GL_CFLAGS=""
    GL_LIBS="-lGL"
    AC_SUBST(GL_CFLAGS)
    AC_SUBST(GL_LIBS)
  fi

  CPPFLAGS=$OLDCPPFLAGS
  LIBS=$OLDLIBS

  AC_MSG_CHECKING([if GL support is enabled])
  if test "$enable_gl" = "yes"; then
    AC_MSG_RESULT([yes])

    AC_DEFINE(USE_GL)
  else
    AC_MSG_RESULT([no])
  fi
  AM_CONDITIONAL([USE_GL], [test "$enable_gl" = "yes"])
])
