#include <stdio.h>
#include <unistd.h>
#include <X11/Xlib.h>

int main () {
  Display   *display;
  Window     win;
  XEvent     report;
  Atom       _net_fs, _net_state;
  XEvent     msg;
  int        x=10,y=10,h=100,w=400;

  display = XOpenDisplay(NULL);

  if (display == NULL) {
    fprintf(stderr, "couldn't connect to X server :0\n");
    return 0;
  }

  _net_state = XInternAtom(display, "_NET_WM_STATE", False);
  _net_fs = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);

  win = XCreateWindow(display, RootWindow(display, 0),
		      x, y, w, h, 10, CopyFromParent, CopyFromParent,
		      CopyFromParent, 0, NULL);

  XSetWindowBackground(display,win,WhitePixel(display,0)); 

  XMapWindow(display, win);
  XFlush(display);
  sleep(2);

  msg.xclient.type = ClientMessage;
  msg.xclient.message_type = _net_state;
  msg.xclient.display = display;
  msg.xclient.window = win;
  msg.xclient.format = 32;
  msg.xclient.data.l[0] = 2; // toggle
  msg.xclient.data.l[1] = _net_fs;
  msg.xclient.data.l[2] = 0l;
  msg.xclient.data.l[3] = 0l;
  msg.xclient.data.l[4] = 0l;
  XSendEvent(display, RootWindow(display, 0), False,
	     StructureNotifyMask | SubstructureNotifyMask, &msg);
  XFlush(display);
  sleep(2);

  msg.xclient.type = ClientMessage;
  msg.xclient.message_type = _net_state;
  msg.xclient.display = display;
  msg.xclient.window = win;
  msg.xclient.format = 32;
  msg.xclient.data.l[0] = 2; // toggle
  msg.xclient.data.l[1] = _net_fs;
  msg.xclient.data.l[2] = 0l;
  msg.xclient.data.l[3] = 0l;
  msg.xclient.data.l[4] = 0l;
  XSendEvent(display, RootWindow(display, 0), False,
	     StructureNotifyMask | SubstructureNotifyMask, &msg);

  XSelectInput(display, win, ExposureMask | StructureNotifyMask);

  while (1) {
    XNextEvent(display, &report);

    switch (report.type) {
    case Expose:
      printf("exposed\n");
      break;
    case ConfigureNotify:
      x = report.xconfigure.x;
      y = report.xconfigure.y;
      w = report.xconfigure.width;
      h = report.xconfigure.height;
      printf("confignotify %i,%i-%ix%i\n",x,y,w,h);
      break;
    }

  }

  return 1;
}
