#include "dock.h"
#include "screen.h"
#include "prop.h"
#include "config.h"
#include "grab.h"
#include "openbox.h"
#include "render/theme.h"

#define DOCK_EVENT_MASK (ButtonPressMask | ButtonReleaseMask | \
                         EnterWindowMask | LeaveWindowMask)
#define DOCKAPP_EVENT_MASK (StructureNotifyMask)

static ObDock *dock;

Strut dock_strut;

void dock_startup()
{
    XSetWindowAttributes attrib;

    STRUT_SET(dock_strut, 0, 0, 0, 0);

    dock = g_new0(ObDock, 1);
    dock->obwin.type = Window_Dock;

    dock->hidden = TRUE;

    attrib.event_mask = DOCK_EVENT_MASK;
    attrib.override_redirect = True;
    dock->frame = XCreateWindow(ob_display, ob_root, 0, 0, 1, 1, 0,
                                RrDepth(ob_rr_inst), InputOutput,
                                RrVisual(ob_rr_inst),
                                CWOverrideRedirect | CWEventMask,
                                &attrib);
    dock->a_frame = RrAppearanceCopy(ob_rr_theme->a_unfocused_title);
    XSetWindowBorder(ob_display, dock->frame,
                     RrColorPixel(ob_rr_theme->b_color));
    XSetWindowBorderWidth(ob_display, dock->frame, ob_rr_theme->bwidth);

    g_hash_table_insert(window_map, &dock->frame, dock);
    stacking_add(DOCK_AS_WINDOW(dock));
    stacking_raise(DOCK_AS_WINDOW(dock));
}

void dock_shutdown()
{
    XDestroyWindow(ob_display, dock->frame);
    RrAppearanceFree(dock->a_frame);
    g_hash_table_remove(window_map, &dock->frame);
    stacking_remove(dock);
}

void dock_add(Window win, XWMHints *wmhints)
{
    ObDockApp *app;
    XWindowAttributes attrib;
    gchar **data;

    app = g_new0(ObDockApp, 1);
    app->obwin.type = Window_DockApp;
    app->win = win;
    app->icon_win = (wmhints->flags & IconWindowHint) ?
        wmhints->icon_window : win;

    if (PROP_GETSS(app->win, wm_class, locale, &data)) {
        if (data[0]) {
	    app->name = g_strdup(data[0]);
            if (data[1])
                app->class = g_strdup(data[1]);
        }
        g_strfreev(data);     
    }

    if (app->name == NULL) app->name = g_strdup("");
    if (app->class == NULL) app->class = g_strdup("");
    
    if (XGetWindowAttributes(ob_display, app->icon_win, &attrib)) {
        app->w = attrib.width;
        app->h = attrib.height;
    } else {
        app->w = app->h = 64;
    }

    dock->dock_apps = g_list_append(dock->dock_apps, app);
    dock_configure();

    XReparentWindow(ob_display, app->icon_win, dock->frame, app->x, app->y);
    /*
      This is the same case as in frame.c for client windows. When Openbox is
      starting, the window is already mapped so we see unmap events occur for
      it. There are 2 unmap events generated that we see, one with the 'event'
      member set the root window, and one set to the client, but both get
      handled and need to be ignored.
    */
    if (ob_state == OB_STATE_STARTING)
	app->ignore_unmaps += 2;

    if (app->win != app->icon_win) {
        /* have to map it so that it can be re-managed on a restart */
        XMoveWindow(ob_display, app->win, -1000, -1000);
        XMapWindow(ob_display, app->win);
    }
    XMapWindow(ob_display, app->icon_win);
    XSync(ob_display, False);

    /* specify that if we exit, the window should not be destroyed and should
       be reparented back to root automatically */
    XChangeSaveSet(ob_display, app->icon_win, SetModeInsert);
    XSelectInput(ob_display, app->icon_win, DOCKAPP_EVENT_MASK);

    grab_button_full(2, 0, app->icon_win,
                     ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
                     GrabModeAsync, OB_CURSOR_MOVE);

    g_hash_table_insert(window_map, &app->icon_win, app);

    g_message("Managed Dock App: 0x%lx (%s)", app->icon_win, app->class);
}

