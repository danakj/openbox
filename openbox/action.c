#include "client.h"
#include "focus.h"
#include "moveresize.h"
#include "menu.h"
#include "prop.h"
#include "stacking.h"
#include "frame.h"
#include "screen.h"
#include "action.h"
#include "dispatch.h"
#include "openbox.h"

#include <glib.h>

typedef struct ActionString {
    char *name;
    void (*func)(union ActionData *);
    void (*setup)(Action *);
} ActionString;

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

void setup_action_directional_focus_north(Action *a)
{
    a->data.diraction.direction = Direction_North;
}

void setup_action_directional_focus_east(Action *a)
{
    a->data.diraction.direction = Direction_East;
}

void setup_action_directional_focus_south(Action *a)
{
    a->data.diraction.direction = Direction_South;
}

void setup_action_directional_focus_west(Action *a)
{
    a->data.diraction.direction = Direction_West;
}

void setup_action_directional_focus_northeast(Action *a)
{
    a->data.diraction.direction = Direction_NorthEast;
}

void setup_action_directional_focus_southeast(Action *a)
{
    a->data.diraction.direction = Direction_SouthEast;
}

void setup_action_directional_focus_southwest(Action *a)
{
    a->data.diraction.direction = Direction_SouthWest;
}

void setup_action_directional_focus_northwest(Action *a)
{
    a->data.diraction.direction = Direction_NorthWest;
}

void setup_action_send_to_desktop(Action *a)
{
    a->data.sendto.follow = TRUE;
}

void setup_action_send_to_np_desktop(Action *a)
{
    a->data.sendtonextprev.wrap = FALSE;
    a->data.sendtonextprev.follow = TRUE;
}

void setup_action_send_to_np_desktop_wrap(Action *a)
{
    a->data.sendtonextprev.wrap = TRUE;
    a->data.sendtonextprev.follow = TRUE;
}

void setup_action_np_desktop(Action *a)
{
    a->data.nextprevdesktop.wrap = FALSE;
}

void setup_action_np_desktop_wrap(Action *a)
{
    a->data.nextprevdesktop.wrap = TRUE;
}

void setup_action_move_keyboard(Action *a)
{
    a->data.moveresize.corner = prop_atoms.net_wm_moveresize_move_keyboard;
}

void setup_action_move(Action *a)
{
    a->data.moveresize.corner = prop_atoms.net_wm_moveresize_move;
}

void setup_action_resize(Action *a)
{
    a->data.moveresize.corner = prop_atoms.net_wm_moveresize_size_topleft;
}

void setup_action_resize_keyboard(Action *a)
{
    a->data.moveresize.corner = prop_atoms.net_wm_moveresize_size_keyboard;
}

void setup_action_cycle_windows_linear_next(Action *a)
{
    a->data.cycle.linear = TRUE;
    a->data.cycle.forward = TRUE;
}

void setup_action_cycle_windows_linear_previous(Action *a)
{
    a->data.cycle.linear = TRUE;
    a->data.cycle.forward = FALSE;
}

void setup_action_cycle_windows_next(Action *a)
{
    a->data.cycle.linear = FALSE;
    a->data.cycle.forward = TRUE;
}

void setup_action_cycle_windows_previous(Action *a)
{
    a->data.cycle.linear = FALSE;
    a->data.cycle.forward = FALSE;
}

void setup_action_movetoedge_north(Action *a)
{
    a->data.diraction.direction = Direction_North;
}

void setup_action_movetoedge_south(Action *a)
{
    a->data.diraction.direction = Direction_South;
}

void setup_action_movetoedge_east(Action *a)
{
    a->data.diraction.direction = Direction_East;
}

void setup_action_movetoedge_west(Action *a)
{
    a->data.diraction.direction = Direction_West;
}

void setup_action_top_layer(Action *a)
{
    a->data.layer.layer = 1;
}

void setup_action_normal_layer(Action *a)
{
    a->data.layer.layer = 0;
}

void setup_action_bottom_layer(Action *a)
{
    a->data.layer.layer = -1;
}

