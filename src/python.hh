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

//! Add a python callback funtion to the back of the hook list
bool python_register(int action, PyObject *callback);
//! Add a python callback funtion to the front of the hook list
bool python_preregister(int action, PyObject *callback);
//! Remove a python callback function from the hook list
bool python_unregister(int action, PyObject *callback);

//! Fire a python callback function
void python_callback(OBActions::ActionType action, Window window,
                     OBWidget::WidgetType type, unsigned int state,
                     long d1 = LONG_MIN, long d2 = LONG_MIN,
                     long d3 = LONG_MIN, long d4 = LONG_MIN);

}

#endif // __python_hh
