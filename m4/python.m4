# PYTHON_DEVEL()
#
# Checks for Python and tries to get the include path to 'Python.h', and
# the libpython library.
# It provides the $(PYTHON_CFLAGS) $(PYTHON_LIBS) output variables.
AC_DEFUN([PYTHON_DEVEL],
[
  AC_REQUIRE([AM_PATH_PYTHON])

  python_prefix=${PYTHON%/bin*}

  # Check for Python include path
  AC_MSG_CHECKING([for Python include path])
  for i in "$python_prefix/include/python$PYTHON_VERSION/" \
           "$python_prefix/include/python/" "$python_prefix/"
  do
    python_path=`find $i -type f -name Python.h -print`
    test "$python_path" && break
  done
  for i in $python_path
  do
    python_path=${python_path%/Python.h}
    break
  done
  if test "$python_path"
  then
    AC_MSG_RESULT([$python_path])
  else
    AC_MSG_ERROR([cannot find Python include path])
  fi
  AC_SUBST([PYTHON_CFLAGS], [-I$python_path])

  # Check for a Python library
  AC_MSG_CHECKING([for Python library])
  PYLIB=""
  for i in "$python_prefix/lib" \
           "$python_prefix/lib/python$PYTHON_VERSION/config" \
           "$python_prefix/lib/python$PYTHON_VERSION/lib" \
           "$python_prefix/lib/python/lib"
  do
    if test -r "$i/libpython$PYTHON_VERSION.so"; then
      PYLIB="$i/libpython$PYTHON_VERSION.so"
      PYTHON_LIBS="-L$i -l$PYVERSION"
      break
    else
      if test -r "$i/lib$PYVERSION.a"; then
        PYLIB="$i/lib$PYVERSION.a"
        PYTHON_LIBS="-L$i -l$PYVERSION -lpthread -ldl -lutil -lm"
        break
      else
        # look for really old versions
        if test -r "$i/libPython.a"; then
          PYLIB="$i/libPython.a"
          PYTHON_LIBS="-L$i -lModules -lPython -lObjects -lParser"
          break
        fi
      fi
    fi
  done
  if test "$python_path"
  then
    AC_MSG_RESULT([$PYLIB])
  else
    AC_MSG_ERROR([cannot find Python library])
  fi
  AC_SUBST([PYTHON_LIBS])
])
