// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __python_hh
#define   __python_hh

/*! @file python.hh
  @brief wee
*/

extern "C" {
#include <X11/Xlib.h>
#include <Python.h>
}

#include <string>
#include <vector>

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
  NUM_MOUSE_CONTEXT
};

enum MouseAction {
  MousePress,
  MouseClick,
  MouseDoubleClick,
  MouseMotion,
  NUM_MOUSE_ACTION
};

enum KeyContext {
  KC_Menu,
  KC_All,
  NUM_KEY_CONTEXT
};

#ifndef SWIG
void python_init(char *argv0);
void python_destroy();
bool python_exec(const std::string &path);
                 
void python_callback(PyObject *func, MouseAction action,
                     Window window, MouseContext context,
                     unsigned int state, unsigned int button,
                     int xroot, int yroot, Time time);

void python_callback(PyObject *func, Window window, unsigned int state,
                     unsigned int key, Time time);


bool python_get_string(const char *name, std::string *value);
bool python_get_stringlist(const char *name, std::vector<std::string> *value);
#endif

PyObject * mbind(const std::string &button, ob::MouseContext context,
                 ob::MouseAction action, PyObject *func);

PyObject * kbind(PyObject *keylist, ob::KeyContext context, PyObject *func);
PyObject * kunbind(PyObject *keylist);
void kunbind_all();
void set_reset_key(const std::string &key);

}

#endif // __python_hh
