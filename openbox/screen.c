#include "debug.h"
#include "openbox.h"
#include "dock.h"
#include "xerror.h"
#include "prop.h"
#include "grab.h"
#include "startupnotify.h"
#include "config.h"
#include "screen.h"
#include "client.h"
#include "frame.h"
#include "focus.h"
#include "popup.h"
#include "extensions.h"
#include "render/render.h"

#include <X11/Xlib.h>
#ifdef HAVE_UNISTD_H
#  include <sys/types.h>
#  include <unistd.h>
#endif
#include <assert.h>

/*! The event mask to grab on the root window */
#define ROOT_EVENTMASK (StructureNotifyMask | PropertyChangeMask | \
			EnterWindowMask | LeaveWindowMask | \
			SubstructureNotifyMask | SubstructureRedirectMask | \
			ButtonPressMask | ButtonReleaseMask | ButtonMotionMask)

guint    screen_num_desktops;
guint    screen_num_monitors;
guint    screen_desktop;
guint    screen_last_desktop;
Size     screen_physical_size;
gboolean screen_showing_desktop;
DesktopLayout screen_desktop_layout;
char   **screen_desktop_names;
Window   screen_support_win;

static Rect  **area; /* array of desktop holding array of xinerama areas */
static Rect  *monitor_area;

static Popup *desktop_cycle_popup;

static gboolean replace_wm()
{
    char *wm_sn;
    Atom wm_sn_atom;
    Window current_wm_sn_owner;
    Time timestamp;

    wm_sn = g_strdup_printf("WM_S%d", ob_screen);
    wm_sn_atom = XInternAtom(ob_display, wm_sn, FALSE);
    g_free(wm_sn);

    current_wm_sn_owner = XGetSelectionOwner(ob_display, wm_sn_atom);
    if (current_wm_sn_owner) {
        if (!ob_replace_wm) {
            g_warning("A window manager is already running on screen %d",
                      ob_screen);
            return FALSE;
        }
        xerror_set_ignore(TRUE);
        xerror_occured = FALSE;

        /* We want to find out when the current selection owner dies */
        XSelectInput(ob_display, current_wm_sn_owner, StructureNotifyMask);
        XSync(ob_display, FALSE);

        xerror_set_ignore(FALSE);
        if (xerror_occured)
            current_wm_sn_owner = None;
    }

    {
        /* Generate a timestamp */
        XEvent event;

        XSelectInput(ob_display, screen_support_win, PropertyChangeMask);

        XChangeProperty(ob_display, screen_support_win,
                        prop_atoms.wm_class, prop_atoms.string,
                        8, PropModeAppend, NULL, 0);
        XWindowEvent(ob_display, screen_support_win,
                     PropertyChangeMask, &event);

        XSelectInput(ob_display, screen_support_win, NoEventMask);

        timestamp = event.xproperty.time;
    }

    XSetSelectionOwner(ob_display, wm_sn_atom, screen_support_win,
                       timestamp);

    if (XGetSelectionOwner(ob_display, wm_sn_atom) != screen_support_win) {
        g_warning("Could not acquire window manager selection on screen %d",
                  ob_screen);
        return FALSE;
    }

    /* Wait for old window manager to go away */
    if (current_wm_sn_owner) {
      XEvent event;
      gulong wait = 0;
      const gulong timeout = G_USEC_PER_SEC * 15; /* wait for 15s max */

      while (wait < timeout) {
          if (XCheckWindowEvent(ob_display, current_wm_sn_owner,
                                StructureNotifyMask, &event) &&
              event.type == DestroyNotify)
              break;
          g_usleep(G_USEC_PER_SEC / 10);
          wait += G_USEC_PER_SEC / 10;
      }

      if (wait >= timeout) {
          g_warning("Timeout expired while waiting for the current WM to die "
                    "on screen %d", ob_screen);
          return FALSE;
      }
    }

    /* Send client message indicating that we are now the WM */
    prop_message(RootWindow(ob_display, ob_screen), prop_atoms.manager,
                 timestamp, wm_sn_atom, 0, 0, SubstructureNotifyMask);


    return TRUE;
}

