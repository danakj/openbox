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

#define X 10
#define Y 10
#define W 100
#define H 100

int main()
{
    Display *display;
    Window win;
    XEvent report, report2;
    XClassHint chint;
    Atom delete_win, protocols;
    int quit;

    struct RrInstance *inst;
    struct RrSurface *sur;
    struct RrColor pri, sec;

    if (!(display = XOpenDisplay(NULL))) {
        fprintf(stderr, "couldn't connect to X server in DISPLAY\n");
        return EXIT_FAILURE;
    }
    XSetErrorHandler(x_error_handler);
    win = XCreateWindow(display, RootWindow(display, DefaultScreen(display)),
                        X, Y, W, H, 0, 
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
    if (!(inst = RrInstanceNew(display, DefaultScreen(display)))) {
        fprintf(stderr, "couldn't initialize the Render library "
                "(no suitable GL support found)\n");
        return EXIT_FAILURE;
    }

    sur = RrSurfaceNew(inst, RR_SURFACE_PLANAR, win, 0);
    RrSurfaceSetArea(sur, X, Y, W, H);
    RrColorSet(&pri, 0, 0, 0, 0);
    RrColorSet(&sec, 1, 1, 1, 0);
    RrPlanarSet(sur, RR_PLANAR_PIPECROSS, &pri, &sec);

    quit = 0;
    while (!quit) {
        XNextEvent(display, &report);
        switch (report.type) {
        case ClientMessage:
            if ((Atom)report.xclient.message_type == protocols)
                if ((Atom)report.xclient.data.l[0] == delete_win)
                    quit = 1;
        case Expose:
            if (XCheckTypedWindowEvent(display, win, ConfigureNotify,
                                       &report2)) {
                XPutBackEvent(display, &report);
                XPutBackEvent(display, &report2);
                /* fall through ... */
            } else {
                while (XCheckTypedWindowEvent(display, win, Expose, &report));
                RrPaint(sur);
                break;
            }
        case ConfigureNotify:
            while (XCheckTypedWindowEvent(display, win, ConfigureNotify,
                                          &report));
            RrSurfaceSetSize(sur,
                             report.xconfigure.width,
                             report.xconfigure.height);
            break;
        }

    }

    RrInstanceFree(inst);

    return 1;
}
