#include "render/render.h"
#include "render/theme.h"

#include <X11/Xlib.h>
#include <stdlib.h>
#include <glib.h>
#ifdef HAVE_SIGNAL_H
#  include <signal.h>
#endif
#ifdef HAVE_SYS_SELECT_H
#  include <sys/select.h>
#endif

#define TITLE_EVENT_MASK (ButtonPressMask | ButtonReleaseMask | \
                          ButtonMotionMask)
#define ROOT_EVENT_MASK (PropertyChangeMask | StructureNotifyMask | \
                         SubstructureNotifyMask)
#define SLITAPP_EVENT_MASK (StructureNotifyMask)

Display *ob_display;
Window ob_root;
int ob_screen;

static struct Slit {
    Window frame;
    Window title;

    /* user-requested position stuff */
    int gravity;
    int user_x, user_y;

    /* actual position (when not auto-hidden) */
    int x, y;
    int w, h;

    gboolean horz;

    Appearance *a_frame;
    Appearance *a_title;

    GList *slit_apps;
} *slit;
static int nslits;

struct SlitApp {
    Window icon_win;
    Window win;
    int x;
    int y;
    int w;
    int h;
};

static Atom atom_atom;
static Atom atom_card;
static Atom atom_theme;
static Atom atom_type;
static Atom atom_type_dock;
static Atom atom_desktop;
static Atom atom_state;
static Atom atom_strut;

static gboolean quit = FALSE;
static gboolean reconfig = FALSE;

void slit_read_theme();
void slit_configure();
void event_handle(XEvent *e);
void slit_add_existing();
void slit_add_app(Window win);
void slit_remove_app(struct Slit *slit, struct SlitApp *app,gboolean reparent);

void sighandler(int signal)
{
    if (signal == SIGUSR1) 
        reconfig =TRUE;
    else
        quit = TRUE;
}

int xerrorhandler(Display *d, XErrorEvent *e)
{
    char errtxt[128];
    XGetErrorText(d, e->error_code, errtxt, 127);
    g_error("X Error: %s", errtxt);
    return 0;
}

int main()
{
    int i;
    guint desk = 0xffffffff;
    XEvent e;
    XSetWindowAttributes attrib;
    struct sigaction action;
    sigset_t sigset;
    int xfd;
    fd_set selset;

    /* set up signal handler */
    sigemptyset(&sigset);
    action.sa_handler = sighandler;
    action.sa_mask = sigset;
    action.sa_flags = SA_NOCLDSTOP;
    sigaction(SIGUSR1, &action, (struct sigaction *) NULL);
    sigaction(SIGPIPE, &action, (struct sigaction *) NULL);
    sigaction(SIGSEGV, &action, (struct sigaction *) NULL);
    sigaction(SIGFPE, &action, (struct sigaction *) NULL);
    sigaction(SIGTERM, &action, (struct sigaction *) NULL);
    sigaction(SIGINT, &action, (struct sigaction *) NULL);
    sigaction(SIGHUP, &action, (struct sigaction *) NULL);

    ob_display = XOpenDisplay(NULL);
    ob_screen = DefaultScreen(ob_display);
    ob_root = RootWindow(ob_display, ob_screen);

   XSetErrorHandler(xerrorhandler);

    render_startup();
    theme_startup();

    atom_atom = XInternAtom(ob_display, "ATOM", False);
    atom_card = XInternAtom(ob_display, "CARDINAL", False);
    atom_theme = XInternAtom(ob_display, "_OPENBOX_THEME", False);
    atom_type = XInternAtom(ob_display, "_NET_WM_WINDOW_TYPE", False);
    atom_type_dock = XInternAtom(ob_display, "_NET_WM_WINDOW_TYPE_DOCK",False);
    atom_desktop =XInternAtom(ob_display, "_NET_WM_DESKTOP", False);
    atom_state = XInternAtom(ob_display, "WM_STATE", False);
    atom_strut = XInternAtom(ob_display, "_NET_WM_STRUT", False);

    nslits = 1;
    slit = g_new0(struct Slit, nslits);

    for (i = 0; i < nslits; ++i) {
        slit[i].horz = TRUE;

        slit[i].frame = XCreateWindow(ob_display, ob_root, 0, 0, 1, 1, 0,
                                      render_depth, InputOutput, render_visual,
                                      0, NULL);
        attrib.event_mask = TITLE_EVENT_MASK;
        slit[i].title = XCreateWindow(ob_display, slit[i].frame, 0, 0, 1, 1, 0,
                                      render_depth, InputOutput, render_visual,
                                      CWEventMask, &attrib);
        XMapWindow(ob_display, slit[i].title);

        XChangeProperty(ob_display, slit[i].frame, atom_type, atom_atom,
                        32, PropModeReplace, (guchar*)&atom_type_dock, 1);

        XChangeProperty(ob_display, slit[i].frame, atom_desktop, atom_card,
                        32, PropModeReplace, (guchar*)&desk, 1);
    }

    slit_read_theme();

    XSelectInput(ob_display, ob_root, ROOT_EVENT_MASK);

    slit_add_existing();

    xfd = ConnectionNumber(ob_display);
    FD_ZERO(&selset);
    FD_SET(xfd, &selset);
    while (!quit) {
        gboolean hadevent = FALSE;
        while (XPending(ob_display)) {
            XNextEvent(ob_display, &e);
            event_handle(&e);
            hadevent = TRUE;
        }
        if (!hadevent) {
            if (reconfig)
                slit_read_theme();

            if (!quit)
                select(xfd + 1, &selset, NULL, NULL, NULL);
        }
    }

    for (i = 0; i < nslits; ++i) {
        while (slit[i].slit_apps)
            slit_remove_app(&slit[i], slit[i].slit_apps->data, TRUE);

        XDestroyWindow(ob_display, slit[i].title);
        XDestroyWindow(ob_display, slit[i].frame);

        appearance_free(slit[i].a_frame);
        appearance_free(slit[i].a_title);
    }

    theme_shutdown();
    render_shutdown();
    XCloseDisplay(ob_display);
    return 0;
}

