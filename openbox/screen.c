#include "openbox.h"
#include "prop.h"
#include "screen.h"
#include "client.h"
#include "frame.h"
#include "engine.h"
#include "focus.h"
#include "dispatch.h"
#include "../render/render.h"

#include <X11/Xlib.h>
#ifdef HAVE_UNISTD_H
#  include <sys/types.h>
#  include <unistd.h>
#endif

/*! The event mask to grab on the root window */
#define ROOT_EVENTMASK (/*ColormapChangeMask |*/ PropertyChangeMask | \
			EnterWindowMask | LeaveWindowMask | \
			SubstructureNotifyMask | SubstructureRedirectMask | \
			ButtonPressMask | ButtonReleaseMask | ButtonMotionMask)

guint    screen_num_desktops    = 0;
guint    screen_desktop         = 0;
Size     screen_physical_size;
gboolean screen_showing_desktop;
DesktopLayout screen_desktop_layout;
GPtrArray *screen_desktop_names;

static Rect  *area = NULL;
static Strut *strut = NULL;
static Window support_window = None;

static void screen_update_area();

static gboolean running;
static int another_running(Display *d, XErrorEvent *e)
{
    (void)d;(void)e;
    g_message("A window manager is already running on screen %d",
	      ob_screen);
    running = TRUE;
    return -1;
}

