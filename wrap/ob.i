// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

%module ob

%{
#include <X11/Xlib.h>
#include "otk/display.hh"
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

%}

%include "ob_openbox.i"
%include "ob_screen.i"
%include "ob_client.i"
%include "ob_frame.i"
%include "ob_python.i"
%include "callback.i"

%import "otk.i"
// for Window etc
%import "X11/X.h"

%inline %{
#include <string>

void set_reset_key(const std::string &key)
{
  ob::openbox->bindings()->setResetKey(key);
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
