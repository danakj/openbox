#include "client.h"
#include "grab.h"
#include "focus.h"
#include "moveresize.h"
#include "menu.h"
#include "prop.h"
#include "stacking.h"
#include "frame.h"
#include "framerender.h"
#include "screen.h"
#include "action.h"
#include "dispatch.h"
#include "openbox.h"

#include <glib.h>

Action *action_new(void (*func)(union ActionData *data))
{
    Action *a = g_new0(Action, 1);
    a->func = func;

    return a;
}

void action_free(Action *a)
{
    if (a == NULL) return;

    /* deal with pointers */
    if (a->func == action_execute || a->func == action_restart)
        g_free(a->data.execute.path);
    else if (a->func == action_showmenu)
        g_free(a->data.showmenu.name);

    g_free(a);
}

Action *action_from_string(char *name)
{
    Action *a = NULL;
    if (!g_ascii_strcasecmp(name, "execute")) {
        a = action_new(action_execute);
    } else if (!g_ascii_strcasecmp(name, "focus")) {
        a = action_new(action_focus);
    } else if (!g_ascii_strcasecmp(name, "unfocus")) {
        a = action_new(action_unfocus);
    } else if (!g_ascii_strcasecmp(name, "iconify")) {
        a = action_new(action_iconify);
    } else if (!g_ascii_strcasecmp(name, "raise")) {
        a = action_new(action_raise);
    } else if (!g_ascii_strcasecmp(name, "lower")) {
        a = action_new(action_lower);
    } else if (!g_ascii_strcasecmp(name, "focusraise")) {
        a = action_new(action_focusraise);
    } else if (!g_ascii_strcasecmp(name, "close")) {
        a = action_new(action_close);
    } else if (!g_ascii_strcasecmp(name, "kill")) {
        a = action_new(action_kill);
    } else if (!g_ascii_strcasecmp(name, "shadelower")) {
        a = action_new(action_shadelower);
    } else if (!g_ascii_strcasecmp(name, "unshaderaise")) {
        a = action_new(action_unshaderaise);
    } else if (!g_ascii_strcasecmp(name, "shade")) {
        a = action_new(action_shade);
    } else if (!g_ascii_strcasecmp(name, "unshade")) {
        a = action_new(action_unshade);
    } else if (!g_ascii_strcasecmp(name, "toggleshade")) {
        a = action_new(action_toggle_shade);
    } else if (!g_ascii_strcasecmp(name, "toggleomnipresent")) {
        a = action_new(action_toggle_omnipresent);
    } else if (!g_ascii_strcasecmp(name, "moverelativehorz")) {
        a = action_new(action_move_relative_horz);
    } else if (!g_ascii_strcasecmp(name, "moverelativevert")) {
        a = action_new(action_move_relative_vert);
    } else if (!g_ascii_strcasecmp(name, "resizerelativehorz")) {
        a = action_new(action_resize_relative_horz);
    } else if (!g_ascii_strcasecmp(name, "resizerelativevert")) {
        a = action_new(action_resize_relative_vert);
    } else if (!g_ascii_strcasecmp(name, "maximizefull")) {
        a = action_new(action_maximize_full);
    } else if (!g_ascii_strcasecmp(name, "unmaximizefull")) {
        a = action_new(action_unmaximize_full);
    } else if (!g_ascii_strcasecmp(name, "togglemaximizefull")) {
        a = action_new(action_toggle_maximize_full);
    } else if (!g_ascii_strcasecmp(name, "maximizehorz")) {
        a = action_new(action_maximize_horz);
    } else if (!g_ascii_strcasecmp(name, "unmaximizehorz")) {
        a = action_new(action_unmaximize_horz);
    } else if (!g_ascii_strcasecmp(name, "togglemaximizehorz")) {
        a = action_new(action_toggle_maximize_horz);
    } else if (!g_ascii_strcasecmp(name, "maximizevert")) {
        a = action_new(action_maximize_vert);
    } else if (!g_ascii_strcasecmp(name, "unmaximizevert")) {
        a = action_new(action_unmaximize_vert);
    } else if (!g_ascii_strcasecmp(name, "togglemaximizevert")) {
        a = action_new(action_toggle_maximize_vert);
    } else if (!g_ascii_strcasecmp(name, "sendtodesktop")) {
        a = action_new(action_send_to_desktop);
        a->data.sendto.follow = TRUE;
    } else if (!g_ascii_strcasecmp(name, "sendtonextdesktop")) {
        a = action_new(action_send_to_next_desktop);
        a->data.sendtonextprev.wrap = FALSE;
        a->data.sendtonextprev.follow = TRUE;
    } else if (!g_ascii_strcasecmp(name, "sendtonextdesktopwrap")) {
        a = action_new(action_send_to_next_desktop);
        a->data.sendtonextprev.wrap = TRUE;
        a->data.sendtonextprev.follow = TRUE;
    } else if (!g_ascii_strcasecmp(name, "sendtopreviousdesktop")) {
        a = action_new(action_send_to_previous_desktop);
        a->data.sendtonextprev.wrap = FALSE;
        a->data.sendtonextprev.follow = TRUE;
    } else if (!g_ascii_strcasecmp(name, "sendtopreviousdesktopwrap")) {
        a = action_new(action_send_to_previous_desktop);
        a->data.sendtonextprev.wrap = TRUE;
        a->data.sendtonextprev.follow = TRUE;
    } else if (!g_ascii_strcasecmp(name, "desktop")) {
        a = action_new(action_desktop);
    } else if (!g_ascii_strcasecmp(name, "nextdesktop")) {
        a = action_new(action_next_desktop);
        a->data.nextprevdesktop.wrap = FALSE;
    } else if (!g_ascii_strcasecmp(name, "nextdesktopwrap")) {
        a = action_new(action_next_desktop);
        a->data.nextprevdesktop.wrap = TRUE;
    } else if (!g_ascii_strcasecmp(name, "previousdesktop")) {
        a = action_new(action_previous_desktop);
        a->data.nextprevdesktop.wrap = FALSE;
    } else if (!g_ascii_strcasecmp(name, "previousdesktopwrap")) {
        a = action_new(action_previous_desktop);
        a->data.nextprevdesktop.wrap = TRUE;
    } else if (!g_ascii_strcasecmp(name, "nextdesktopcolumn")) {
        a = action_new(action_next_desktop_column);
        a->data.nextprevdesktop.wrap = FALSE;
    } else if (!g_ascii_strcasecmp(name, "nextdesktopcolumnwrap")) {
        a = action_new(action_next_desktop_column);
        a->data.nextprevdesktop.wrap = TRUE;
    } else if (!g_ascii_strcasecmp(name, "previousdesktopcolumn")) {
        a = action_new(action_previous_desktop_column);
        a->data.nextprevdesktop.wrap = FALSE;
    } else if (!g_ascii_strcasecmp(name, "previousdesktopcolumnwrap")) {
        a = action_new(action_previous_desktop_column);
        a->data.nextprevdesktop.wrap = TRUE;
    } else if (!g_ascii_strcasecmp(name, "nextdesktoprow")) {
        a = action_new(action_next_desktop_row);
        a->data.nextprevdesktop.wrap = FALSE;
    } else if (!g_ascii_strcasecmp(name, "nextdesktoprowwrap")) {
        a = action_new(action_next_desktop_row);
        a->data.nextprevdesktop.wrap = TRUE;
    } else if (!g_ascii_strcasecmp(name, "previousdesktoprow")) {
        a = action_new(action_previous_desktop_row);
        a->data.nextprevdesktop.wrap = FALSE;
    } else if (!g_ascii_strcasecmp(name, "previousdesktoprowwrap")) {
        a = action_new(action_previous_desktop_row);
        a->data.nextprevdesktop.wrap = TRUE;
    } else if (!g_ascii_strcasecmp(name, "toggledecorations")) {
        a = action_new(action_toggle_decorations);
    } else if (!g_ascii_strcasecmp(name, "keyboardmove")) {
        a = action_new(action_moveresize);
        a->data.moveresize.corner = prop_atoms.net_wm_moveresize_move_keyboard;
    } else if (!g_ascii_strcasecmp(name, "move")) {
        a = action_new(action_moveresize);
        a->data.moveresize.corner = prop_atoms.net_wm_moveresize_move;
    } else if (!g_ascii_strcasecmp(name, "resize")) {
        a = action_new(action_moveresize);
        a->data.moveresize.corner = prop_atoms.net_wm_moveresize_size_topleft;
    } else if (!g_ascii_strcasecmp(name, "keyboardresize")) {
        a = action_new(action_moveresize);
        a->data.moveresize.corner = prop_atoms.net_wm_moveresize_size_keyboard;
    } else if (!g_ascii_strcasecmp(name, "restart")) {
        a = action_new(action_restart);
    } else if (!g_ascii_strcasecmp(name, "exit")) {
        a = action_new(action_exit);
    } else if (!g_ascii_strcasecmp(name, "showmenu")) {
        a = action_new(action_showmenu);
    } else if (!g_ascii_strcasecmp(name, "nextwindowlinear")) {
        a = action_new(action_cycle_windows);
        a->data.cycle.linear = TRUE;
        a->data.cycle.forward = TRUE;
    } else if (!g_ascii_strcasecmp(name, "previouswindowlinear")) {
        a = action_new(action_cycle_windows);
        a->data.cycle.linear = TRUE;
        a->data.cycle.forward = FALSE;
    } else if (!g_ascii_strcasecmp(name, "nextwindow")) {
        a = action_new(action_cycle_windows);
        a->data.cycle.linear = FALSE;
        a->data.cycle.forward = TRUE;
    } else if (!g_ascii_strcasecmp(name, "previouswindow")) {
        a = action_new(action_cycle_windows);
        a->data.cycle.linear = FALSE;
        a->data.cycle.forward = FALSE;
    }
    
    return a;
}

