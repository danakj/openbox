#include "debug.h"
#include "client.h"
#include "focus.h"
#include "moveresize.h"
#include "menu.h"
#include "prop.h"
#include "stacking.h"
#include "frame.h"
#include "screen.h"
#include "action.h"
#include "openbox.h"
#include "grab.h"

#include <glib.h>

typedef struct ActionString {
    char *name;
    void (*func)(union ActionData *);
    void (*setup)(ObAction *);
} ActionString;

ObAction *action_new(void (*func)(union ActionData *data))
{
    ObAction *a = g_new0(ObAction, 1);
    a->func = func;

    return a;
}

void action_free(ObAction *a)
{
    if (a == NULL) return;

    /* deal with pointers */
    if (a->func == action_execute || a->func == action_restart)
        g_free(a->data.execute.path);
    else if (a->func == action_showmenu)
        g_free(a->data.showmenu.name);

    g_free(a);
}

void setup_action_directional_focus_north(ObAction *a)
{ 
    a->data.interdiraction.inter.any.interactive = TRUE;
    a->data.interdiraction.direction = OB_DIRECTION_NORTH;
}

void setup_action_directional_focus_east(ObAction *a)
{
    a->data.interdiraction.inter.any.interactive = TRUE;
    a->data.interdiraction.direction = OB_DIRECTION_EAST;
}

void setup_action_directional_focus_south(ObAction *a)
{
    a->data.interdiraction.inter.any.interactive = TRUE;
    a->data.interdiraction.direction = OB_DIRECTION_SOUTH;
}

void setup_action_directional_focus_west(ObAction *a)
{
    a->data.interdiraction.inter.any.interactive = TRUE;
    a->data.interdiraction.direction = OB_DIRECTION_WEST;
}

void setup_action_directional_focus_northeast(ObAction *a)
{
    a->data.interdiraction.inter.any.interactive = TRUE;
    a->data.interdiraction.direction = OB_DIRECTION_NORTHEAST;
}

void setup_action_directional_focus_southeast(ObAction *a)
{
    a->data.interdiraction.inter.any.interactive = TRUE;
    a->data.interdiraction.direction = OB_DIRECTION_SOUTHEAST;
}

void setup_action_directional_focus_southwest(ObAction *a)
{
    a->data.interdiraction.inter.any.interactive = TRUE;
    a->data.interdiraction.direction = OB_DIRECTION_SOUTHWEST;
}

void setup_action_directional_focus_northwest(ObAction *a)
{
    a->data.interdiraction.inter.any.interactive = TRUE;
    a->data.interdiraction.direction = OB_DIRECTION_NORTHWEST;
}

void setup_action_send_to_desktop(ObAction *a)
{
    a->data.sendto.follow = TRUE;
}

void setup_action_send_to_desktop_prev(ObAction *a)
{
    a->data.sendtodir.inter.any.interactive = TRUE;
    a->data.sendtodir.dir = OB_DIRECTION_WEST;
    a->data.sendtodir.linear = TRUE;
    a->data.sendtodir.wrap = TRUE;
    a->data.sendtodir.follow = TRUE;
}

void setup_action_send_to_desktop_next(ObAction *a)
{
    a->data.sendtodir.inter.any.interactive = TRUE;
    a->data.sendtodir.dir = OB_DIRECTION_EAST;
    a->data.sendtodir.linear = TRUE;
    a->data.sendtodir.wrap = TRUE;
    a->data.sendtodir.follow = TRUE;
}

void setup_action_send_to_desktop_left(ObAction *a)
{
    a->data.sendtodir.inter.any.interactive = TRUE;
    a->data.sendtodir.dir = OB_DIRECTION_WEST;
    a->data.sendtodir.linear = FALSE;
    a->data.sendtodir.wrap = TRUE;
    a->data.sendtodir.follow = TRUE;
}

void setup_action_send_to_desktop_right(ObAction *a)
{
    a->data.sendtodir.inter.any.interactive = TRUE;
    a->data.sendtodir.dir = OB_DIRECTION_EAST;
    a->data.sendtodir.linear = FALSE;
    a->data.sendtodir.wrap = TRUE;
    a->data.sendtodir.follow = TRUE;
}

void setup_action_send_to_desktop_up(ObAction *a)
{
    a->data.sendtodir.inter.any.interactive = TRUE;
    a->data.sendtodir.dir = OB_DIRECTION_NORTH;
    a->data.sendtodir.linear = FALSE;
    a->data.sendtodir.wrap = TRUE;
    a->data.sendtodir.follow = TRUE;
}

