#include "client.h"
#include "stacking.h"
#include "frame.h"
#include "screen.h"
#include "action.h"
#include "dispatch.h"
#include "openbox.h"

#include <glib.h>

Action *action_new(void (*func)(union ActionData *data))
{
    Action *a = g_new(Action, 1);
    a->func = func;

    /* deal with pointers */
    if (func == action_execute)
        a->data.execute.path = NULL;

    return a;
}

void action_free(Action *a)
{
    /* deal with pointers */
    if (a->func == action_execute)
        g_free(a->data.execute.path);

    g_free(a);
}

void action_execute(union ActionData *data)
{
    GError *e = NULL;
    if (!g_spawn_command_line_async(data->execute.path, &e)) {
        g_warning("failed to execute '%s': %s",
                  data->execute.path, e->message);
    }
}

void action_focus(union ActionData *data)
{
    client_focus(data->client.c);
}

void action_unfocus (union ActionData *data)
{
    client_unfocus(data->client.c);
}

void action_iconify(union ActionData *data)
{
    client_iconify(data->client.c, TRUE, TRUE);
}

void action_focusraise(union ActionData *data)
{
    client_focus(data->client.c);
    stacking_raise(data->client.c);
}

void action_raise(union ActionData *data)
{
    stacking_raise(data->client.c);
}

void action_lower(union ActionData *data)
{
    stacking_lower(data->client.c);
}

void action_close(union ActionData *data)
{
    client_close(data->client.c);
}

void action_kill(union ActionData *data)
{
    client_kill(data->client.c);
}

void action_shade(union ActionData *data)
{
    client_shade(data->client.c, TRUE);
}

void action_unshade(union ActionData *data)
{
    client_shade(data->client.c, FALSE);
}

void action_toggle_shade(union ActionData *data)
{
    client_shade(data->client.c, !data->client.c->shaded);
}

void action_toggle_omnipresent(union ActionData *data)
{
    client_set_desktop(data->client.c, data->client.c->desktop == DESKTOP_ALL ?
                       screen_desktop : DESKTOP_ALL);
}

void action_move_relative(union ActionData *data)
{
    Client *c = data->relative.c;
    client_configure(c, Corner_TopLeft,
                     c->area.x + data->relative.dx,
                     c->area.y + data->relative.dy,
                     c->area.width, c->area.height, TRUE, TRUE);
}

void action_resize_relative(union ActionData *data)
{
    Client *c = data->relative.c;
    client_configure(c, Corner_TopLeft, c->area.x, c->area.y,
                     c->area.width + data->relative.dx,
                     c->area.height + data->relative.dy, TRUE, TRUE);
}

void action_maximize_full(union ActionData *data)
{
    client_maximize(data->client.c, TRUE, 0, TRUE);
}

void action_unmaximize_full(union ActionData *data)
{
    client_maximize(data->client.c, FALSE, 0, TRUE);
}

void action_toggle_maximize_full(union ActionData *data)
{
    client_maximize(data->client.c,
                    !(data->client.c->max_horz || data->client.c->max_vert),
                    0, TRUE);
}

void action_maximize_horz(union ActionData *data)
{
    client_maximize(data->client.c, TRUE, 1, TRUE);
}

void action_unmaximize_horz(union ActionData *data)
{
    client_maximize(data->client.c, FALSE, 1, TRUE);
}

void action_toggle_maximize_horz(union ActionData *data)
{
    client_maximize(data->client.c, !data->client.c->max_horz, 1, TRUE);
}

void action_maximize_vert(union ActionData *data)
{
    client_maximize(data->client.c, TRUE, 2, TRUE);
}

void action_unmaximize_vert(union ActionData *data)
{
    client_maximize(data->client.c, FALSE, 2, TRUE);
}

void action_toggle_maximize_vert(union ActionData *data)
{
    client_maximize(data->client.c, !data->client.c->max_vert, 2, TRUE);
}

void action_send_to_desktop(union ActionData *data)
{
    if (data->sendto.desktop < screen_num_desktops ||
        data->sendto.desktop == DESKTOP_ALL)
        client_set_desktop(data->sendto.c, data->sendto.desktop);
}

void action_send_to_next_desktop(union ActionData *data)
{
    guint d;

    d = screen_desktop + 1;
    if (d >= screen_num_desktops) {
        if (!data->sendtonextprev.wrap) return;
        d = 0;
    }
    client_set_desktop(data->sendtonextprev.c, d);
    if (data->sendtonextprev.follow) screen_set_desktop(d);
}

void action_send_to_previous_desktop(union ActionData *data)
{
    guint d;

    d = screen_desktop - 1;
    if (d >= screen_num_desktops) {
        if (!data->sendtonextprev.wrap) return;
        d = screen_num_desktops - 1;
    }
    client_set_desktop(data->sendtonextprev.c, d);
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
        break;
        }
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
    Client *c = data->client.c;
    c->disabled_decorations = c->disabled_decorations ? 0 : ~0;
    client_setup_decor_and_functions(c);
}

void action_move(union ActionData *data)
{
    Client *c = data->move.c;
    int x = data->move.x;
    int y = data->move.y;

    dispatch_move(c, &x, &y);

    frame_frame_gravity(c->frame, &x, &y); /* get where the client should be */
    client_configure(c, Corner_TopLeft, x, y, c->area.width, c->area.height,
                     TRUE, data->move.final);
}

void action_resize(union ActionData *data)
{
    Client *c = data->resize.c;
    int w = data->resize.x - c->frame->size.left - c->frame->size.right;
    int h = data->resize.y - c->frame->size.top - c->frame->size.bottom;

    /* XXX window snapping/struts */

    client_configure(c, data->resize.corner, c->area.x, c->area.y, w, h,
                     TRUE, data->resize.final);
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
