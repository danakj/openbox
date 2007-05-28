/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   confignotify.c for the Openbox window manager
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
    XEvent     msg;
    Atom       _net_max[2],_net_state;
    int        x=10,y=10,h=100,w=100;

    display = XOpenDisplay(NULL);

    if (display == NULL) {
        fprintf(stderr, "couldn't connect to X server :0\n");
        return 0;
    }

    _net_state = XInternAtom(display, "_NET_WM_STATE", False);
    _net_max[0] = XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_VERT", False);
    _net_max[1] = XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);

    win = XCreateWindow(display, RootWindow(display, 0),
                        x, y, w, h, 0, CopyFromParent, CopyFromParent,
                        CopyFromParent, 0, NULL);

    XSetWindowBackground(display,win,WhitePixel(display,0)); 
    XChangeProperty(display, win, _net_state, XA_ATOM, 32,
                    PropModeReplace, (unsigned char*)&_net_max, 2);

    XSelectInput(display, win, (ExposureMask | StructureNotifyMask |
                                GravityNotify));

    XMapWindow(display, win);
    XFlush(display);

    //sleep(1);
    //XResizeWindow(display, win, w+5, h+5);
    //XMoveWindow(display, win, x, y);

    while (1) {
        XNextEvent(display, &report);

        switch (report.type) {
        case MapNotify:
            printf("map notify\n");
            break;
        case Expose:
            printf("exposed\n");
            break;
        case GravityNotify:
            printf("gravity notify event 0x%x window 0x%x x %d y %d\n",
                   report.xgravity.event, report.xgravity.window,
                   report.xgravity.x, report.xgravity.y);
            break;
        case ConfigureNotify: {
            int se = report.xconfigure.send_event;
            int event = report.xconfigure.event;
            int window = report.xconfigure.window;
            int x = report.xconfigure.x;
            int y = report.xconfigure.y;
            int w = report.xconfigure.width;
            int h = report.xconfigure.height;
            int bw = report.xconfigure.border_width;
            int above = report.xconfigure.above;
            int or = report.xconfigure.override_redirect;
            printf("confignotify send %d ev 0x%x win 0x%x %i,%i-%ix%i bw %i\n"
                   "             above 0x%x ovrd %d\n",
                   se,event,window,x,y,w,h,bw,above,or);
            break;
        }
        }

    }

    return 1;
}