gboolean screen_annex()
{
    XSetWindowAttributes attrib;
    pid_t pid;
    gint i, num_support;
    guint32 *supported;

    /* create the netwm support window */
    attrib.override_redirect = TRUE;
    screen_support_win = XCreateWindow(ob_display,
                                       RootWindow(ob_display, ob_screen),
                                       -100, -100, 1, 1, 0,
                                       CopyFromParent, InputOutput,
                                       CopyFromParent,
                                       CWOverrideRedirect, &attrib);
    XMapRaised(ob_display, screen_support_win);

    if (!replace_wm()) {
        XDestroyWindow(ob_display, screen_support_win);
        return FALSE;
    }

    xerror_set_ignore(TRUE);
    xerror_occured = FALSE;
    XSelectInput(ob_display, RootWindow(ob_display, ob_screen),
                 ROOT_EVENTMASK);
    xerror_set_ignore(FALSE);
    if (xerror_occured) {
        g_warning("A window manager is already running on screen %d",
                  ob_screen);

        XDestroyWindow(ob_display, screen_support_win);
	return FALSE;
    }


    screen_set_root_cursor();

    /* set the OPENBOX_PID hint */
    pid = getpid();
    PROP_SET32(RootWindow(ob_display, ob_screen),
               openbox_pid, cardinal, pid);

    /* set supporting window */
    PROP_SET32(RootWindow(ob_display, ob_screen),
               net_supporting_wm_check, window, screen_support_win);

    /* set properties on the supporting window */
    PROP_SETS(screen_support_win, net_wm_name, "Openbox");
    PROP_SET32(screen_support_win, net_supporting_wm_check,
               window, screen_support_win);

    /* set the _NET_SUPPORTED_ATOMS hint */
    num_support = 50;
    i = 0;
    supported = g_new(guint32, num_support);
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
    supported[i++] = prop_atoms.net_moveresize_window;
    supported[i++] = prop_atoms.net_wm_moveresize;
    g_assert(i == num_support);
/*
  supported[] = prop_atoms.net_wm_action_stick;
*/

    PROP_SETA32(RootWindow(ob_display, ob_screen),
                net_supported, atom, supported, num_support);
    g_free(supported);

    return TRUE;
}

void screen_startup(gboolean reconfig)
{
    GSList *it;
    guint i;

    desktop_cycle_popup = popup_new(FALSE);

    if (!reconfig)
        /* get the initial size */
        screen_resize();

    /* set the names */
    screen_desktop_names = g_new(char*,
                                 g_slist_length(config_desktops_names) + 1);
    for (i = 0, it = config_desktops_names; it; ++i, it = it->next)
        screen_desktop_names[i] = it->data; /* dont strdup */
    screen_desktop_names[i] = NULL;
    PROP_SETSS(RootWindow(ob_display, ob_screen),
               net_desktop_names, screen_desktop_names);
    g_free(screen_desktop_names); /* dont free the individual strings */
    screen_desktop_names = NULL;

    if (!reconfig)
        screen_num_desktops = 0;
    screen_set_num_desktops(config_desktops_num);
    if (!reconfig) {
        screen_set_desktop(0);

        /* don't start in showing-desktop mode */
        screen_showing_desktop = FALSE;
        PROP_SET32(RootWindow(ob_display, ob_screen),
                   net_showing_desktop, cardinal, screen_showing_desktop);

        screen_update_layout();
    }
}