void action_execute(union ActionData *data)
{
    GError *e = NULL;
    if (data->execute.path)
        if (!g_spawn_command_line_async(data->execute.path, &e)) {
            g_warning("failed to execute '%s': %s",
                      data->execute.path, e->message);
        }
}

void action_focus(union ActionData *data)
{
    if (data->client.c)
        client_focus(data->client.c);
}

void action_unfocus (union ActionData *data)
{
    if (data->client.c)
        client_unfocus(data->client.c);
}

void action_iconify(union ActionData *data)
{
    if (data->client.c)
        client_iconify(data->client.c, TRUE, TRUE);
}

void action_focusraise(union ActionData *data)
{
    if (data->client.c) {
        client_focus(data->client.c);
        stacking_raise(data->client.c);
    }
}

void action_raise(union ActionData *data)
{
    if (data->client.c)
        stacking_raise(data->client.c);
}

void action_unshaderaise(union ActionData *data)
{
    if (data->client.c) {
        if (data->client.c->shaded)
            client_shade(data->client.c, FALSE);
        else
            stacking_raise(data->client.c);
    }
}

void action_shadelower(union ActionData *data)
{
    if (data->client.c) {
        if (data->client.c->shaded)
            stacking_lower(data->client.c);
        else
            client_shade(data->client.c, TRUE);
    }
}

