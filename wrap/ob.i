// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

%module ob

%{
#include "config.h"

#include "frame.hh"
#include "openbox.hh"
#include "screen.hh"
#include "client.hh"
#include "bindings.hh"
#include "actions.hh"
#include "python.hh"
#include "otk/otk.hh"
%}

%include "stl.i"
//%include std_list.i
//%template(ClientList) std::list<Client*>;
%include "callback.i"

%immutable ob::openbox;
/*
%ignore ob::openbox;
%inline %{
  ob::Openbox *Openbox_instance() { return ob::openbox; }
%};
*/

%ignore ob::Screen::clients;
%{
  #include <iterator>
%}
%extend ob::Screen {
  Client *client(int i) {
    if (i < 0 || i >= (int)self->clients.size())
      return NULL;
    ob::Client::List::iterator it = self->clients.begin();
    std::advance(it,i);
    return *it;
  }
  int clientCount() const {
    return (int) self->clients.size();
  }
};

%import "otk.i"

%import "actions.hh"

%include "openbox.hh"
%include "screen.hh"
%include "client.hh"
%include "frame.hh"
%include "python.hh"

// for Window etc
%import "X11/X.h"

%inline %{
void set_reset_key(const std::string &key)
{
  ob::openbox->bindings()->setResetKey(key);
}

void send_client_msg(Window target, Atom type, Window about,
                     long data=0, long data1=0, long data2=0,
                     long data3=0, long data4=0)
{
  XEvent e;
  e.xclient.type = ClientMessage;
  e.xclient.format = 32;
  e.xclient.message_type = type;
  e.xclient.window = about;
  e.xclient.data.l[0] = data;
  e.xclient.data.l[1] = data1;
  e.xclient.data.l[2] = data2;
  e.xclient.data.l[3] = data3;
  e.xclient.data.l[4] = data4;

  XSendEvent(**otk::display, target, false,
             SubstructureRedirectMask | SubstructureNotifyMask,
             &e);
}

void execute(const std::string &bin, int screen=0)
{
  if (screen >= ScreenCount(**otk::display))
    screen = 0;
  otk::bexec(bin, otk::display->screenInfo(screen)->displayString());
}

%};

// globals
%pythoncode %{
openbox = cvar.openbox;
%}
