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
    a->data.diraction.direction = OB_DIRECTION_NORTH;
}

void setup_action_directional_focus_east(Action *a)
{
    a->data.diraction.direction = OB_DIRECTION_EAST;
}

void setup_action_directional_focus_south(Action *a)
{
    a->data.diraction.direction = OB_DIRECTION_SOUTH;
}

void setup_action_directional_focus_west(Action *a)
{
    a->data.diraction.direction = OB_DIRECTION_WEST;
}

void setup_action_directional_focus_northeast(Action *a)
{
    a->data.diraction.direction = OB_DIRECTION_NORTHEAST;
}

void setup_action_directional_focus_southeast(Action *a)
{
    a->data.diraction.direction = OB_DIRECTION_SOUTHEAST;
}

void setup_action_directional_focus_southwest(Action *a)
{
    a->data.diraction.direction = OB_DIRECTION_SOUTHWEST;
}

void setup_action_directional_focus_northwest(Action *a)
{
    a->data.diraction.direction = OB_DIRECTION_NORTHWEST;
}

void setup_action_send_to_desktop(Action *a)
{
    a->data.sendto.follow = TRUE;
}

void setup_action_send_to_desktop_direction(Action *a)
{
    a->data.sendtodir.wrap = TRUE;
    a->data.sendtodir.follow = TRUE;
}

void setup_action_desktop_direction(Action *a)
{
    a->data.desktopdir.wrap = TRUE;
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
    a->data.diraction.direction = OB_DIRECTION_NORTH;
}

void setup_action_movetoedge_south(Action *a)
{
    a->data.diraction.direction = OB_DIRECTION_SOUTH;
}

void setup_action_movetoedge_east(Action *a)
{
    a->data.diraction.direction = OB_DIRECTION_EAST;
}

