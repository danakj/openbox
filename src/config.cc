// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#include "config.h"

#include "config.hh"

extern "C" {
#include <Python.h>
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
  PyRun_SimpleString("import config;");
  // set up access to the python global variables
  PyObject *obmodule = PyImport_AddModule("config");
  obdict = PyModule_GetDict(obmodule);

  std::vector<otk::ustring> names;
  python_get_stringlist("DESKTOP_NAMES", &names);

  python_get_string("THEME", &theme);

  if (!python_get_string("TITLEBAR_LAYOUT", &titlebar_layout))
    titlebar_layout = "NTIMC";

  if (!python_get_long("DOUBLE_CLICK_DELAY", &double_click_delay))
    double_click_delay = 300;
  if (!python_get_long("DRAG_THRESHOLD", &drag_threshold))
    drag_threshold = 3;
  if (!python_get_long("NUMBER_OF_DESKTOPS", (long*)&num_desktops))
    num_desktops = 1;

  otk::ustring s;
  long w, h;
  if (python_get_string("DEFAULT_ICON", &s) && s.bytes() > 2 &&
      python_get_long("DEFAULT_ICON_WIDTH", &w) &&
      python_get_long("DEFAULT_ICON_HEIGHT", &h) &&
      (unsigned)(w * h) == s.bytes() / sizeof(unsigned long)) {
    default_icon = new unsigned long[s.bytes() / sizeof(unsigned long) + 2];
    default_icon[0] = w;
    default_icon[1] = h;
    memcpy(default_icon + 2, s.data(), s.bytes());
  } else {
    default_icon = 0;
  }
      
  icon_length = s.bytes();
}

Config::~Config()
{
  if (default_icon) delete [] default_icon;
}

}
