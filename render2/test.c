#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/shape.h>
#include <string.h>
#include <stdlib.h>
#include "render.h"
#include <glib.h>

static int x_error_handler(Display * disp, XErrorEvent * error)
{
    char buf[1024];
    XGetErrorText(disp, error->error_code, buf, 1024);
    printf("%s\n", buf);
    return 0;
}

int main()
{
    Display *display;
    Window win;
    struct RrInstance *inst;
    XEvent report;
    XClassHint chint;
    Atom delete_win, protocols;
    int quit;

    if (!(display = XOpenDisplay(NULL))) {
        fprintf(stderr, "couldn't connect to X server in DISPLAY\n");
        return EXIT_FAILURE;
    }
    XSetErrorHandler(x_error_handler);
    win = XCreateWindow(display, RootWindow(display, DefaultScreen(display)),
                        10, 10, 100, 100, 0, 
                        CopyFromParent,   /* depth */
                        CopyFromParent,   /* class */
                        CopyFromParent,   /* visual */
                        0,                /* valuemask */
                        0);               /* attributes */
    XMapWindow(display, win);
    XSelectInput(display, win, ExposureMask | StructureNotifyMask);

    chint.res_name = "rendertest";
    chint.res_class = "Rendertest";
    XSetClassHint(display, win, &chint);

    delete_win = XInternAtom(display, "WM_DELETE_WINDOW", False);
    protocols = XInternAtom(display, "WM_PROTOCOLS", False);
    XSetWMProtocols(display, win, &delete_win, 1);

    /* init Render */
    if (!(inst = RrInit(display, DefaultScreen(display)))) {
        fprintf(stderr, "couldn't initialize the Render library "
                "(no suitable GL support found)\n");
        return EXIT_FAILURE;
    }

    /*paint(win, look);*/
    quit = 0;
    while (!quit) {
        XNextEvent(display, &report);
        switch (report.type) {
        case ClientMessage:
            if ((Atom)report.xclient.message_type == protocols)
                if ((Atom)report.xclient.data.l[0] == delete_win)
                    quit = 1;
        case Expose:
            break;
        case ConfigureNotify:
            /*look->area.width = report.xconfigure.width;
            look->area.height = report.xconfigure.height;
            paint(win, look);*/
            break;
        }

    }

    RrDestroy(inst);

    return 1;
}
