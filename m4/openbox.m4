# OB_DEBUG()
#
# Check if the user has requested a debug build.
# Sets the DEBUG or NDEBUG variables as appropriate
# Sets the CVS environment variable when building CVS sources.
AC_DEFUN([OB_DEBUG],
[
    AC_MSG_CHECKING([build type])

    AC_ARG_ENABLE([strict-ansi],
    AC_HELP_STRING([--enable-strict-ansi],[Enable strict ANSI compliance build [[default=no]]]),
    [STRICT=$enableval], [STRICT="no"])
    if test "$GCC" = "yes" && test "$STRICT" = "yes"; then
	CFLAGS="$CFLAGS -ansi -pedantic -D_XOPEN_SOURCE"
    fi

    AC_ARG_ENABLE([debug],
    AC_HELP_STRING([--enable-debug],[build a debug version [[default=no]]]),
    [DEBUG=$enableval], [DEBUG="no"])

    TEST=""
    test "${PACKAGE_VERSION%*alpha*}" != "$PACKAGE_VERSION" && TEST="yes"
    test "${PACKAGE_VERSION%*beta*}" != "$PACKAGE_VERSION" && TEST="yes"
    test "$TEST" = "yes" && DEBUG="yes"

    if test "$DEBUG" = "yes"; then
	MSG="DEBUG"
    else
	MSG="RELEASE"
    fi
    if test "$TEST" = "yes"; then
	MSG="$MSG (test release)"
    fi
    if test "$STRICT" = "yes"; then
	MSG="$MSG with strict ANSI compliance"
    fi
    AC_MSG_RESULT([$MSG])
    
    test "$DEBUG" = "yes" && \
	AC_DEFINE([DEBUG], [1], [Creating a debug build])
])


# OB_COMPILER_FLAGS()
#
# Check what compiler is being used for compilation.
# It sets the CFLAGS variable appropriately for the compiler, including flags
# for debug builds.
AC_DEFUN([OB_COMPILER_FLAGS],
[
    AC_REQUIRE([AC_PROG_CPP])
    AC_REQUIRE([AC_PROG_CC])

    FLAGS=""

    if test "$DEBUG" = "yes"; then
        FLAGS="-DDEBUG"
    else
        FLAGS="-DNDEBUG -DG_DISABLE_ASSERT"
    fi

    # Check what compiler we are using
    AC_MSG_CHECKING([for GNU CC])
    if test "$GCC" = "yes"; then
	AC_MSG_RESULT([yes])
	if test "$DEBUG" = "yes"; then
	    FLAGS="$FLAGS -O0 -g -fno-inline"
	    FLAGS="$FLAGS -Wall -Wsign-compare -Waggregate-return"
	    FLAGS="$FLAGS -Wcast-qual -Wbad-function-cast -Wpointer-arith"
            # for Python.h
	    #FLAGS="$FLAGS -Wno-long-long"
	fi
	if test "$STRICT" = "yes"; then
	    FLAGS="$FLAGS -ansi -pedantic -D_XOPEN_SOURCE"
	fi
	FLAGS="$FLAGS -fno-strict-aliasing"
    fi
    AC_MSG_CHECKING([for compiler specific flags])
    AC_MSG_RESULT([$FLAGS])
    CFLAGS="$CFLAGS $FLAGS"
])

AC_DEFUN([OB_NLS],
[
    AC_ARG_ENABLE([nls],
    AC_HELP_STRING([--enable-nls],[Enable NLS translations [[default=yes]]]),
    [NLS=$enableval], [NLS="yes"])

    if test "$NLS" = yes; then
	DEFS="$DEFS -DENABLE_NLS"
    fi
])

