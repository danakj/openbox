// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __python_hh
#define   __python_hh

/*! @file python.hh
  @brief wee
*/

#include "otk/point.hh"
#include "otk/rect.hh"

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
  MC_Handle,
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

enum EventAction {
  EventEnterWindow,
  EventLeaveWindow,
  EventNewWindow,
  EventCloseWindow,
  NUM_EVENTS
};

#ifndef SWIG

// *** MotionData can be (and is) cast ButtonData!! (in actions.cc) *** //
typedef struct {
  PyObject_HEAD;
  Window window;
  Time time;
  unsigned int state;
  unsigned int button;
  MouseContext context;
  MouseAction action;
  int xroot;
  int yroot;
  int pressx;
  int pressy;
  int press_clientx;
  int press_clienty;
  int press_clientwidth;
  int press_clientheight;
} MotionData;

// *** MotionData can be (and is) cast ButtonData!! (in actions.cc) *** //
typedef struct {
  PyObject_HEAD;
  Window window;
  Time time;
  unsigned int state;
  unsigned int button;
  MouseContext context;
  MouseAction action;
} ButtonData;

typedef struct {
  PyObject_HEAD;
  Window window;
  unsigned int state;
  EventAction action;
} EventData;

typedef struct {
  PyObject_HEAD;
  Window window;
  Time time;
  unsigned int state;
  unsigned int key;
} KeyData;

void python_init(char *argv0);
void python_destroy();
bool python_exec(const std::string &path);
                 
MotionData *new_motion_data(Window window, Time time, unsigned int state,
                            unsigned int button, MouseContext context,
                            MouseAction action, int xroot, int yroot,
                            const otk::Point &initpos,
                            const otk::Rect &initarea);
ButtonData *new_button_data(Window window, Time time, unsigned int state,
                            unsigned int button, MouseContext context,
                            MouseAction action);
EventData *new_event_data(Window window, EventAction action,
                           unsigned int state);
KeyData *new_key_data(Window window, Time time, unsigned int state,
                      unsigned int key);

void python_callback(PyObject *func, PyObject *data);

bool python_get_long(const char *name, long *value);
bool python_get_string(const char *name, std::string *value);
bool python_get_stringlist(const char *name, std::vector<std::string> *value);
#endif

PyObject *mbind(const std::string &button, ob::MouseContext context,
                ob::MouseAction action, PyObject *func);

PyObject *kbind(PyObject *keylist, ob::KeyContext context, PyObject *func);

PyObject *ebind(ob::EventAction action, PyObject *func);

void set_reset_key(const std::string &key);

}

#endif // __python_hh