gboolean screen_annex()
{
    XErrorHandler old;
    pid_t pid;
    int i, num_support;
    Atom *supported;

    running = FALSE;
    old = XSetErrorHandler(another_running);
    XSelectInput(ob_display, ob_root, ROOT_EVENTMASK);
    XSync(ob_display, FALSE);
    XSetErrorHandler(old);
    if (running)
	return FALSE;

    g_message("Managing screen %d", ob_screen);

    /* set the mouse cursor for the root window (the default cursor) */
    XDefineCursor(ob_display, ob_root, ob_cursors.left_ptr);

    /* set the OPENBOX_PID hint */
    pid = getpid();
    PROP_SET32(ob_root, openbox_pid, cardinal, pid);

    /* create the netwm support window */
    support_window = XCreateSimpleWindow(ob_display, ob_root, 0,0,1,1,0,0,0);

    /* set supporting window */
    PROP_SET32(ob_root, net_supporting_wm_check, window, support_window);

    /* set properties on the supporting window */
    PROP_SETS(support_window, net_wm_name, utf8, "Openbox");
    PROP_SET32(support_window, net_supporting_wm_check, window,support_window);

    /* set the _NET_SUPPORTED_ATOMS hint */
    num_support = 48;
    i = 0;
    supported = g_new(Atom, num_support);
    supported[i++] = prop_atoms.net_current_desktop;
    supported[i++] = prop_atoms.net_number_of_desktops;
    supported[i++] = prop_atoms.net_desktop_geometry;
    supported[i++] = prop_atoms.net_desktop_viewport;
    supported[i++] = prop_atoms.net_active_window;
    supported[i++] = prop_atoms.net_workarea;
    supported[i++] = prop_atoms.net_client_list;
    supported[i++] = prop_atoms.net_client_list_stacking;
    supported[i++] = prop_atoms.net_desktop_names;
    supported[i++] = prop_atoms.net_close_window;
    supported[i++] = prop_atoms.net_desktop_layout;
    supported[i++] = prop_atoms.net_showing_desktop;
    supported[i++] = prop_atoms.net_wm_name;
    supported[i++] = prop_atoms.net_wm_visible_name;
    supported[i++] = prop_atoms.net_wm_icon_name;
    supported[i++] = prop_atoms.net_wm_visible_icon_name;
    supported[i++] = prop_atoms.net_wm_desktop;
    supported[i++] = prop_atoms.net_wm_strut;
    supported[i++] = prop_atoms.net_wm_window_type;
    supported[i++] = prop_atoms.net_wm_window_type_desktop;
    supported[i++] = prop_atoms.net_wm_window_type_dock;
    supported[i++] = prop_atoms.net_wm_window_type_toolbar;
    supported[i++] = prop_atoms.net_wm_window_type_menu;
    supported[i++] = prop_atoms.net_wm_window_type_utility;
    supported[i++] = prop_atoms.net_wm_window_type_splash;
    supported[i++] = prop_atoms.net_wm_window_type_dialog;
    supported[i++] = prop_atoms.net_wm_window_type_normal;
    supported[i++] = prop_atoms.net_wm_allowed_actions;
    supported[i++] = prop_atoms.net_wm_action_move;
    supported[i++] = prop_atoms.net_wm_action_resize;
    supported[i++] = prop_atoms.net_wm_action_minimize;
    supported[i++] = prop_atoms.net_wm_action_shade;
    supported[i++] = prop_atoms.net_wm_action_maximize_horz;
    supported[i++] = prop_atoms.net_wm_action_maximize_vert;
    supported[i++] = prop_atoms.net_wm_action_fullscreen;
    supported[i++] = prop_atoms.net_wm_action_change_desktop;
    supported[i++] = prop_atoms.net_wm_action_close;
    supported[i++] = prop_atoms.net_wm_state;
    supported[i++] = prop_atoms.net_wm_state_modal;
    supported[i++] = prop_atoms.net_wm_state_maximized_vert;
    supported[i++] = prop_atoms.net_wm_state_maximized_horz;
    supported[i++] = prop_atoms.net_wm_state_shaded;
    supported[i++] = prop_atoms.net_wm_state_skip_taskbar;
    supported[i++] = prop_atoms.net_wm_state_skip_pager;
    supported[i++] = prop_atoms.net_wm_state_hidden;
    supported[i++] = prop_atoms.net_wm_state_fullscreen;
    supported[i++] = prop_atoms.net_wm_state_above;
    supported[i++] = prop_atoms.net_wm_state_below;
    g_assert(i == num_support);
/*
  supported[] = prop_atoms.net_wm_moveresize;
  supported[] = prop_atoms.net_wm_moveresize_size_topleft;
  supported[] = prop_atoms.net_wm_moveresize_size_topright;
  supported[] = prop_atoms.net_wm_moveresize_size_bottomleft;
  supported[] = prop_atoms.net_wm_moveresize_size_bottomright;
  supported[] = prop_atoms.net_wm_moveresize_move;
  supported[] = prop_atoms.net_wm_action_stick;
*/

    PROP_SET32A(ob_root, net_supported, atom, supported, num_support);
    g_free(supported);

    return TRUE;
}

void screen_startup()
{
    screen_desktop_names = g_ptr_array_new();
     
    /* get the initial size */
    screen_resize();

    screen_num_desktops = 0;
    screen_set_num_desktops(4);
    screen_desktop = 0;
    screen_set_desktop(0);

    /* don't start in showing-desktop mode */
    screen_showing_desktop = FALSE;
    PROP_SET32(ob_root, net_showing_desktop, cardinal, screen_showing_desktop);

    screen_update_layout();
}

void screen_shutdown()
{
    guint i;

    XSelectInput(ob_display, ob_root, NoEventMask);

    PROP_ERASE(ob_root, openbox_pid); /* we're not running here no more! */
    PROP_ERASE(ob_root, net_supported); /* not without us */
    PROP_ERASE(ob_root, net_showing_desktop); /* don't keep this mode */

    XDestroyWindow(ob_display, support_window);

    for (i = 0; i < screen_desktop_names->len; ++i)
	g_free(g_ptr_array_index(screen_desktop_names, i));
    g_ptr_array_free(screen_desktop_names, TRUE);
    g_free(strut);
    g_free(area);
}

