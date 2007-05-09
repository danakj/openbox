/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   extentsrequest.c for the Openbox window manager
   Copyright (c) 2003-2007   Dana Jansens

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   See the COPYING file for a copy of the GNU General Public License.
*/

#include <stdio.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

int main () {
  Display   *display;
  Window     win;
  XEvent     report;
  Atom       _request, _extents, _type, _normal, _desktop;
  XEvent     msg;
  int        x=10,y=10,h=100,w=400;

  display = XOpenDisplay(NULL);

  if (display == NULL) {
    fprintf(stderr, "couldn't connect to X server :0\n");
    return 0;
  }

  _type = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
  _normal = XInternAtom(display, "_NET_WM_WINDOW_TYPE_NORMAL", False);
  _desktop = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DESKTOP", False);
  _request = XInternAtom(display, "_NET_REQUEST_FRAME_EXTENTS", False);
  _extents = XInternAtom(display, "_NET_FRAME_EXTENTS", False);

  win = XCreateWindow(display, RootWindow(display, 0),
		      x, y, w, h, 10, CopyFromParent, CopyFromParent,
		      CopyFromParent, 0, NULL);
  XSelectInput(display, win, PropertyChangeMask);

  printf("requesting for type normal\n");
  XChangeProperty(display, win, _type, XA_ATOM, 32,
                  PropModeReplace, (unsigned char*)&_normal, 1);
  msg.xclient.type = ClientMessage;
  msg.xclient.message_type = _request;
  msg.xclient.display = display;
  msg.xclient.window = win;
  msg.xclient.format = 32;
  msg.xclient.data.l[0] = 0l;
  msg.xclient.data.l[1] = 0l;
  msg.xclient.data.l[2] = 0l;
  msg.xclient.data.l[3] = 0l;
  msg.xclient.data.l[4] = 0l;
  XSendEvent(display, RootWindow(display, 0), False,
             SubstructureNotifyMask | SubstructureRedirectMask, &msg);
  XFlush(display);

  printf("waiting for extents\n");
  while (1) {
    XNextEvent(display, &report);

    if (report.type == PropertyNotify &&
        report.xproperty.atom == _extents)
    {
        Atom ret_type;
        int ret_format;
        unsigned long ret_items, ret_bytesleft;
        unsigned long *prop_return;
        XGetWindowProperty(display, win, _extents, 0, 4,
                           False, XA_CARDINAL, &ret_type, &ret_format,
                           &ret_items, &ret_bytesleft,
                           (unsigned char**) &prop_return);
        if (ret_type == XA_CARDINAL && ret_format == 32 && ret_items == 4)
            printf("got new extents %d, %d, %d, %d\n",
                   prop_return[0], prop_return[1], prop_return[2],
                   prop_return[3]);
        break;
    }
  }

  printf("requesting for type desktop\n");
  XChangeProperty(display, win, _type, XA_ATOM, 32,
                  PropModeReplace, (unsigned char*)&_desktop, 1);
  msg.xclient.type = ClientMessage;
  msg.xclient.message_type = _request;
  msg.xclient.display = display;
  msg.xclient.window = win;
  msg.xclient.format = 32;
  msg.xclient.data.l[0] = 0l;
  msg.xclient.data.l[1] = 0l;
  msg.xclient.data.l[2] = 0l;
  msg.xclient.data.l[3] = 0l;
  msg.xclient.data.l[4] = 0l;
  XSendEvent(display, RootWindow(display, 0), False,
	     SubstructureNotifyMask | SubstructureRedirectMask, &msg);
  XFlush(display);

  printf("waiting for extents\n");
  while (1) {
    XNextEvent(display, &report);

    if (report.type == PropertyNotify &&
        report.xproperty.atom == _extents)
    {
        Atom ret_type;
        int ret_format;
        unsigned long ret_items, ret_bytesleft;
        unsigned long *prop_return;
        XGetWindowProperty(display, win, _extents, 0, 4,
                           False, XA_CARDINAL, &ret_type, &ret_format,
                           &ret_items, &ret_bytesleft,
                           (unsigned char**) &prop_return);
        if (ret_type == XA_CARDINAL && ret_format == 32 && ret_items == 4)
            printf("got new extents %d, %d, %d, %d\n",
                   prop_return[0], prop_return[1], prop_return[2],
                   prop_return[3]);
        break;
    }
  }

  return 1;
}
