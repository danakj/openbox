/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   showhide.c for the Openbox window manager
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
#include <X11/Xutil.h>

int main () {
  Display   *display;
  Window     win;
  XEvent     report;
  XEvent     msg;
  int        x=50,y=50,h=100,w=400;

  display = XOpenDisplay(NULL);

  if (display == NULL) {
    fprintf(stderr, "couldn't connect to X server :0\n");
    return 0;
  }

  while (1) {
      win = XCreateWindow(display, RootWindow(display, 0),
                          x, y, w, h, 10, CopyFromParent, CopyFromParent,
                          CopyFromParent, 0, NULL);
      XSetWindowBackground(display,win,WhitePixel(display,0)); 
      XMapWindow(display, win);
      XFlush(display);
      usleep(1000);
      XDestroyWindow(display, win);
      XSync(display, False);

      break;
      sleep(2);
  }

  return 1;
}
