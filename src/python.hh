// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __python_hh
#define   __python_hh

/*! @file python.hh
  @brief wee
*/

#include "otk/point.hh"
#include "otk/rect.hh"
#include "otk/property.hh"
#include "otk/display.hh"
#include "otk/ustring.hh"

extern "C" {
#include <X11/Xlib.h>
#include <Python.h>
}

#include <string>
#include <vector>

namespace ob {

class Client;

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
  EventPlaceWindow,
  EventNewWindow,
  EventCloseWindow,
  EventStartup,
  EventShutdown,
  EventFocus,
  EventBell,
  NUM_EVENTS
};

class MouseData {
public:
  int screen;
  Client *client;
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

  MouseData(int screen, Client *client, Time time, unsigned int state,
            unsigned int button, MouseContext context, MouseAction action,
            int xroot, int yroot, const otk::Point &initpos,
            const otk::Rect &initarea) {
    this->screen = screen;
    this->client = client;
    this->time   = time;
    this->state  = state;
    this->button = button;
    this->context= context;
    this->action = action;
    this->xroot  = xroot;
    this->yroot  = yroot;
    this->pressx = initpos.x();
    this->pressy = initpos.y();
    this->press_clientx      = initarea.x();
    this->press_clienty      = initarea.y();
    this->press_clientwidth  = initarea.width();
    this->press_clientheight = initarea.height();
  }
  MouseData(int screen, Client *client, Time time, unsigned int state,
            unsigned int button, MouseContext context, MouseAction action) {
    this->screen = screen;
    this->client = client;
    this->time   = time;
    this->state  = state;
    this->button = button;
    this->context= context;
    this->action = action;
    this->xroot  = xroot;
    this->yroot  = yroot;
    this->pressx = 0;
    this->pressy = 0;
    this->press_clientx      = 0;
    this->press_clienty      = 0;
    this->press_clientwidth  = 0;
    this->press_clientheight = 0;
  }
};

class EventData {
public:
  int screen;
  Client *client;
  unsigned int state;
  EventAction action;

  EventData(int screen, Client *client, EventAction action,
            unsigned int state) {
    this->screen = screen;
    this->client = client;
    this->action = action;
    this->state  = state;
  }
};

class KeyData {
public:
  int screen;
  Client *client;
  Time time;
  unsigned int state;
  std::string key;

  KeyData(int screen, Client *client, Time time, unsigned int state,
          unsigned int key) {
    this->screen = screen;
    this->client = client;
    this->time   = time;
    this->state  = state;
    this->key    = XKeysymToString(XKeycodeToKeysym(**otk::display,
                                                    key, 0));
  }
};

#ifndef SWIG

void python_init(char *argv0);
void python_destroy();
bool python_exec(const std::string &path);

bool python_get_long(const char *name, long *value);
bool python_get_string(const char *name, otk::ustring *value);
bool python_get_stringlist(const char *name, std::vector<otk::ustring> *value);

/***********************************************
 * These are found in openbox.i, not python.cc *
 ***********************************************/
void python_callback(PyObject *func, MouseData *data);
void python_callback(PyObject *func, EventData *data);
void python_callback(PyObject *func, KeyData *data);

#endif // SWIG

PyObject *mbind(const std::string &button, ob::MouseContext context,
                ob::MouseAction action, PyObject *func);

PyObject *kbind(PyObject *keylist, ob::KeyContext context, PyObject *func);

PyObject *ebind(ob::EventAction action, PyObject *func);

void set_reset_key(const std::string &key);

PyObject *send_client_msg(Window target, Atom type, Window about,
                          long data, long data1 = 0, long data2 = 0,
                          long data3 = 0, long data4 = 0);
}


#endif // __python_hh
