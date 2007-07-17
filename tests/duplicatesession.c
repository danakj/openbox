/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   duplicatesession.c for the Openbox window manager
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

#include <string.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

int main (int argc, char **argv) {
  Display   *display;
  Window     win1, win2;
  XEvent     report;
  int        x=10,y=10,h=100,w=400;
  XSizeHints size;
  XTextProperty name;
  Atom sm_id, enc;

  display = XOpenDisplay(NULL);

  if (display == NULL) {
    fprintf(stderr, "couldn't connect to X server :0\n");
    return 0;
  }

  sm_id = XInternAtom(display,"SM_CLIENT_ID",False);
  enc = XInternAtom(display,"STRING",False);

  win1 = XCreateWindow(display, RootWindow(display, 0),
                       x, y, w, h, 10, CopyFromParent, CopyFromParent,
                       CopyFromParent, 0, NULL);
  win2 = XCreateWindow(display, RootWindow(display, 0),
                       x, y, w, h, 10, CopyFromParent, CopyFromParent,
                       CopyFromParent, 0, NULL);

  XSetWindowBackground(display,win1,WhitePixel(display,0));
  XSetWindowBackground(display,win2,BlackPixel(display,0));

  XChangeProperty(display, win1, sm_id, enc, 8,
                  PropModeAppend, "abcdefg", strlen("abcdefg"));
  XChangeProperty(display, win2, sm_id, enc, 8,
                  PropModeAppend, "abcdefg", strlen("abcdefg"));

  XFlush(display);
  XMapWindow(display, win1);
  XMapWindow(display, win2);

  while (1)
    XNextEvent(display, &report);

  return 1;
}
