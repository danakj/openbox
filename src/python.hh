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

struct MouseContext {
  enum MC {
    Frame,
    Titlebar,
    Handle,
    Window,
    MaximizeButton,
    CloseButton,
    IconifyButton,
    AllDesktopsButton,
    Grip,
    Root,
    MenuItem,
    NUM_MOUSE_CONTEXT
  };
};

struct MouseAction {
  enum MA {
    Press,
    Click,
    DoubleClick,
    Motion,
    NUM_MOUSE_ACTION
  };
};

struct KeyContext {
  enum KC {
    Menu,
    All,
    NUM_KEY_CONTEXT
  };
};

struct KeyAction {
  enum KA {
    Press,
    Release,
    NUM_KEY_ACTION
  };
};

struct EventAction {
  enum EA {
    EnterWindow,
    LeaveWindow,
    PlaceWindow,
    NewWindow,
    CloseWindow,
    Startup,
    Shutdown,
    Focus,
    Bell,
    UrgentWindow,
    NUM_EVENTS
  };
};

class MouseData {
public:
  int screen;
  Client *client;
  Time time;
  unsigned int state;
  unsigned int button;
  MouseContext::MC context;
  MouseAction::MA action;
  int xroot;
  int yroot;
  int pressx;
  int pressy;
  int press_clientx;
  int press_clienty;
  int press_clientwidth;
  int press_clientheight;

  MouseData(int screen, Client *client, Time time, unsigned int state,
            unsigned int button, MouseContext::MC context,
            MouseAction::MA action, int xroot, int yroot,
            const otk::Point &initpos, const otk::Rect &initarea) {
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
            unsigned int button, MouseContext::MC context,
            MouseAction::MA action) {
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
  EventAction::EA action;

  EventData(int screen, Client *client, EventAction::EA action,
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
  char *key;
  KeyAction::KA action;

  KeyData(int screen, Client *client, Time time, unsigned int state,
          unsigned int key, KeyAction::KA action) {
    this->screen = screen;
    this->client = client;
    this->time   = time;
    this->state  = state;
    this->key    = XKeysymToString(XKeycodeToKeysym(**otk::display,
                                                    key, 0));
    this->action = action;
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

PyObject *mbind(const std::string &button, ob::MouseContext::MC context,
                ob::MouseAction::MA action, PyObject *func);

PyObject *kbind(PyObject *keylist, ob::KeyContext::KC context, PyObject *func);

PyObject *kgrab(int screen, PyObject *func);
PyObject *kungrab();

PyObject *ebind(ob::EventAction::EA action, PyObject *func);

void set_reset_key(const std::string &key);

PyObject *send_client_msg(Window target, Atom type, Window about,
                          long data, long data1 = 0, long data2 = 0,
                          long data3 = 0, long data4 = 0);


void execute(const std::string &bin, int screen=0);

}


#endif // __python_hh
