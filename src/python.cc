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

typedef struct {
  PyObject_HEAD;
  OBActions::ActionType action;
  Window window;
  OBWidget::WidgetType type;
  unsigned int state;
  unsigned int button;
  int xroot;
  int yroot;
  Time time;
} ActionData;

typedef struct {
  PyObject_HEAD;
  Window window;
  unsigned int state;
  unsigned int key;
  Time time;
} BindingData;

static void ActionDataDealloc(ActionData *self)
{
  PyObject_Del((PyObject*)self);
}

static void BindingDataDealloc(BindingData *self)
{
  PyObject_Del((PyObject*)self);
}

PyObject *ActionData_action(ActionData *self, PyObject *args)
{
  if(!PyArg_ParseTuple(args,":action")) return NULL;
  return PyLong_FromLong((int)self->action);
}

PyObject *ActionData_window(ActionData *self, PyObject *args)
{
  if(!PyArg_ParseTuple(args,":window")) return NULL;
  return PyLong_FromLong(self->window);
}

PyObject *ActionData_target(ActionData *self, PyObject *args)
{
  if(!PyArg_ParseTuple(args,":target")) return NULL;
  return PyLong_FromLong((int)self->type);
}

PyObject *ActionData_modifiers(ActionData *self, PyObject *args)
{
  if(!PyArg_ParseTuple(args,":modifiers")) return NULL;
  return PyLong_FromUnsignedLong(self->state);
}

PyObject *ActionData_button(ActionData *self, PyObject *args)
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

PyObject *ActionData_xroot(ActionData *self, PyObject *args)
{
  if(!PyArg_ParseTuple(args,":xroot")) return NULL;
  return PyLong_FromLong(self->xroot);
}

PyObject *ActionData_yroot(ActionData *self, PyObject *args)
{
  if(!PyArg_ParseTuple(args,":yroot")) return NULL;
  return PyLong_FromLong(self->yroot);
}

PyObject *ActionData_time(ActionData *self, PyObject *args)
{
  if(!PyArg_ParseTuple(args,":time")) return NULL;
  return PyLong_FromLong(self->time);
}

static PyMethodDef ActionData_methods[] = {
  {"action", (PyCFunction)ActionData_action, METH_VARARGS,
   "Return the action being executed."},
  {"window", (PyCFunction)ActionData_window, METH_VARARGS,
   "Return the client window id."},
  {"target", (PyCFunction)ActionData_target, METH_VARARGS,
   "Return the target type that the action is occuring on."},
  {"modifiers", (PyCFunction)ActionData_modifiers, METH_VARARGS,
   "Return the modifier keys state."},
  {"button", (PyCFunction)ActionData_button, METH_VARARGS,
   "Return the number of the pressed button (1-5)."},
  {"xroot", (PyCFunction)ActionData_xroot, METH_VARARGS,
   "Return the X-position of the mouse cursor on the root window."},
  {"yroot", (PyCFunction)ActionData_yroot, METH_VARARGS,
   "Return the Y-position of the mouse cursor on the root window."},
  {"time", (PyCFunction)ActionData_time, METH_VARARGS,
   "Return the time at which the event occured."},
  {NULL, NULL, 0, NULL}
};

PyObject *BindingData_window(BindingData *self, PyObject *args)
{
  if(!PyArg_ParseTuple(args,":window")) return NULL;
  return PyLong_FromLong(self->window);
}

PyObject *BindingData_modifiers(BindingData *self, PyObject *args)
{
  if(!PyArg_ParseTuple(args,":modifiers")) return NULL;
  return PyLong_FromUnsignedLong(self->state);
}

PyObject *BindingData_key(BindingData *self, PyObject *args)
{
  if(!PyArg_ParseTuple(args,":key")) return NULL;
  return PyString_FromString(
    XKeysymToString(XKeycodeToKeysym(otk::OBDisplay::display, self->key, 0)));

}

PyObject *BindingData_time(BindingData *self, PyObject *args)
{
  if(!PyArg_ParseTuple(args,":time")) return NULL;
  return PyLong_FromLong(self->time);
}

static PyMethodDef BindingData_methods[] = {
  {"window", (PyCFunction)BindingData_window, METH_VARARGS,
   "Return the client window id."},
  {"modifiers", (PyCFunction)BindingData_modifiers, METH_VARARGS,
   "Return the modifier keys state."},
  {"key", (PyCFunction)BindingData_key, METH_VARARGS,
   "Return the name of the pressed key."},
  {"time", (PyCFunction)BindingData_time, METH_VARARGS,
   "Return the time at which the event occured."},
  {NULL, NULL, 0, NULL}
};

static PyObject *ActionDataGetAttr(PyObject *obj, char *name)
{
  return Py_FindMethod(ActionData_methods, obj, name);
}

static PyObject *BindingDataGetAttr(PyObject *obj, char *name)
{
  return Py_FindMethod(BindingData_methods, obj, name);
}

static PyTypeObject ActionData_Type = {
  PyObject_HEAD_INIT(NULL)
  0,
  "ActionData",
  sizeof(ActionData),
  0,
  (destructor)ActionDataDealloc,
  0,
  (getattrfunc)ActionDataGetAttr,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

static PyTypeObject BindingData_Type = {
  PyObject_HEAD_INIT(NULL)
  0,
  "BindingData",
  sizeof(BindingData),
  0,
  (destructor)BindingDataDealloc,
  0,
  (getattrfunc)BindingDataGetAttr,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

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
  ActionData_Type.ob_type = &PyType_Type;
  BindingData_Type.ob_type = &PyType_Type;
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

static void call(PyObject *func, PyObject *data)
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

void python_callback(PyObject *func, OBActions::ActionType action,
                     Window window, OBWidget::WidgetType type,
                     unsigned int state, unsigned int button,
                     int xroot, int yroot, Time time)
{
  assert(func);
  
  ActionData *data = PyObject_New(ActionData, &ActionData_Type);
  data->action = action;
  data->window = window;
  data->type   = type;
  data->state  = state;
  data->button = button;
  data->xroot  = xroot;
  data->yroot  = yroot;
  data->time   = time;

  call(func, (PyObject*)data);
  Py_DECREF(data);
}

void python_callback(PyObject *func, Window window, unsigned int state,
			 unsigned int key, Time time)
{
  if (!func) return;

  BindingData *data = PyObject_New(BindingData, &BindingData_Type);
  data->window = window;
  data->state  = state;
  data->key    = key;
  data->time   = time;

  call(func, (PyObject*)data);
  Py_DECREF(data);
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

}
