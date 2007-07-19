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
  Atom       _restack;
  XEvent     msg;
  int        x=10,y=10,h=100,w=400;

  display = XOpenDisplay(NULL);

  if (display == NULL) {
    fprintf(stderr, "couldn't connect to X server :0\n");
    return 0;
  }

  _restack = XInternAtom(display, "_NET_RESTACK_WINDOW", False);

  win = XCreateWindow(display, RootWindow(display, 0),
                      x, y, w, h, 10, CopyFromParent, CopyFromParent,
                      CopyFromParent, 0, NULL);
  XSetWindowBackground(display,win,WhitePixel(display,0));

  XMapWindow(display, win);
  XFlush(display);

  printf("requesting bottom in 3\n");
  sleep(3);

  msg.xclient.type = ClientMessage;
  msg.xclient.message_type = _restack;
  msg.xclient.display = display;
  msg.xclient.window = win;
  msg.xclient.format = 32;
  msg.xclient.data.l[0] = 2l;
  msg.xclient.data.l[1] = 0l;
  msg.xclient.data.l[2] = Below;
  msg.xclient.data.l[3] = 0l;
  msg.xclient.data.l[4] = 0l;
  XSendEvent(display, RootWindow(display, 0), False,
             SubstructureNotifyMask | SubstructureRedirectMask, &msg);
  XFlush(display);

  printf("requesting top in 3\n");
  sleep(3);

  msg.xclient.type = ClientMessage;
  msg.xclient.message_type = _restack;
  msg.xclient.display = display;
  msg.xclient.window = win;
  msg.xclient.format = 32;
  msg.xclient.data.l[0] = 2l;
  msg.xclient.data.l[1] = 0l;
  msg.xclient.data.l[2] = Above;
  msg.xclient.data.l[3] = 0l;
  msg.xclient.data.l[4] = 0l;
  XSendEvent(display, RootWindow(display, 0), False,
             SubstructureNotifyMask | SubstructureRedirectMask, &msg);
  XFlush(display);

  printf("requesting bottomif in 3\n");
  sleep(3);

  msg.xclient.type = ClientMessage;
  msg.xclient.message_type = _restack;
  msg.xclient.display = display;
  msg.xclient.window = win;
  msg.xclient.format = 32;
  msg.xclient.data.l[0] = 2l;
  msg.xclient.data.l[1] = 0l;
  msg.xclient.data.l[2] = BottomIf;
  msg.xclient.data.l[3] = 0l;
  msg.xclient.data.l[4] = 0l;
  XSendEvent(display, RootWindow(display, 0), False,
             SubstructureNotifyMask | SubstructureRedirectMask, &msg);
  XFlush(display);

  printf("requesting topif in 3\n");
  sleep(3);

  msg.xclient.type = ClientMessage;
  msg.xclient.message_type = _restack;
  msg.xclient.display = display;
  msg.xclient.window = win;
  msg.xclient.format = 32;
  msg.xclient.data.l[0] = 2l;
  msg.xclient.data.l[1] = 0l;
  msg.xclient.data.l[2] = TopIf;
  msg.xclient.data.l[3] = 0l;
  msg.xclient.data.l[4] = 0l;
  XSendEvent(display, RootWindow(display, 0), False,
             SubstructureNotifyMask | SubstructureRedirectMask, &msg);
  XFlush(display);

  printf("requesting opposite in 3\n");
  sleep(3);

  msg.xclient.type = ClientMessage;
  msg.xclient.message_type = _restack;
  msg.xclient.display = display;
  msg.xclient.window = win;
  msg.xclient.format = 32;
  msg.xclient.data.l[0] = 2l;
  msg.xclient.data.l[1] = 0l;
  msg.xclient.data.l[2] = Opposite;
  msg.xclient.data.l[3] = 0l;
  msg.xclient.data.l[4] = 0l;
  XSendEvent(display, RootWindow(display, 0), False,
             SubstructureNotifyMask | SubstructureRedirectMask, &msg);
  XFlush(display);

  while (1) {
    XNextEvent(display, &report);
  }

  return 1;
}
