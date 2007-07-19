/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   fallback.c for the Openbox window manager
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
  Window     one, two;
  XEvent     report;

  display = XOpenDisplay(NULL);

  if (display == NULL) {
    fprintf(stderr, "couldn't connect to X server :0\n");
    return 0;
  }

  one = XCreateWindow(display, RootWindow(display, 0),
                      0,0,200,200, 10, CopyFromParent, CopyFromParent,
			 CopyFromParent, 0, 0);
  two = XCreateWindow(display, RootWindow(display, 0),
                      0,0,150,150, 10, CopyFromParent, CopyFromParent,
                      CopyFromParent, 0, 0);

  XSetWindowBackground(display,one,WhitePixel(display,0));
  XSetWindowBackground(display,two,BlackPixel(display,0));

  XSetTransientForHint(display, two, one);

  XMapWindow(display, one);
  XFlush(display);
  usleep(1000);

  XMapWindow(display, two);
  XFlush(display);
  usleep(1000);

  XDestroyWindow(display, two);
  XFlush(display);
  usleep(1000);

  XDestroyWindow(display, one);
  XSync(display, False);

  return 1;
}
