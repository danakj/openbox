// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

%module otk_property

%{
#include "config.h"
#include "property.hh"
%}

%include "otk_ustring.i"

%typemap(python,in) const otk::Property::StringVect & (otk::Property::StringVect temp) {
  if (PyList_Check($input)) {
    int s = PyList_Size($input);
    temp = otk::Property::StringVect(s);
    for (int i = 0; i < s; ++i) {
      PyObject *o = PyList_GetItem($input, i);
      if (PyString_Check(o)) {
        temp[i] = PyString_AsString(o);
      } else {
        SWIG_exception(SWIG_TypeError, "list of strings expected");
      }
    }
    $1 = &temp;
  } else {
    SWIG_exception(SWIG_TypeError, "list expected");
  }
}

%typemap(python,in) unsigned long value[] (unsigned long *temp) {
  if (PyList_Check($input)) {
    int s = PyList_Size($input);
    temp = new unsigned long[s];
    for (int i = 0; i < s; ++i) {
      PyObject *o = PyList_GetItem($input, i);
      if (PyInt_Check(o)) {
        temp[i] = PyInt_AsLong(o);
      } else if (PyLong_Check(o)) {
        temp[i] = PyLong_AsLong(o);
      } else {
        SWIG_exception(SWIG_TypeError, "list of numbers expected");
      }
    }
    $1 = temp;
  } else {
    SWIG_exception(SWIG_TypeError, "list expected");
  }
}

%typemap(python,freearg) unsigned long value[] {
  delete [] $1;
}

%typemap(python,in,numinputs=0) otk::ustring *value (otk::ustring temp) {
  $1 = &temp;
}

%typemap(python,argout) otk::ustring *value {
  PyObject *tuple;
  int s;
  if (PyTuple_Check($result)) {
    s = PyTuple_Size($result);
    _PyTuple_Resize(&$result, s + 1);
    tuple = $result;
  } else {
    tuple = PyTuple_New(2);
    PyTuple_SET_ITEM(tuple, 0, $result);
    Py_INCREF($result);
    s = 1;
  }

  PyTuple_SET_ITEM(tuple, s, PyString_FromString($1->c_str()));
  $result = tuple;
}

%typemap(python,in,numinputs=0) unsigned long *value (unsigned long temp) {
  $1 = &temp;
}

%typemap(python,argout) unsigned long *value {
  PyObject *s = PyLong_FromLong(*$1);
  $result = s;
}

%typemap(python,in,numinputs=0) unsigned long *nelements (unsigned long temp) {
  $1 = &temp;
}

%typemap(python,in) unsigned long *nelements (unsigned long temp) {
  temp = (unsigned)PyLong_AsLong($input);
  $1 = &temp;
}

%typemap(python,in,numinputs=0) unsigned long **value (unsigned long *temp) {
  $1 = &temp;
}

%typemap(python,argout) (unsigned long *nelements, unsigned long **value) {
  PyObject *tuple;
  int s;
  if (PyTuple_Check($result)) {
    s = PyTuple_Size($result);
    _PyTuple_Resize(&$result, s + 2);
    tuple = $result;
  } else {
    tuple = PyTuple_New(3);
    PyTuple_SET_ITEM(tuple, 0, $result);
    Py_INCREF($result);
    s = 1;
  }

  int sz = *$1;

  PyTuple_SET_ITEM(tuple, s++, PyLong_FromLong(sz));
  
  PyObject *r = PyList_New(sz);
  for (int i = 0; i < sz; ++i) {
    PyList_SET_ITEM(r, i, PyLong_FromLong((*$2)[i]));
  }
  PyTuple_SET_ITEM(tuple, s, r);
  $result = tuple;
}

%typemap(python,in,numinputs=0) StringVect *strings (StringVect temp) {
  $1 = &temp;
}

%typemap(python,argout) (unsigned long *nelements, StringVect *strings) {
  PyObject *tuple;
  int s;
  if (PyTuple_Check($result)) {
    s = PyTuple_Size($result);
    _PyTuple_Resize(&$result, s + 2);
    tuple = $result;
  } else {
    tuple = PyTuple_New(3);
    PyTuple_SET_ITEM(tuple, 0, $result);
    Py_INCREF($result);
    s = 1;
  }

  int sz = *$1;

  PyTuple_SET_ITEM(tuple, s++, PyLong_FromLong(sz));
  
  PyObject *r = PyList_New(sz);
  for (int i = 0; i < sz; ++i) {
    PyList_SET_ITEM(r, i, PyString_FromString((*$2)[i].c_str()));
  }
  PyTuple_SET_ITEM(tuple, s, r);
  $result = tuple;
}

namespace otk {

%immutable otk::Property::atoms;

%ignore Property::NUM_STRING_TYPE;
%ignore Property::initialize();

%rename(getNum) Property::get(Window, Atom, Atom, unsigned long*);
%rename(getNums) Property::get(Window, Atom, Atom, unsigned long*,
                               unsigned long**);
%rename(getString) Property::get(Window, Atom, StringType, ustring*);
%rename(getStrings) Property::get(Window, Atom, StringType, unsigned long*,
                                  StringVect);

}

%include "property.hh"