void screen_shutdown(gboolean reconfig)
{
    Rect **r;

    popup_free(desktop_cycle_popup);

    if (!reconfig) {
        XSelectInput(ob_display, RootWindow(ob_display, ob_screen),
                     NoEventMask);

        /* we're not running here no more! */
        PROP_ERASE(RootWindow(ob_display, ob_screen), openbox_pid);
        /* not without us */
        PROP_ERASE(RootWindow(ob_display, ob_screen), net_supported);
        /* don't keep this mode */
        PROP_ERASE(RootWindow(ob_display, ob_screen), net_showing_desktop);

        XDestroyWindow(ob_display, screen_support_win);
    }

    g_strfreev(screen_desktop_names);
    screen_desktop_names = NULL;
    for (r = area; *r; ++r)
        g_free(*r);
    g_free(area);
    area = NULL;
}

void screen_resize()
{
    static int oldw = 0, oldh = 0;
    int w, h;
    GList *it;
    guint32 geometry[2];

    w = WidthOfScreen(ScreenOfDisplay(ob_display, ob_screen));
    h = HeightOfScreen(ScreenOfDisplay(ob_display, ob_screen));

    if (w == oldw && h == oldh) return;

    oldw = w; oldh = h;

    /* Set the _NET_DESKTOP_GEOMETRY hint */
    screen_physical_size.width = geometry[0] = w;
    screen_physical_size.height = geometry[1] = h;
    PROP_SETA32(RootWindow(ob_display, ob_screen),
                net_desktop_geometry, cardinal, geometry, 2);

    if (ob_state() == OB_STATE_STARTING)
	return;

    screen_update_areas();
    dock_configure();

    for (it = client_list; it; it = it->next)
        client_move_onscreen(it->data, FALSE);
}

void screen_set_num_desktops(guint num)
{
    guint i, old;
    guint32 *viewport;
    GList *it;

    g_assert(num > 0);

    old = screen_num_desktops;
    screen_num_desktops = num;
    PROP_SET32(RootWindow(ob_display, ob_screen),
               net_number_of_desktops, cardinal, num);

    /* set the viewport hint */
    viewport = g_new0(guint32, num * 2);
    PROP_SETA32(RootWindow(ob_display, ob_screen),
                net_desktop_viewport, cardinal, viewport, num * 2);
    g_free(viewport);

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
        ObClient *c = it->data;
        if (c->desktop >= num && c->desktop != DESKTOP_ALL)
            client_set_desktop(c, num - 1, FALSE);
    }

    /* change our struts/area to match (after moving windows) */
    screen_update_areas();

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
    PROP_SET32(RootWindow(ob_display, ob_screen),
               net_current_desktop, cardinal, num);

    if (old == num) return;

    screen_last_desktop = old;

    ob_debug("Moving to desktop %d\n", num+1);

    /* show windows before hiding the rest to lessen the enter/leave events */

    /* show windows from top to bottom */
    for (it = stacking_list; it != NULL; it = it->next) {
        if (WINDOW_IS_CLIENT(it->data)) {
            ObClient *c = it->data;
            if (!c->frame->visible && client_should_show(c))
                frame_show(c->frame);
        }
    }

    /* hide windows from bottom to top */
    for (it = g_list_last(stacking_list); it != NULL; it = it->prev) {
        if (WINDOW_IS_CLIENT(it->data)) {
            ObClient *c = it->data;
            if (c->frame->visible && !client_should_show(c))
                frame_hide(c->frame);
        }
    }

    /* focus the last focused window on the desktop, and ignore enter events
       from the switch so it doesnt mess with the focus */
    while (XCheckTypedEvent(ob_display, EnterNotify, &e));
#ifdef DEBUG_FOCUS
    ob_debug("switch fallback\n");
#endif
    focus_fallback(OB_FOCUS_FALLBACK_DESKTOP);
#ifdef DEBUG_FOCUS
    ob_debug("/switch fallback\n");
#endif
}

