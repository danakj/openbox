// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#include "python.hh"
#include "openbox.hh"
#include "actions.hh"
#include "python.hh"
#include "bindings.hh"
#include "otk/display.hh"

extern "C" {
// The initializer in openbox_wrap.cc
extern void init_openbox(void);
// The initializer in otk_wrap.cc
extern void init_otk(void);
}

namespace ob {

static PyObject *obdict = NULL;

// ************************************************************* //
// Define some custom types which are passed to python callbacks //
// ************************************************************* //

static void dealloc(PyObject *self)
{
  PyObject_Del(self);
}

PyObject *MotionData_window(MotionData *self, PyObject *args)
{
  if(!PyArg_ParseTuple(args,":window")) return NULL;
  return PyLong_FromLong(self->window);
}

PyObject *MotionData_context(MotionData *self, PyObject *args)
{
  if(!PyArg_ParseTuple(args,":context")) return NULL;
  return PyLong_FromLong((int)self->context);
}

PyObject *MotionData_action(MotionData *self, PyObject *args)
{
  if(!PyArg_ParseTuple(args,":action")) return NULL;
  return PyLong_FromLong((int)self->action);
}

PyObject *MotionData_modifiers(MotionData *self, PyObject *args)
{
  if(!PyArg_ParseTuple(args,":modifiers")) return NULL;
  return PyLong_FromUnsignedLong(self->state);
}

PyObject *MotionData_button(MotionData *self, PyObject *args)
{
  if(!PyArg_ParseTuple(args,":button")) return NULL;
  int b = 0;
  switch (self->button) {
  case Button5: b++;
  case Button4: b++;
  case Button3: b++;
  case Button2: b++;
  case Button1: b++;
  default: ;
  }
  return PyLong_FromLong(b);
}

PyObject *MotionData_xroot(MotionData *self, PyObject *args)
{
  if(!PyArg_ParseTuple(args,":xroot")) return NULL;
  return PyLong_FromLong(self->xroot);
}

PyObject *MotionData_yroot(MotionData *self, PyObject *args)
{
  if(!PyArg_ParseTuple(args,":yroot")) return NULL;
  return PyLong_FromLong(self->yroot);
}

PyObject *MotionData_pressx(MotionData *self, PyObject *args)
{
  if(!PyArg_ParseTuple(args,":pressx")) return NULL;
  return PyLong_FromLong(self->pressx);
}

PyObject *MotionData_pressy(MotionData *self, PyObject *args)
{
  if(!PyArg_ParseTuple(args,":pressy")) return NULL;
  return PyLong_FromLong(self->pressy);
}


PyObject *MotionData_press_clientx(MotionData *self, PyObject *args)
{
  if(!PyArg_ParseTuple(args,":press_clientx")) return NULL;
  return PyLong_FromLong(self->press_clientx);
}

PyObject *MotionData_press_clienty(MotionData *self, PyObject *args)
{
  if(!PyArg_ParseTuple(args,":press_clienty")) return NULL;
  return PyLong_FromLong(self->press_clienty);
}

PyObject *MotionData_press_clientwidth(MotionData *self, PyObject *args)
{
  if(!PyArg_ParseTuple(args,":press_clientwidth")) return NULL;
  return PyLong_FromLong(self->press_clientwidth);
}

PyObject *MotionData_press_clientheight(MotionData *self, PyObject *args)
{
  if(!PyArg_ParseTuple(args,":press_clientheight")) return NULL;
  return PyLong_FromLong(self->press_clientheight);
}

PyObject *MotionData_time(MotionData *self, PyObject *args)
{
  if(!PyArg_ParseTuple(args,":time")) return NULL;
  return PyLong_FromLong(self->time);
}

static PyMethodDef MotionData_methods[] = {
  {"action", (PyCFunction)MotionData_action, METH_VARARGS,
   "Return the action being executed."},
  {"window", (PyCFunction)MotionData_window, METH_VARARGS,
   "Return the client window id."},
  {"context", (PyCFunction)MotionData_context, METH_VARARGS,
   "Return the context that the action is occuring in."},
  {"modifiers", (PyCFunction)MotionData_modifiers, METH_VARARGS,
   "Return the modifier keys state."},
  {"button", (PyCFunction)MotionData_button, METH_VARARGS,
   "Return the number of the pressed button (1-5)."},
  {"xroot", (PyCFunction)MotionData_xroot, METH_VARARGS,
   "Return the X-position of the mouse cursor on the root window."},
  {"yroot", (PyCFunction)MotionData_yroot, METH_VARARGS,
   "Return the Y-position of the mouse cursor on the root window."},
  {"pressx", (PyCFunction)MotionData_pressx, METH_VARARGS,
   "Return the X-position of the mouse cursor at the start of the drag."},
  {"pressy", (PyCFunction)MotionData_pressy, METH_VARARGS,
   "Return the Y-position of the mouse cursor at the start of the drag."},
  {"press_clientx", (PyCFunction)MotionData_press_clientx, METH_VARARGS,
   "Return the X-position of the client at the start of the drag."},
  {"press_clienty", (PyCFunction)MotionData_press_clienty, METH_VARARGS,
   "Return the Y-position of the client at the start of the drag."},
  {"press_clientwidth", (PyCFunction)MotionData_press_clientwidth,
   METH_VARARGS,
   "Return the width of the client at the start of the drag."},
  {"press_clientheight", (PyCFunction)MotionData_press_clientheight,
   METH_VARARGS,
   "Return the height of the client at the start of the drag."},
  {"time", (PyCFunction)MotionData_time, METH_VARARGS,
   "Return the time at which the event occured."},
  {NULL, NULL, 0, NULL}
};

static PyMethodDef ButtonData_methods[] = {
  {"action", (PyCFunction)MotionData_action, METH_VARARGS,
   "Return the action being executed."},
  {"context", (PyCFunction)MotionData_context, METH_VARARGS,
   "Return the context that the action is occuring in."},
  {"window", (PyCFunction)MotionData_window, METH_VARARGS,
   "Return the client window id."},
  {"modifiers", (PyCFunction)MotionData_modifiers, METH_VARARGS,
   "Return the modifier keys state."},
  {"button", (PyCFunction)MotionData_button, METH_VARARGS,
   "Return the number of the pressed button (1-5)."},
  {"time", (PyCFunction)MotionData_time, METH_VARARGS,
   "Return the time at which the event occured."},
  {NULL, NULL, 0, NULL}
};

PyObject *KeyData_key(KeyData *self, PyObject *args)
{
  if(!PyArg_ParseTuple(args,":key")) return NULL;
  return PyString_FromString(
    XKeysymToString(XKeycodeToKeysym(otk::OBDisplay::display, self->key, 0)));

}

static PyMethodDef KeyData_methods[] = {
  {"window", (PyCFunction)MotionData_window, METH_VARARGS,
   "Return the client window id."},
  {"modifiers", (PyCFunction)MotionData_modifiers, METH_VARARGS,
   "Return the modifier keys state."},
  {"key", (PyCFunction)KeyData_key, METH_VARARGS,
   "Return the name of the pressed key."},
  {"time", (PyCFunction)MotionData_time, METH_VARARGS,
   "Return the time at which the event occured."},
  {NULL, NULL, 0, NULL}
};

static PyObject *MotionDataGetAttr(PyObject *obj, char *name)
{
  return Py_FindMethod(MotionData_methods, obj, name);
}

static PyObject *ButtonDataGetAttr(PyObject *obj, char *name)
{
  return Py_FindMethod(ButtonData_methods, obj, name);
}

static PyObject *KeyDataGetAttr(PyObject *obj, char *name)
{
  return Py_FindMethod(KeyData_methods, obj, name);
}

static PyTypeObject MotionData_Type = {
  PyObject_HEAD_INIT(NULL)
  0,
  "MotionData",
  sizeof(MotionData),
  0,
  dealloc,
  0,
  (getattrfunc)MotionDataGetAttr,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

static PyTypeObject ButtonData_Type = {
  PyObject_HEAD_INIT(NULL)
  0,
  "ButtonData",
  sizeof(ButtonData),
  0,
  dealloc,
  0,
  (getattrfunc)ButtonDataGetAttr,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

static PyTypeObject KeyData_Type = {
  PyObject_HEAD_INIT(NULL)
  0,
  "KeyData",
  sizeof(KeyData),
  0,
  dealloc,
  0,
  (getattrfunc)KeyDataGetAttr,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

MotionData *new_motion_data(Window window, Time time, unsigned int state,
                          unsigned int button, MouseContext context,
                          MouseAction action, int xroot, int yroot,
                          const otk::Point &initpos, const otk::Rect &initarea)
{
  MotionData *data = PyObject_New(MotionData, &MotionData_Type);
  data->window = window;
  data->time   = time;
  data->state  = state;
  data->button = button;
  data->context= context;
  data->action = action;
  data->xroot  = xroot;
  data->yroot  = yroot;
  data->pressx = initpos.x();
  data->pressy = initpos.y();
  data->press_clientx      = initarea.x();
  data->press_clienty      = initarea.y();
  data->press_clientwidth  = initarea.width();
  data->press_clientheight = initarea.height();
  return data;
}

ButtonData *new_button_data(Window window, Time time, unsigned int state,
                          unsigned int button, MouseContext context,
                          MouseAction action)
{
  ButtonData *data = PyObject_New(ButtonData, &ButtonData_Type);
  data->window = window;
  data->time   = time;
  data->state  = state;
  data->button = button;
  data->context= context;
  data->action = action;
  return data;
}

KeyData *new_key_data(Window window, Time time, unsigned int state,
                       unsigned int key)
{
  KeyData *data = PyObject_New(KeyData, &KeyData_Type);
  data->window = window;
  data->time   = time;
  data->state  = state;
  data->key    = key;
  return data;
}

// **************** //
// End custom types //
// **************** //

void python_init(char *argv0)
{
  Py_SetProgramName(argv0);
  Py_Initialize();
  init_otk();
  init_openbox();
  PyRun_SimpleString("from _otk import *; from _openbox import *;");
  PyRun_SimpleString("openbox = Openbox_instance()");

  // set up access to the python global variables
  PyObject *obmodule = PyImport_AddModule("__main__");
  obdict = PyModule_GetDict(obmodule);

  // set up the custom types
  MotionData_Type.ob_type = &PyType_Type;
  ButtonData_Type.ob_type = &PyType_Type;
  KeyData_Type.ob_type = &PyType_Type;
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

void python_callback(PyObject *func, PyObject *data)
{
  PyObject *arglist;
  PyObject *result;

  arglist = Py_BuildValue("(O)", data);
  
  // call the callback
  result = PyEval_CallObject(func, arglist);
  if (!result) {
    // an exception occured in the script, display it
    PyErr_Print();
  }

  Py_XDECREF(result);
  Py_DECREF(arglist);
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

/*
PyObject * python_register(int action, PyObject *func, bool infront = false)
{
  if (!PyCallable_Check(func)) {
    PyErr_SetString(PyExc_TypeError, "Invalid callback function.");
    return NULL;
  }

  if (!ob::Openbox::instance->actions()->registerCallback(
        (ob::OBActions::ActionType)action, func, infront)) {
    PyErr_SetString(PyExc_RuntimeError, "Unable to register action callback.");
    return NULL;
  }
  Py_INCREF(Py_None); return Py_None;
}

PyObject *unregister(int action, PyObject *func)
{
  if (!PyCallable_Check(func)) {
    PyErr_SetString(PyExc_TypeError, "Invalid callback function.");
    return NULL;
  }

  if (!ob::Openbox::instance->actions()->unregisterCallback(
        (ob::OBActions::ActionType)action, func)) {
    PyErr_SetString(PyExc_RuntimeError, "Unable to unregister action callback.");
    return NULL;
  }
  Py_INCREF(Py_None); return Py_None;
}

PyObject *unregister_all(int action)
{
  if (!ob::Openbox::instance->actions()->unregisterAllCallbacks(
        (ob::OBActions::ActionType)action)) {
    PyErr_SetString(PyExc_RuntimeError,
                    "Unable to unregister action callbacks.");
    return NULL;
  }
  Py_INCREF(Py_None); return Py_None;
}
*/
PyObject * mbind(const std::string &button, ob::MouseContext context,
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

PyObject * kbind(PyObject *keylist, ob::KeyContext context, PyObject *func)
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

  if (!ob::Openbox::instance->bindings()->addKey(vectkeylist, func)) {
    PyErr_SetString(PyExc_RuntimeError,"Unable to add binding.");
    return NULL;
  }
  Py_INCREF(Py_None); return Py_None;
}

PyObject * kunbind(PyObject *keylist)
{
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

  ob::Openbox::instance->bindings()->removeKey(vectkeylist);
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

}