void setup_action_send_to_desktop_down(ObAction *a)
{
    a->data.sendtodir.inter.any.interactive = TRUE;
    a->data.sendtodir.dir = OB_DIRECTION_SOUTH;
    a->data.sendtodir.linear = FALSE;
    a->data.sendtodir.wrap = TRUE;
    a->data.sendtodir.follow = TRUE;
}

void setup_action_desktop_prev(ObAction *a)
{
    a->data.desktopdir.inter.any.interactive = TRUE;
    a->data.desktopdir.dir = OB_DIRECTION_WEST;
    a->data.desktopdir.linear = TRUE;
    a->data.desktopdir.wrap = TRUE;
}

void setup_action_desktop_next(ObAction *a)
{
    a->data.desktopdir.inter.any.interactive = TRUE;
    a->data.desktopdir.dir = OB_DIRECTION_EAST;
    a->data.desktopdir.linear = TRUE;
    a->data.desktopdir.wrap = TRUE;
}

void setup_action_desktop_left(ObAction *a)
{
    a->data.desktopdir.inter.any.interactive = TRUE;
    a->data.desktopdir.dir = OB_DIRECTION_WEST;
    a->data.desktopdir.linear = FALSE;
    a->data.desktopdir.wrap = TRUE;
}

void setup_action_desktop_right(ObAction *a)
{
    a->data.desktopdir.inter.any.interactive = TRUE;
    a->data.desktopdir.dir = OB_DIRECTION_EAST;
    a->data.desktopdir.linear = FALSE;
    a->data.desktopdir.wrap = TRUE;
}

void setup_action_desktop_up(ObAction *a)
{
    a->data.desktopdir.inter.any.interactive = TRUE;
    a->data.desktopdir.dir = OB_DIRECTION_NORTH;
    a->data.desktopdir.linear = FALSE;
    a->data.desktopdir.wrap = TRUE;
}

void setup_action_desktop_down(ObAction *a)
{
    a->data.desktopdir.inter.any.interactive = TRUE;
    a->data.desktopdir.dir = OB_DIRECTION_SOUTH;
    a->data.desktopdir.linear = FALSE;
    a->data.desktopdir.wrap = TRUE;
}

void setup_action_move_keyboard(ObAction *a)
{
    a->data.moveresize.corner = prop_atoms.net_wm_moveresize_move_keyboard;
}

void setup_action_move(ObAction *a)
{
    a->data.moveresize.corner = prop_atoms.net_wm_moveresize_move;
}

void setup_action_resize(ObAction *a)
{
    a->data.moveresize.corner = prop_atoms.net_wm_moveresize_size_topleft;
}

void setup_action_resize_keyboard(ObAction *a)
{
    a->data.moveresize.corner = prop_atoms.net_wm_moveresize_size_keyboard;
}

void setup_action_cycle_windows_linear_next(ObAction *a)
{
    a->data.cycle.inter.any.interactive = TRUE;
    a->data.cycle.linear = TRUE;
    a->data.cycle.forward = TRUE;
}

void setup_action_cycle_windows_linear_previous(ObAction *a)
{
    a->data.cycle.inter.any.interactive = TRUE;
    a->data.cycle.linear = TRUE;
    a->data.cycle.forward = FALSE;
}

void setup_action_cycle_windows_next(ObAction *a)
{
    a->data.cycle.inter.any.interactive = TRUE;
    a->data.cycle.linear = FALSE;
    a->data.cycle.forward = TRUE;
}

void setup_action_cycle_windows_previous(ObAction *a)
{
    a->data.cycle.inter.any.interactive = TRUE;
    a->data.cycle.linear = FALSE;
    a->data.cycle.forward = FALSE;
}

void setup_action_movetoedge_north(ObAction *a)
{
    a->data.diraction.direction = OB_DIRECTION_NORTH;
}

void setup_action_movetoedge_south(ObAction *a)
{
    a->data.diraction.direction = OB_DIRECTION_SOUTH;
}

void setup_action_movetoedge_east(ObAction *a)
{
    a->data.diraction.direction = OB_DIRECTION_EAST;
}

void setup_action_movetoedge_west(ObAction *a)
{
    a->data.diraction.direction = OB_DIRECTION_WEST;
}