void setup_action_movetoedge_west(Action *a)
{
    a->data.diraction.direction = OB_DIRECTION_WEST;
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
        "sendtodesktopright",
        action_send_to_desktop_right,
        setup_action_send_to_desktop_direction
    },
    {
        "sendtodesktopleft",
        action_send_to_desktop_left,
        setup_action_send_to_desktop_direction
    },
    {
        "sendtodesktopup",
        action_send_to_desktop_up,
        setup_action_send_to_desktop_direction
    },
    {
        "sendtodesktopdown",
        action_send_to_desktop_down,
        setup_action_send_to_desktop_direction
    },
    {
        "desktop",
        action_desktop,
        NULL
    },
    {
        "desktopright",
        action_desktop_right,
        setup_action_desktop_direction
    },
    {
        "desktopleft",
        action_desktop_left,
        setup_action_desktop_direction
    },
    {
        "desktopup",
        action_desktop_up,
        setup_action_desktop_direction
    },
    {
        "desktopdown",
        action_desktop_down,
        setup_action_desktop_direction
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
        "toggleshowdesktop",
        action_toggle_show_desktop,
        NULL
    },
    {
        "showdesktop",
        action_show_desktop,
        NULL
    },
    {
        "unshowdesktop",
        action_unshow_desktop,
        NULL
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

Action *action_parse(xmlDocPtr doc, xmlNodePtr node)
{
    char *actname;
    Action *act = NULL;
    xmlNodePtr n;

    if (parse_attr_string("name", node, &actname)) {
        if ((act = action_from_string(actname))) {
            if (act->func == action_execute || act->func == action_restart) {
                if ((n = parse_find_node("execute", node->xmlChildrenNode)))
                    act->data.execute.path = parse_string(doc, n);
            } else if (act->func == action_showmenu) {
                if ((n = parse_find_node("menu", node->xmlChildrenNode)))
                    act->data.showmenu.name = parse_string(doc, n);
            } else if (act->func == action_desktop) {
                if ((n = parse_find_node("desktop", node->xmlChildrenNode)))
                    act->data.desktop.desk = parse_int(doc, n);
                if (act->data.desktop.desk > 0) act->data.desktop.desk--;
            } else if (act->func == action_send_to_desktop) {
                if ((n = parse_find_node("desktop", node->xmlChildrenNode)))
                    act->data.sendto.desk = parse_int(doc, n);
                if (act->data.sendto.desk > 0) act->data.sendto.desk--;
            } else if (act->func == action_move_relative_horz ||
                       act->func == action_move_relative_vert ||
                       act->func == action_resize_relative_horz ||
                       act->func == action_resize_relative_vert) {
                if ((n = parse_find_node("delta", node->xmlChildrenNode)))
                    act->data.relative.delta = parse_int(doc, n);
            } else if (act->func == action_desktop_right ||
                       act->func == action_desktop_left ||
                       act->func == action_desktop_up ||
                       act->func == action_desktop_down) {
                if ((n = parse_find_node("wrap", node->xmlChildrenNode))) {
                    g_message("WRAP %d", parse_bool(doc, n));
                    act->data.desktopdir.wrap = parse_bool(doc, n);
                }
            } else if (act->func == action_send_to_desktop_right ||
                       act->func == action_send_to_desktop_left ||
                       act->func == action_send_to_desktop_up ||
                       act->func == action_send_to_desktop_down) {
                if ((n = parse_find_node("wrap", node->xmlChildrenNode)))
                    act->data.sendtodir.wrap = parse_bool(doc, n);
                if ((n = parse_find_node("follow", node->xmlChildrenNode)))
                    act->data.sendtodir.follow = parse_bool(doc, n);
            }
        }
    }
    return act;
}

void action_execute(union ActionData *data)
{
    GError *e = NULL;
    char *cmd;
    if (data->execute.path) {
        cmd = g_filename_from_utf8(data->execute.path, -1, NULL, NULL, NULL);
        if (cmd) {
            if (!g_spawn_command_line_async(cmd, &e)) {
                g_warning("failed to execute '%s': %s",
                          cmd, e->message);
            }
        } else {
            g_warning("failed to convert '%s' from utf8", data->execute.path);
        }
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
        client_configure(c, OB_CORNER_TOPLEFT,
                         c->area.x + data->relative.delta, c->area.y,
                         c->area.width, c->area.height, TRUE, TRUE);
}

void action_move_relative_vert(union ActionData *data)
{
    Client *c = data->relative.c;
    if (c)
        client_configure(c, OB_CORNER_TOPLEFT,
                         c->area.x, c->area.y + data->relative.delta,
                         c->area.width, c->area.height, TRUE, TRUE);
}

void action_resize_relative_horz(union ActionData *data)
{
    Client *c = data->relative.c;
    if (c)
        client_configure(c, OB_CORNER_TOPLEFT, c->area.x, c->area.y,
                         c->area.width +
                         data->relative.delta * c->size_inc.width,
                         c->area.height, TRUE, TRUE);
}

void action_resize_relative_vert(union ActionData *data)
{
    Client *c = data->relative.c;
    if (c && !c->shaded)
        client_configure(c, OB_CORNER_TOPLEFT, c->area.x, c->area.y,
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

void action_desktop(union ActionData *data)
{
    if (data->desktop.desk < screen_num_desktops ||
        data->desktop.desk == DESKTOP_ALL)
        screen_set_desktop(data->desktop.desk);
}

static void cur_row_col(guint *r, guint *c)
{
    switch (screen_desktop_layout.orientation) {
    case OB_ORIENTATION_HORZ:
        switch (screen_desktop_layout.start_corner) {
        case OB_CORNER_TOPLEFT:
            *r = screen_desktop / screen_desktop_layout.columns;
            *c = screen_desktop % screen_desktop_layout.columns;
            break;
        case OB_CORNER_BOTTOMLEFT:
            *r = screen_desktop_layout.rows - 1 -
                screen_desktop / screen_desktop_layout.columns;
            *c = screen_desktop % screen_desktop_layout.columns;
            break;
        case OB_CORNER_TOPRIGHT:
            *r = screen_desktop / screen_desktop_layout.columns;
            *c = screen_desktop_layout.columns - 1 -
                screen_desktop % screen_desktop_layout.columns;
            break;
        case OB_CORNER_BOTTOMRIGHT:
            *r = screen_desktop_layout.rows - 1 -
                screen_desktop / screen_desktop_layout.columns;
            *c = screen_desktop_layout.columns - 1 -
                screen_desktop % screen_desktop_layout.columns;
            break;
        }
        break;
    case OB_ORIENTATION_VERT:
        switch (screen_desktop_layout.start_corner) {
        case OB_CORNER_TOPLEFT:
            *r = screen_desktop % screen_desktop_layout.rows;
            *c = screen_desktop / screen_desktop_layout.rows;
            break;
        case OB_CORNER_BOTTOMLEFT:
            *r = screen_desktop_layout.rows - 1 -
                screen_desktop % screen_desktop_layout.rows;
            *c = screen_desktop / screen_desktop_layout.rows;
            break;
        case OB_CORNER_TOPRIGHT:
            *r = screen_desktop % screen_desktop_layout.rows;
            *c = screen_desktop_layout.columns - 1 -
                screen_desktop / screen_desktop_layout.rows;
            break;
        case OB_CORNER_BOTTOMRIGHT:
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

void action_desktop_right(union ActionData *data)
{
    guint r, c, d;

    cur_row_col(&r, &c);
    ++c;
    if (c >= screen_desktop_layout.columns) {
        if (!data->desktopdir.wrap) return;
        c = 0;
    }
    d = translate_row_col(r, c);
    if (d >= screen_num_desktops) {
        if (!data->desktopdir.wrap) return;
        ++c;
    }
    d = translate_row_col(r, c);
    if (d < screen_num_desktops)
        screen_set_desktop(d);
}

void action_send_to_desktop_right(union ActionData *data)
{
    guint r, c, d;

    if (data->sendtodir.c) {
        cur_row_col(&r, &c);
        ++c;
        if (c >= screen_desktop_layout.columns) {
            if (!data->sendtodir.wrap) return;
            c = 0;
        }
        d = translate_row_col(r, c);
        if (d >= screen_num_desktops) {
            if (!data->sendtodir.wrap) return;
            ++c;
        }
        d = translate_row_col(r, c);
        if (d < screen_num_desktops) {
            client_set_desktop(data->sendtodir.c, d, data->sendtodir.follow);
            if (data->sendtodir.follow) screen_set_desktop(d);
        }
    }
}

void action_desktop_left(union ActionData *data)
{
    guint r, c, d;

    cur_row_col(&r, &c);
    --c;
    if (c >= screen_desktop_layout.columns) {
        if (!data->desktopdir.wrap) return;
        c = screen_desktop_layout.columns - 1;
    }
    d = translate_row_col(r, c);
    if (d >= screen_num_desktops) {
        if (!data->desktopdir.wrap) return;
        --c;
    }
    d = translate_row_col(r, c);
    if (d < screen_num_desktops)
        screen_set_desktop(d);
}

void action_send_to_desktop_left(union ActionData *data)
{
    guint r, c, d;

    if (data->sendtodir.c) {
        cur_row_col(&r, &c);
        --c;
        if (c >= screen_desktop_layout.columns) {
            if (!data->sendtodir.wrap) return;
            c = screen_desktop_layout.columns - 1;
        }
        d = translate_row_col(r, c);
        if (d >= screen_num_desktops) {
            if (!data->sendtodir.wrap) return;
            --c;
        }
        d = translate_row_col(r, c);
        if (d < screen_num_desktops) {
            client_set_desktop(data->sendtodir.c, d, data->sendtodir.follow);
            if (data->sendtodir.follow) screen_set_desktop(d);
        }
    }
}

void action_desktop_down(union ActionData *data)
{
    guint r, c, d;

    cur_row_col(&r, &c);
    ++r;
    if (r >= screen_desktop_layout.rows) {
        if (!data->desktopdir.wrap) return;
        r = 0;
    }
    d = translate_row_col(r, c);
    if (d >= screen_num_desktops) {
        if (!data->desktopdir.wrap) return;
        ++r;
    }
    d = translate_row_col(r, c);
    if (d < screen_num_desktops)
        screen_set_desktop(d);
}

void action_send_to_desktop_down(union ActionData *data)
{
    guint r, c, d;

    if (data->sendtodir.c) {
        cur_row_col(&r, &c);
        ++r;
        if (r >= screen_desktop_layout.rows) {
            if (!data->sendtodir.wrap) return;
            r = 0;
        }
        d = translate_row_col(r, c);
        if (d >= screen_num_desktops) {
            if (!data->sendtodir.wrap) return;
            ++r;
        }
        d = translate_row_col(r, c);
        if (d < screen_num_desktops) {
            client_set_desktop(data->sendtodir.c, d, data->sendtodir.follow);
            if (data->sendtodir.follow) screen_set_desktop(d);
        }
    }
}

void action_desktop_up(union ActionData *data)
{
    guint r, c, d;

    cur_row_col(&r, &c);
    --r;
    if (r >= screen_desktop_layout.rows) {
        if (!data->desktopdir.wrap) return;
        r = screen_desktop_layout.rows - 1;
    }
    d = translate_row_col(r, c);
    if (d >= screen_num_desktops) {
        if (!data->desktopdir.wrap) return;
        --r;
    }
    d = translate_row_col(r, c);
    if (d < screen_num_desktops)
        screen_set_desktop(d);
}

void action_send_to_desktop_up(union ActionData *data)
{
    guint r, c, d;

    if (data->sendtodir.c) {
        cur_row_col(&r, &c);
        --r;
        if (r >= screen_desktop_layout.rows) {
            if (!data->sendtodir.wrap) return;
            r = screen_desktop_layout.rows - 1;
        }
        d = translate_row_col(r, c);
        if (d >= screen_num_desktops) {
            if (!data->sendtodir.wrap) return;
            --r;
        }
        d = translate_row_col(r, c);
        if (d < screen_num_desktops) {
            client_set_desktop(data->sendtodir.c, d, data->sendtodir.follow);
            if (data->sendtodir.follow) screen_set_desktop(d);
        }
    }
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
    ob_restart_path = g_strdup(data->execute.path);
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
    case OB_DIRECTION_NORTH:
        y = 0;
        break;
    case OB_DIRECTION_WEST:
        x = 0;
        break;
    case OB_DIRECTION_SOUTH:
        y = h - c->frame->area.height;
        break;
    case OB_DIRECTION_EAST:
        x = w - c->frame->area.width;
        break;
    default:
        g_assert_not_reached();
    }
    frame_frame_gravity(c->frame, &x, &y);
    client_configure(c, OB_CORNER_TOPLEFT,
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

void action_toggle_show_desktop(union ActionData *data)
{
    screen_show_desktop(!screen_showing_desktop);
}

void action_show_desktop(union ActionData *data)
{
    screen_show_desktop(TRUE);
}

void action_unshow_desktop(union ActionData *data)
{
    screen_show_desktop(FALSE);
}
