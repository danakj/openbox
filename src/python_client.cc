// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "python_client.hh"
#include "openbox.hh"

namespace ob {

extern "C" {

PyObject *get_client_dict(PyObject* self, PyObject* args)
{
  if (!PyArg_ParseTuple(args, ":get_client_dict"))
    return NULL;
  return PyDictProxy_New(Openbox::instance->pyclients());
}



PyObject *getWindow(PyObject* self, PyObject* args)
{
  if (!PyArg_ParseTuple(args, ":getWindow"))
    return NULL;
  return PyLong_FromLong(((PyClientObject*)self)->window);
}



static PyMethodDef attr_methods[] = {
  {"getWindow", getWindow, METH_VARARGS,
   "Return the window id."},
  {NULL, NULL, 0, NULL}           /* sentinel */
};

static PyObject *getattr(PyObject *obj, char *name)
{
  return Py_FindMethod(attr_methods, obj, name);
}



static void client_dealloc(PyObject* self)
{
  PyObject_Del(self);
}

PyTypeObject PyClient_Type = {
  PyObject_HEAD_INIT(NULL)
  0,
  "Client",
  sizeof(PyClientObject),
  0,
  client_dealloc, /*tp_dealloc*/
  0,          /*tp_print*/
  getattr,    /*tp_getattr*/
  0,          /*tp_setattr*/
  0,          /*tp_compare*/
  0,          /*tp_repr*/
  0,          /*tp_as_number*/
  0,          /*tp_as_sequence*/
  0,          /*tp_as_mapping*/
  0,          /*tp_hash */
};

}
}
