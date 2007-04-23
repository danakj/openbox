/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   icons.c for the Openbox window manager
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

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

Window findClient(Display *d, Window win)
{
    Window r, *children;
    unsigned int n, i;
    Atom state = XInternAtom(d, "WM_STATE", True);
    Atom ret_type;
    int ret_format;
    unsigned long ret_items, ret_bytesleft;
    unsigned long *prop_return;

    XQueryTree(d, win, &r, &r, &children, &n);
    for (i = 0; i < n; ++i) {
        Window w = findClient(d, children[i]);
        if (w) return w;
    }

    // try me
    XGetWindowProperty(d, win, state, 0, 1,
                       False, state, &ret_type, &ret_format,
                       &ret_items, &ret_bytesleft,
                       (unsigned char**) &prop_return); 
    if (ret_type == None || ret_items < 1)
        return None;
    return win; // found it!
}

int main(int argc, char **argv)
{
    Display *d = XOpenDisplay(NULL);
    int s = DefaultScreen(d);
    Atom net_wm_icon = XInternAtom(d, "_NET_WM_ICON", True);
    Atom ret_type;
    unsigned int winw = 0, winh = 0;
    int ret_format;
    unsigned long ret_items, ret_bytesleft;
    const int MAX_IMAGES = 10;
    unsigned long *prop_return[MAX_IMAGES];
    XImage *i[MAX_IMAGES];
    long offset = 0;
    unsigned int image = 0;
    unsigned int j; // loop counter
    Window id, win;
    Pixmap p;
    Cursor cur;
    XEvent ev;
  
    printf("Click on a window with an icon...\n");
  
    //int id = strtol(argv[1], NULL, 16);
    XUngrabPointer(d, CurrentTime);
    cur = XCreateFontCursor(d, XC_crosshair);
    XGrabPointer(d, RootWindow(d, s), False, ButtonPressMask, GrabModeAsync,
                 GrabModeAsync, None, cur, CurrentTime);
    while (1) {
        XNextEvent(d, &ev);
        if (ev.type == ButtonPress) {
            XUngrabPointer(d, CurrentTime);
            id = findClient(d, ev.xbutton.subwindow);
            break;
        }
    }

    printf("Using window 0x%lx\n", id);
  
    do {
        unsigned int w, h;
    
        XGetWindowProperty(d, id, net_wm_icon, offset++, 1,
                           False, XA_CARDINAL, &ret_type, &ret_format,
                           &ret_items, &ret_bytesleft,
                           (unsigned char**) &prop_return[image]);
        if (ret_type == None || ret_items < 1) {
            printf("No icon found\n");
            return 1;
        }
        w = prop_return[image][0];
        XFree(prop_return[image]);

        XGetWindowProperty(d, id, net_wm_icon, offset++, 1,
                           False, XA_CARDINAL, &ret_type, &ret_format,
                           &ret_items, &ret_bytesleft,
                           (unsigned char**) &prop_return[image]);
        if (ret_type == None || ret_items < 1) {
            printf("Failed to get height\n");
            return 1;
        }
        h = prop_return[image][0];
        XFree(prop_return[image]);

        XGetWindowProperty(d, id, net_wm_icon, offset, w*h,
                           False, XA_CARDINAL, &ret_type, &ret_format,
                           &ret_items, &ret_bytesleft,
                           (unsigned char**) &prop_return[image]);
        if (ret_type == None || ret_items < w*h) {
            printf("Failed to get image data\n");
            return 1;
        }
        offset += w*h;

        printf("Found icon with size %dx%d\n", w, h);
  
        i[image] = XCreateImage(d, DefaultVisual(d, s), DefaultDepth(d, s),
                                ZPixmap, 0, NULL, w, h, 32, 0);
        assert(i[image]);
        i[image]->byte_order = LSBFirst;
        i[image]->data = (char*)prop_return[image];
        for (j = 0; j < w*h; j++) {
            unsigned char alpha = (unsigned char)i[image]->data[j*4+3];
            unsigned char r = (unsigned char) i[image]->data[j*4+0];
            unsigned char g = (unsigned char) i[image]->data[j*4+1];
            unsigned char b = (unsigned char) i[image]->data[j*4+2];

            // background color
            unsigned char bgr = 0;
            unsigned char bgg = 0;
            unsigned char bgb = 0;
      
            r = bgr + (r - bgr) * alpha / 256;
            g = bgg + (g - bgg) * alpha / 256;
            b = bgb + (b - bgb) * alpha / 256;

            i[image]->data[j*4+0] = (char) r;
            i[image]->data[j*4+1] = (char) g;
            i[image]->data[j*4+2] = (char) b;
        }

        winw += w;
        if (h > winh) winh = h;

        ++image;
    } while (ret_bytesleft > 0 && image < MAX_IMAGES);

    win = XCreateSimpleWindow(d, RootWindow(d, s), 0, 0, winw, winh,
                              0, 0, 0);
    assert(win);
    XMapWindow(d, win);

    p = XCreatePixmap(d, win, winw, winh, DefaultDepth(d, s));
    XFillRectangle(d, p, DefaultGC(d, s), 0, 0, winw, winh);

    for (j = 0; j < image; ++j) {
        static unsigned int x = 0;

        XPutImage(d, p, DefaultGC(d, s), i[j], 0, 0, x, 0,
                  i[j]->width, i[j]->height);
        x += i[j]->width;
        XDestroyImage(i[j]);
    }
    
    XSetWindowBackgroundPixmap(d, win, p);
    XClearWindow(d, win);

    XFlush(d);

    getchar();

    XFreePixmap(d, p);
    XCloseDisplay(d);
}
