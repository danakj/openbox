#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

int main () {
  Display   *display;
  Window     parent, child;
  XEvent     report;
  Atom       state, modal;
  int        x=10,y=10,h=400,w=400;
  XEvent ce;

  display = XOpenDisplay(NULL);

  if (display == NULL) {
    fprintf(stderr, "couldn't connect to X server :0\n");
    return 0;
  }

  state = XInternAtom(display, "_NET_WM_STATE", True);
  modal = XInternAtom(display, "_NET_WM_STATE_MODAL", True);

  parent = XCreateWindow(display, RootWindow(display, 0),
			 x, y, w, h, 10, CopyFromParent, CopyFromParent,
			 CopyFromParent, 0, 0);
  child = XCreateWindow(display, RootWindow(display, 0),
			x, y, w/2, h/2, 10, CopyFromParent, CopyFromParent,
			CopyFromParent, 0, 0);

  XSetWindowBackground(display,parent,WhitePixel(display,0)); 
  XSetWindowBackground(display,child,BlackPixel(display,0)); 

  XSetTransientForHint(display, child, parent);
  
  XMapWindow(display, parent);
  XMapWindow(display, child);
  XFlush(display);

  ce.xclient.type = ClientMessage;
  ce.xclient.message_type = state;
  ce.xclient.display = display;
  ce.xclient.window = child;
  ce.xclient.format = 32;
  ce.xclient.data.l[0] = 1;
  ce.xclient.data.l[1] = modal;
  ce.xclient.data.l[2] = 0;
  XSendEvent(display, RootWindow(display, DefaultScreen(display)),
	     False, SubstructureNotifyMask | SubstructureRedirectMask, &ce);

  ce.xclient.data.l[0] = 0;
  XSendEvent(display, RootWindow(display, DefaultScreen(display)),
	     False, SubstructureNotifyMask | SubstructureRedirectMask, &ce);
  
  while (1) {
    XNextEvent(display, &report);
  }

  return 1;
}
