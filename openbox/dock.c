#include "dock.h"
#include "screen.h"
#include "config.h"
#include "grab.h"
#include "openbox.h"
#include "render/theme.h"

#define DOCK_EVENT_MASK (ButtonPressMask | ButtonReleaseMask | \
                         EnterWindowMask | LeaveWindowMask)
#define DOCKAPP_EVENT_MASK (StructureNotifyMask)

static Dock *dock;

Strut dock_strut;

void dock_startup()
{
    XSetWindowAttributes attrib;

    STRUT_SET(dock_strut, 0, 0, 0, 0);

    dock = g_new0(struct Dock, 1);
    dock->obwin.type = Window_Dock;

    dock->hidden = TRUE;

    attrib.event_mask = DOCK_EVENT_MASK;
    attrib.override_redirect = True;
    dock->frame = XCreateWindow(ob_display, ob_root, 0, 0, 1, 1, 0,
                                render_depth, InputOutput, render_visual,
                                CWOverrideRedirect | CWEventMask,
                                &attrib);
    dock->a_frame = appearance_copy(theme_a_unfocused_title);
    XSetWindowBorder(ob_display, dock->frame, theme_b_color->pixel);
    XSetWindowBorderWidth(ob_display, dock->frame, theme_bwidth);

    g_hash_table_insert(window_map, &dock->frame, dock);
    stacking_add(DOCK_AS_WINDOW(dock));
    stacking_raise(DOCK_AS_WINDOW(dock));
}

void dock_shutdown()
{
    XDestroyWindow(ob_display, dock->frame);
    appearance_free(dock->a_frame);
    g_hash_table_remove(window_map, &dock->frame);
    stacking_remove(dock);
}

void dock_add(Window win, XWMHints *wmhints)
{
    DockApp *app;
    XWindowAttributes attrib;

    app = g_new0(DockApp, 1);
    app->obwin.type = Window_DockApp;
    app->win = win;
    app->icon_win = (wmhints->flags & IconWindowHint) ?
        wmhints->icon_window : win;
    
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
    if (ob_state == State_Starting)
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
                     GrabModeAsync, ob_cursors.move);

    g_hash_table_insert(window_map, &app->icon_win, app);

    g_message("Managed Dock App: 0x%lx", app->icon_win);
}

void dock_remove_all()
{
    while (dock->dock_apps)
        dock_remove(dock->dock_apps->data, TRUE);
}

void dock_remove(DockApp *app, gboolean reparent)
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

    g_message("Unmanaged Dock App: 0x%lx", app->icon_win);

    g_free(app);
}