void screen_resize()
{
    /* XXX RandR support here? */
    int geometry[2];

    /* Set the _NET_DESKTOP_GEOMETRY hint */
    geometry[0] = WidthOfScreen(ScreenOfDisplay(ob_display, ob_screen));
    geometry[1] = HeightOfScreen(ScreenOfDisplay(ob_display, ob_screen));
    PROP_SET32A(ob_root, net_desktop_geometry, cardinal, geometry, 2);
    screen_physical_size.width = geometry[0];
    screen_physical_size.height = geometry[1];

    if (ob_state == State_Starting)
	return;

    screen_update_struts();

    /* XXX adjust more stuff ? */
}

void screen_set_num_desktops(guint num)
{
    guint i, old;
    gulong *viewport;
    GList *it;

    g_assert(num > 0);

    old = screen_num_desktops;
    screen_num_desktops = num;
    PROP_SET32(ob_root, net_number_of_desktops, cardinal, num);

    /* set the viewport hint */
    viewport = g_new0(gulong, num * 2);
    PROP_SET32A(ob_root, net_desktop_viewport, cardinal, viewport, num * 2);
    g_free(viewport);

    /* change our struts/area to match */
    screen_update_struts();

    /* the number of rows/columns will differ */
    screen_update_layout();

    /* may be some unnamed desktops that we need to fill in with names */
    screen_update_desktop_names();

    /* update the focus lists */
    /* free our lists for the desktops which have disappeared */
    for (i = num; i < old; ++i)
        g_list_free(focus_order[i]);
    /* realloc the array */
    focus_order = g_renew(GList*, focus_order, num);
    /* set the new lists to be empty */
    for (i = old; i < num; ++i)
        focus_order[i] = NULL;

    /* move windows on desktops that will no longer exist! */
    for (it = client_list; it != NULL; it = it->next) {
        Client *c = it->data;
        if (c->desktop >= num)
            client_set_desktop(c, num - 1, FALSE);
    }

    dispatch_ob(Event_Ob_NumDesktops, num, old);

    /* change our desktop if we're on one that no longer exists! */
    if (screen_desktop >= screen_num_desktops)
	screen_set_desktop(num - 1);
}

void screen_set_desktop(guint num)
{
    GList *it;
    guint old;
    XEvent e;
     
    g_assert(num < screen_num_desktops);

    old = screen_desktop;
    screen_desktop = num;
    PROP_SET32(ob_root, net_current_desktop, cardinal, num);

    if (old == num) return;

    g_message("Moving to desktop %d", num+1);

    /* show windows before hiding the rest to lessen the enter/leave events */

    /* show windows from top to bottom */
    for (it = stacking_list; it != NULL; it = it->next) {
        Client *c = it->data;
	if (!c->frame->visible && client_should_show(c))
            engine_frame_show(c->frame);
    }

    /* hide windows from bottom to top */
    for (it = g_list_last(stacking_list); it != NULL; it = it->prev) {
        Client *c = it->data;
	if (c->frame->visible && !client_should_show(c))
            engine_frame_hide(c->frame);
    }

    /* focus the last focused window on the desktop, and ignore enter events
       from the switch so it doesnt mess with the focus */
    XSync(ob_display, FALSE);
    while (XCheckTypedEvent(ob_display, EnterNotify, &e));
    focus_fallback(Fallback_Desktop);

    dispatch_ob(Event_Ob_Desktop, num, old);
}

