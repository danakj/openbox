# GL_DEVEL()
#
# Check for the presence of OpenGL development headers and libraries.
# Sets the GL_CFLAGS and GL_LIBS variables as appropriate.
AC_DEFUN([GL_DEVEL],
[
  AC_REQUIRE([X11_DEVEL])

  # Store these
  OLDLIBS=$LIBS
  OLDCPPFLAGS=$CPPFLAGS

  AC_CHECK_LIB([GL], [glXGetConfig],
               ,
               [
                 AC_MSG_ERROR([A valid libGL could not be found.])
               ])

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
    AC_MSG_ERROR([Could not compile against GL])
  ])

  GL_CFLAGS=""
  GL_LIBS="-lGL"
  AC_SUBST(GL_CFLAGS)
  AC_SUBST(GL_LIBS)

  CPPFLAGS=$OLDCPPFLAGS
  LIBS=$OLDLIBS
])