void setup_action_growtoedge_north(ObAction *a)
{
    a->data.diraction.direction = OB_DIRECTION_NORTH;
}

void setup_action_growtoedge_south(ObAction *a)
{
    a->data.diraction.direction = OB_DIRECTION_SOUTH;
}

void setup_action_growtoedge_east(ObAction *a)
{
    a->data.diraction.direction = OB_DIRECTION_EAST;
}

void setup_action_growtoedge_west(ObAction *a)
{
    a->data.diraction.direction = OB_DIRECTION_WEST;
}

void setup_action_top_layer(ObAction *a)
{
    a->data.layer.layer = 1;
}

void setup_action_normal_layer(ObAction *a)
{
    a->data.layer.layer = 0;
}

void setup_action_bottom_layer(ObAction *a)
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
        "activate",
        action_activate,
        NULL,
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
        "raiselower",
        action_raiselower,
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
        "sendtodesktopnext",
        action_send_to_desktop_dir,
        setup_action_send_to_desktop_next
    },
    {
        "sendtodesktopprevious",
        action_send_to_desktop_dir,
        setup_action_send_to_desktop_prev
    },
    {
        "sendtodesktopright",
        action_send_to_desktop_dir,
        setup_action_send_to_desktop_right
    },
    {
        "sendtodesktopleft",
        action_send_to_desktop_dir,
        setup_action_send_to_desktop_left
    },
    {
        "sendtodesktopup",
        action_send_to_desktop_dir,
        setup_action_send_to_desktop_up
    },
    {
        "sendtodesktopdown",
        action_send_to_desktop_dir,
        setup_action_send_to_desktop_down
    },
    {
        "desktop",
        action_desktop,
        NULL
    },
    {
        "desktopnext",
        action_desktop_dir,
        setup_action_desktop_next
    },
    {
        "desktopprevious",
        action_desktop_dir,
        setup_action_desktop_prev
    },
    {
        "desktopright",
        action_desktop_dir,
        setup_action_desktop_right
    },
    {
        "desktopleft",
        action_desktop_dir,
        setup_action_desktop_left
    },
    {
        "desktopup",
        action_desktop_dir,
        setup_action_desktop_up
    },
    {
        "desktopdown",
        action_desktop_dir,
        setup_action_desktop_down
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
        "desktoplast",
        action_desktop_last,
        NULL
    },
    {
        "reconfigure",
        action_reconfigure,
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
        "growtoedgenorth",
        action_growtoedge,
        setup_action_growtoedge_north
    },
    {
        "growtoedgesouth",
        action_growtoedge,
        setup_action_growtoedge_south
    },
    {
        "growtoedgewest",
        action_growtoedge,
        setup_action_growtoedge_west
    },
    {
        "growtoedgeeast",
        action_growtoedge,
        setup_action_growtoedge_east
    },
    {
        NULL,
        NULL,
        NULL
    }
};

ObAction *action_from_string(char *name)
{
    ObAction *a = NULL;
    int i;

    for (i = 0; actionstrings[i].name; i++)
        if (!g_ascii_strcasecmp(name, actionstrings[i].name)) {
            a = action_new(actionstrings[i].func);
            if (actionstrings[i].setup)
                actionstrings[i].setup(a);
            break;
        }
    if (!a)
        g_warning("Invalid action '%s' requested. No such action exists.",
                  name);
    return a;
}

