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
	GC gc;
	Pixmap pm;
	Appearance *look;

	int grabbed = 0;
	Window root;
        XGCValues values;
	XEvent report;
	int h = 500, w = 500, tmp;
	XVisualInfo *vi;
	int i;

	ob_display = XOpenDisplay(NULL);
	XSetErrorHandler(x_error_handler);
	ob_screen = DefaultScreen(ob_display);
	ob_root = RootWindow(ob_display, ob_screen);
	win =
	    XCreateWindow(ob_display, RootWindow(ob_display, 0)
                          , 10, 10, w, h, 10, 
                          CopyFromParent,	/* depth */
			  CopyFromParent,	/* class */
			  CopyFromParent,	/* visual */
			  0,	                /* valuemask */
			  0);	                /* attributes */
	XMapWindow(ob_display, win);
	XSelectInput(ob_display, win, ExposureMask | StructureNotifyMask);
	root = RootWindow (ob_display, DefaultScreen (ob_display));
	render_startup();

	look = appearance_new(0);
	look->surface.grad = Background_Pyramid;
	look->surface.secondary = color_parse("Yellow");
	look->surface.primary = color_parse("Blue");
        look->surface.interlaced = FALSE;
        look->area.x = 0;
        look->area.y = 0;
        look->area.width = 500;
        look->area.height = 500;
	if (ob_display == NULL) {
		fprintf(stderr, "couldn't connect to X server :0\n");
		return 0;
	}

	paint(win, look);
	while (1) {
		XNextEvent(ob_display, &report);
		switch (report.type) {
		case Expose:
		break;
		case ConfigureNotify:
			look->area.width = report.xconfigure.width;
			look->area.height = report.xconfigure.height;
			paint(win, look);
		break;
		}

	}

	return 1;
}