static void get_row_col(guint d, guint *r, guint *c)
{
    switch (screen_desktop_layout.orientation) {
    case OB_ORIENTATION_HORZ:
        switch (screen_desktop_layout.start_corner) {
        case OB_CORNER_TOPLEFT:
            *r = d / screen_desktop_layout.columns;
            *c = d % screen_desktop_layout.columns;
            break;
        case OB_CORNER_BOTTOMLEFT:
            *r = screen_desktop_layout.rows - 1 -
                d / screen_desktop_layout.columns;
            *c = d % screen_desktop_layout.columns;
            break;
        case OB_CORNER_TOPRIGHT:
            *r = d / screen_desktop_layout.columns;
            *c = screen_desktop_layout.columns - 1 -
                d % screen_desktop_layout.columns;
            break;
        case OB_CORNER_BOTTOMRIGHT:
            *r = screen_desktop_layout.rows - 1 -
                d / screen_desktop_layout.columns;
            *c = screen_desktop_layout.columns - 1 -
                d % screen_desktop_layout.columns;
            break;
        }
        break;
    case OB_ORIENTATION_VERT:
        switch (screen_desktop_layout.start_corner) {
        case OB_CORNER_TOPLEFT:
            *r = d % screen_desktop_layout.rows;
            *c = d / screen_desktop_layout.rows;
            break;
        case OB_CORNER_BOTTOMLEFT:
            *r = screen_desktop_layout.rows - 1 -
                d % screen_desktop_layout.rows;
            *c = d / screen_desktop_layout.rows;
            break;
        case OB_CORNER_TOPRIGHT:
            *r = d % screen_desktop_layout.rows;
            *c = screen_desktop_layout.columns - 1 -
                d / screen_desktop_layout.rows;
            break;
        case OB_CORNER_BOTTOMRIGHT:
            *r = screen_desktop_layout.rows - 1 -
                d % screen_desktop_layout.rows;
            *c = screen_desktop_layout.columns - 1 -
                d / screen_desktop_layout.rows;
            break;
        }
        break;
    }
}

static guint translate_row_col(guint r, guint c)
{
    switch (screen_desktop_layout.orientation) {
    case OB_ORIENTATION_HORZ:
        switch (screen_desktop_layout.start_corner) {
        case OB_CORNER_TOPLEFT:
            return r % screen_desktop_layout.rows *
                screen_desktop_layout.columns +
                c % screen_desktop_layout.columns;
        case OB_CORNER_BOTTOMLEFT:
            return (screen_desktop_layout.rows - 1 -
                    r % screen_desktop_layout.rows) *
                screen_desktop_layout.columns +
                c % screen_desktop_layout.columns;
        case OB_CORNER_TOPRIGHT:
            return r % screen_desktop_layout.rows *
                screen_desktop_layout.columns +
                (screen_desktop_layout.columns - 1 -
                 c % screen_desktop_layout.columns);
        case OB_CORNER_BOTTOMRIGHT:
            return (screen_desktop_layout.rows - 1 -
                    r % screen_desktop_layout.rows) *
                screen_desktop_layout.columns +
                (screen_desktop_layout.columns - 1 -
                 c % screen_desktop_layout.columns);
        }
    case OB_ORIENTATION_VERT:
        switch (screen_desktop_layout.start_corner) {
        case OB_CORNER_TOPLEFT:
            return c % screen_desktop_layout.columns *
                screen_desktop_layout.rows +
                r % screen_desktop_layout.rows;
        case OB_CORNER_BOTTOMLEFT:
            return c % screen_desktop_layout.columns *
                screen_desktop_layout.rows +
                (screen_desktop_layout.rows - 1 -
                 r % screen_desktop_layout.rows);
        case OB_CORNER_TOPRIGHT:
            return (screen_desktop_layout.columns - 1 -
                    c % screen_desktop_layout.columns) *
                screen_desktop_layout.rows +
                r % screen_desktop_layout.rows;
        case OB_CORNER_BOTTOMRIGHT:
            return (screen_desktop_layout.columns - 1 -
                    c % screen_desktop_layout.columns) *
                screen_desktop_layout.rows +
                (screen_desktop_layout.rows - 1 -
                 r % screen_desktop_layout.rows);
        }
    }
    g_assert_not_reached();
    return 0;
}

