// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __python_hh
#define   __python_hh

/*! @file python.hh
  @brief wee
*/

#include "actions.hh"
#include "widget.hh"
#include "bindings.hh"

extern "C" {
#include <Python.h>
}

#include <string>

namespace ob {

enum MouseContext {
  MC_Frame,
  MC_Titlebar,
  MC_Window,
  MC_MaximizeButton,
  MC_CloseButton,
  MC_IconifyButton,
  MC_StickyButton,
  MC_Grip,
  MC_Root,
  MC_MenuItem,
  MC_All,
  NUM_MOUSE_CONTEXT
}
enum KeyContext {
  KC_Menu,
  KC_All,
  NUM_KEY_CONTEXT
}

#ifndef SWIG
void python_init(char *argv0);
void python_destroy();
bool python_exec(const std::string &path);
                 
void python_callback(PyObject *func, OBActions::ActionType action,
                     Window window, OBWidget::WidgetType type,
                     unsigned int state, unsigned int button,
                     int xroot, int yroot, Time time);

void python_callback(PyObject *func, Window window, unsigned int state,
                     unsigned int key, Time time);


bool python_get_string(const char *name, std::string *value);
bool python_get_stringlist(const char *name, std::vector<std::string> *value);
#endif

}

#endif // __python_hh