void dock_configure()
{
    GList *it;
    int spot;
    int gravity;

    dock->w = dock->h = spot = 0;

    for (it = dock->dock_apps; it; it = it->next) {
        struct DockApp *app = it->data;
        if (config_dock_horz) {
            app->x = spot;
            app->y = 0;
            dock->w += app->w;
            dock->h = MAX(dock->h, app->h);
            spot += app->w;
        } else {
            app->x = 0;
            app->y = spot;
            dock->w = MAX(dock->w, app->w);
            dock->h += app->h;
            spot += app->h;
        }

        XMoveWindow(ob_display, app->icon_win, app->x, app->y);
    }

    /* used for calculating offsets */
    dock->w += theme_bwidth * 2;
    dock->h += theme_bwidth * 2;

    /* calculate position */
    switch (config_dock_pos) {
    case DockPos_Floating:
        dock->x = config_dock_x;
        dock->y = config_dock_y;
        gravity = NorthWestGravity;
        break;
    case DockPos_TopLeft:
        dock->x = 0;
        dock->y = 0;
        gravity = NorthWestGravity;
        break;
    case DockPos_Top:
        dock->x = screen_physical_size.width / 2;
        dock->y = 0;
        gravity = NorthGravity;
        break;
    case DockPos_TopRight:
        dock->x = screen_physical_size.width;
        dock->y = 0;
        gravity = NorthEastGravity;
        break;
    case DockPos_Left:
        dock->x = 0;
        dock->y = screen_physical_size.height / 2;
        gravity = WestGravity;
        break;
    case DockPos_Right:
        dock->x = screen_physical_size.width;
        dock->y = screen_physical_size.height / 2;
        gravity = EastGravity;
        break;
    case DockPos_BottomLeft:
        dock->x = 0;
        dock->y = screen_physical_size.height;
        gravity = SouthWestGravity;
        break;
    case DockPos_Bottom:
        dock->x = screen_physical_size.width / 2;
        dock->y = screen_physical_size.height;
        gravity = SouthGravity;
        break;
    case DockPos_BottomRight:
        dock->x = screen_physical_size.width;
        dock->y = screen_physical_size.height;
        gravity = SouthEastGravity;
        break;
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
        switch (config_dock_pos) {
        case DockPos_Floating:
            break;
        case DockPos_TopLeft:
            if (config_dock_horz)
                dock->y -= dock->h - theme_bwidth;
            else
                dock->x -= dock->w - theme_bwidth;
            break;
        case DockPos_Top:
            dock->y -= dock->h - theme_bwidth;
            break;
        case DockPos_TopRight:
            if (config_dock_horz)
                dock->y -= dock->h - theme_bwidth;
            else
                dock->x += dock->w - theme_bwidth;
            break;
        case DockPos_Left:
            dock->x -= dock->w - theme_bwidth;
            break;
        case DockPos_Right:
            dock->x += dock->w - theme_bwidth;
            break;
        case DockPos_BottomLeft:
            if (config_dock_horz)
                dock->y += dock->h - theme_bwidth;
            else
                dock->x -= dock->w - theme_bwidth;
            break;
        case DockPos_Bottom:
            dock->y += dock->h - theme_bwidth;
            break;
        case DockPos_BottomRight:
            if (config_dock_horz)
                dock->y += dock->h - theme_bwidth;
            else
                dock->x += dock->w - theme_bwidth;
            break;
        }    
    }

    /* set the strut */
    switch (config_dock_pos) {
    case DockPos_Floating:
        STRUT_SET(dock_strut, 0, 0, 0, 0);
        break;
    case DockPos_TopLeft:
        if (config_dock_horz)
            STRUT_SET(dock_strut, 0, dock->h, 0, 0);
        else
            STRUT_SET(dock_strut, dock->w, 0, 0, 0);
        break;
    case DockPos_Top:
        STRUT_SET(dock_strut, 0, dock->h, 0, 0);
        break;
    case DockPos_TopRight:
        if (config_dock_horz)
            STRUT_SET(dock_strut, 0, dock->h, 0, 0);
        else
            STRUT_SET(dock_strut, 0, 0, dock->w, 0);
        break;
    case DockPos_Left:
        STRUT_SET(dock_strut, dock->w, 0, 0, 0);
        break;
    case DockPos_Right:
        STRUT_SET(dock_strut, 0, 0, dock->w, 0);
        break;
    case DockPos_BottomLeft:
        if (config_dock_horz)
            STRUT_SET(dock_strut, 0, 0, 0, dock->h);
        else
            STRUT_SET(dock_strut, dock->w, 0, 0, 0);
        break;
    case DockPos_Bottom:
        STRUT_SET(dock_strut, 0, 0, 0, dock->h);
        break;
    case DockPos_BottomRight:
        if (config_dock_horz)
            STRUT_SET(dock_strut, 0, 0, 0, dock->h);
        else
            STRUT_SET(dock_strut, 0, 0, dock->w, 0);
        break;
    }

    /* not used for actually sizing shit */
    dock->w -= theme_bwidth * 2;
    dock->h -= theme_bwidth * 2;

    if (dock->w > 0 && dock->h > 0) {
        RECT_SET(dock->a_frame->area, 0, 0, dock->w, dock->h);
        XMoveResizeWindow(ob_display, dock->frame,
                          dock->x, dock->y, dock->w, dock->h);

        paint(dock->frame, dock->a_frame);
        XMapWindow(ob_display, dock->frame);
    } else
        XUnmapWindow(ob_display, dock->frame);

    /* but they are useful outside of this function! */
    dock->w += theme_bwidth * 2;
    dock->h += theme_bwidth * 2;

    screen_update_struts();
}

void dock_app_configure(DockApp *app, int w, int h)
{
    app->w = w;
    app->h = h;
    dock_configure();
}

void dock_app_drag(DockApp *app, XMotionEvent *e)
{
    DockApp *over = NULL;
    GList *it;
    int x, y;
    gboolean after;

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
    for (it = dock->dock_apps; it; it = it->next) {
        over = it->data;
        if (config_dock_horz) {
            if (x >= over->x && x < over->x + over->w)
                break;
        } else {
            if (y >= over->y && y < over->y + over->h)
                break;
        }
    }
    if (!it || app == over) return;

    x -= over->x;
    y -= over->y;

    if (config_dock_horz)
        after = (x > over->w / 2);
    else
        after = (y > over->h / 2);

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
