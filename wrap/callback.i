// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

%include "std_string.i"

%{
/*
  Calls a python callback for the MouseCallback function type
 */
static void PythonMouseCallback(ob::MouseData *data, void *pyfunc)
{
  PyObject *arglist, *pdata, *result;

  pdata = SWIG_NewPointerObj((void *) data, SWIGTYPE_p_ob__MouseData, 0);
  arglist = Py_BuildValue("(O)", pdata);
  Py_DECREF(pdata);

  // call the callback
  result = PyEval_CallObject((PyObject*)pyfunc, arglist);
  if (!result || PyErr_Occurred()) {
    // an exception occured in the script, display it
    PyErr_Print();
  }

  Py_XDECREF(result);
  Py_DECREF(arglist);
}

/*
  Calls a python callback for the KeyCallback function type
 */
static void PythonKeyCallback(ob::KeyData *data, void *pyfunc)
{
  PyObject *arglist, *pdata, *result;

  pdata = SWIG_NewPointerObj((void *) data, SWIGTYPE_p_ob__KeyData, 0);
  arglist = Py_BuildValue("(O)", pdata);
  Py_DECREF(pdata);

  // call the callback
  result = PyEval_CallObject((PyObject*)pyfunc, arglist);
  if (!result || PyErr_Occurred()) {
    // an exception occured in the script, display it
    PyErr_Print();
  }

  Py_XDECREF(result);
  Py_DECREF(arglist);
}

/*
  Calls a python callback for the EventCallback function type
 */
static void PythonEventCallback(ob::EventData *data, void *pyfunc)
{
  PyObject *arglist, *pdata, *result;

  pdata = SWIG_NewPointerObj((void *) data, SWIGTYPE_p_ob__EventData, 0);
  arglist = Py_BuildValue("(O)", pdata);
  Py_DECREF(pdata);

  // call the callback
  result = PyEval_CallObject((PyObject*)pyfunc, arglist);
  if (!result || PyErr_Occurred()) {
    // an exception occured in the script, display it
    PyErr_Print();
  }

  Py_XDECREF(result);
  Py_DECREF(arglist);
}
%}

// for all of these, PyErr_SetString is called before they return a false!
%exception mbind {
  $action
  if (!result) return NULL;
}
%exception kbind {
  $action
  if (!result) return NULL;
}
%exception ebind {
  $action
  if (!result) return NULL;
}
%exception kgrab {
  $action
  if (!result) return NULL;
}
%exception mgrab {
  $action
  if (!result) return NULL;
}

// Grab a Python function object as a Python object.
%typemap(python,in) PyObject *func {
  if (!PyCallable_Check($input)) {
    PyErr_SetString(PyExc_TypeError, "Excepting a callable object.");
    return NULL;
  }
  $1 = $input;
}

%inline %{
#include "bindings.hh"
  
bool mbind(const std::string &button, ob::MouseContext::MC context,
           ob::MouseAction::MA action, PyObject *func)
{
  if(context < 0 || context >= ob::MouseContext::NUM_MOUSE_CONTEXT) {
    PyErr_SetString(PyExc_ValueError, "Invalid MouseContext");
    return false;
  }
  if(action < 0 || action >= ob::MouseAction::NUM_MOUSE_ACTION) {
    PyErr_SetString(PyExc_ValueError, "Invalid MouseAction");
    return false;
  }
  
  if (!ob::openbox->bindings()->addButton(button, context,
                                          action, PythonMouseCallback, func)) {
    PyErr_SetString(PyExc_RuntimeError,"Unable to add binding.");
    return false;
  }
  Py_INCREF(func);
  return true;
}

bool ebind(ob::EventAction::EA action, PyObject *func)
{
  if(action < 0 || action >= ob::EventAction::NUM_EVENT_ACTION) {
    PyErr_SetString(PyExc_ValueError, "Invalid EventAction");
    return false;
  }

  if (!ob::openbox->bindings()->addEvent(action, PythonEventCallback, func)) {
    PyErr_SetString(PyExc_RuntimeError,"Unable to add binding.");
    return false;
  }
  Py_INCREF(func);
  return true;
}

bool kgrab(int screen, PyObject *func)
{
  if (!ob::openbox->bindings()->grabKeyboard(screen,
                                             PythonKeyCallback, func)) {
    PyErr_SetString(PyExc_RuntimeError,"Unable to grab keybaord.");
    return false;
  }
  Py_INCREF(func);
  return true;
}

void kungrab()
{
  ob::openbox->bindings()->ungrabKeyboard();
}

bool mgrab(int screen)
{
  if (!ob::openbox->bindings()->grabPointer(screen)) {
    PyErr_SetString(PyExc_RuntimeError,"Unable to grab pointer.");
    return false;
  }
  return true;
}

void mungrab()
{
  ob::openbox->bindings()->ungrabPointer();
}

bool kbind(PyObject *keylist, ob::KeyContext::KC context, PyObject *func)
{
  if (!PyList_Check(keylist)) {
    PyErr_SetString(PyExc_TypeError, "Invalid keylist. Not a list.");
    return false;
  }
  if(context < 0 || context >= ob::KeyContext::NUM_KEY_CONTEXT) {
    PyErr_SetString(PyExc_ValueError, "Invalid KeyContext");
    return false;
  }

  ob::Bindings::StringVect vectkeylist;
  for (int i = 0, end = PyList_Size(keylist); i < end; ++i) {
    PyObject *str = PyList_GetItem(keylist, i);
    if (!PyString_Check(str)) {
      PyErr_SetString(PyExc_TypeError,
                     "Invalid keylist. It must contain only strings.");
      return false;
    }
    vectkeylist.push_back(PyString_AsString(str));
  }

  (void)context; // XXX use this sometime!
  if (!ob::openbox->bindings()->addKey(vectkeylist, PythonKeyCallback, func)) {
    PyErr_SetString(PyExc_RuntimeError,"Unable to add binding.");
    return false;
  }
  Py_INCREF(func);
  return true;
}

%};
