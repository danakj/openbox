// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

%module otk_timer

%{
#include "config.h"
#include "timer.hh"
%}

%{
  struct PythonCallbackData {
    PyObject *pyfunc;
    void *data;
  };
 
  /*
    Calls a python callback for the TimeoutHandler function type
  */
  static void PythonCallback(PythonCallbackData *calldata) {
    PyObject *arglist, *result;
      
    arglist = Py_BuildValue("(O)", calldata->data);
      
    // call the callback
    result = PyEval_CallObject((PyObject*)calldata->pyfunc, arglist);
    if (!result || PyErr_Occurred()) {
      // an exception occured in the script, display it
      PyErr_Print();
    }
      
    Py_XDECREF(result);
    Py_DECREF(arglist);
  }
%}

// Grab a Python function object as a Python object.
%typemap(python,in) PyObject *func {
  if (!PyCallable_Check($input)) {
    PyErr_SetString(PyExc_TypeError, "Excepting a callable object.");
    return NULL;
  }
  $1 = $input;
}

namespace otk {

%ignore Timer::Timer(long, TimeoutHandler, void*);
%ignore Timer::operator delete(void*);
%ignore Timer::initialize();
%ignore Timer::destroy();
%ignore Timer::dispatchTimers(bool);
%ignore Timer::nearestTimeout(struct timeval&);

%extend Timer {
  Timer(long, PyObject*, PyObject*);

  // if you don't call stop() before the object disappears, the timer will
  // keep firing forever
  void stop() {
    delete self;
  }
}

}

%{
  static otk::Timer *new_otk_Timer(long delay,
                                   PyObject *func, PyObject *data) {
    PythonCallbackData *d = new PythonCallbackData;
    d->pyfunc = func;
    d->data = data;
    return new otk::Timer(delay,
                          (otk::Timer::TimeoutHandler)&PythonCallback, d);
    // the PythonCallbackData is leaked.. XXX
  }
%}

%include "timer.hh"
