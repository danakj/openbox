#include <stdio.h>
#include <X11/Xlib.h>
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

Display *ob_display;
int ob_screen;
Window ob_root;

int main()
{
	Window win;
        RrInstance *inst;
	RrAppearance *look;

	Window root;
	XEvent report;
	int h = 500, w = 500;

	ob_display = XOpenDisplay(NULL);
	XSetErrorHandler(x_error_handler);
	ob_screen = DefaultScreen(ob_display);
	ob_root = RootWindow(ob_display, ob_screen);
	win =
	    XCreateWindow(ob_display, RootWindow(ob_display, 0),
                          10, 10, w, h, 10, 
                          CopyFromParent,	/* depth */
			  CopyFromParent,	/* class */
			  CopyFromParent,	/* visual */
			  0,	                /* valuemask */
			  0);	                /* attributes */
	XMapWindow(ob_display, win);
	XSelectInput(ob_display, win, ExposureMask | StructureNotifyMask);
	root = RootWindow (ob_display, DefaultScreen (ob_display));
        inst = RrInstanceNew(ob_display, ob_screen);

	look = RrAppearanceNew(inst, 0);
	look->surface.grad = RR_SURFACE_PYRAMID;
	look->surface.secondary = RrColorParse(inst, "Yellow");
	look->surface.primary = RrColorParse(inst, "Blue");
        look->surface.interlaced = FALSE;
	if (ob_display == NULL) {
		fprintf(stderr, "couldn't connect to X server :0\n");
		return 0;
	}

	RrPaint(look, win, w, h);
	while (1) {
		XNextEvent(ob_display, &report);
		switch (report.type) {
		case Expose:
		break;
		case ConfigureNotify:
                    RrPaint(look, win,
                            report.xconfigure.width,
                            report.xconfigure.height);
		break;
		}

	}

        RrAppearanceFree (look);
        RrInstanceFree (inst);

	return 1;
}
