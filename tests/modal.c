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
  XChangeProperty(display, child, state, XA_ATOM, 32,
		  PropModeReplace, (unsigned char*)&modal, 1);
  
  XMapWindow(display, parent);
  XMapWindow(display, child);
  XFlush(display);

  while (1) {
    XNextEvent(display, &report);
  }

  return 1;
}