ActionString actionstrings[] =
{
    {
        "execute", 
        action_execute, 
        NULL
    },
    {
        "directionalfocusnorth", 
        action_directional_focus, 
        setup_action_directional_focus_north
    },
    {
        "directionalfocuseast", 
        action_directional_focus, 
        setup_action_directional_focus_east
    },
    {
        "directionalfocussouth", 
        action_directional_focus, 
        setup_action_directional_focus_south
    },
    {
        "directionalfocuswest",
        action_directional_focus,
        setup_action_directional_focus_west
    },
    {
        "directionalfocusnortheast",
        action_directional_focus,
        setup_action_directional_focus_northeast
    },
    {
        "directionalfocussoutheast",
        action_directional_focus,
        setup_action_directional_focus_southeast
    },
    {
        "directionalfocussouthwest",
        action_directional_focus,
        setup_action_directional_focus_southwest
    },
    {
        "directionalfocusnorthwest",
        action_directional_focus,
        setup_action_directional_focus_northwest
    },
    {
        "focus",
        action_focus,
        NULL,
    },
    {
        "unfocus",
        action_unfocus,
        NULL
    },
    {
        "iconify",
        action_iconify,
        NULL
    },
    {
        "raise",
        action_raise,
        NULL
    },
    {
        "lower",
        action_lower,
        NULL
    },
    {
        "focusraise",
        action_focusraise,
        NULL
    },
    {
        "close",
        action_close,
        NULL
    },
    {
        "kill",
        action_kill,
        NULL
    },
    {
        "shadelower",
        action_shadelower,
        NULL
    },
    {
        "unshaderaise",
        action_unshaderaise,
        NULL
    },
    {
        "shade",
        action_shade,
        NULL
    },
    {
        "unshade",
        action_unshade,
        NULL
    },
    {
        "toggleshade",
        action_toggle_shade,
        NULL
    },
    {
        "toggleomnipresent",
        action_toggle_omnipresent,
        NULL
    },
    {
        "moverelativehorz",
        action_move_relative_horz,
        NULL
    },
    {
        "moverelativevert",
        action_move_relative_vert,
        NULL
    },
    {
        "resizerelativehorz",
        action_resize_relative_horz,
        NULL
    },
    {
        "resizerelativevert",
        action_resize_relative_vert,
        NULL
    },
    {
        "maximizefull",
        action_maximize_full,
        NULL
    },
    {
        "unmaximizefull",
        action_unmaximize_full,
        NULL
    },
    {
        "togglemaximizefull",
        action_toggle_maximize_full,
        NULL
    },
    {
        "maximizehorz",
        action_maximize_horz,
        NULL
    },
    {
        "unmaximizehorz",
        action_unmaximize_horz,
        NULL
    },
    {
        "togglemaximizehorz",
        action_toggle_maximize_horz,
        NULL
    },
    {
        "maximizevert",
        action_maximize_vert,
        NULL
    },
    {
        "unmaximizevert",
        action_unmaximize_vert,
        NULL
    },
    {
        "togglemaximizevert",
        action_toggle_maximize_vert,
        NULL
    },
    {
        "sendtodesktop",
        action_send_to_desktop,
        setup_action_send_to_desktop
    },
    {
        "sendtonextdesktop",
        action_send_to_next_desktop,
        setup_action_send_to_np_desktop
    },
    {
        "sendtonextdesktopwrap",
        action_send_to_next_desktop,
        setup_action_send_to_np_desktop_wrap
    },
    {
        "sendtopreviousdesktop",
        action_send_to_previous_desktop,
        setup_action_send_to_np_desktop
    },
    {
        "sendtopreviousdesktopwrap",
        action_send_to_previous_desktop,
        setup_action_send_to_np_desktop_wrap
    },
    {
        "desktop",
        action_desktop,
        NULL
    },
    {
        "nextdesktop",
        action_next_desktop,
        setup_action_np_desktop
    },
    {
        "nextdesktopwrap",
        action_next_desktop,
        setup_action_np_desktop_wrap
    },
    {
        "previousdesktop",
        action_previous_desktop,
        setup_action_np_desktop
    },
    {
        "previousdesktopwrap",
        action_previous_desktop,
        setup_action_np_desktop_wrap
    },
    {
        "nextdesktopcolumn",
        action_next_desktop_column,
        setup_action_np_desktop
    },
    {
        "nextdesktopcolumnwrap",
        action_next_desktop_column,
        setup_action_np_desktop_wrap
    },
    {
        "previousdesktopcolumn",
        action_previous_desktop_column,
        setup_action_np_desktop
    },
    {
        "previousdesktopcolumnwrap",
        action_previous_desktop_column,
        setup_action_np_desktop_wrap
    },
    {
        "nextdesktoprow",
        action_next_desktop_row,
        setup_action_np_desktop
    },
    {
        "nextdesktoprowwrap",
        action_next_desktop_row,
        setup_action_np_desktop_wrap
    },
    {
        "previousdesktoprow",
        action_previous_desktop_row,
        setup_action_np_desktop
    },
    {
        "previousdesktoprowwrap",
        action_previous_desktop_row,
        setup_action_np_desktop_wrap
    },
    {
        "toggledecorations",
        action_toggle_decorations,
        NULL
    },
    {
        "keyboardmove", 
        action_moveresize,
        setup_action_move_keyboard
    },
    {
        "move",
        action_moveresize,
        setup_action_move
    },
    {
        "resize",
        action_moveresize,
        setup_action_resize
    },
    {
        "keyboardresize",
        action_moveresize,
        setup_action_resize_keyboard
    },
    {
        "restart",
        action_restart,
        NULL
    },
    {
        "exit",
        action_exit,
        NULL
    },
    {
        "showmenu",
        action_showmenu,
        NULL
    },
    {
        "sendtotoplayer",
        action_send_to_layer,
        setup_action_top_layer
    },
    {
        "togglealwaysontop",
        action_toggle_layer,
        setup_action_top_layer
    },
    {
        "sendtonormallayer",
        action_send_to_layer,
        setup_action_normal_layer
    },
    {
        "sendtobottomlayer",
        action_send_to_layer,
        setup_action_bottom_layer
    },
    {
        "togglealwaysonbottom",
        action_toggle_layer,
        setup_action_bottom_layer
    },
    {
        "nextwindowlinear",
        action_cycle_windows,
        setup_action_cycle_windows_linear_next
    },
    {
        "previouswindowlinear",
        action_cycle_windows,
        setup_action_cycle_windows_linear_previous
    },
    {
        "nextwindow",
        action_cycle_windows,
        setup_action_cycle_windows_next
    },
    {
        "previouswindow",
        action_cycle_windows,
        setup_action_cycle_windows_previous
    },
    {
        "movetoedgenorth",
        action_movetoedge,
        setup_action_movetoedge_north
    },
    {
        "movetoedgesouth",
        action_movetoedge,
        setup_action_movetoedge_south
    },
    {
        "movetoedgewest",
        action_movetoedge,
        setup_action_movetoedge_west
    },
    {
        "movetoedgeeast",
        action_movetoedge,
        setup_action_movetoedge_east
    },
    {
        NULL,
        NULL,
        NULL
    }
};

