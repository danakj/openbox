// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

%module openbox

%{
#ifdef HAVE_CONFIG_H
#  include "../config.h"
#endif

#include "openbox.hh"
#include "screen.hh"
#include "client.hh"
%}

%include stl.i
//%include std_list.i
//%template(ClientList) std::list<OBClient*>;


%ignore ob::Openbox::instance;
%ignore ob::OBScreen::clients;

%include "openbox.hh"
%include "screen.hh"
%include "client.hh"

%inline %{
  ob::Openbox *Openbox_instance() { return ob::Openbox::instance; }
%};

%{
  #include <iterator>
%}
%extend ob::OBScreen {
  OBClient *client(int i) {
    ob::OBScreen::ClientList::iterator it = self->clients.begin();
    std::advance(it,i);
    return *it;
  }
  int clientCount() const {
    return (int) self->clients.size();
  }
};