void dock_remove_all()
{
    while (dock->dock_apps)
        dock_remove(dock->dock_apps->data, TRUE);
}

void dock_remove(ObDockApp *app, gboolean reparent)
{
    ungrab_button(2, 0, app->icon_win);
    XSelectInput(ob_display, app->icon_win, NoEventMask);
    /* remove the window from our save set */
    XChangeSaveSet(ob_display, app->icon_win, SetModeDelete);
    XSync(ob_display, False);

    g_hash_table_remove(window_map, &app->icon_win);

    if (reparent)
	XReparentWindow(ob_display, app->icon_win, ob_root, app->x, app->y);

    dock->dock_apps = g_list_remove(dock->dock_apps, app);
    dock_configure();

    g_message("Unmanaged Dock App: 0x%lx (%s)", app->icon_win, app->class);

    g_free(app->name);
    g_free(app->class);
    g_free(app);
}

void dock_configure()
{
    GList *it;
    gint spot;
    gint gravity;
    gint minw, minh;
    gint strw, strh;
    Rect *a;

    RrMinsize(dock->a_frame, &minw, &minh);

    dock->w = dock->h = 0;

    /* get the size */
    for (it = dock->dock_apps; it; it = it->next) {
        ObDockApp *app = it->data;
        switch (config_dock_orient) {
        case OB_ORIENTATION_HORZ:
            dock->w += app->w;
            dock->h = MAX(dock->h, app->h);
            break;
        case OB_ORIENTATION_VERT:
            dock->w = MAX(dock->w, app->w);
            dock->h += app->h;
            break;
        }
    }

    spot = (config_dock_orient == OB_ORIENTATION_HORZ ? minw : minh) / 2;

    /* position the apps */
    for (it = dock->dock_apps; it; it = it->next) {
        ObDockApp *app = it->data;
        switch (config_dock_orient) {
        case OB_ORIENTATION_HORZ:
            app->x = spot;
            app->y = (dock->h - app->h) / 2;
            spot += app->w;
            break;
        case OB_ORIENTATION_VERT:
            app->x = (dock->w - app->w) / 2;
            app->y = spot;
            spot += app->h;
            break;
        }

        XMoveWindow(ob_display, app->icon_win, app->x, app->y);
    }

    /* used for calculating offsets */
    dock->w += ob_rr_theme->bwidth * 2;
    dock->h += ob_rr_theme->bwidth * 2;

    a = screen_physical_area();

    /* calculate position */
    if (config_dock_floating) {
        dock->x = config_dock_x;
        dock->y = config_dock_y;
        gravity = NorthWestGravity;
    } else {
        switch (config_dock_pos) {
        case OB_DIRECTION_NORTHWEST:
            dock->x = 0;
            dock->y = 0;
            gravity = NorthWestGravity;
            break;
        case OB_DIRECTION_NORTH:
            dock->x = a->width / 2;
            dock->y = 0;
            gravity = NorthGravity;
            break;
        case OB_DIRECTION_NORTHEAST:
            dock->x = a->width;
            dock->y = 0;
            gravity = NorthEastGravity;
            break;
        case OB_DIRECTION_WEST:
            dock->x = 0;
            dock->y = a->height / 2;
            gravity = WestGravity;
            break;
        case OB_DIRECTION_EAST:
            dock->x = a->width;
            dock->y = a->height / 2;
            gravity = EastGravity;
            break;
        case OB_DIRECTION_SOUTHWEST:
            dock->x = 0;
            dock->y = a->height;
            gravity = SouthWestGravity;
            break;
        case OB_DIRECTION_SOUTH:
            dock->x = a->width / 2;
            dock->y = a->height;
            gravity = SouthGravity;
            break;
        case OB_DIRECTION_SOUTHEAST:
            dock->x = a->width;
            dock->y = a->height;
            gravity = SouthEastGravity;
            break;
        }
    }

    switch(gravity) {
    case NorthGravity:
    case CenterGravity:
    case SouthGravity:
        dock->x -= dock->w / 2;
        break;
    case NorthEastGravity:
    case EastGravity:
    case SouthEastGravity:
        dock->x -= dock->w;
        break;
    }
    switch(gravity) {
    case WestGravity:
    case CenterGravity:
    case EastGravity:
        dock->y -= dock->h / 2;
        break;
    case SouthWestGravity:
    case SouthGravity:
    case SouthEastGravity:
        dock->y -= dock->h;
        break;
    }

    if (config_dock_hide && dock->hidden) {
        if (!config_dock_floating) {
            switch (config_dock_pos) {
            case OB_DIRECTION_NORTHWEST:
                switch (config_dock_orient) {
                case OB_ORIENTATION_HORZ:
                    dock->y -= dock->h - ob_rr_theme->bwidth;
                    break;
                case OB_ORIENTATION_VERT:
                    dock->x -= dock->w - ob_rr_theme->bwidth;
                    break;
                }
                break;
            case OB_DIRECTION_NORTH:
                dock->y -= dock->h - ob_rr_theme->bwidth;
                break;
            case OB_DIRECTION_NORTHEAST:
                switch (config_dock_orient) {
                case OB_ORIENTATION_HORZ:
                    dock->y -= dock->h - ob_rr_theme->bwidth;
                    break;
                case OB_ORIENTATION_VERT:
                    dock->x += dock->w - ob_rr_theme->bwidth;
                    break;
                }
                break;
            case OB_DIRECTION_WEST:
                dock->x -= dock->w - ob_rr_theme->bwidth;
                break;
            case OB_DIRECTION_EAST:
                dock->x += dock->w - ob_rr_theme->bwidth;
                break;
            case OB_DIRECTION_SOUTHWEST:
                switch (config_dock_orient) {
                case OB_ORIENTATION_HORZ:
                    dock->y += dock->h - ob_rr_theme->bwidth;
                    break;
                case OB_ORIENTATION_VERT:
                    dock->x -= dock->w - ob_rr_theme->bwidth;
                    break;
                } break;
            case OB_DIRECTION_SOUTH:
                dock->y += dock->h - ob_rr_theme->bwidth;
                break;
            case OB_DIRECTION_SOUTHEAST:
                switch (config_dock_orient) {
                case OB_ORIENTATION_HORZ:
                    dock->y += dock->h - ob_rr_theme->bwidth;
                    break;
                case OB_ORIENTATION_VERT:
                    dock->x += dock->w - ob_rr_theme->bwidth;
                    break;
                }
                break;
            }    
        }
    }

    if (!config_dock_floating && config_dock_hide) {
        strw = strh = ob_rr_theme->bwidth;
    } else {
        strw = dock->w;
        strh =  dock->h;
    }

    /* set the strut */
    if (config_dock_floating) {
        STRUT_SET(dock_strut, 0, 0, 0, 0);
    } else {
        switch (config_dock_pos) {
        case OB_DIRECTION_NORTHWEST:
            switch (config_dock_orient) {
            case OB_ORIENTATION_HORZ:
                STRUT_SET(dock_strut, 0, strh, 0, 0);
                break;
            case OB_ORIENTATION_VERT:
                STRUT_SET(dock_strut, strw, 0, 0, 0);
                break;
            }
            break;
        case OB_DIRECTION_NORTH:
            STRUT_SET(dock_strut, 0, strh, 0, 0);
            break;
        case OB_DIRECTION_NORTHEAST:
            switch (config_dock_orient) {
            case OB_ORIENTATION_HORZ:
                STRUT_SET(dock_strut, 0, strh, 0, 0);
                break;
            case OB_ORIENTATION_VERT:
                STRUT_SET(dock_strut, 0, 0, strw, 0);
                break;
            }
            break;
        case OB_DIRECTION_WEST:
            STRUT_SET(dock_strut, strw, 0, 0, 0);
            break;
        case OB_DIRECTION_EAST:
            STRUT_SET(dock_strut, 0, 0, strw, 0);
            break;
        case OB_DIRECTION_SOUTHWEST:
            switch (config_dock_orient) {
            case OB_ORIENTATION_HORZ:
                STRUT_SET(dock_strut, 0, 0, 0, strh);
                break;
            case OB_ORIENTATION_VERT:
                STRUT_SET(dock_strut, strw, 0, 0, 0);
                break;
            }
            break;
        case OB_DIRECTION_SOUTH:
            STRUT_SET(dock_strut, 0, 0, 0, strh);
            break;
        case OB_DIRECTION_SOUTHEAST:
            switch (config_dock_orient) {
            case OB_ORIENTATION_HORZ:
                STRUT_SET(dock_strut, 0, 0, 0, strh);
                break;
            case OB_ORIENTATION_VERT:
                STRUT_SET(dock_strut, 0, 0, strw, 0);
                break;
            }
            break;
        }
    }

    dock->w += minw;
    dock->h += minh;

    /* not used for actually sizing shit */
    dock->w -= ob_rr_theme->bwidth * 2;
    dock->h -= ob_rr_theme->bwidth * 2;

    if (dock->w > 0 && dock->h > 0) {
        XMoveResizeWindow(ob_display, dock->frame,
                          dock->x, dock->y, dock->w, dock->h);

        RrPaint(dock->a_frame, dock->frame, dock->w, dock->h);
        XMapWindow(ob_display, dock->frame);
    } else
        XUnmapWindow(ob_display, dock->frame);

    /* but they are useful outside of this function! */
    dock->w += ob_rr_theme->bwidth * 2;
    dock->h += ob_rr_theme->bwidth * 2;

    screen_update_areas();
}

