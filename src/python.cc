// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#include "python.hh"
#include "openbox.hh"
#include "actions.hh"
#include "python.hh"
#include "bindings.hh"
#include "otk/display.hh"
#include "otk/util.hh"

extern "C" {
// The initializer in openbox_wrap.cc / otk_wrap.cc
extern void init_ob(void);
extern void init_otk(void);
}

namespace ob {

static PyObject *obdict = NULL;

void python_init(char *argv0)
{
  // start the python engine
  Py_SetProgramName(argv0);
  Py_Initialize();
  // initialize the C python module
  init_otk();
  init_ob();
  // prepend the openbox directories for python scripts to the sys path
  PyRun_SimpleString("import sys");
  PyRun_SimpleString("sys.path.insert(0, '" SCRIPTDIR "')");
  PyRun_SimpleString(const_cast<char*>(("sys.path.insert(0, '" +
                                        otk::expandTilde("~/.openbox/python") +
                                        "')").c_str()));
  PyRun_SimpleString("import ob; import otk;");// import config;");
  // set up convenience global variables
  PyRun_SimpleString("ob.openbox = ob.Openbox_instance()");
  PyRun_SimpleString("otk.display = otk.Display_instance()");

  // set up access to the python global variables
  PyObject *obmodule = PyImport_AddModule("config");
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
  if (!(val && PyInt_Check(val))) return false;
  
  *value = PyInt_AsLong(val);
  return true;
}

bool python_get_string(const char *name, otk::ustring *value)
{
  PyObject *val = PyDict_GetItemString(obdict, const_cast<char*>(name));
  if (!(val && PyString_Check(val))) return false;
  
  *value = PyString_AsString(val);
  return true;
}

bool python_get_stringlist(const char *name, std::vector<otk::ustring> *value)
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

PyObject *mbind(const std::string &button, ob::MouseContext::MC context,
                ob::MouseAction::MA action, PyObject *func)
{
  if (!PyCallable_Check(func)) {
    PyErr_SetString(PyExc_TypeError, "Invalid callback function.");
    return NULL;
  }
  
  if (!ob::openbox->bindings()->addButton(button, context,
                                          action, func)) {
    PyErr_SetString(PyExc_RuntimeError,"Unable to add binding.");
    return NULL;
  }
  Py_INCREF(Py_None); return Py_None;
}

PyObject *ebind(ob::EventAction::EA action, PyObject *func)
{
  if (!PyCallable_Check(func)) {
    PyErr_SetString(PyExc_TypeError, "Invalid callback function.");
    return NULL;
  }
  
  if (!ob::openbox->bindings()->addEvent(action, func)) {
    PyErr_SetString(PyExc_RuntimeError,"Unable to add binding.");
    return NULL;
  }
  Py_INCREF(Py_None); return Py_None;
}

PyObject *kgrab(int screen, PyObject *func)
{
  if (!PyCallable_Check(func)) {
    PyErr_SetString(PyExc_TypeError, "Invalid callback function.");
    return NULL;
  }

  if (!ob::openbox->bindings()->grabKeyboard(screen, func)) {
    PyErr_SetString(PyExc_RuntimeError,"Unable to grab keybaord.");
    return NULL;
  }
  Py_INCREF(Py_None); return Py_None;
}

PyObject *kungrab()
{
  ob::openbox->bindings()->ungrabKeyboard();
  Py_INCREF(Py_None); return Py_None;
}

PyObject *mgrab(int screen)
{
  if (!ob::openbox->bindings()->grabPointer(screen)) {
    PyErr_SetString(PyExc_RuntimeError,"Unable to grab pointer.");
    return NULL;
  }
  Py_INCREF(Py_None); return Py_None;
}

PyObject *mungrab()
{
  ob::openbox->bindings()->ungrabPointer();
  Py_INCREF(Py_None); return Py_None;
}

PyObject *kbind(PyObject *keylist, ob::KeyContext::KC context, PyObject *func)
{
  if (!PyCallable_Check(func)) {
    PyErr_SetString(PyExc_TypeError, "Invalid callback function.");
    return NULL;
  }
  if (!PyList_Check(keylist)) {
    PyErr_SetString(PyExc_TypeError, "Invalid keylist. Not a list.");
    return NULL;
  }

  ob::Bindings::StringVect vectkeylist;
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
  if (!ob::openbox->bindings()->addKey(vectkeylist, func)) {
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
  
  ob::Bindings::StringVect vectkeylist;
  for (int i = 0, end = PyList_Size(keylist); i < end; ++i) {
    PyObject *str = PyList_GetItem(keylist, i);
    if (!PyString_Check(str)) {
      PyErr_SetString(PyExc_TypeError,
                     "Invalid keylist. It must contain only strings.");
      return NULL;
    }
    vectkeylist.push_back(PyString_AsString(str));
  }

  if (!ob::openbox->bindings()->removeKey(vectkeylist, func)) {
      PyErr_SetString(PyExc_RuntimeError, "Could not remove callback.");
      return NULL;
  }
  Py_INCREF(Py_None); return Py_None;
}

void kunbind_all()
{
  ob::openbox->bindings()->removeAllKeys();
}

void set_reset_key(const std::string &key)
{
  ob::openbox->bindings()->setResetKey(key);
}

PyObject *send_client_msg(Window target, Atom type, Window about,
                          long data, long data1, long data2,
                          long data3, long data4)
{
  XEvent e;
  e.xclient.type = ClientMessage;
  e.xclient.format = 32;
  e.xclient.message_type = type;
  e.xclient.window = about;
  e.xclient.data.l[0] = data;
  e.xclient.data.l[1] = data1;
  e.xclient.data.l[2] = data2;
  e.xclient.data.l[3] = data3;
  e.xclient.data.l[4] = data4;

  XSendEvent(**otk::display, target, false,
             SubstructureRedirectMask | SubstructureNotifyMask,
             &e);
  Py_INCREF(Py_None); return Py_None;
}

void execute(const std::string &bin, int screen)
{
  if (screen >= ScreenCount(**otk::display))
    screen = 0;
  otk::bexec(bin, otk::display->screenInfo(screen)->displayString());
}

}