void action_lower(union ActionData *data)
{
    if (data->client.c)
        stacking_lower(data->client.c);
}

void action_close(union ActionData *data)
{
    if (data->client.c)
        client_close(data->client.c);
}

void action_kill(union ActionData *data)
{
    if (data->client.c)
        client_kill(data->client.c);
}

void action_shade(union ActionData *data)
{
    if (data->client.c)
        client_shade(data->client.c, TRUE);
}

void action_unshade(union ActionData *data)
{
    if (data->client.c)
        client_shade(data->client.c, FALSE);
}

void action_toggle_shade(union ActionData *data)
{
    if (data->client.c)
        client_shade(data->client.c, !data->client.c->shaded);
}

void action_toggle_omnipresent(union ActionData *data)
{ 
    if (data->client.c)
        client_set_desktop(data->client.c,
                           data->client.c->desktop == DESKTOP_ALL ?
                           screen_desktop : DESKTOP_ALL, FALSE);
}

void action_move_relative_horz(union ActionData *data)
{
    Client *c = data->relative.c;
    if (c)
        client_configure(c, Corner_TopLeft,
                         c->area.x + data->relative.delta, c->area.y,
                         c->area.width, c->area.height, TRUE, TRUE);
}

void action_move_relative_vert(union ActionData *data)
{
    Client *c = data->relative.c;
    if (c)
        client_configure(c, Corner_TopLeft,
                         c->area.x, c->area.y + data->relative.delta,
                         c->area.width, c->area.height, TRUE, TRUE);
}

