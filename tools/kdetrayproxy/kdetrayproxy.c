#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>
#include <sys/time.h>

typedef struct IList {
    Window win;
    int ignore_unmaps;

    struct IList *next;
} IList;

Display *display;
Window root;
Atom winhint;
Atom roothint;
int xfd;
IList *list;

void init();
void eventloop();
void handleevent(XEvent *e);
void addicon(Window win);
void removeicon(Window win, int unmap);
int issystray(Atom *a, int n);
void updatehint();
Window findclient(Window win);
int ignore_errors(Display *d, XErrorEvent *e);
void wait_time(unsigned int t);

int main()
{
    init();
    updatehint();
    eventloop();
    return 0;
}

void init()
{
    display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "Could not open display\n");
        exit(EXIT_FAILURE);
    }

    xfd = ConnectionNumber(display);

    root = RootWindowOfScreen(DefaultScreenOfDisplay(display));

    winhint = XInternAtom(display, "_KDE_NET_WM_SYSTEM_TRAY_WINDOW_FOR", 0);
    roothint = XInternAtom(display, "_KDE_NET_SYSTEM_TRAY_WINDOWS", 0);

    XSelectInput(display, root, SubstructureNotifyMask);
}

void eventloop()
{
    XEvent e;
    fd_set set;

    while (1) {
        int event = False;
        while (XPending(display)) {
            event = True;
            XNextEvent(display, &e);
            handleevent(&e);
        }
        if (!event) {
            FD_ZERO(&set);
            FD_SET(xfd, &set);
            select(xfd + 1, &set, NULL, NULL, NULL);
        }
    }
}

void handleevent(XEvent *e)
{
    switch (e->type) {
    case MapNotify:
    {
        Atom *a;
        int n;
        Window w;

        w = findclient(e->xmap.window);
        if (w) {
            a = XListProperties(display, w, &n);
            if (issystray(a, n))
                addicon(w);
            XFree(a);
        }
        break;
    }
    case UnmapNotify:
        removeicon(e->xunmap.window, True);
        break;
    case DestroyNotify:
        removeicon(e->xdestroywindow.window, False);
        break;
    }
}

int ignore_errors(Display *d, XErrorEvent *e)
{
    (void)d; (void)e;
    return 1;
}

void addicon(Window win)
{
    IList *it;

    for (it = list; it; it = it->next)
        if (it->win == win) return; /* duplicate */

    it = list;
    list = malloc(sizeof(IList));
    list->win = win;
    list->ignore_unmaps = 2;
    list->next = it;

    XSelectInput(display, win, StructureNotifyMask);
    /* if i set the root hint too fast the dock app can fuck itself up */
    wait_time(1000000 / 8);
    updatehint();
}

void removeicon(Window win, int unmap)
{
    IList *it, *last = NULL;
    void *old;

    for (it = list; it; last = it, it = it->next)
        if (it->win == win) {
            if (it->ignore_unmaps && unmap) {
                it->ignore_unmaps--;
                return;
            }

            if (!last)
                list = it->next;
            else
                last->next = it->next;

            XSync(display, False);
            old = XSetErrorHandler(ignore_errors);
            XSelectInput(display, win, NoEventMask);
            XSync(display, False);
            XSetErrorHandler(old);
            free(it);

            updatehint();
        }
}

int issystray(Atom *a, int n)
{
    int i, r = False;

    for (i = 0; i < n; ++i) { 
        if (a[i] == winhint) {
            r = True;
            break;
        }
    }
    return r;
}

void updatehint()
{
    IList *it;
    int *wins, n, i;

    for (it = list, n = 0; it; it = it->next, ++n) ;
    if (n) {
        wins = malloc(sizeof(int) * n);
        for (it = list, i = 0; it; it = it->next, ++i)
            wins[i] = it->win;
    } else
        wins = NULL;
    XChangeProperty(display, root, roothint, XA_WINDOW, 32, PropModeReplace,
                    (unsigned char*) wins, n);
}

Window findclient(Window win)
{
  Window r, *children;
  unsigned int n, i;
  Atom state = XInternAtom(display, "WM_STATE", True);
  Atom ret_type;
  int ret_format;
  unsigned long ret_items, ret_bytesleft;
  unsigned long *prop_return;

  XQueryTree(display, win, &r, &r, &children, &n);
  for (i = 0; i < n; ++i) {
    Window w = findclient(children[i]);
    if (w) return w;
  }

  /* try me */
  XGetWindowProperty(display, win, state, 0, 1,
		     False, state, &ret_type, &ret_format,
		     &ret_items, &ret_bytesleft,
		     (unsigned char**) &prop_return); 
  if (ret_type == None || ret_items < 1)
    return None;
  return win; /* found it! */
}

void wait_time(unsigned int t)
{
    struct timeval time;
    time.tv_sec = 0;
    time.tv_usec = t;
    select(1, NULL, NULL, NULL, &time);
}