Action *action_from_string(char *name)
{
    Action *a = NULL;
    int i;

    for (i = 0; actionstrings[i].name; i++)
        if (!g_ascii_strcasecmp(name, actionstrings[i].name)) {
            a = action_new(actionstrings[i].func);
            if (actionstrings[i].setup)
                actionstrings[i].setup(a);
            break;
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
        stacking_raise(CLIENT_AS_WINDOW(data->client.c));
    }
}

void action_raise(union ActionData *data)
{
    if (data->client.c)
        stacking_raise(CLIENT_AS_WINDOW(data->client.c));
}

void action_unshaderaise(union ActionData *data)
{
    if (data->client.c) {
        if (data->client.c->shaded)
            client_shade(data->client.c, FALSE);
        else
            stacking_raise(CLIENT_AS_WINDOW(data->client.c));
    }
}

void action_shadelower(union ActionData *data)
{
    if (data->client.c) {
        if (data->client.c->shaded)
            stacking_lower(CLIENT_AS_WINDOW(data->client.c));
        else
            client_shade(data->client.c, TRUE);
    }
}

void action_lower(union ActionData *data)
{
    if (data->client.c)
        stacking_lower(CLIENT_AS_WINDOW(data->client.c));
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
                         c->area.width +
                         data->relative.delta * c->size_inc.width,
                         c->area.height, TRUE, TRUE);
}