Window find_client(Window win)
{
  Window r, *children;
  unsigned int n, i;
  Atom ret_type;
  int ret_format;
  unsigned long ret_items, ret_bytesleft;
  unsigned long *prop_return;

  XQueryTree(ob_display, win, &r, &r, &children, &n);
  for (i = 0; i < n; ++i) {
    Window w = find_client(children[i]);
    if (w) return w;
  }

  /* try me */
  XGetWindowProperty(ob_display, win, atom_state, 0, 1,
		     False, atom_state, &ret_type, &ret_format,
		     &ret_items, &ret_bytesleft,
		     (unsigned char**) &prop_return); 
  if (ret_type == None || ret_items < 1)
    return None;
  return win; /* found it! */
}

void event_handle(XEvent *e)
{
    int i;
    Window win;
    static guint button = 0;
    int sw, sh;
    int xpos, ypos;
    int x, y, g;

    switch (e->type) {
    case ButtonPress:
        if (!button) {
            button = e->xbutton.button;
        }
        break;
    case ButtonRelease:
        if (button == e->xbutton.button)
            button = 0;
        break;
    case MotionNotify:
        if (button == 1) {
            for (i = 0; i < nslits; ++i)
                if (slit[i].title == e->xmotion.window) {
                    /* pick a corner and move it */
                    sw = WidthOfScreen(ScreenOfDisplay(ob_display, ob_screen));
                    sh = HeightOfScreen(ScreenOfDisplay(ob_display,ob_screen));

                    if (e->xmotion.x_root < sw / 3) /* left edge */
                        xpos = 0;
                    else if (e->xmotion.x_root < sw / 3 * 2) /* middle */
                        xpos = 1;
                    else /* right edge */
                        xpos = 2;
                    if (e->xmotion.y_root < sh / 3) /* top edge */
                        ypos = 0;
                    else if (e->xmotion.y_root < sh / 3 * 2) /* middle */
                        ypos = 1;
                    else /* bottom edge */
                        ypos = 2;

                    if (xpos == 1 && ypos == 1)
                        return; /* cant go in middle middle */

                    if (xpos == 0) {
                        if (ypos == 0) {
                            x = 0;
                            y = 0;
                            g = NorthWestGravity;
                        } else if (ypos == 1) {
                            x = 0;
                            y = sh / 2;
                            g = WestGravity;
                        } else {
                            x = 0;
                            y = sh;
                            g = SouthWestGravity;
                        }
                    } else if (xpos == 1) {
                        if (ypos == 0) {
                            x = sw / 2;
                            y = 0;
                            g = NorthGravity;
                        } else {
                            x = sw / 2;
                            y = sh;
                            g = SouthGravity;
                        }
                    } else {
                        if (ypos == 0) {
                            x = sw;
                            y = 0;
                            g = NorthEastGravity;
                        } else if (ypos == 1) {
                            x = sw;
                            y = sh / 2;
                            g = EastGravity;
                        } else {
                            x = sw;
                            y = sh;
                            g = SouthEastGravity;
                        }
                    }
                    if (x != slit[i].x || y != slit[i].y ||
                        g != slit[i].gravity) {
                        slit[i].user_x = x;
                        slit[i].user_y = y;
                        slit[i].gravity = g;
                        slit_configure();
                    }
                }
        }
        break;
    case PropertyNotify:
        g_message("PropertyNotify on 0x%lx", e->xproperty.window);
        if (e->xproperty.window == ob_root) {
            if (e->xproperty.atom == atom_theme)
                slit_read_theme();
        }
        break;
    case ConfigureNotify:
        g_message("ConfigureNotify on 0x%lx", e->xconfigure.window);
        if (e->xconfigure.window == ob_root) {
            slit_configure();
            return;
        }

        /* an owned slitapp? */
        for (i = 0; i < nslits; ++i) {
            GList *it;

            for (it = slit[i].slit_apps; it; it = it->next) {
                struct SlitApp *app = it->data;
                if (e->xconfigure.window == app->icon_win) {
                    if (app->w != e->xconfigure.width ||
                        app->h != e->xconfigure.height) {
                        g_message("w %d h %d w %d h %d",
                                  app->w, e->xconfigure.width,
                                  app->h, e->xconfigure.height);
                        app->w = e->xconfigure.width;
                        app->h = e->xconfigure.height;
                        slit_configure();
                    }
                    return;
                }
            }
        }
        break;
    case MapNotify:
        g_message("MapNotify on 0x%lx", e->xmap.window);

        win = find_client(e->xmap.window);
        if (!win) return;

        for (i = 0; i < nslits; ++i)
            if (win == slit[i].frame)
                return;

        slit_add_app(win);
        break;
    case UnmapNotify:
        g_message("UnmapNotify on 0x%lx", e->xunmap.window);
        for (i = 0; i < nslits; ++i) {
            GList *it;

            for (it = slit[i].slit_apps; it; it = it->next) {
                struct SlitApp *app = it->data;
                if (e->xunmap.window == app->icon_win) {
                    gboolean r;
                    XEvent e;

                    r = !XCheckTypedWindowEvent(ob_display, app->icon_win,
                                                DestroyNotify, &e);
                    if (r) {
                        if (XCheckTypedWindowEvent(ob_display, app->icon_win,
                                                   ReparentNotify, &e)) {
                            XPutBackEvent(ob_display, &e);
                            r = FALSE;
                        }
                    }
                    slit_remove_app(&slit[i], app, r);
                    break;
                }
            }
        }
        break;
    case ReparentNotify:
        g_message("ReparentNotify on 0x%lx", e->xdestroywindow.window);
        for (i = 0; i < nslits; ++i) {
            GList *it;

            for (it = slit[i].slit_apps; it; it = it->next) {
                struct SlitApp *app = it->data;
                if (e->xdestroywindow.window == app->icon_win) {
                    slit_remove_app(&slit[i], app, FALSE);
                    break;
                }
            }
        }
    case DestroyNotify:
        g_message("DestroyNotify on 0x%lx", e->xdestroywindow.window);
        for (i = 0; i < nslits; ++i) {
            GList *it;

            for (it = slit[i].slit_apps; it; it = it->next) {
                struct SlitApp *app = it->data;
                if (e->xdestroywindow.window == app->icon_win) {
                    slit_remove_app(&slit[i], app, FALSE);
                    break;
                }
            }
        }
        break;
    }
}