void screen_update_layout()
{
    unsigned long *data = NULL;

    /* defaults */
    screen_desktop_layout.orientation = prop_atoms.net_wm_orientation_horz;
    screen_desktop_layout.start_corner = prop_atoms.net_wm_topleft;
    screen_desktop_layout.rows = 1;
    screen_desktop_layout.columns = screen_num_desktops;

    if (PROP_GET32A(ob_root, net_desktop_layout, cardinal, data, 4)) {
	if (data[0] == prop_atoms.net_wm_orientation_vert)
	    screen_desktop_layout.orientation = data[0];
	if (data[3] == prop_atoms.net_wm_topright)
	    screen_desktop_layout.start_corner = data[3];
	else if (data[3] == prop_atoms.net_wm_bottomright)
	    screen_desktop_layout.start_corner = data[3];
	else if (data[3] == prop_atoms.net_wm_bottomleft)
	    screen_desktop_layout.start_corner = data[3];

	/* fill in a zero rows/columns */
	if (!(data[1] == 0 && data[2] == 0)) { /* both 0's is bad data.. */
	    if (data[1] == 0) {
		data[1] = (screen_num_desktops +
			   screen_num_desktops % data[2]) / data[2];
	    } else if (data[2] == 0) {
		data[2] = (screen_num_desktops +
			   screen_num_desktops % data[1]) / data[1];
	    }
	    screen_desktop_layout.columns = data[1];
	    screen_desktop_layout.rows = data[2];
	}

	/* bounds checking */
	if (screen_desktop_layout.orientation ==
	    prop_atoms.net_wm_orientation_horz) {
	    if (screen_desktop_layout.rows > screen_num_desktops)
		screen_desktop_layout.rows = screen_num_desktops;
	    if (screen_desktop_layout.columns > ((screen_num_desktops +
						  screen_num_desktops %
						  screen_desktop_layout.rows) /
						 screen_desktop_layout.rows))
		screen_desktop_layout.columns =
		    (screen_num_desktops + screen_num_desktops %
		     screen_desktop_layout.rows) /
		    screen_desktop_layout.rows;
	} else {
	    if (screen_desktop_layout.columns > screen_num_desktops)
		screen_desktop_layout.columns = screen_num_desktops;
	    if (screen_desktop_layout.rows > ((screen_num_desktops +
					       screen_num_desktops %
					       screen_desktop_layout.columns) /
					      screen_desktop_layout.columns))
		screen_desktop_layout.rows =
		    (screen_num_desktops + screen_num_desktops %
		     screen_desktop_layout.columns) /
		    screen_desktop_layout.columns;
	}
	g_free(data);
    }
}

void screen_update_desktop_names()
{
    guint i;

    /* empty the array */
    for (i = 0; i < screen_desktop_names->len; ++i)
	g_free(g_ptr_array_index(screen_desktop_names, i));
    g_ptr_array_set_size(screen_desktop_names, 0);

    PROP_GETSA(ob_root, net_desktop_names, utf8, screen_desktop_names);

    while (screen_desktop_names->len < screen_num_desktops)
	g_ptr_array_add(screen_desktop_names, g_strdup("Unnamed Desktop"));
}

void screen_show_desktop(gboolean show)
{
    GList *it;
     
    if (show == screen_showing_desktop) return; /* no change */

    screen_showing_desktop = show;

    if (show) {
	/* bottom to top */
	for (it = g_list_last(stacking_list); it != NULL; it = it->prev) {
	    Client *client = it->data;
	    if (client->frame->visible && !client_should_show(client))
                engine_frame_hide(client->frame);
	}
    } else {
        /* top to bottom */
	for (it = stacking_list; it != NULL; it = it->next) {
	    Client *client = it->data;
	    if (!client->frame->visible && client_should_show(client))
                engine_frame_show(client->frame);
	}
    }

    show = !!show; /* make it boolean */
    PROP_SET32(ob_root, net_showing_desktop, cardinal, show);

    dispatch_ob(Event_Ob_ShowDesktop, show, 0);
}

void screen_install_colormap(Client *client, gboolean install)
{
    XWindowAttributes wa;

    if (client == NULL) {
	if (install)
	    XInstallColormap(ob_display, render_colormap);
	else
	    XUninstallColormap(ob_display, render_colormap);
    } else {
	if (XGetWindowAttributes(ob_display, client->window, &wa) &&
            wa.colormap != None) {
	    if (install)
		XInstallColormap(ob_display, wa.colormap);
	    else
		XUninstallColormap(ob_display, wa.colormap);
	}
    }
}