ObAction *action_parse(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node)
{
    char *actname;
    ObAction *act = NULL;
    xmlNodePtr n;

    if (parse_attr_string("name", node, &actname)) {
        if ((act = action_from_string(actname))) {
            if (act->func == action_execute || act->func == action_restart) {
                if ((n = parse_find_node("execute", node->xmlChildrenNode))) {
                    gchar *s = parse_string(doc, n);
                    act->data.execute.path = ob_expand_tilde(s);
                    g_free(s);
                }
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
            } else if (act->func == action_desktop_dir) {
                if ((n = parse_find_node("wrap", node->xmlChildrenNode)))
                    act->data.desktopdir.wrap = parse_bool(doc, n);
            } else if (act->func == action_send_to_desktop) {
                if ((n = parse_find_node("follow", node->xmlChildrenNode)))
                    act->data.sendto.follow = parse_bool(doc, n);
            } else if (act->func == action_send_to_desktop_dir) {
                if ((n = parse_find_node("wrap", node->xmlChildrenNode)))
                    act->data.sendtodir.wrap = parse_bool(doc, n);
                if ((n = parse_find_node("follow", node->xmlChildrenNode)))
                    act->data.sendtodir.follow = parse_bool(doc, n);
            } else if (act->func == action_activate) {
                if ((n = parse_find_node("here", node->xmlChildrenNode)))
                    act->data.activate.here = parse_bool(doc, n);
            }
        }
        g_free(actname);
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

void action_activate(union ActionData *data)
{
    if (data->activate.any.c)
        client_activate(data->activate.any.c, data->activate.here);
}

void action_focus(union ActionData *data)
{
    if (data->client.any.c)
        client_focus(data->client.any.c);
}

void action_unfocus (union ActionData *data)
{
    if (data->client.any.c)
        client_unfocus(data->client.any.c);
}

void action_iconify(union ActionData *data)
{
    if (data->client.any.c)
        client_iconify(data->client.any.c, TRUE, TRUE);
}

void action_raiselower(union ActionData *data)
{
    ObClient *c = data->client.any.c;
    GList *it;
    gboolean raise = FALSE;

    if (!c) return;

    for (it = stacking_list; it; it = g_list_next(it)) {
        ObClient *cit = it->data;

        if (cit == c) break;
        if (client_normal(cit) == client_normal(c) &&
            cit->layer == c->layer &&
            cit->frame->visible)
        {
            if (RECT_INTERSECTS_RECT(cit->frame->area, c->frame->area)) {
                raise = TRUE;
                break;
            }
        }
    }

    if (raise)
        stacking_raise(CLIENT_AS_WINDOW(c));
    else
        stacking_lower(CLIENT_AS_WINDOW(c));
}

void action_raise(union ActionData *data)
{
    if (data->client.any.c)
        stacking_raise(CLIENT_AS_WINDOW(data->client.any.c));
}

void action_unshaderaise(union ActionData *data)
{
    if (data->client.any.c) {
        if (data->client.any.c->shaded)
            client_shade(data->client.any.c, FALSE);
        else
            stacking_raise(CLIENT_AS_WINDOW(data->client.any.c));
    }
}

void action_shadelower(union ActionData *data)
{
    if (data->client.any.c) {
        if (data->client.any.c->shaded)
            stacking_lower(CLIENT_AS_WINDOW(data->client.any.c));
        else
            client_shade(data->client.any.c, TRUE);
    }
}

void action_lower(union ActionData *data)
{
    if (data->client.any.c)
        stacking_lower(CLIENT_AS_WINDOW(data->client.any.c));
}

void action_close(union ActionData *data)
{
    if (data->client.any.c)
        client_close(data->client.any.c);
}

void action_kill(union ActionData *data)
{
    if (data->client.any.c)
        client_kill(data->client.any.c);
}

void action_shade(union ActionData *data)
{
    if (data->client.any.c)
        client_shade(data->client.any.c, TRUE);
}

void action_unshade(union ActionData *data)
{
    if (data->client.any.c)
        client_shade(data->client.any.c, FALSE);
}

void action_toggle_shade(union ActionData *data)
{
    if (data->client.any.c)
        client_shade(data->client.any.c, !data->client.any.c->shaded);
}

void action_toggle_omnipresent(union ActionData *data)
{ 
    if (data->client.any.c)
        client_set_desktop(data->client.any.c,
                           data->client.any.c->desktop == DESKTOP_ALL ?
                           screen_desktop : DESKTOP_ALL, FALSE);
}

void action_move_relative_horz(union ActionData *data)
{
    ObClient *c = data->relative.any.c;
    if (c) {
        grab_pointer(TRUE, OB_CURSOR_NONE);
        client_move(c, c->area.x + data->relative.delta, c->area.y);
        grab_pointer(FALSE, OB_CURSOR_NONE);
    }
}

void action_move_relative_vert(union ActionData *data)
{
    ObClient *c = data->relative.any.c;
    if (c) {
        grab_pointer(TRUE, OB_CURSOR_NONE);
        client_move(c, c->area.x, c->area.y + data->relative.delta);
        grab_pointer(FALSE, OB_CURSOR_NONE);
    }
}

void action_resize_relative_horz(union ActionData *data)
{
    ObClient *c = data->relative.any.c;
    if (c) {
        grab_pointer(TRUE, OB_CURSOR_NONE);
        client_resize(c,
                      c->area.width + data->relative.delta * c->size_inc.width,
                      c->area.height);
        grab_pointer(FALSE, OB_CURSOR_NONE);
    }
}

void action_resize_relative_vert(union ActionData *data)
{
    ObClient *c = data->relative.any.c;
    if (c && !c->shaded) {
        grab_pointer(TRUE, OB_CURSOR_NONE);
        client_resize(c, c->area.width, c->area.height +
                      data->relative.delta * c->size_inc.height);
        grab_pointer(FALSE, OB_CURSOR_NONE);
    }
}

void action_maximize_full(union ActionData *data)
{
    if (data->client.any.c)
        client_maximize(data->client.any.c, TRUE, 0, TRUE);
}

void action_unmaximize_full(union ActionData *data)
{
    if (data->client.any.c)
        client_maximize(data->client.any.c, FALSE, 0, TRUE);
}

void action_toggle_maximize_full(union ActionData *data)
{
    if (data->client.any.c)
        client_maximize(data->client.any.c,
                        !(data->client.any.c->max_horz ||
                          data->client.any.c->max_vert),
                        0, TRUE);
}

void action_maximize_horz(union ActionData *data)
{
    if (data->client.any.c)
        client_maximize(data->client.any.c, TRUE, 1, TRUE);
}

void action_unmaximize_horz(union ActionData *data)
{
    if (data->client.any.c)
        client_maximize(data->client.any.c, FALSE, 1, TRUE);
}

void action_toggle_maximize_horz(union ActionData *data)
{
    if (data->client.any.c)
        client_maximize(data->client.any.c,
                        !data->client.any.c->max_horz, 1, TRUE);
}

void action_maximize_vert(union ActionData *data)
{
    if (data->client.any.c)
        client_maximize(data->client.any.c, TRUE, 2, TRUE);
}

void action_unmaximize_vert(union ActionData *data)
{
    if (data->client.any.c)
        client_maximize(data->client.any.c, FALSE, 2, TRUE);
}

void action_toggle_maximize_vert(union ActionData *data)
{
    if (data->client.any.c)
        client_maximize(data->client.any.c, !data->client.any.c->max_vert, 2, TRUE);
}

void action_send_to_desktop(union ActionData *data)
{
    ObClient *c = data->sendto.any.c;

    if (!c || !client_normal(c)) return;

    if (data->sendto.desk < screen_num_desktops ||
        data->sendto.desk == DESKTOP_ALL) {
        client_set_desktop(c, data->sendto.desk, data->sendto.follow);
        if (data->sendto.follow)
            screen_set_desktop(data->sendto.desk);
    }
}

void action_desktop(union ActionData *data)
{
    if (data->desktop.desk < screen_num_desktops ||
        data->desktop.desk == DESKTOP_ALL)
        screen_set_desktop(data->desktop.desk);
}

void action_desktop_dir(union ActionData *data)
{
    guint d;

    d = screen_cycle_desktop(data->desktopdir.dir,
                             data->desktopdir.wrap,
                             data->sendtodir.linear,
                             data->desktopdir.inter.any.interactive,
                             data->desktopdir.inter.final,
                             data->desktopdir.inter.cancel);
    screen_set_desktop(d);
}

void action_send_to_desktop_dir(union ActionData *data)
{
    ObClient *c = data->sendtodir.inter.any.c;
    guint d;

    if (!c || !client_normal(c)) return;

    d = screen_cycle_desktop(data->sendtodir.dir, data->sendtodir.wrap,
                             data->sendtodir.linear,
                             data->sendtodir.inter.any.interactive,
                             data->sendtodir.inter.final,
                             data->sendtodir.inter.cancel);
    client_set_desktop(c, d, data->sendtodir.follow);
    if (data->sendtodir.follow)
        screen_set_desktop(d);
}

void action_desktop_last(union ActionData *data)
{
    screen_set_desktop(screen_last_desktop);
}

void action_toggle_decorations(union ActionData *data)
{
    ObClient *c = data->client.any.c;

    if (!c) return;

    c->decorate = !c->decorate;
    client_setup_decor_and_functions(c);
}

void action_moveresize(union ActionData *data)
{
    ObClient *c = data->moveresize.any.c;

    if (!c || !client_normal(c)) return;

    moveresize_start(c, data->moveresize.x, data->moveresize.y,
                     data->moveresize.button, data->moveresize.corner);
}

void action_reconfigure(union ActionData *data)
{
    ob_reconfigure();
}

void action_restart(union ActionData *data)
{
    ob_restart_other(data->execute.path);
}

void action_exit(union ActionData *data)
{
    ob_exit();
}

void action_showmenu(union ActionData *data)
{
    if (data->showmenu.name) {
        menu_show(data->showmenu.name, data->showmenu.x, data->showmenu.y,
                  data->showmenu.any.c);
    }
}

void action_cycle_windows(union ActionData *data)
{
    focus_cycle(data->cycle.forward, data->cycle.linear,
                data->cycle.inter.any.interactive,
                data->cycle.inter.final, data->cycle.inter.cancel);
}

void action_directional_focus(union ActionData *data)
{
    focus_directional_cycle(data->interdiraction.direction,
                            data->interdiraction.inter.any.interactive,
                            data->interdiraction.inter.final,
                            data->interdiraction.inter.cancel);
}

void action_movetoedge(union ActionData *data)
{
    int x, y;
    ObClient *c = data->diraction.any.c;

    if (!c)
        return;
    x = c->frame->area.x;
    y = c->frame->area.y;
    
    switch(data->diraction.direction) {
    case OB_DIRECTION_NORTH:
        y = client_directional_edge_search(c, OB_DIRECTION_NORTH);
        break;
    case OB_DIRECTION_WEST:
        x = client_directional_edge_search(c, OB_DIRECTION_WEST);
        break;
    case OB_DIRECTION_SOUTH:
        y = client_directional_edge_search(c, OB_DIRECTION_SOUTH) -
            c->frame->area.height;
        break;
    case OB_DIRECTION_EAST:
        x = client_directional_edge_search(c, OB_DIRECTION_EAST) -
            c->frame->area.width;
        break;
    default:
        g_assert_not_reached();
    }
    frame_frame_gravity(c->frame, &x, &y);
    grab_pointer(TRUE, OB_CURSOR_NONE);
    client_move(c, x, y);
    grab_pointer(FALSE, OB_CURSOR_NONE);

}

void action_growtoedge(union ActionData *data)
{
    int x, y, width, height, dest;
    ObClient *c = data->diraction.any.c;
    Rect *a = screen_area(c->desktop);

    if (!c)
        return;
    
    x = c->frame->area.x;
    y = c->frame->area.y;
    width = c->frame->area.width;
    height = c->frame->area.height;

    switch(data->diraction.direction) {
    case OB_DIRECTION_NORTH:
        dest = client_directional_edge_search(c, OB_DIRECTION_NORTH);
        if (a->y == y)
            height = c->frame->area.height / 2;
        else {
            height = c->frame->area.y + c->frame->area.height - dest;
            y = dest;
        }
        break;
    case OB_DIRECTION_WEST:
        dest = client_directional_edge_search(c, OB_DIRECTION_WEST);
        if (a->x == x)
            width = c->frame->area.width / 2;
        else {
            width = c->frame->area.x + c->frame->area.width - dest;
            x = dest;
        }
        break;
    case OB_DIRECTION_SOUTH:
        dest = client_directional_edge_search(c, OB_DIRECTION_SOUTH);
        if (a->y + a->height == y + c->frame->area.height) {
            height = c->frame->area.height / 2;
            y = a->y + a->height - height;
        } else
            height = dest - c->frame->area.y;
        y += (height - c->frame->area.height) % c->size_inc.height;
        height -= (height - c->frame->area.height) % c->size_inc.height;
        break;
    case OB_DIRECTION_EAST:
        dest = client_directional_edge_search(c, OB_DIRECTION_EAST);
        if (a->x + a->width == x + c->frame->area.width) {
            width = c->frame->area.width / 2;
            x = a->x + a->width - width;
        } else
            width = dest - c->frame->area.x;
        x += (width - c->frame->area.width) % c->size_inc.width;
        width -= (width - c->frame->area.width) % c->size_inc.width;
        break;
    default:
        g_assert_not_reached();
    }
    frame_frame_gravity(c->frame, &x, &y);
    width -= c->frame->size.left + c->frame->size.right;
    height -= c->frame->size.top + c->frame->size.bottom;
    grab_pointer(TRUE, OB_CURSOR_NONE);
    client_move_resize(c, x, y, width, height);
    grab_pointer(FALSE, OB_CURSOR_NONE);
}

void action_send_to_layer(union ActionData *data)
{
    if (data->layer.any.c)
        client_set_layer(data->layer.any.c, data->layer.layer);
}

void action_toggle_layer(union ActionData *data)
{
    ObClient *c = data->layer.any.c;

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
