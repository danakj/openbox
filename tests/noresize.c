#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

int main () {
  XSetWindowAttributes xswa;
  unsigned long        xswamask;
  Display   *display;
  Window     win;
  XEvent     report;
  int        x=10,y=10,h=100,w=400;
  XSizeHints size;

  display = XOpenDisplay(NULL);

  if (display == NULL) {
    fprintf(stderr, "couldn't connect to X server :0\n");
    return 0;
  }

  xswa.win_gravity = StaticGravity;
  xswamask = CWWinGravity;

  win = XCreateWindow(display, RootWindow(display, 0),
		      x, y, w, h, 10, CopyFromParent, CopyFromParent,
		      CopyFromParent, xswamask, &xswa);

  XSetWindowBackground(display,win,WhitePixel(display,0)); 

  size.flags = PMinSize | PMaxSize;
  size.max_width = 0;
  size.min_width = w;
  size.max_height = 0;
  size.min_height = h;
  XSetWMNormalHints(display, win, &size);
  
  XMapWindow(display, win);
  XFlush(display);

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