void action_resize_relative_horz(union ActionData *data)
{
    Client *c = data->relative.c;
    if (c)
        client_configure(c, Corner_TopLeft, c->area.x, c->area.y,
                         c->area.width + data->relative.delta,
                         c->area.height, TRUE, TRUE);
}

void action_resize_relative_vert(union ActionData *data)
{
    Client *c = data->relative.c;
    if (c && !c->shaded)
        client_configure(c, Corner_TopLeft, c->area.x, c->area.y,
                         c->area.width, c->area.height + data->relative.delta,
                         TRUE, TRUE);
}

void action_maximize_full(union ActionData *data)
{
    if (data->client.c)
        client_maximize(data->client.c, TRUE, 0, TRUE);
}

void action_unmaximize_full(union ActionData *data)
{
    if (data->client.c)
        client_maximize(data->client.c, FALSE, 0, TRUE);
}

void action_toggle_maximize_full(union ActionData *data)
{
    if (data->client.c)
        client_maximize(data->client.c,
                        !(data->client.c->max_horz ||
                          data->client.c->max_vert),
                        0, TRUE);
}

void action_maximize_horz(union ActionData *data)
{
    if (data->client.c)
        client_maximize(data->client.c, TRUE, 1, TRUE);
}

void action_unmaximize_horz(union ActionData *data)
{
    if (data->client.c)
        client_maximize(data->client.c, FALSE, 1, TRUE);
}

void action_toggle_maximize_horz(union ActionData *data)
{
    if (data->client.c)
        client_maximize(data->client.c, !data->client.c->max_horz, 1, TRUE);
}

void action_maximize_vert(union ActionData *data)
{
    if (data->client.c)
        client_maximize(data->client.c, TRUE, 2, TRUE);
}

void action_unmaximize_vert(union ActionData *data)
{
    if (data->client.c)
        client_maximize(data->client.c, FALSE, 2, TRUE);
}

void action_toggle_maximize_vert(union ActionData *data)
{
    if (data->client.c)
        client_maximize(data->client.c, !data->client.c->max_vert, 2, TRUE);
}

void action_send_to_desktop(union ActionData *data)
{
    if (data->sendto.c) {
        if (data->sendto.desk < screen_num_desktops ||
            data->sendto.desk == DESKTOP_ALL) {
            client_set_desktop(data->desktop.c,
                               data->sendto.desk, data->sendto.follow);
            if (data->sendto.follow) screen_set_desktop(data->sendto.desk);
        }
    }
}

void action_send_to_next_desktop(union ActionData *data)
{
    guint d;

    if (!data->sendtonextprev.c) return;

    d = screen_desktop + 1;
    if (d >= screen_num_desktops) {
        if (!data->sendtonextprev.wrap) return;
        d = 0;
    }
    client_set_desktop(data->sendtonextprev.c, d, data->sendtonextprev.follow);
    if (data->sendtonextprev.follow) screen_set_desktop(d);
}

void action_send_to_previous_desktop(union ActionData *data)
{
    guint d;

    if (!data->sendtonextprev.c) return;

    d = screen_desktop - 1;
    if (d >= screen_num_desktops) {
        if (!data->sendtonextprev.wrap) return;
        d = screen_num_desktops - 1;
    }
    client_set_desktop(data->sendtonextprev.c, d, data->sendtonextprev.follow);
    if (data->sendtonextprev.follow) screen_set_desktop(d);
}

void action_desktop(union ActionData *data)
{
    if (data->desktop.desk < screen_num_desktops ||
        data->desktop.desk == DESKTOP_ALL)
        screen_set_desktop(data->desktop.desk);
}

