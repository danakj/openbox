// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#include "python.hh"
#include "openbox.hh"
#include "actions.hh"
#include "python.hh"
#include "bindings.hh"
#include "otk/display.hh"
#include "otk/util.hh"

extern "C" {
// The initializer in openbox_wrap.cc
extern void init_openbox(void);
}

namespace ob {

static PyObject *obdict = NULL;

void python_init(char *argv0)
{
  // start the python engine
  Py_SetProgramName(argv0);
  Py_Initialize();
  // initialize the C python module
  init_openbox();
  // include the openbox directories for python scripts in the sys path
  PyRun_SimpleString("import sys");
  PyRun_SimpleString("sys.path.append('" SCRIPTDIR "')");
  PyRun_SimpleString(const_cast<char*>(("sys.path.append('" +
                                        otk::expandTilde("~/.openbox/python") +
                                        "')").c_str()));
  // import the otk and openbox modules into the main namespace
  PyRun_SimpleString("from openbox import *;");
  // set up convenience global variables
  PyRun_SimpleString("openbox = Openbox_instance()");

  // set up access to the python global variables
  PyObject *obmodule = PyImport_AddModule("__main__");
  obdict = PyModule_GetDict(obmodule);
}

void python_destroy()
{
  Py_DECREF(obdict);
}

bool python_exec(const std::string &path)
{
  FILE *rcpyfd = fopen(path.c_str(), "r");
  if (!rcpyfd) {
    printf("failed to load python file %s\n", path.c_str());
    return false;
  }
  PyRun_SimpleFile(rcpyfd, const_cast<char*>(path.c_str()));
  fclose(rcpyfd);
  return true;
}

bool python_get_long(const char *name, long *value)
{
  PyObject *val = PyDict_GetItemString(obdict, const_cast<char*>(name));
  if (!(val && PyLong_Check(val))) return false;
  
  *value = PyLong_AsLong(val);
  return true;
}

bool python_get_string(const char *name, std::string *value)
{
  PyObject *val = PyDict_GetItemString(obdict, const_cast<char*>(name));
  if (!(val && PyString_Check(val))) return false;
  
  *value = PyString_AsString(val);
  return true;
}

bool python_get_stringlist(const char *name, std::vector<std::string> *value)
{
  PyObject *val = PyDict_GetItemString(obdict, const_cast<char*>(name));
  if (!(val && PyList_Check(val))) return false;

  for (int i = 0, end = PyList_Size(val); i < end; ++i) {
    PyObject *str = PyList_GetItem(val, i);
    if (PyString_Check(str))
      value->push_back(PyString_AsString(str));
  }
  return true;
}

// ************************************* //
// Stuff for calling from Python scripts //
// ************************************* //

PyObject *mbind(const std::string &button, ob::MouseContext context,
                ob::MouseAction action, PyObject *func)
{
  if (!PyCallable_Check(func)) {
    PyErr_SetString(PyExc_TypeError, "Invalid callback function.");
    return NULL;
  }
  
  if (!ob::Openbox::instance->bindings()->addButton(button, context,
                                                    action, func)) {
    PyErr_SetString(PyExc_RuntimeError,"Unable to add binding.");
    return NULL;
  }
  Py_INCREF(Py_None); return Py_None;
}

PyObject *ebind(ob::EventAction action, PyObject *func)
{
  if (!PyCallable_Check(func)) {
    PyErr_SetString(PyExc_TypeError, "Invalid callback function.");
    return NULL;
  }
  
  if (!ob::Openbox::instance->bindings()->addEvent(action, func)) {
    PyErr_SetString(PyExc_RuntimeError,"Unable to add binding.");
    return NULL;
  }
  Py_INCREF(Py_None); return Py_None;
}

PyObject *kbind(PyObject *keylist, ob::KeyContext context, PyObject *func)
{
  if (!PyCallable_Check(func)) {
    PyErr_SetString(PyExc_TypeError, "Invalid callback function.");
    return NULL;
  }
  if (!PyList_Check(keylist)) {
    PyErr_SetString(PyExc_TypeError, "Invalid keylist. Not a list.");
    return NULL;
  }

  ob::OBBindings::StringVect vectkeylist;
  for (int i = 0, end = PyList_Size(keylist); i < end; ++i) {
    PyObject *str = PyList_GetItem(keylist, i);
    if (!PyString_Check(str)) {
      PyErr_SetString(PyExc_TypeError,
                     "Invalid keylist. It must contain only strings.");
      return NULL;
    }
    vectkeylist.push_back(PyString_AsString(str));
  }

  (void)context; // XXX use this sometime!
  if (!ob::Openbox::instance->bindings()->addKey(vectkeylist, func)) {
    PyErr_SetString(PyExc_RuntimeError,"Unable to add binding.");
    return NULL;
  }
  Py_INCREF(Py_None); return Py_None;
}

PyObject *kunbind(PyObject *keylist, PyObject *func)
{
  if (!PyList_Check(keylist)) {
    PyErr_SetString(PyExc_TypeError, "Invalid keylist. Not a list.");
    return NULL;
  }
  if (!PyCallable_Check(func)) {
    PyErr_SetString(PyExc_TypeError, "Invalid callback function.");
    return NULL;
  }
  
  ob::OBBindings::StringVect vectkeylist;
  for (int i = 0, end = PyList_Size(keylist); i < end; ++i) {
    PyObject *str = PyList_GetItem(keylist, i);
    if (!PyString_Check(str)) {
      PyErr_SetString(PyExc_TypeError,
                     "Invalid keylist. It must contain only strings.");
      return NULL;
    }
    vectkeylist.push_back(PyString_AsString(str));
  }

  if (!ob::Openbox::instance->bindings()->removeKey(vectkeylist, func)) {
      PyErr_SetString(PyExc_RuntimeError, "Could not remove callback.");
      return NULL;
  }
  Py_INCREF(Py_None); return Py_None;
}

void kunbind_all()
{
  ob::Openbox::instance->bindings()->removeAllKeys();
}

void set_reset_key(const std::string &key)
{
  ob::Openbox::instance->bindings()->setResetKey(key);
}

PyObject *send_client_msg(Window target, int type, Window about,
                          long data, long data1, long data2,
                          long data3, long data4)
{
  if (type < 0 || type >= otk::OBProperty::NUM_ATOMS) {
      PyErr_SetString(PyExc_TypeError,
                     "Invalid atom type. Must be from otk::OBProperty::Atoms");
      return NULL;
  }
  
  XEvent e;
  e.xclient.type = ClientMessage;
  e.xclient.format = 32;
  e.xclient.message_type =
    Openbox::instance->property()->atom((otk::OBProperty::Atoms)type);
  e.xclient.window = about;
  e.xclient.data.l[0] = data;
  e.xclient.data.l[1] = data1;
  e.xclient.data.l[2] = data2;
  e.xclient.data.l[3] = data3;
  e.xclient.data.l[4] = data4;

  XSendEvent(otk::OBDisplay::display, target, false,
             SubstructureRedirectMask | SubstructureNotifyMask,
             &e);
  Py_INCREF(Py_None); return Py_None;
}

}
