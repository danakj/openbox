#include "client.h"
#include "stacking.h"
#include "screen.h"

#include <glib.h>

void action_execute(char *path)
{
    GError *e;
    if (!g_spawn_command_line_async(path, &e)) {
        g_warning("failed to execute '%s': %s", path, e->message);
    }
}

void action_iconify(Client *c)
{
    client_iconify(c, TRUE, TRUE);
}

void action_raise(Client *c)
{
    stacking_raise(c);
}

void action_lower(Client *c)
{
    stacking_lower(c);
}

void action_close(Client *c)
{
    client_close(c);
}

void action_shade(Client *c)
{
    client_shade(c, TRUE);
}

void action_unshade(Client *c)
{
    client_shade(c, FALSE);
}

void action_toggle_shade(Client *c)
{
    client_shade(c, !c->shaded);
}

void action_toggle_omnipresent(Client *c)
{
    client_set_desktop(c, c->desktop == DESKTOP_ALL ?
                       screen_desktop : DESKTOP_ALL);
}

void action_move_relative(Client *c, int dx, int dy)
{
    client_configure(c, Corner_TopLeft, c->area.x + dx, c->area.y + dy,
                     c->area.width, c->area.height, TRUE, TRUE);
}

void action_resize_relative(Client *c, int dx, int dy)
{
    client_configure(c, Corner_TopLeft, c->area.x, c->area.y,
                     c->area.width + dx, c->area.height + dy, TRUE, TRUE);
}

void action_maximize_full(Client *c)
{
    client_maximize(c, TRUE, 0, TRUE);
}

void action_unmaximize_full(Client *c)
{
    client_maximize(c, FALSE, 0, TRUE);
}

void action_toggle_maximize_full(Client *c)
{
    client_maximize(c, !(c->max_horz || c->max_vert), 0, TRUE);
}

void action_maximize_horz(Client *c)
{
    client_maximize(c, TRUE, 1, TRUE);
}

void action_unmaximize_horz(Client *c)
{
    client_maximize(c, FALSE, 1, TRUE);
}

void action_toggle_maximize_horz(Client *c)
{
    client_maximize(c, !c->max_horz, 1, TRUE);
}

void action_maximize_vert(Client *c)
{
    client_maximize(c, TRUE, 2, TRUE);
}

void action_unmaximize_vert(Client *c)
{
    client_maximize(c, FALSE, 2, TRUE);
}

void action_toggle_maximize_vert(Client *c)
{
    client_maximize(c, !c->max_vert, 2, TRUE);
}

void action_send_to_desktop(Client *c, guint desktop)
{
    if (desktop < screen_num_desktops || desktop == DESKTOP_ALL)
        client_set_desktop(c, desktop);
}

void action_send_to_next_desktop(Client *c, gboolean wrap, gboolean follow)
{
    guint d;

    d = screen_desktop + 1;
    if (d >= screen_num_desktops) {
        if (!wrap) return;
        d = 0;
    }
    client_set_desktop(c, d);
    if (follow) screen_set_desktop(d);
}

void action_send_to_previous_desktop(Client *c, gboolean wrap, gboolean follow)
{
    guint d;

    d = screen_desktop - 1;
    if (d >= screen_num_desktops) {
        if (!wrap) return;
        d = screen_num_desktops - 1;
    }
    client_set_desktop(c, d);
    if (follow) screen_set_desktop(d);
}

void action_desktop(guint desktop)
{
    if (desktop < screen_num_desktops || desktop == DESKTOP_ALL)
        screen_set_desktop(desktop);
}

void action_next_desktop(gboolean wrap)
{
    guint d;

    d = screen_desktop + 1;
    if (d >= screen_num_desktops) {
        if (!wrap) return;
        d = 0;
    }
    screen_set_desktop(d);
}

void action_previous_desktop(gboolean wrap)
{
    guint d;

    d = screen_desktop - 1;
    if (d >= screen_num_desktops) {
        if (!wrap) return;
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

void action_next_desktop_column(gboolean wrap)
{
    guint r, c, d;

    cur_row_col(&r, &c);
    ++c;
    d = translate_row_col(r, c);
    if (d >= screen_num_desktops) {
        if (!wrap) return;
        c = 0;
    }
    if (d >= screen_num_desktops)
        ++c;
    d = translate_row_col(r, c);
    if (d < screen_num_desktops)
        screen_set_desktop(d);
}

void action_previous_desktop_column(gboolean wrap)
{
    guint r, c, d;

    cur_row_col(&r, &c);
    --c;
    d = translate_row_col(r, c);
    if (d >= screen_num_desktops) {
        if (!wrap) return;
        c = screen_desktop_layout.columns - 1;
    }
    if (d >= screen_num_desktops)
        --c;
    d = translate_row_col(r, c);
    if (d < screen_num_desktops)
        screen_set_desktop(d);
}

void action_next_desktop_row(gboolean wrap)
{
    guint r, c, d;

    cur_row_col(&r, &c);
    ++r;
    d = translate_row_col(r, c);
    if (d >= screen_num_desktops) {
        if (!wrap) return;
        r = 0;
    }
    if (d >= screen_num_desktops)
        ++r;
    d = translate_row_col(r, c);
    if (d < screen_num_desktops)
        screen_set_desktop(d);
}

void action_previous_desktop_row(gboolean wrap)
{
    guint r, c, d;

    cur_row_col(&r, &c);
    --r;
    d = translate_row_col(r, c);
    if (d >= screen_num_desktops) {
        if (!wrap) return;
        c = screen_desktop_layout.rows - 1;
    }
    if (d >= screen_num_desktops)
        --r;
    d = translate_row_col(r, c);
    if (d < screen_num_desktops)
        screen_set_desktop(d);
}

void action_toggle_decorations(Client *c)
{
    c->disabled_decorations = c->disabled_decorations ? 0 : ~0;
    client_setup_decor_and_functions(c);
}
