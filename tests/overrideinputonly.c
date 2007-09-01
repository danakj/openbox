/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   override.c for the Openbox window manager
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

int main (int argc, char *argv[]) {
  XSetWindowAttributes xswa;
  unsigned long        xswamask;
  Display   *display;
  Window     win;
  XEvent     report;
  int        i,x=0,y=0,h=1,w=1;

  for (i=0; i < argc; i++) {
    if (!strcmp(argv[i], "-g") || !strcmp(argv[i], "-geometry")) {
      XParseGeometry(argv[++i], &x, &y, &w, &h);
    }
  }

  display = XOpenDisplay(NULL);

  if (display == NULL) {
    fprintf(stderr, "couldn't connect to X server :0\n");
    return 0;
  }

  xswa.override_redirect = True;
  xswamask = CWOverrideRedirect;

  win = XCreateWindow(display, RootWindow(display, 0),
		      x, y, w, h, 0, 0, InputOnly,
		      CopyFromParent, xswamask, &xswa);

  XMapWindow(display, win);
  XFlush(display);

  while (1) {
    XNextEvent(display, &report);
  }

  return 1;
}