void slit_add_existing()
{
    unsigned int i, nchild;
    int j;
    Window w, *children;
    XWindowAttributes attrib;

    XQueryTree(ob_display, ob_root, &w, &w, &children, &nchild);

    for (i = 0; i < nchild; ++i) {
        for (j = 0; j < nslits; ++j)
            if (children[i] == slit[j].frame)
                continue;
        if (children[i] == None)
            continue;
        if ((children[i] = find_client(children[i])) == None)
            continue;
	if (XGetWindowAttributes(ob_display, children[i], &attrib)) {
	    if (attrib.override_redirect) continue;

            slit_add_app(children[i]);
	}
    }
    XFree(children);
}

void slit_add_app(Window win)
{
    int i;
    XWMHints *h;
    XWindowAttributes attrib;
    struct Slit *s;

    s = &slit[0];

    if ((h = XGetWMHints(ob_display, win))) {
        if (h->flags & StateHint && h->initial_state == WithdrawnState) {
            struct SlitApp *app = g_new(struct SlitApp, 1);

            app->win = win;
            app->icon_win = (h->flags & IconWindowHint) ?
                h->icon_window : win;

            XFree(h);

            for (i = 0; i < nslits; ++i) {
                GList *it;
                for (it = slit[i].slit_apps; it; it = it->next)
                    if (app->icon_win ==
                        ((struct SlitApp*)it->data)->icon_win)
                        /* already managed! */
                        return;
            }

            if (XGetWindowAttributes(ob_display, app->icon_win, &attrib)) {
                app->w = attrib.width;
                app->h = attrib.height;
            } else {
                g_free(app);
                app = NULL;
            }

            if (app) {
                s->slit_apps = g_list_append(s->slit_apps, app);
                slit_configure();
                XReparentWindow(ob_display, app->icon_win,
                                s->frame, app->x, app->y);
/*                if (app->win != app->icon_win)
                  XUnmapWindow(ob_display, app->win);*/
                XSync(ob_display, False);
                XSelectInput(ob_display, app->icon_win,
                             SLITAPP_EVENT_MASK);
            }
            g_message("Managed: 0x%lx", app->icon_win);
        } else
            XFree(h);
    }
}