static void popup_cycle(guint d, gboolean show)
{
    Rect *a;

    if (!show) {
        popup_hide(desktop_cycle_popup);
    } else {
        a = screen_physical_area_monitor(0);
        popup_position(desktop_cycle_popup, CenterGravity,
                       a->x + a->width / 2, a->y + a->height / 2);
        /* XXX the size and the font extents need to be related on some level
         */
        popup_size(desktop_cycle_popup, POPUP_WIDTH, POPUP_HEIGHT);

        popup_set_text_align(desktop_cycle_popup, RR_JUSTIFY_CENTER);

        popup_show(desktop_cycle_popup,
                   screen_desktop_names[d], NULL);
    }
}

guint screen_cycle_desktop(ObDirection dir, gboolean wrap, gboolean linear,
                           gboolean dialog, gboolean done, gboolean cancel)
{
    static gboolean first = TRUE;
    static gboolean lin;
    static guint origd, d;
    guint r, c;

    if (cancel) {
        d = origd;
        goto done_cycle;
    } else if (done && dialog) {
        goto done_cycle;
    }
    if (first) {
        first = FALSE;
        lin = linear;
        d = origd = screen_desktop;
    }

    get_row_col(d, &r, &c);

    if (lin) {
        switch (dir) {
        case OB_DIRECTION_EAST:
            if (d < screen_num_desktops - 1)
                ++d;
            else if (wrap)
                d = 0;
            break;
        case OB_DIRECTION_WEST:
            if (d > 0)
                --d;
            else if (wrap)
                d = screen_num_desktops - 1;
            break;
        default:
            assert(0);
            return screen_desktop;
        }
    } else {
        switch (dir) {
        case OB_DIRECTION_EAST:
            ++c;
            if (c >= screen_desktop_layout.columns) {
                if (!wrap) return d = screen_desktop;
                c = 0;
            }
            d = translate_row_col(r, c);
            if (d >= screen_num_desktops) {
                if (!wrap) return d = screen_desktop;
                ++c;
            }
            break;
        case OB_DIRECTION_WEST:
            --c;
            if (c >= screen_desktop_layout.columns) {
                if (!wrap) return d = screen_desktop;
                c = screen_desktop_layout.columns - 1;
            }
            d = translate_row_col(r, c);
            if (d >= screen_num_desktops) {
                if (!wrap) return d = screen_desktop;
                --c;
            }
            break;
        case OB_DIRECTION_SOUTH:
            ++r;
            if (r >= screen_desktop_layout.rows) {
                if (!wrap) return d = screen_desktop;
                r = 0;
            }
            d = translate_row_col(r, c);
            if (d >= screen_num_desktops) {
                if (!wrap) return d = screen_desktop;
                ++r;
            }
            break;
        case OB_DIRECTION_NORTH:
            --r;
            if (r >= screen_desktop_layout.rows) {
                if (!wrap) return d = screen_desktop;
                r = screen_desktop_layout.rows - 1;
            }
            d = translate_row_col(r, c);
            if (d >= screen_num_desktops) {
                if (!wrap) return d = screen_desktop;
                --r;
            }
            break;
        default:
            assert(0);
            return d = screen_desktop;
        }

        d = translate_row_col(r, c);
    }

    if (dialog) {
        popup_cycle(d, TRUE);
        return d;
    }

done_cycle:
    first = TRUE;

    popup_cycle(0, FALSE);

    return d;
}

