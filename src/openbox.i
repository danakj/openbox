// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

%module openbox

%{
#ifdef HAVE_CONFIG_H
#  include "../config.h"
#endif

#include "openbox.hh"
#include "screen.hh"
#include "client.hh"
#include "bindings.hh"
#include "actions.hh"
%}

%include "stl.i"
%include "exception.i"
//%include std_list.i
//%template(ClientList) std::list<OBClient*>;

%ignore ob::Openbox::instance;
%inline %{
  ob::Openbox *Openbox_instance() { return ob::Openbox::instance; }
%};

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
%import "actions.hh"

%include "openbox.hh"
%include "screen.hh"
%include "client.hh"
%include "python.hh"

// for Mod1Mask etc
%include "X11/X.h"