void action_next_desktop(union ActionData *data)
{
    guint d;

    d = screen_desktop + 1;
    if (d >= screen_num_desktops) {
        if (!data->nextprevdesktop.wrap) return;
        d = 0;
    }
    screen_set_desktop(d);
}

void action_previous_desktop(union ActionData *data)
{
    guint d;

    d = screen_desktop - 1;
    if (d >= screen_num_desktops) {
        if (!data->nextprevdesktop.wrap) return;
        d = screen_num_desktops - 1;
    }
    screen_set_desktop(d);
}

static void cur_row_col(guint *r, guint *c)
{
    switch (screen_desktop_layout.orientation) {
    case Orientation_Horz:
        switch (screen_desktop_layout.start_corner) {
        case Corner_TopLeft:
            *r = screen_desktop / screen_desktop_layout.columns;
            *c = screen_desktop % screen_desktop_layout.columns;
            break;
        case Corner_BottomLeft:
            *r = screen_desktop_layout.rows - 1 -
                screen_desktop / screen_desktop_layout.columns;
            *c = screen_desktop % screen_desktop_layout.columns;
            break;
        case Corner_TopRight:
            *r = screen_desktop / screen_desktop_layout.columns;
            *c = screen_desktop_layout.columns - 1 -
                screen_desktop % screen_desktop_layout.columns;
            break;
        case Corner_BottomRight:
            *r = screen_desktop_layout.rows - 1 -
                screen_desktop / screen_desktop_layout.columns;
            *c = screen_desktop_layout.columns - 1 -
                screen_desktop % screen_desktop_layout.columns;
            break;
        }
        break;
    case Orientation_Vert:
        switch (screen_desktop_layout.start_corner) {
        case Corner_TopLeft:
            *r = screen_desktop % screen_desktop_layout.rows;
            *c = screen_desktop / screen_desktop_layout.rows;
            break;
        case Corner_BottomLeft:
            *r = screen_desktop_layout.rows - 1 -
                screen_desktop % screen_desktop_layout.rows;
            *c = screen_desktop / screen_desktop_layout.rows;
            break;
        case Corner_TopRight:
            *r = screen_desktop % screen_desktop_layout.rows;
            *c = screen_desktop_layout.columns - 1 -
                screen_desktop / screen_desktop_layout.rows;
            break;
        case Corner_BottomRight:
            *r = screen_desktop_layout.rows - 1 -
                screen_desktop % screen_desktop_layout.rows;
            *c = screen_desktop_layout.columns - 1 -
                screen_desktop / screen_desktop_layout.rows;
            break;
        }
        break;
    }
}

static guint translate_row_col(guint r, guint c)
{
    switch (screen_desktop_layout.orientation) {
    case Orientation_Horz:
        switch (screen_desktop_layout.start_corner) {
        case Corner_TopLeft:
            return r * screen_desktop_layout.columns + c;
        case Corner_BottomLeft:
            return (screen_desktop_layout.rows - 1 - r) *
                screen_desktop_layout.columns + c;
        case Corner_TopRight:
            return r * screen_desktop_layout.columns +
                (screen_desktop_layout.columns - 1 - c);
        case Corner_BottomRight:
            return (screen_desktop_layout.rows - 1 - r) *
                screen_desktop_layout.columns +
                (screen_desktop_layout.columns - 1 - c);
        }
    case Orientation_Vert:
        switch (screen_desktop_layout.start_corner) {
        case Corner_TopLeft:
            return c * screen_desktop_layout.rows + r;
        case Corner_BottomLeft:
            return c * screen_desktop_layout.rows +
                (screen_desktop_layout.rows - 1 - r);
        case Corner_TopRight:
            return (screen_desktop_layout.columns - 1 - c) *
                screen_desktop_layout.rows + r;
        case Corner_BottomRight:
            return (screen_desktop_layout.columns - 1 - c) *
                screen_desktop_layout.rows +
                (screen_desktop_layout.rows - 1 - r);
        }
    }
    g_assert_not_reached();
    return 0;
}

