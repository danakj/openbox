// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __python_hh
#define   __python_hh

/*! @file python.hh
  @brief wee
*/

#include "actions.hh"
#include "widget.hh"

extern "C" {
#include <Python.h>
}

namespace ob {

bool python_register(int action, PyObject *callback);
bool python_unregister(int action, PyObject *callback);

void python_callback(OBActions::ActionType action, Window window,
                     OBWidget::WidgetType type, unsigned int state,
                     long d1 = 0, long d2 = 0);

}

#endif // __python_hh
