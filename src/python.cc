// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#include "python.hh"
#include "openbox.hh"
#include "otk/display.hh"

#include <vector>
#include <algorithm>

namespace ob {

typedef std::vector<PyObject*> FunctionList;

static FunctionList callbacks[OBActions::NUM_ACTIONS];
static FunctionList bindfuncs;

bool python_register(int action, PyObject *callback)
{
  if (action < 0 || action >= OBActions::NUM_ACTIONS ||
      action == OBActions::Action_KeyPress) {
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
  if (action < 0 || action >= OBActions::NUM_ACTIONS ||
      action == OBActions::Action_KeyPress) {
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
  if (action < 0 || action >= OBActions::NUM_ACTIONS ||
      action == OBActions::Action_KeyPress) {
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






bool python_bind_key(PyObject *keylist, PyObject *callback)
{
  if (!PyList_Check(keylist)) {
    PyErr_SetString(PyExc_AssertionError, "Invalid keylist. Not a list.");
    return false;
  }
  if (!PyCallable_Check(callback)) {
    PyErr_SetString(PyExc_AssertionError, "Invalid callback function.");
    return false;
  }

  OBBindings::StringVect vectkeylist;
  for (int i = 0, end = PyList_Size(keylist); i < end; ++i) {
    PyObject *str = PyList_GetItem(keylist, i);
    if (!PyString_Check(str)) {
      PyErr_SetString(PyExc_AssertionError,
                      "Invalid keylist. It must contain only strings.");
      return false;
    }
    vectkeylist.push_back(PyString_AsString(str));
  }

  // the id is what the binding class can call back with so it doesnt have to
  // worry about the python function pointer
  int id = bindfuncs.size();
  if (Openbox::instance->bindings()->add_key(vectkeylist, id)) {
    Py_XINCREF(callback);              // Add a reference to new callback
    bindfuncs.push_back(callback);
    return true;
  } else {
    PyErr_SetString(PyExc_AssertionError,"Unable to create binding. Invalid.");
    return false;
  }
}

bool python_unbind_key(PyObject *keylist)
{
  if (!PyList_Check(keylist)) {
    PyErr_SetString(PyExc_AssertionError, "Invalid keylist. Not a list.");
    return false;
  }

  OBBindings::StringVect vectkeylist;
  for (int i = 0, end = PyList_Size(keylist); i < end; ++i) {
    PyObject *str = PyList_GetItem(keylist, i);
    if (!PyString_Check(str)) {
      PyErr_SetString(PyExc_AssertionError,
                      "Invalid keylist. It must contain only strings.");
      return false;
    }
    vectkeylist.push_back(PyString_AsString(str));
  }

  int id;
  if ((id =
       Openbox::instance->bindings()->remove_key(vectkeylist)) >= 0) {
    assert(bindfuncs[id]); // shouldn't be able to remove it twice
    Py_XDECREF(bindfuncs[id]);  // Dispose of previous callback
    // important note: we don't erase the item from the list cuz that would
    // ruin all the id's that are in use. simply nullify it.
    bindfuncs[id] = 0;
    return true;
  }
  
  return false;
}

void python_set_reset_key(const std::string &key)
{
  Openbox::instance->bindings()->setResetKey(key);
}

bool python_bind_mouse(const std::string &button, PyObject *callback)
{
  if (!PyCallable_Check(callback)) {
    PyErr_SetString(PyExc_AssertionError, "Invalid callback function.");
    return false;
  }

  // the id is what the binding class can call back with so it doesnt have to
  // worry about the python function pointer
  int id = bindfuncs.size();
  if (Openbox::instance->bindings()->add_mouse(button, id)) {
    Py_XINCREF(callback);              // Add a reference to new callback
    bindfuncs.push_back(callback);
    return true;
  } else {
    PyErr_SetString(PyExc_AssertionError,"Unable to create binding. Invalid.");
    return false;
  }
}

bool python_unbind_mouse(const std::string &button)
{
  int id;
  if ((id =
       Openbox::instance->bindings()->remove_mouse(button)) >= 0) {
    assert(bindfuncs[id]); // shouldn't be able to remove it twice
    Py_XDECREF(bindfuncs[id]);  // Dispose of previous callback
    // important note: we don't erase the item from the list cuz that would
    // ruin all the id's that are in use. simply nullify it.
    bindfuncs[id] = 0;
    return true;
  }
  
  return false;
}

void python_unbind_all()
{
  Openbox::instance->bindings()->remove_all();
}


void python_callback_binding(int id, OBActions::ActionType action,
                             Window window, unsigned int state,
                             unsigned int keybutton, Time time)
{
  assert(action >= 0 && action < OBActions::NUM_ACTIONS);

  if (!bindfuncs[id]) return; // the key was unbound

  PyObject *arglist;
  PyObject *result;

  arglist = Py_BuildValue("ilisl", action, window, state,
                          XKeysymToString(
                            XKeycodeToKeysym(otk::OBDisplay::display,
                                             keybutton, 0)),
                          time);

  // call the callback
  result = PyEval_CallObject(bindfuncs[id], arglist);
  if (result) {
    Py_DECREF(result);
  } else {
    // an exception occured in the script, display it
    PyErr_Print();
  }

  Py_DECREF(arglist);
}

}