void screen_update_layout()
{
    ObOrientation orient;
    ObCorner corner;
    guint rows;
    guint cols;
    guint32 *data;
    guint num;
    gboolean valid = FALSE;

    if (PROP_GETA32(RootWindow(ob_display, ob_screen),
                    net_desktop_layout, cardinal, &data, &num)) {
        if (num == 3 || num == 4) {

            if (data[0] == prop_atoms.net_wm_orientation_vert)
                orient = OB_ORIENTATION_VERT;
            else if (data[0] == prop_atoms.net_wm_orientation_horz)
                orient = OB_ORIENTATION_HORZ;
            else
                goto screen_update_layout_bail;

            if (num < 4)
                corner = OB_CORNER_TOPLEFT;
            else {
                if (data[3] == prop_atoms.net_wm_topleft)
                    corner = OB_CORNER_TOPLEFT;
                else if (data[3] == prop_atoms.net_wm_topright)
                    corner = OB_CORNER_TOPRIGHT;
                else if (data[3] == prop_atoms.net_wm_bottomright)
                    corner = OB_CORNER_BOTTOMRIGHT;
                else if (data[3] == prop_atoms.net_wm_bottomleft)
                    corner = OB_CORNER_BOTTOMLEFT;
                else
                    goto screen_update_layout_bail;
            }

            /* fill in a zero rows/columns */
            if ((data[1] == 0 && data[2] == 0) || /* both 0's is bad data.. */
                (data[1] != 0 && data[2] != 0)) { /* no 0's is bad data.. */
                goto screen_update_layout_bail;
            } else {
                if (data[1] == 0) {
                    data[1] = (screen_num_desktops +
                               screen_num_desktops % data[2]) / data[2];
                } else if (data[2] == 0) {
                    data[2] = (screen_num_desktops +
                               screen_num_desktops % data[1]) / data[1];
                }
                cols = data[1];
                rows = data[2];
            }

            /* bounds checking */
            if (orient == OB_ORIENTATION_HORZ) {
                rows = MIN(rows, screen_num_desktops);
                cols = MIN(cols, ((screen_num_desktops +
                                     (screen_num_desktops % rows)) / rows));
            } else {
                cols = MIN(cols, screen_num_desktops);
                rows = MIN(rows, ((screen_num_desktops +
                                     (screen_num_desktops % cols)) / cols));
            }

            valid = TRUE;
        }
    screen_update_layout_bail:
        g_free(data);
    }

    if (!valid) {
        /* defaults */
        orient = OB_ORIENTATION_HORZ;
        corner = OB_CORNER_TOPLEFT;
        rows = 1;
        cols = screen_num_desktops;
    }

    screen_desktop_layout.orientation = orient;
    screen_desktop_layout.start_corner = corner;
    screen_desktop_layout.rows = rows;
    screen_desktop_layout.columns = cols;
}

void screen_update_desktop_names()
{
    guint i;

    /* empty the array */
    g_strfreev(screen_desktop_names);
    screen_desktop_names = NULL;

    if (PROP_GETSS(RootWindow(ob_display, ob_screen),
                   net_desktop_names, utf8, &screen_desktop_names))
        for (i = 0; screen_desktop_names[i] && i <= screen_num_desktops; ++i);
    else
        i = 0;
    if (i <= screen_num_desktops) {
        screen_desktop_names = g_renew(char*, screen_desktop_names,
                                       screen_num_desktops + 1);
        screen_desktop_names[screen_num_desktops] = NULL;
        for (; i < screen_num_desktops; ++i)
            screen_desktop_names[i] = g_strdup("Unnamed Desktop");
    }
}

void screen_show_desktop(gboolean show)
{
    GList *it;
     
    if (show == screen_showing_desktop) return; /* no change */

    screen_showing_desktop = show;

    if (show) {
	/* bottom to top */
	for (it = g_list_last(stacking_list); it != NULL; it = it->prev) {
            if (WINDOW_IS_CLIENT(it->data)) {
                ObClient *client = it->data;
                if (client->frame->visible && !client_should_show(client))
                    frame_hide(client->frame);
            }
	}
    } else {
        /* top to bottom */
	for (it = stacking_list; it != NULL; it = it->next) {
            if (WINDOW_IS_CLIENT(it->data)) {
                ObClient *client = it->data;
                if (!client->frame->visible && client_should_show(client))
                    frame_show(client->frame);
            }
	}
    }

    if (show) {
        /* focus desktop */
        for (it = focus_order[screen_desktop]; it; it = it->next)
            if (((ObClient*)it->data)->type == OB_CLIENT_TYPE_DESKTOP &&
                client_focus(it->data))
                break;
    } else {
        focus_fallback(OB_FOCUS_FALLBACK_NOFOCUS);
    }

    show = !!show; /* make it boolean */
    PROP_SET32(RootWindow(ob_display, ob_screen),
               net_showing_desktop, cardinal, show);
}

