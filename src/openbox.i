// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

%module openbox

%{
#ifdef HAVE_CONFIG_H
#  include "../config.h"
#endif

#include "openbox.hh"
#include "screen.hh"
#include "client.hh"
#include "bindings.hh"
#include "actions.hh"
%}

%include stl.i
%include exception.i
//%include std_list.i
//%template(ClientList) std::list<OBClient*>;

%ignore ob::Openbox::instance;
%inline %{
  ob::Openbox *Openbox_instance() { return ob::Openbox::instance; }
%};

// stuff for scripting callbacks!
%inline %{
  enum ActionType {
    Action_ButtonPress,
    Action_ButtonRelease,
    Action_Click,
    Action_DoubleClick,
    Action_EnterWindow,
    Action_LeaveWindow,
    Action_KeyPress,
    Action_MouseMotion
  };
  enum WidgetType {
    Type_Frame,
    Type_Titlebar,
    Type_Handle,
    Type_Plate,
    Type_Label,
    Type_MaximizeButton,
    Type_CloseButton,
    Type_IconifyButton,
    Type_StickyButton,
    Type_LeftGrip,
    Type_RightGrip,
    Type_Client,
    Type_Root
  };
%}
%rename(register) python_register;
%inline %{
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

PyObject * unregister(int action, PyObject *func)
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

PyObject * unregister_all(int action)
{
  if (!ob::Openbox::instance->actions()->unregisterAllCallbacks(
        (ob::OBActions::ActionType)action)) {
    PyErr_SetString(PyExc_RuntimeError,
                    "Unable to unregister action callbacks.");
    return NULL;
  }
  Py_INCREF(Py_None); return Py_None;
}

PyObject * bind(PyObject *keylist, PyObject *func)
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

  if (!ob::Openbox::instance->bindings()->add(vectkeylist, func)) {
    PyErr_SetString(PyExc_RuntimeError,"Unable to add binding.");
    return NULL;
  }
  Py_INCREF(Py_None); return Py_None;
}

PyObject * unbind(PyObject *keylist)
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

  ob::Openbox::instance->bindings()->remove(vectkeylist);
  Py_INCREF(Py_None); return Py_None;
}

void unbind_all()
{
  ob::Openbox::instance->bindings()->removeAll();
}

void set_reset_key(const std::string &key)
{
  ob::Openbox::instance->bindings()->setResetKey(key);
}
%}

%ignore ob::OBScreen::clients;
%{
  #include <iterator>
%}
%extend ob::OBScreen {
  OBClient *client(int i) {
    if (i >= (int)self->clients.size())
      return NULL;
    ob::OBScreen::ClientList::iterator it = self->clients.begin();
    std::advance(it,i);
    return *it;
  }
  int clientCount() const {
    return (int) self->clients.size();
  }
};

%import "../otk/eventdispatcher.hh"
%import "../otk/eventhandler.hh"
%import "widget.hh"
%import "actions.hh"

%include "openbox.hh"
%include "screen.hh"
%include "client.hh"

// for Mod1Mask etc
%include "X11/X.h"
