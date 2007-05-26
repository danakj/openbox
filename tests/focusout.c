/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   focusout.c for the Openbox window manager
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
    Window     win, child;
    XEvent     report;
    Atom       _net_fs, _net_state;
    XEvent     msg;
    int        x=50,y=50,h=100,w=400;
    XWMHints   hint;

    display = XOpenDisplay(NULL);

    if (display == NULL) {
        fprintf(stderr, "couldn't connect to X server :0\n");
        return 0;
    }

    win = XCreateWindow(display, RootWindow(display, 0),
                        x, y, w, h, 10, CopyFromParent, CopyFromParent,
                        CopyFromParent, 0, NULL);
    child = XCreateWindow(display, win,
                        10, 10, w-20, h-20, 0, CopyFromParent, CopyFromParent,
                        CopyFromParent, 0, NULL);

    XSetWindowBackground(display,win,WhitePixel(display,0)); 
    XSetWindowBackground(display,child,BlackPixel(display,0)); 

    XSelectInput(display, win,
                 FocusChangeMask|EnterWindowMask|LeaveWindowMask);
    XMapWindow(display, win);
    XMapWindow(display, child);

    while (1) {
        const char *mode, *detail;

        XNextEvent(display, &report);

        switch (report.type) {
        case ButtonPress:
            printf("button press\n");
            printf("type        : %d\n", report.xbutton.type);
            printf("serial      : %d\n", report.xbutton.serial);
            printf("send_event  : %d\n", report.xbutton.send_event);
            printf("display     : 0x%x\n", report.xbutton.display);
            printf("window      : 0x%x\n", report.xbutton.window);
            printf("root        : 0x%x\n", report.xbutton.root);
            printf("subwindow   : 0x%x\n", report.xbutton.subwindow);
            printf("time        : %d\n", report.xbutton.time);
            printf("x, y        : %d, %d\n", report.xbutton.x,
                   report.xbutton.y);
            printf("rootx, rooty: %d, %d\n", report.xbutton.x_root,
                   report.xbutton.y_root);
            printf("state       : 0x%x\n", report.xbutton.state);
            printf("button      : %d\n", report.xbutton.button);
            printf("same_screen : %d\n", report.xbutton.same_screen);
            printf("---\n");
            break;
        case MotionNotify:
            printf("motion\n");
            printf("type        : %d\n", report.xmotion.type);
            printf("serial      : %d\n", report.xmotion.serial);
            printf("send_event  : %d\n", report.xmotion.send_event);
            printf("display     : 0x%x\n", report.xmotion.display);
            printf("window      : 0x%x\n", report.xmotion.window);
            printf("root        : 0x%x\n", report.xmotion.root);
            printf("subwindow   : 0x%x\n", report.xmotion.subwindow);
            printf("time        : %d\n", report.xmotion.time);
            printf("x, y        : %d, %d\n", report.xmotion.x,
                   report.xmotion.y);
            printf("rootx, rooty: %d, %d\n", report.xmotion.x_root,
                   report.xmotion.y_root);
            printf("state       : 0x%x\n", report.xmotion.state);
            printf("is_hint     : %d\n", report.xmotion.is_hint);
            printf("same_screen : %d\n", report.xmotion.same_screen);
            printf("---\n");
            if (XGrabPointer(display, win, False, ButtonReleaseMask,
                             GrabModeAsync, GrabModeAsync, None, None,
                             report.xmotion.time) == GrabSuccess)
                printf("GrabSuccess\n");
            else
                printf("GrabFail\n");
            break;
        case ButtonRelease:
            XUngrabPointer(display, report.xbutton.time);
            break;
        case FocusIn:
            switch (report.xfocus.mode) {
            case NotifyNormal: mode = "NotifyNormal"; break;
            case NotifyGrab: mode = "NotifyGrab"; break;
            case NotifyUngrab: mode = "NotifyUngrab"; break;
            }

            switch (report.xfocus.detail) {
            case NotifyAncestor: detail = "NotifyAncestor"; break;
            case NotifyVirtual: detail = "NotifyVirtual"; break;
            case NotifyInferior: detail = "NotifyInferior"; break;
            case NotifyNonlinear: detail = "NotifyNonlinear"; break;
            case NotifyNonlinearVirtual: detail = "NotifyNonlinearVirtual"; break;
            case NotifyPointer: detail = "NotifyPointer"; break;
            case NotifyPointerRoot: detail = "NotifyPointerRoot"; break;
            case NotifyDetailNone: detail = "NotifyDetailNone"; break;
            }
            printf("focusin\n");
            printf("type      : %d\n", report.xfocus.type);
            printf("serial    : %d\n", report.xfocus.serial);
            printf("send_event: %d\n", report.xfocus.send_event);
            printf("display   : 0x%x\n", report.xfocus.display);
            printf("window    : 0x%x\n", report.xfocus.window);
            printf("mode      : %s\n", mode);
            printf("detail    : %s\n", detail);
            printf("---\n");
            break;
        case FocusOut:
            switch (report.xfocus.mode) {
            case NotifyNormal: mode = "NotifyNormal"; break;
            case NotifyGrab: mode = "NotifyGrab"; break;
            case NotifyUngrab: mode = "NotifyUngrab"; break;
            }

            switch (report.xfocus.detail) {
            case NotifyAncestor: detail = "NotifyAncestor"; break;
            case NotifyVirtual: detail = "NotifyVirtual"; break;
            case NotifyInferior: detail = "NotifyInferior"; break;
            case NotifyNonlinear: detail = "NotifyNonlinear"; break;
            case NotifyNonlinearVirtual: detail = "NotifyNonlinearVirtual"; break;
            case NotifyPointer: detail = "NotifyPointer"; break;
            case NotifyPointerRoot: detail = "NotifyPointerRoot"; break;
            case NotifyDetailNone: detail = "NotifyDetailNone"; break;
            }
            printf("focusout\n");
            printf("type      : %d\n", report.xfocus.type);
            printf("serial    : %d\n", report.xfocus.serial);
            printf("send_event: %d\n", report.xfocus.send_event);
            printf("display   : 0x%x\n", report.xfocus.display);
            printf("window    : 0x%x\n", report.xfocus.window);
            printf("mode      : %s\n", mode);
            printf("detail    : %s\n", detail);
            printf("---\n");
            break;
        case EnterNotify:
            switch (report.xcrossing.mode) {
            case NotifyNormal: mode = "NotifyNormal"; break;
            case NotifyGrab: mode = "NotifyGrab"; break;
            case NotifyUngrab: mode = "NotifyUngrab"; break;
            }

            switch (report.xcrossing.detail) {
            case NotifyAncestor: detail = "NotifyAncestor"; break;
            case NotifyVirtual: detail = "NotifyVirtual"; break;
            case NotifyInferior: detail = "NotifyInferior"; break;
            case NotifyNonlinear: detail = "NotifyNonlinear"; break;
            case NotifyNonlinearVirtual: detail = "NotifyNonlinearVirtual"; break;
            case NotifyPointer: detail = "NotifyPointer"; break;
            case NotifyPointerRoot: detail = "NotifyPointerRoot"; break;
            case NotifyDetailNone: detail = "NotifyDetailNone"; break;
            }
            printf("enternotify\n");
            printf("type      : %d\n", report.xcrossing.type);
            printf("serial    : %d\n", report.xcrossing.serial);
            printf("send_event: %d\n", report.xcrossing.send_event);
            printf("display   : 0x%x\n", report.xcrossing.display);
            printf("window    : 0x%x\n", report.xcrossing.window);
            printf("mode      : %s\n", mode);
            printf("detail    : %s\n", detail);
            printf("---\n");
            break;
        case LeaveNotify:
            switch (report.xcrossing.mode) {
            case NotifyNormal: mode = "NotifyNormal"; break;
            case NotifyGrab: mode = "NotifyGrab"; break;
            case NotifyUngrab: mode = "NotifyUngrab"; break;
            }

            switch (report.xcrossing.detail) {
            case NotifyAncestor: detail = "NotifyAncestor"; break;
            case NotifyVirtual: detail = "NotifyVirtual"; break;
            case NotifyInferior: detail = "NotifyInferior"; break;
            case NotifyNonlinear: detail = "NotifyNonlinear"; break;
            case NotifyNonlinearVirtual: detail = "NotifyNonlinearVirtual"; break;
            case NotifyPointer: detail = "NotifyPointer"; break;
            case NotifyPointerRoot: detail = "NotifyPointerRoot"; break;
            case NotifyDetailNone: detail = "NotifyDetailNone"; break;
            }
            printf("leavenotify\n");
            printf("type      : %d\n", report.xcrossing.type);
            printf("serial    : %d\n", report.xcrossing.serial);
            printf("send_event: %d\n", report.xcrossing.send_event);
            printf("display   : 0x%x\n", report.xcrossing.display);
            printf("window    : 0x%x\n", report.xcrossing.window);
            printf("mode      : %s\n", mode);
            printf("detail    : %s\n", detail);
            printf("---\n");
            break;
        }
    }

    return 1;
}