void screen_install_colormap(ObClient *client, gboolean install)
{
    XWindowAttributes wa;

    if (client == NULL) {
	if (install)
	    XInstallColormap(RrDisplay(ob_rr_inst), RrColormap(ob_rr_inst));
	else
	    XUninstallColormap(RrDisplay(ob_rr_inst), RrColormap(ob_rr_inst));
    } else {
	if (XGetWindowAttributes(ob_display, client->window, &wa) &&
            wa.colormap != None) {
            xerror_set_ignore(TRUE);
	    if (install)
		XInstallColormap(RrDisplay(ob_rr_inst), wa.colormap);
	    else
		XUninstallColormap(RrDisplay(ob_rr_inst), wa.colormap);
            xerror_set_ignore(FALSE);
	}
    }
}

void screen_update_areas()
{
    guint i, x;
    guint32 *dims;
    GList *it;

    g_free(monitor_area);
    extensions_xinerama_screens(&monitor_area, &screen_num_monitors);

    if (area) {
        for (i = 0; area[i]; ++i)
            g_free(area[i]);
        g_free(area);
    }

    area = g_new(Rect*, screen_num_desktops + 2);
    for (i = 0; i < screen_num_desktops + 1; ++i)
        area[i] = g_new(Rect, screen_num_monitors + 1);
    area[i] = NULL;
     
    dims = g_new(guint32, 4 * screen_num_desktops);

    for (i = 0; i < screen_num_desktops + 1; ++i) {
        Strut s;
        int l, r, t, b;

        /* calc the xinerama areas */
        for (x = 0; x < screen_num_monitors; ++x) {
            area[i][x] = monitor_area[x];
            if (x == 0) {
                l = monitor_area[x].x;
                t = monitor_area[x].y;
                r = monitor_area[x].x + monitor_area[x].width - 1;
                b = monitor_area[x].y + monitor_area[x].height - 1;
            } else {
                l = MIN(l, monitor_area[x].x);
                t = MIN(t, monitor_area[x].y);
                r = MAX(r, monitor_area[x].x + monitor_area[x].width - 1);
                b = MAX(b, monitor_area[x].y + monitor_area[x].height - 1);
            }
        }
        RECT_SET(area[i][x], l, t, r - l + 1, b - t + 1);

        /* apply struts */
        STRUT_SET(s, 0, 0, 0, 0);
        for (it = client_list; it; it = it->next)
            STRUT_ADD(s, ((ObClient*)it->data)->strut);
        STRUT_ADD(s, dock_strut);

        if (s.left) {
            int o;

            /* find the left-most xin heads, i do this in 2 loops :| */
            o = area[i][0].x;
            for (x = 1; x < screen_num_monitors; ++x)
                o = MIN(o, area[i][x].x);

            for (x = 0; x < screen_num_monitors; ++x) {
                int edge = o + s.left - area[i][x].x;
                if (edge > 0) {
                    area[i][x].x += edge;
                    area[i][x].width -= edge;
                }
            }

            area[i][screen_num_monitors].x += s.left;
            area[i][screen_num_monitors].width -= s.left;
        }
        if (s.top) {
            int o;

            /* find the left-most xin heads, i do this in 2 loops :| */
            o = area[i][0].y;
            for (x = 1; x < screen_num_monitors; ++x)
                o = MIN(o, area[i][x].y);

            for (x = 0; x < screen_num_monitors; ++x) {
                int edge = o + s.top - area[i][x].y;
                if (edge > 0) {
                    area[i][x].y += edge;
                    area[i][x].height -= edge;
                }
            }

            area[i][screen_num_monitors].y += s.top;
            area[i][screen_num_monitors].height -= s.top;
        }
        if (s.right) {
            int o;

            /* find the bottom-most xin heads, i do this in 2 loops :| */
            o = area[i][0].x + area[i][0].width - 1;
            for (x = 1; x < screen_num_monitors; ++x)
                o = MAX(o, area[i][x].x + area[i][x].width - 1);

            for (x = 0; x < screen_num_monitors; ++x) {
                int edge = (area[i][x].x + area[i][x].width - 1) -
                    (o - s.right);
                if (edge > 0)
                    area[i][x].width -= edge;
            }

            area[i][screen_num_monitors].width -= s.right;
        }
        if (s.bottom) {
            int o;

            /* find the bottom-most xin heads, i do this in 2 loops :| */
            o = area[i][0].y + area[i][0].height - 1;
            for (x = 1; x < screen_num_monitors; ++x)
                o = MAX(o, area[i][x].y + area[i][x].height - 1);

            for (x = 0; x < screen_num_monitors; ++x) {
                int edge = (area[i][x].y + area[i][x].height - 1) -
                    (o - s.bottom);
                if (edge > 0)
                    area[i][x].height -= edge;
            }

            area[i][screen_num_monitors].height -= s.bottom;
        }

        /* XXX when dealing with partial struts, if its in a single
           xinerama area, then only subtract it from that area's space
        for (x = 0; x < screen_num_monitors; ++x) {
	    GList *it;


               do something smart with it for the 'all xinerama areas' one...

	    for (it = client_list; it; it = it->next) {

                XXX if gunna test this shit, then gotta worry about when
                the client moves between xinerama heads..

                if (RECT_CONTAINS_RECT(((ObClient*)it->data)->frame->area,
                                       area[i][x])) {

                }            
            }
        }
        */

        /* XXX optimize when this is run? */

        /* the area has changed, adjust all the maximized 
           windows */
        for (it = client_list; it; it = it->next) {
            ObClient *c = it->data; 
            if (i < screen_num_desktops) {
                if (c->desktop == i)
                    client_reconfigure(c);
            } else if (c->desktop == DESKTOP_ALL)
                client_reconfigure(c);
        }
        if (i < screen_num_desktops) {
            /* don't set these for the 'all desktops' area */
            dims[(i * 4) + 0] = area[i][screen_num_monitors].x;
            dims[(i * 4) + 1] = area[i][screen_num_monitors].y;
            dims[(i * 4) + 2] = area[i][screen_num_monitors].width;
            dims[(i * 4) + 3] = area[i][screen_num_monitors].height;
        }
    }
    PROP_SETA32(RootWindow(ob_display, ob_screen), net_workarea, cardinal,
                dims, 4 * screen_num_desktops);

    g_free(dims);
}