void dock_app_configure(ObDockApp *app, gint w, gint h)
{
    app->w = w;
    app->h = h;
    dock_configure();
}

void dock_app_drag(ObDockApp *app, XMotionEvent *e)
{
    ObDockApp *over = NULL;
    GList *it;
    gint x, y;
    gboolean after;
    gboolean stop;

    x = e->x_root;
    y = e->y_root;

    /* are we on top of the dock? */
    if (!(x >= dock->x &&
          y >= dock->y &&
          x < dock->x + dock->w &&
          y < dock->y + dock->h))
        return;

    x -= dock->x;
    y -= dock->y;

    /* which dock app are we on top of? */
    stop = FALSE;
    for (it = dock->dock_apps; it && !stop; it = it->next) {
        over = it->data;
        switch (config_dock_orient) {
        case OB_ORIENTATION_HORZ:
            if (x >= over->x && x < over->x + over->w)
                stop = TRUE;
            break;
        case OB_ORIENTATION_VERT:
            if (y >= over->y && y < over->y + over->h)
                stop = TRUE;
            break;
        }
    }
    if (!it || app == over) return;

    x -= over->x;
    y -= over->y;

    switch (config_dock_orient) {
    case OB_ORIENTATION_HORZ:
        after = (x > over->w / 2);
        break;
    case OB_ORIENTATION_VERT:
        after = (y > over->h / 2);
        break;
    }

    /* remove before doing the it->next! */
    dock->dock_apps = g_list_remove(dock->dock_apps, app);

    if (after) it = it->next;

    dock->dock_apps = g_list_insert_before(dock->dock_apps, it, app);
    dock_configure();
}

static void hide_timeout(void *n)
{
    /* dont repeat */
    timer_stop(dock->hide_timer);
    dock->hide_timer = NULL;

    /* hide */
    dock->hidden = TRUE;
    dock_configure();
}

void dock_hide(gboolean hide)
{
    if (dock->hidden == hide || !config_dock_hide)
        return;
    if (!hide) {
        /* show */
        dock->hidden = FALSE;
        dock_configure();

        /* if was hiding, stop it */
        if (dock->hide_timer) {
            timer_stop(dock->hide_timer);
            dock->hide_timer = NULL;
        }
    } else {
        g_assert(!dock->hide_timer);
        dock->hide_timer = timer_start(config_dock_hide_timeout * 1000,
                                       (TimeoutHandler)hide_timeout,
                                       NULL);
    }
}
