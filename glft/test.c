#include "glft.h"
#include <stdio.h>


#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/shape.h>
#include <string.h>
#include <stdlib.h>
#include "render.h"
#include <glib.h>
#include <GL/glx.h>

static int x_error_handler(Display * disp, XErrorEvent * error)
{
    char buf[1024];
    XGetErrorText(disp, error->error_code, buf, 1024);
    printf("%s\n", buf);
    return 0;
}

#define X 10
#define Y 10
#define W 500
#define H 500

int main(int argc, char **argv)
{
    Display *display;
    Window win;
    XVisualInfo *vi;
    XEvent report, report2;
    XClassHint chint;
    Atom delete_win, protocols;
    GLXContext cont;
    int quit;
    int config[] =
        { GLX_DEPTH_SIZE, 1, GLX_DOUBLEBUFFER, GLX_RGBA, None };

    struct GlftFont *font;

    if (argc < 3) {
        printf("Usage: %s fontname text\n", argv[0]);
        return 1;
    }

    if (!GlftInit()) return 1;

    font = GlftFontOpen(argv[1]);

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
    XSelectInput(display, win, ExposureMask | StructureNotifyMask);
    XMapWindow(display, win);

    vi = glXChooseVisual(display, DefaultScreen(display), config);
    if (vi == NULL)
            printf("no conforming visual\n");
    cont = glXCreateContext(display, vi, NULL, GL_TRUE);
    if (cont == NULL)
        printf("context creation failed\n");
    glXMakeCurrent(display, win, cont);



    chint.res_name = "rendertest";
    chint.res_class = "Rendertest";
    XSetClassHint(display, win, &chint);

    delete_win = XInternAtom(display, "WM_DELETE_WINDOW", False);
    protocols = XInternAtom(display, "WM_PROTOCOLS", False);
    XSetWMProtocols(display, win, &delete_win, 1);
    glClearColor(0.0, 0.0, 1.0, 0.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, W, -100, H+100, 0, 10);
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_LINE_SMOOTH);

    quit = 0;
    while (!quit) {
        XNextEvent(display, &report);
        switch (report.type) {
        case ClientMessage:
            if ((Atom)report.xclient.message_type == protocols)
                if ((Atom)report.xclient.data.l[0] == delete_win)
                    quit = 1;
        case Expose:
            glClear(GL_COLOR_BUFFER_BIT);
            GlftRenderString(font, argv[2], strlen(argv[2]), 0, 0);
            glXSwapBuffers(display, win);
        case ConfigureNotify:
            break;
        }

    }
    GlftFontClose(font);

    return 1;
}
