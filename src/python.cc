// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#include "python.hh"

#include <vector>
#include <algorithm>

namespace ob {

typedef std::vector<PyObject*> FunctionList;

static FunctionList callbacks[OBActions::NUM_ACTIONS];

bool python_register(int action, PyObject *callback)
{
  if (action < 0 || action >= OBActions::NUM_ACTIONS) {
    PyErr_SetString(PyExc_AssertionError, "Invalid action type.");
    return false;
  }
  if (!PyCallable_Check(callback)) {
    PyErr_SetString(PyExc_AssertionError, "Invalid callback function.");
    return false;
  }
  
  FunctionList::iterator it = std::find(callbacks[action].begin(),
					callbacks[action].end(),
					callback);
  if (it == callbacks[action].end()) { // not already in there
    Py_XINCREF(callback);              // Add a reference to new callback
    callbacks[action].push_back(callback);
  }
  return true;
}

bool python_preregister(int action, PyObject *callback)
{
  if (action < 0 || action >= OBActions::NUM_ACTIONS) {
    PyErr_SetString(PyExc_AssertionError, "Invalid action type.");
    return false;
  }
  if (!PyCallable_Check(callback)) {
    PyErr_SetString(PyExc_AssertionError, "Invalid callback function.");
    return false;
  }
  
  FunctionList::iterator it = std::find(callbacks[action].begin(),
					callbacks[action].end(),
					callback);
  if (it == callbacks[action].end()) { // not already in there
    Py_XINCREF(callback);              // Add a reference to new callback
    callbacks[action].insert(callbacks[action].begin(), callback);
  }
  return true;
}

bool python_unregister(int action, PyObject *callback)
{
  if (action < 0 || action >= OBActions::NUM_ACTIONS) {
    PyErr_SetString(PyExc_AssertionError, "Invalid action type.");
    return false;
  }
  if (!PyCallable_Check(callback)) {
    PyErr_SetString(PyExc_AssertionError, "Invalid callback function.");
    return false;
  }

  FunctionList::iterator it = std::find(callbacks[action].begin(),
					callbacks[action].end(),
					callback);
  if (it != callbacks[action].end()) { // its been registered before
    Py_XDECREF(*it);                   // Dispose of previous callback
    callbacks[action].erase(it);
  }
  return true;
}

bool python_unregister_all(int action)
{
  if (action < 0 || action >= OBActions::NUM_ACTIONS) {
    PyErr_SetString(PyExc_AssertionError, "Invalid action type.");
    return false;
  }

  while (!callbacks[action].empty()) {
    Py_XDECREF(callbacks[action].back());
    callbacks[action].pop_back();
  }
  return true;
}

void python_callback(OBActions::ActionType action, Window window,
                     OBWidget::WidgetType type, unsigned int state,
                     long d1, long d2, long d3, long d4)
{
  PyObject *arglist;
  PyObject *result;

  assert(action >= 0 && action < OBActions::NUM_ACTIONS);

  if (d4 != LONG_MIN)
    arglist = Py_BuildValue("iliillll", action, window, type, state,
                            d1, d2, d3, d4);
  else if (d3 != LONG_MIN)
    arglist = Py_BuildValue("iliilll", action, window, type, state,
                            d1, d2, d3);
  else if (d2 != LONG_MIN)
    arglist = Py_BuildValue("iliill", action, window, type, state, d1, d2);
  else if (d1 != LONG_MIN)
    arglist = Py_BuildValue("iliil", action, window, type, state, d1);
  else
    arglist = Py_BuildValue("ilii", action, window, type, state);

  FunctionList::iterator it, end = callbacks[action].end();
  for (it = callbacks[action].begin(); it != end; ++it) {
    // call the callback
    result = PyEval_CallObject(*it, arglist);
    if (result) {
      Py_DECREF(result);
    } else {
      // an exception occured in the script, display it
      PyErr_Print();
    }
  }

  Py_DECREF(arglist);
}

}