void action_resize_relative_vert(union ActionData *data)
{
    Client *c = data->relative.c;
    if (c && !c->shaded)
        client_configure(c, Corner_TopLeft, c->area.x, c->area.y,
                         c->area.width, c->area.height +
                         data->relative.delta * c->size_inc.height,
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
            return r % screen_desktop_layout.rows *
                screen_desktop_layout.columns +
                c % screen_desktop_layout.columns;
        case Corner_BottomLeft:
            return (screen_desktop_layout.rows - 1 -
                    r % screen_desktop_layout.rows) *
                screen_desktop_layout.columns +
                c % screen_desktop_layout.columns;
        case Corner_TopRight:
            return r % screen_desktop_layout.rows *
                screen_desktop_layout.columns +
                (screen_desktop_layout.columns - 1 -
                 c % screen_desktop_layout.columns);
        case Corner_BottomRight:
            return (screen_desktop_layout.rows - 1 -
                    r % screen_desktop_layout.rows) *
                screen_desktop_layout.columns +
                (screen_desktop_layout.columns - 1 -
                 c % screen_desktop_layout.columns);
        }
    case Orientation_Vert:
        switch (screen_desktop_layout.start_corner) {
        case Corner_TopLeft:
            return c % screen_desktop_layout.columns *
                screen_desktop_layout.rows +
                r % screen_desktop_layout.rows;
        case Corner_BottomLeft:
            return c % screen_desktop_layout.columns *
                screen_desktop_layout.rows +
                (screen_desktop_layout.rows - 1 -
                 r % screen_desktop_layout.rows);
        case Corner_TopRight:
            return (screen_desktop_layout.columns - 1 -
                    c % screen_desktop_layout.columns) *
                screen_desktop_layout.rows +
                r % screen_desktop_layout.rows;
        case Corner_BottomRight:
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

void action_next_desktop_column(union ActionData *data)
{
    guint r, c, d;

    cur_row_col(&r, &c);
    ++c;
    if (c >= screen_desktop_layout.columns)
        c = 0;
    d = translate_row_col(r, c);
    if (d >= screen_num_desktops) {
        if (!data->nextprevdesktop.wrap) return;
        ++c;
    }
    d = translate_row_col(r, c);
    if (d < screen_num_desktops)
        screen_set_desktop(d);
}

void action_previous_desktop_column(union ActionData *data)
{
    guint r, c, d;

    cur_row_col(&r, &c);
    --c;
    if (c >= screen_desktop_layout.columns)
        c = screen_desktop_layout.columns - 1;
    d = translate_row_col(r, c);
    if (d >= screen_num_desktops) {
        if (!data->nextprevdesktop.wrap) return;
        --c;
    }
    d = translate_row_col(r, c);
    if (d < screen_num_desktops)
        screen_set_desktop(d);
}

void action_next_desktop_row(union ActionData *data)
{
    guint r, c, d;

    cur_row_col(&r, &c);
    ++r;
    if (r >= screen_desktop_layout.rows)
        r = 0;
    d = translate_row_col(r, c);
    if (d >= screen_num_desktops) {
        if (!data->nextprevdesktop.wrap) return;
        ++r;
    }
    d = translate_row_col(r, c);
    if (d < screen_num_desktops)
        screen_set_desktop(d);
}

void action_previous_desktop_row(union ActionData *data)
{
    guint r, c, d;

    cur_row_col(&r, &c);
    --r;
    if (r >= screen_desktop_layout.rows)
        r = screen_desktop_layout.rows - 1;
    d = translate_row_col(r, c);
    if (d >= screen_num_desktops) {
        if (!data->nextprevdesktop.wrap) return;
        --r;
    }
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

void action_cycle_windows(union ActionData *data)
{
    Client *c;
    
    c = focus_cycle(data->cycle.forward, data->cycle.linear, data->cycle.final,
                    data->cycle.cancel);
}

void action_directional_focus(union ActionData *data)
{
    Client *nf;

    if (!data->diraction.c)
        return;
    if ((nf = client_find_directional(data->diraction.c,
                                      data->diraction.direction)))
        client_activate(nf);
}

void action_movetoedge(union ActionData *data)
{
    int x, y, h, w;
    Client *c = data->diraction.c;

    if (!c)
        return;
    x = c->frame->area.x;
    y = c->frame->area.y;
    
    h = screen_area(c->desktop)->height;
    w = screen_area(c->desktop)->width;
    switch(data->diraction.direction) {
    case Direction_North:
        y = 0;
        break;
    case Direction_West:
        x = 0;
        break;
    case Direction_South:
        y = h - c->frame->area.height;
        break;
    case Direction_East:
        x = w - c->frame->area.width;
        break;
    }
    frame_frame_gravity(c->frame, &x, &y);
    client_configure(c, Corner_TopLeft,
                     x, y, c->area.width, c->area.height, TRUE, TRUE);

}

void action_send_to_layer(union ActionData *data)
{
    if (data->layer.c)
        client_set_layer(data->layer.c, data->layer.layer);
}

void action_toggle_layer(union ActionData *data)
{
    Client *c = data->layer.c;

    if (c) {
        if (data->layer.layer < 0)
            client_set_layer(c, c->below ? 0 : -1);
        else if (data->layer.layer > 0)
            client_set_layer(c, c->above ? 0 : 1);
    }
}
