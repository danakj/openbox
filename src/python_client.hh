// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __pythonclient_hh
#define   __pythonclient_hh

/*! @file python_client.hh
  @brief Python stuff
*/

#include "python.hh"
#include "client.hh"

namespace ob {

extern "C" {

typedef struct {
  PyObject_HEAD
  Window window;
  OBClient *client;
} PyClientObject;

extern PyTypeObject PyClient_Type;

PyObject *get_client_dict(PyObject* self, PyObject* args);

}
}

#endif // __pythonclient_hh
