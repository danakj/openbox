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

/* Will find the frame for this window and stack itself right above it */
#define GO_ABOVE 0x4c0003c

#include <stdio.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

int main () {
  Display   *display;
  Window     win, frame, a, p, *c;
  unsigned int n;
  XWindowChanges changes;
  XEvent     report;
  XEvent     msg;
  int        x=10,y=10,h=100,w=400;

  display = XOpenDisplay(NULL);

  if (display == NULL) {
    fprintf(stderr, "couldn't connect to X server :0\n");
    return 0;
  }

  win = XCreateWindow(display, RootWindow(display, 0),
                      x, y, w, h, 10, CopyFromParent, CopyFromParent,
                      CopyFromParent, 0, NULL);
  XSetWindowBackground(display,win,WhitePixel(display,0));

  XMapWindow(display, win);
  XFlush(display);

  printf("requesting move in 10\n");
  sleep(10);

  frame = win;
  while (XQueryTree(display, frame, &a, &p, &c, &n) && p != a)
      frame = p;

  changes.sibling = GO_ABOVE;
  while (XQueryTree(display, changes.sibling, &a, &p, &c, &n) && p != a)
      changes.sibling = p;

  changes.stack_mode = Above;
  XConfigureWindow(display, frame, CWSibling | CWStackMode, &changes);
  XFlush(display);

  printf("moved 0x%lx above 0x%lx\n", frame, changes.sibling);

  while (1) {
    XNextEvent(display, &report);
  }

  return 1;
}
