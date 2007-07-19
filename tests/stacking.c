/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   stacking.c for the Openbox window manager
   Copyright (c) 2007        Dana Jansens

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
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

int main () {
  Display   *display;
  Window     parent, child1, child2, group;
  XEvent     report;
  int        x=10,y=10,h=400,w=400;
  XWMHints  *hints;

  display = XOpenDisplay(NULL);

  if (display == NULL) {
    fprintf(stderr, "couldn't connect to X server :0\n");
    return 0;
  }

  group = XCreateWindow(display, RootWindow(display, 0),
                        x, y, w, h, 10, CopyFromParent, CopyFromParent,
                        CopyFromParent, 0, 0);
  parent = XCreateWindow(display, RootWindow(display, 0),
			 x, y, w, h, 10, CopyFromParent, CopyFromParent,
			 CopyFromParent, 0, 0);
  child1 = XCreateWindow(display, RootWindow(display, 0),
                         x, y, w/2, h/2, 10, CopyFromParent, CopyFromParent,
                         CopyFromParent, 0, 0);
  child2 = XCreateWindow(display, RootWindow(display, 0),
                         x, y, w/2, h/2, 10, CopyFromParent, CopyFromParent,
                         CopyFromParent, 0, 0);

  XSetWindowBackground(display,parent,WhitePixel(display,0));
  XSetWindowBackground(display,child1,BlackPixel(display,0));
  XSetWindowBackground(display,child2,WhitePixel(display,0));

  hints = XAllocWMHints();
  hints->flags = WindowGroupHint;
  hints->window_group = group;
  XSetWMHints(display, parent, hints);
  XSetWMHints(display, child1, hints);
  XSetWMHints(display, child2, hints);
  XFree(hints);

  XSetTransientForHint(display, child1,
                       RootWindow(display, DefaultScreen(display)));
  XSetTransientForHint(display, child2, parent);

  XMapWindow(display, parent);
  XFlush(display);
  sleep(3);
  XMapWindow(display, child1);
  XFlush(display);
  sleep(3);
  XMapWindow(display, child2);

  while (1) {
    XNextEvent(display, &report);
  }

  return 1;
}
