// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#include "config.h"

#include "config.hh"

extern "C" {
#include <Python.h>

#include "gettext.h"
#define _(str) gettext(str)
}

#include <cstring>

namespace ob {

static PyObject *obdict = NULL;

bool python_get_long(const char *name, long *value)
{
  PyObject *val = PyDict_GetItemString(obdict, const_cast<char*>(name));
  if (!(val && PyInt_Check(val))) return false;
  
  *value = PyInt_AsLong(val);
  return true;
}

bool python_get_string(const char *name, otk::ustring *value)
{
  PyObject *val = PyDict_GetItemString(obdict, const_cast<char*>(name));
  if (!(val && PyString_Check(val))) return false;

  std::string temp(PyString_AsString(val), PyString_Size(val));
  *value = temp;
  return true;
}

bool python_get_stringlist(const char *name, std::vector<otk::ustring> *value)
{
  PyObject *val = PyDict_GetItemString(obdict, const_cast<char*>(name));
  if (!(val && PyList_Check(val))) return false;

  value->clear();
  
  for (int i = 0, end = PyList_Size(val); i < end; ++i) {
    PyObject *str = PyList_GetItem(val, i);
    if (PyString_Check(str))
      value->push_back(PyString_AsString(str));
  }
  return true;
}

Config::Config()
{
  // set up access to the python global variables
  PyObject *obmodule = PyImport_ImportModule("config");
  obdict = PyModule_GetDict(obmodule);
  Py_DECREF(obmodule);

  python_get_stringlist("DESKTOP_NAMES", &desktop_names);

  python_get_string("THEME", &theme);

  if (!python_get_string("TITLEBAR_LAYOUT", &titlebar_layout)) {
    fprintf(stderr, _("Unable to load config.%s\n"), "TITLEBAR_LAYOUT");
    ::exit(1);
  }

  if (!python_get_long("DOUBLE_CLICK_DELAY", &double_click_delay)) {
    fprintf(stderr, _("Unable to load config.%s\n"), "DOUBLE_CLICK_DELAY");
    ::exit(1);
  }
  if (!python_get_long("DRAG_THRESHOLD", &drag_threshold)) {
    fprintf(stderr, _("Unable to load config.%s\n"), "DRAG_THRESHOLD");
    ::exit(1);
  }
  if (!python_get_long("NUMBER_OF_DESKTOPS", (long*)&num_desktops)) {
    fprintf(stderr, _("Unable to load config.%s\n"), "NUMBER_OF_DESKTOPS");
    ::exit(1);
  }
}

Config::~Config()
{
}

}
