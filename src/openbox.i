// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

%module openbox

%{
#ifdef HAVE_CONFIG_H
#  include "../config.h"
#endif

#include "openbox.hh"
#include "screen.hh"
#include "client.hh"
#include "python.hh"
%}


%include stl.i
//%include std_list.i
//%template(ClientList) std::list<OBClient*>;

%ignore ob::Openbox::instance;
%inline %{
  ob::Openbox *Openbox_instance() { return ob::Openbox::instance; }
%};

// stuff for registering callbacks!

%inline %{
  enum ActionType {
    Action_ButtonPress,
    Action_ButtonRelease,
    Action_EnterWindow,
    Action_LeaveWindow,
    Action_KeyPress,
    Action_MouseMotion
  };
%}
%ignore ob::python_callback;
%rename(register) ob::python_register;
%rename(unregister) ob::python_unregister;

%ignore ob::OBScreen::clients;
%{
  #include <iterator>
%}
%extend ob::OBScreen {
  OBClient *client(int i) {
    if (i >= (int)self->clients.size())
      return NULL;
    ob::OBScreen::ClientList::iterator it = self->clients.begin();
    std::advance(it,i);
    return *it;
  }
  int clientCount() const {
    return (int) self->clients.size();
  }
};

%import "../otk/eventdispatcher.hh"
%import "../otk/eventhandler.hh"
%import "widget.hh"

%include "openbox.hh"
%include "screen.hh"
%include "client.hh"
%include "python.hh"

// for Mod1Mask etc
%include "X11/X.h"
