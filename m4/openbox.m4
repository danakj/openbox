# OB_DEBUG()
#
# Check if the user has requested a debug build.
# Sets the DEBUG or NDEBUG variables as appropriate
AC_DEFUN([OB_DEBUG],
[
  DEBUG="no"
  AC_MSG_CHECKING([build target type])

  AC_ARG_ENABLE([debug],
  [  --enable-debug          build a debug version default=no],
  [DEBUG=$enableval],[])
  if test "$DEBUG" = "yes"; then
    AC_MSG_RESULT([debug])
    AC_DEFINE([DEBUG], [1], [Creating a debug build])
  else
    AC_MSG_RESULT([release])
    AC_DEFINE([NDEBUG], [1], [Creating a release build])
  fi
])


# OB_COMPILER_FLAGS()
#
# Check what compiler is being used for compilation.
# It sets the CXXFLAGS variable appropriately for the compiler, including flags
# for debug builds.
AC_DEFUN([OB_COMPILER_FLAGS],
[
  AC_REQUIRE([AC_PROG_CXXCPP])
  AC_REQUIRE([AC_PROG_CXX])

  FLAGS=""

  # Check what compiler we are using
  AC_MSG_CHECKING([for GNU C++])
  if test "$GXX" = "yes"; then
    AC_MSG_RESULT([yes])
    FLAGS="-Wall -W"
    # -pedantic
    test "$DEBUG" = "yes" && FLAGS="$FLAGS -g -fno-inline"
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
        FLAGS="-LANG:std"
      else
        AC_MSG_RESULT([already set.])
      fi
    fi
  fi
  AC_MSG_CHECKING([for compiler specific flags])
  AC_MSG_RESULT([$FLAGS])
  CXXFLAGS="$CXXFLAGS $FLAGS"
])