Rect *screen_area(guint desktop)
{
    return screen_area_monitor(desktop, screen_num_monitors);
}

Rect *screen_area_monitor(guint desktop, guint head)
{
    if (head > screen_num_monitors)
        return NULL;
    if (desktop >= screen_num_desktops) {
	if (desktop == DESKTOP_ALL)
	    return &area[screen_num_desktops][head];
	return NULL;
    }
    return &area[desktop][head];
}

Rect *screen_physical_area()
{
    return screen_physical_area_monitor(screen_num_monitors);
}

Rect *screen_physical_area_monitor(guint head)
{
    if (head > screen_num_monitors)
        return NULL;
    return &monitor_area[head];
}

void screen_set_root_cursor()
{
    if (sn_app_starting())
        XDefineCursor(ob_display, RootWindow(ob_display, ob_screen),
                      ob_cursor(OB_CURSOR_BUSY));
    else
        XDefineCursor(ob_display, RootWindow(ob_display, ob_screen),
                      ob_cursor(OB_CURSOR_POINTER));
}

gboolean screen_pointer_pos(int *x, int *y)
{
    Window w;
    int i;
    guint u;

    return !!XQueryPointer(ob_display, RootWindow(ob_display, ob_screen),
                           &w, &w, x, y, &i, &i, &u);
}
