/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   cursorio.c for the Openbox window manager
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
#include <X11/cursorfont.h>

int main () {
  Display   *display;
  Window     win, child;
  XEvent     report;
  int        x=10,y=10,h=100,w=400,b=10;
  XSetWindowAttributes a;

  display = XOpenDisplay(NULL);

  if (display == NULL) {
    fprintf(stderr, "couldn't connect to X server :0\n");
    return 0;
  }

  win = XCreateWindow(display, RootWindow(display, 0),
		      x, y, w, h, b, CopyFromParent, CopyFromParent,
		      CopyFromParent, 0, NULL);

  a.cursor = XCreateFontCursor(display, XC_watch); 
  child = XCreateWindow(display, win,
                        x+w/8, y+h/8, 3*w/4, 3*h/4, 0,
                        CopyFromParent, InputOnly,
                        CopyFromParent, CWCursor, &a);

  XMapWindow(display, child);
  XMapWindow(display, win);
  XFlush(display);

  while (1) {
    XNextEvent(display, &report);
  }

  return 1;
}