void slit_remove_app(struct Slit *slit, struct SlitApp *app, gboolean reparent)
{

    XSelectInput(ob_display, app->icon_win, NoEventMask);
    XSync(ob_display, False);
    if (reparent) {
        g_message("reparenting");
/*        if (app->win != app->icon_win)
          XMapWindow(ob_display, app->win);*/
        XReparentWindow(ob_display, app->icon_win, ob_root, 0, 0);
    }

    g_free(app);
    slit->slit_apps = g_list_remove(slit->slit_apps, app);
    slit_configure();
}

void slit_read_theme()
{
    XTextProperty prop;
    int i;
    char *theme = NULL;

    if (XGetTextProperty(ob_display, ob_root, &prop,
                         XInternAtom(ob_display, "_OPENBOX_THEME", False))) {
        theme = theme_load((char*)prop.value);
        XFree(prop.value);
    } else
        theme = theme_load(NULL);
 
    g_free(theme);
    if (!theme) exit(EXIT_FAILURE);

    for (i = 0; i < nslits; ++i) {
        appearance_free(slit[i].a_frame);
        appearance_free(slit[i].a_title);

        slit[i].a_frame = appearance_copy(theme_a_unfocused_title);
        slit[i].a_title = appearance_copy(theme_a_unfocused_title);

        XSetWindowBorder(ob_display, slit[i].frame, theme_b_color->pixel);
        XSetWindowBorderWidth(ob_display, slit[i].frame, theme_bwidth);
        XSetWindowBorder(ob_display, slit[i].frame, BlackPixel(ob_display, ob_screen));
        XSetWindowBorderWidth(ob_display, slit[i].frame, 30);
    }

    slit_configure();
}

void slit_configure()
{
    int i;
    int titleh;
    GList *it;
    int spot;

    titleh = 4;

    for (i = 0; i < nslits; ++i) {
        if (slit[i].horz) {
            slit[i].w = titleh;
            slit[i].h = 0;
        } else {
            slit[i].w = 0;
            slit[i].h = titleh;
        }
        spot = titleh;

        for (it = slit[i].slit_apps; it; it = it->next) {
            struct SlitApp *app = it->data;
            if (slit[i].horz) {
                g_message("%d", spot);
                app->x = spot;
                app->y = 0;
                slit[i].w += app->w;
                slit[i].h = MAX(slit[i].h, app->h);
                spot += app->w;
            } else {
                app->x = 0;
                app->y = spot;
                slit[i].w = MAX(slit[i].h, app->w);
                slit[i].h += app->h;
                spot += app->h;
            }

            XMoveWindow(ob_display, app->icon_win, app->x, app->y);
        }

        /* calculate position */
        slit[i].x = slit[i].user_x;
        slit[i].y = slit[i].user_y;

        switch(slit[i].gravity) {
        case NorthGravity:
        case CenterGravity:
        case SouthGravity:
            slit[i].x -= slit[i].w / 2;
            break;
        case NorthEastGravity:
        case EastGravity:
        case SouthEastGravity:
            slit[i].x -= slit[i].w;
            break;
        }
        switch(slit[i].gravity) {
        case WestGravity:
        case CenterGravity:
        case EastGravity:
            slit[i].y -= slit[i].h / 2;
            break;
        case SouthWestGravity:
        case SouthGravity:
        case SouthEastGravity:
            slit[i].y -= slit[i].h;
            break;
        }

        if (slit[i].w > 0 && slit[i].h > 0) {
            RECT_SET(slit[i].a_frame->area, 0, 0, slit[i].w, slit[i].h);
            XMoveResizeWindow(ob_display, slit[i].frame,
                              slit[i].x - theme_bwidth,
                              slit[i].y - theme_bwidth,
                              slit[i].w, slit[i].h);

            if (slit[i].horz) {
                RECT_SET(slit[i].a_title->area, 0, 0, titleh, slit[i].h);
                XMoveResizeWindow(ob_display, slit[i].title, 0, 0,
                                  titleh, slit[i].h);
            } else {
                RECT_SET(slit[i].a_title->area, 0, 0, slit[i].w, titleh);
                XMoveResizeWindow(ob_display, slit[i].title, 0, 0,
                                  slit[i].w, titleh);
            }

            paint(slit[i].frame, slit[i].a_frame);
            paint(slit[i].title, slit[i].a_title);
            XMapWindow(ob_display, slit[i].frame);
        } else
            XUnmapWindow(ob_display, slit[i].frame);
    }
}
