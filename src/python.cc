// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "python.hh"
#include "python_client.hh"
#include "openbox.hh"

namespace ob {

extern "C" {

static PyObject *shit(PyObject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, ":shit"))
    return NULL;

  printf("SHIT CALLED!@!\n");

  return Py_None;
}
  


static PyMethodDef OBMethods[] = {
  {"shit", shit, METH_VARARGS,
   "Do some shit, yo!"},

  {"get_client_dict", get_client_dict, METH_VARARGS,
   "Get the list of all clients"},

  {NULL, NULL, 0, NULL}
};

void initopenbox()
{
  PyClient_Type.ob_type = &PyType_Type;
  
  Py_InitModule("openbox", OBMethods);
}
}

}
