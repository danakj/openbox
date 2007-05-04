/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   grouptran2.c for the Openbox window manager
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
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

int main () {
  Display   *display;
  Window     main, grouptran, child, group;
  XEvent     report;
  XWMHints  *wmhints;

  display = XOpenDisplay(NULL);

  if (display == NULL) {
    fprintf(stderr, "couldn't connect to X server :0\n");
    return 0;
  }

  group = XCreateWindow(display, RootWindow(display, 0),
                        0,0,1,1, 10, CopyFromParent, CopyFromParent,
			 CopyFromParent, 0, 0);

  main = XCreateWindow(display, RootWindow(display, 0),
                      0,0,100,100, 10, CopyFromParent, CopyFromParent,
			 CopyFromParent, 0, 0);
  grouptran = XCreateWindow(display, RootWindow(display, 0),
                            10,10,80,180, 10, CopyFromParent, CopyFromParent,
                            CopyFromParent, 0, 0);
  child = XCreateWindow(display, RootWindow(display, 0),
                        20,20,60,60, 10, CopyFromParent, CopyFromParent,
                        CopyFromParent, 0, 0);

  XSetWindowBackground(display,main,WhitePixel(display,0)); 
  XSetWindowBackground(display,grouptran,BlackPixel(display,0)); 
  XSetWindowBackground(display,child,WhitePixel(display,0)); 

  XSetTransientForHint(display, grouptran, RootWindow(display,0));
  XSetTransientForHint(display, child, grouptran);

  wmhints = XAllocWMHints();

  wmhints->flags = WindowGroupHint;
  wmhints->window_group = group;

  XSetWMHints(display, main, wmhints);
  XSetWMHints(display, grouptran, wmhints);
  XSetWMHints(display, child, wmhints);

  XFree(wmhints);
  
  XMapWindow(display, main);
  XMapWindow(display, grouptran);
  XMapWindow(display, child);
  XFlush(display);

  while (1) {
    XNextEvent(display, &report);
  }

  return 1;
}