void screen_update_struts()
{
    GList *it;
    guint i;
     
    g_free(strut);
    strut = g_new0(Strut, screen_num_desktops + 1);

    for (it = client_list; it != NULL; it = it->next) {
	Client *c = it->data;
	if (c->iconic) continue; /* these dont count in the strut */
    
	if (c->desktop == 0xffffffff) {
	    for (i = 0; i < screen_num_desktops; ++i)
		STRUT_ADD(strut[i], c->strut);
	} else {
	    g_assert(c->desktop < screen_num_desktops);
	    STRUT_ADD(strut[c->desktop], c->strut);
	}
	/* apply to the 'all desktops' strut */
	STRUT_ADD(strut[screen_num_desktops], c->strut);
    }
    screen_update_area();
}

static void screen_update_area()
{
    guint i;
    gulong *dims;

    g_free(area);
    area = g_new0(Rect, screen_num_desktops + 1);
     
    dims = g_new(unsigned long, 4 * screen_num_desktops);
    for (i = 0; i < screen_num_desktops + 1; ++i) {
	Rect old_area = area[i];
/*
  #ifdef    XINERAMA
  // reset to the full areas
  if (isXineramaActive())
  xineramaUsableArea = getXineramaAreas();
  #endif // XINERAMA
*/

	RECT_SET(area[i], strut[i].left, strut[i].top,
		 screen_physical_size.width - (strut[i].left +
					       strut[i].right),
		 screen_physical_size.height - (strut[i].top +
						strut[i].bottom));
    
/*
  #ifdef    XINERAMA
  if (isXineramaActive()) {
  // keep each of the ximerama-defined areas inside the strut
  RectList::iterator xit, xend = xineramaUsableArea.end();
  for (xit = xineramaUsableArea.begin(); xit != xend; ++xit) {
  if (xit->x() < usableArea.x()) {
  xit->setX(usableArea.x());
  xit->setWidth(xit->width() - usableArea.x());
  }
  if (xit->y() < usableArea.y()) {
  xit->setY(usableArea.y());
  xit->setHeight(xit->height() - usableArea.y());
  }
  if (xit->x() + xit->width() > usableArea.width())
  xit->setWidth(usableArea.width() - xit->x());
  if (xit->y() + xit->height() > usableArea.height())
  xit->setHeight(usableArea.height() - xit->y());
  }
  }
  #endif // XINERAMA
*/
	if (!RECT_EQUAL(old_area, area[i])) {
	    /* the area has changed, adjust all the maximized windows */
	    GList *it;
	    for (it = client_list; it; it = it->next) {
		Client *c = it->data;
		if (i < screen_num_desktops) {
		    if (c->desktop == i)
			client_remaximize(c);
		} else {
		    /* the 'all desktops' size */
		    if (c->desktop == DESKTOP_ALL)
			client_remaximize(c);
		}
	    }
	}

	/* don't set these for the 'all desktops' area */
	if (i < screen_num_desktops) {
	    dims[(i * 4) + 0] = area[i].x;
	    dims[(i * 4) + 1] = area[i].y;
	    dims[(i * 4) + 2] = area[i].width;
	    dims[(i * 4) + 3] = area[i].height;
	}
    }
    PROP_SET32A(ob_root, net_workarea, cardinal,
		dims, 4 * screen_num_desktops);
    g_free(dims);
}

Rect *screen_area(guint desktop)
{
    if (desktop >= screen_num_desktops) {
	if (desktop == DESKTOP_ALL)
	    return &area[screen_num_desktops];
	return NULL;
    }
    return &area[desktop];
}

Strut *screen_strut(guint desktop)
{
    if (desktop >= screen_num_desktops) {
	if (desktop == DESKTOP_ALL)
	    return &strut[screen_num_desktops];
	return NULL;
    }
    return &strut[desktop];
}