void action_next_desktop_column(union ActionData *data)
{
    guint r, c, d;

    cur_row_col(&r, &c);
    ++c;
    d = translate_row_col(r, c);
    if (d >= screen_num_desktops) {
        if (!data->nextprevdesktop.wrap) return;
        c = 0;
    }
    if (d >= screen_num_desktops)
        ++c;
    d = translate_row_col(r, c);
    if (d < screen_num_desktops)
        screen_set_desktop(d);
}

void action_previous_desktop_column(union ActionData *data)
{
    guint r, c, d;

    cur_row_col(&r, &c);
    --c;
    d = translate_row_col(r, c);
    if (d >= screen_num_desktops) {
        if (!data->nextprevdesktop.wrap) return;
        c = screen_desktop_layout.columns - 1;
    }
    if (d >= screen_num_desktops)
        --c;
    d = translate_row_col(r, c);
    if (d < screen_num_desktops)
        screen_set_desktop(d);
}

void action_next_desktop_row(union ActionData *data)
{
    guint r, c, d;

    cur_row_col(&r, &c);
    ++r;
    d = translate_row_col(r, c);
    if (d >= screen_num_desktops) {
        if (!data->nextprevdesktop.wrap) return;
        r = 0;
    }
    if (d >= screen_num_desktops)
        ++r;
    d = translate_row_col(r, c);
    if (d < screen_num_desktops)
        screen_set_desktop(d);
}

void action_previous_desktop_row(union ActionData *data)
{
    guint r, c, d;

    cur_row_col(&r, &c);
    --r;
    d = translate_row_col(r, c);
    if (d >= screen_num_desktops) {
        if (!data->nextprevdesktop.wrap) return;
        c = screen_desktop_layout.rows - 1;
    }
    if (d >= screen_num_desktops)
        --r;
    d = translate_row_col(r, c);
    if (d < screen_num_desktops)
        screen_set_desktop(d);
}

void action_toggle_decorations(union ActionData *data)
{
    Client *c = data->client.c;;

    if (!c) return;

    c->disabled_decorations = c->disabled_decorations ? 0 : ~0;
    client_setup_decor_and_functions(c);
}

void action_moveresize(union ActionData *data)
{
    Client *c = data->moveresize.c;

    if (!c || !client_normal(c)) return;

    moveresize_start(c, data->moveresize.x, data->moveresize.y,
                     data->moveresize.button, data->moveresize.corner);
}

void action_restart(union ActionData *data)
{
    ob_restart_path = data->execute.path;
    ob_shutdown = ob_restart = TRUE;
}

void action_exit(union ActionData *data)
{
    ob_shutdown = TRUE;
}

void action_showmenu(union ActionData *data)
{
    if (data->showmenu.name) {
        menu_show(data->showmenu.name, data->showmenu.x, data->showmenu.y,
                  data->showmenu.c);
    }
}

static void popup_cycle(Client *c, gboolean hide)
{
    XSetWindowAttributes attrib;
    static Window coords = None;

    if (coords == None) {
        attrib.override_redirect = TRUE;
        coords = XCreateWindow(ob_display, ob_root,
                               0, 0, 1, 1, 0, render_depth, InputOutput,
                               render_visual, CWOverrideRedirect, &attrib);
        g_assert(coords != None);

        grab_pointer(TRUE, None);

        XMapWindow(ob_display, coords);
    }

    if (hide) {
        XDestroyWindow(ob_display, coords);
        coords = None;

        grab_pointer(FALSE, None);
    } else {
        Rect *a;
        Size s;

        a = screen_area(c->desktop);

        framerender_size_popup_label(c->title, &s);
        XMoveResizeWindow(ob_display, coords,
                          a->x + (a->width - s.width) / 2,
                          a->y + (a->height - s.height) / 2,
                          s.width, s.height);
        framerender_popup_label(coords, &s, c->title);
    }
}

void action_cycle_windows(union ActionData *data)
{
    Client *c;
    
    c = focus_cycle(data->cycle.forward, data->cycle.linear, data->cycle.final,
                    data->cycle.cancel);
    popup_cycle(c, !c || data->cycle.final || data->cycle.cancel);
}

