#include "grab.h"
#include "framerender.h"
#include "prop.h"
#include "client.h"
#include "dispatch.h"
#include "openbox.h"

#include <X11/Xlib.h>
#include <glib.h>

gboolean moveresize_in_progress = FALSE;
Client *moveresize_client = NULL;

static gboolean moving = FALSE; /* TRUE - moving, FALSE - resizing */

static Window coords = None;
static int start_x, start_y, start_cx, start_cy, start_cw, start_ch;
static int cur_x, cur_y;
static guint button;
static guint32 corner;
static Corner lockcorner;

static guint button_return, button_escape, button_left, button_right,
    button_up, button_down;

#define POPUP_X (10)
#define POPUP_Y (10)

void moveresize_startup()
{
    button_return = XKeysymToKeycode(ob_display, XStringToKeysym("Return"));
    button_escape = XKeysymToKeycode(ob_display, XStringToKeysym("Escape"));
    button_left = XKeysymToKeycode(ob_display, XStringToKeysym("Left"));
    button_right = XKeysymToKeycode(ob_display, XStringToKeysym("Right"));
    button_up = XKeysymToKeycode(ob_display, XStringToKeysym("Up"));
    button_down = XKeysymToKeycode(ob_display, XStringToKeysym("Down"));
}

static void popup_coords(char *format, int a, int b)
{
    XSetWindowAttributes attrib;
    Size s;
    char *text;

    if (coords == None) {
        attrib.override_redirect = TRUE;
        coords = XCreateWindow(ob_display, ob_root,
                               0, 0, 1, 1, 0, render_depth, InputOutput,
                               render_visual, CWOverrideRedirect, &attrib);
        g_assert(coords != None);

        XMapWindow(ob_display, coords);
    }

    text = g_strdup_printf(format, a, b);
    framerender_size_popup_label(text, &s);
    XMoveResizeWindow(ob_display, coords,
                      POPUP_X, POPUP_Y, s.width, s.height);
    framerender_popup_label(coords, &s, text);
    g_free(text);
}

void moveresize_start(Client *c, int x, int y, guint b, guint32 cnr)
{
    Cursor cur;

    g_assert(!moveresize_in_progress);

    moveresize_client = c;
    start_cx = c->frame->area.x;
    start_cy = c->frame->area.y;
    start_cw = c->area.width;
    start_ch = c->area.height;
    start_x = x;
    start_y = y;
    if (corner == prop_atoms.net_wm_moveresize_move_keyboard ||
        corner == prop_atoms.net_wm_moveresize_size_keyboard)
        button = 0; /* mouse can't end it without being pressed first */
    else
        button = b;
    corner = cnr;

    if (corner == prop_atoms.net_wm_moveresize_move ||
        corner == prop_atoms.net_wm_moveresize_move_keyboard) {
        cur_x = start_cx;
        cur_y = start_cy;
        moving = TRUE;
    } else {
        cur_x = start_cw;
        cur_y = start_ch;
        moving = FALSE;
    }

    moveresize_in_progress = TRUE;

    if (corner == prop_atoms.net_wm_moveresize_size_topleft)
        cur = ob_cursors.tl;
    else if (corner == prop_atoms.net_wm_moveresize_size_top)
        cur = ob_cursors.t;
    else if (corner == prop_atoms.net_wm_moveresize_size_topright)
        cur = ob_cursors.tr;
    else if (corner == prop_atoms.net_wm_moveresize_size_right)
        cur = ob_cursors.r;
    else if (corner == prop_atoms.net_wm_moveresize_size_bottomright)
        cur = ob_cursors.br;
    else if (corner == prop_atoms.net_wm_moveresize_size_bottom)
        cur = ob_cursors.b;
    else if (corner == prop_atoms.net_wm_moveresize_size_bottomleft)
        cur = ob_cursors.bl;
    else if (corner == prop_atoms.net_wm_moveresize_size_left)
        cur = ob_cursors.l;
    else if (corner == prop_atoms.net_wm_moveresize_size_keyboard)
        cur = ob_cursors.br;
    else if (corner == prop_atoms.net_wm_moveresize_move)
        cur = ob_cursors.move;
    else if (corner == prop_atoms.net_wm_moveresize_move_keyboard)
        cur = ob_cursors.move;
    else
        g_assert_not_reached();

    grab_pointer(TRUE, cur);
    grab_keyboard(TRUE);
}

void moveresize_end(gboolean cancel)
{
    grab_keyboard(FALSE);
    grab_pointer(FALSE, None);

    XDestroyWindow(ob_display, coords);
    coords = None;

    if (moving) {
        client_configure(moveresize_client, Corner_TopLeft,
                         (cancel ? start_cx : cur_x),
                         (cancel ? start_cy : cur_y),
                         start_cw, start_ch, TRUE, TRUE);
    } else {
        client_configure(moveresize_client, lockcorner,
                         moveresize_client->area.x,
                         moveresize_client->area.y,
                         (cancel ? start_cw : cur_x),
                         (cancel ? start_ch : cur_y), TRUE, TRUE);
    }

    moveresize_in_progress = FALSE;
    moveresize_client = NULL;
}

static void do_move()
{
    dispatch_move(moveresize_client, &cur_x, &cur_y);

    popup_coords("X:  %d  Y:  %d", cur_x, cur_y);

    /* get where the client should be */
    frame_frame_gravity(moveresize_client->frame, &cur_x, &cur_y);
    client_configure(moveresize_client, Corner_TopLeft, cur_x, cur_y,
                     start_cw, start_ch, TRUE, FALSE);
}

static void do_resize()
{
    /* dispatch_resize needs the frame size */
    cur_x += moveresize_client->frame->size.left +
        moveresize_client->frame->size.right;
    cur_y += moveresize_client->frame->size.top +
        moveresize_client->frame->size.bottom;

    dispatch_resize(moveresize_client, &cur_x, &cur_y, lockcorner);

    cur_x -= moveresize_client->frame->size.left +
        moveresize_client->frame->size.right;
    cur_y -= moveresize_client->frame->size.top +
        moveresize_client->frame->size.bottom;
    
    client_configure(moveresize_client, lockcorner, moveresize_client->area.x,
                     moveresize_client->area.y, cur_x, cur_y, TRUE, FALSE);

    popup_coords("W:  %d  H:  %d", moveresize_client->logical_size.width,
                 moveresize_client->logical_size.height);
}

void moveresize_event(XEvent *e)
{
    g_assert(moveresize_in_progress);

    if (e->type == ButtonPress) {
        if (!button) {
            start_x = e->xbutton.x_root;
            start_y = e->xbutton.y_root;
            button = e->xbutton.button; /* this will end it now */
        }
    } else if (e->type == ButtonRelease) {
        if (!button || e->xbutton.button == button) {
            moveresize_end(FALSE);
        }
    } else if (e->type == MotionNotify) {
        if (moving) {
            cur_x = start_cx + e->xmotion.x_root - start_x;
            cur_y = start_cy + e->xmotion.y_root - start_y;
            do_move();
        } else {
            if (corner == prop_atoms.net_wm_moveresize_size_topleft) {
                cur_x = start_cw - (e->xmotion.x_root - start_x);
                cur_y = start_ch - (e->xmotion.y_root - start_y);
                lockcorner = Corner_BottomRight;
            } else if (corner == prop_atoms.net_wm_moveresize_size_top) {
                cur_x = start_cw;
                cur_y = start_ch - (e->xmotion.y_root - start_y);
                lockcorner = Corner_BottomRight;
            } else if (corner == prop_atoms.net_wm_moveresize_size_topright) {
                cur_x = start_cw + (e->xmotion.x_root - start_x);
                cur_y = start_ch - (e->xmotion.y_root - start_y);
                lockcorner = Corner_BottomLeft;
            } else if (corner == prop_atoms.net_wm_moveresize_size_right) { 
                cur_x = start_cw + (e->xmotion.x_root - start_x);
                cur_y = start_ch;
                lockcorner = Corner_BottomLeft;
            } else if (corner ==
                       prop_atoms.net_wm_moveresize_size_bottomright) {
                cur_x = start_cw + (e->xmotion.x_root - start_x);
                cur_y = start_ch + (e->xmotion.y_root - start_y);
                lockcorner = Corner_TopLeft;
            } else if (corner == prop_atoms.net_wm_moveresize_size_bottom) {
                cur_x = start_cw;
                cur_y = start_ch + (e->xmotion.y_root - start_y);
                lockcorner = Corner_TopLeft;
            } else if (corner ==
                       prop_atoms.net_wm_moveresize_size_bottomleft) {
                cur_x = start_cw - (e->xmotion.x_root - start_x);
                cur_y = start_ch + (e->xmotion.y_root - start_y);
                lockcorner = Corner_TopRight;
            } else if (corner == prop_atoms.net_wm_moveresize_size_left) {
                cur_x = start_cw - (e->xmotion.x_root - start_x);
                cur_y = start_ch;
                lockcorner = Corner_TopRight;
            } else if (corner == prop_atoms.net_wm_moveresize_size_keyboard) {
                cur_x = start_cw + (e->xmotion.x_root - start_x);
                cur_y = start_ch + (e->xmotion.y_root - start_y);
                lockcorner = Corner_TopLeft;
            } else
                g_assert_not_reached();

            do_resize();
        }
    } else if (e->type == KeyPress) {
        if (e->xkey.keycode == button_escape)
            moveresize_end(TRUE);
        else if (e->xkey.keycode == button_return)
            moveresize_end(FALSE);
        else {
            if (corner == prop_atoms.net_wm_moveresize_size_keyboard) {
                if (e->xkey.keycode == button_right)
                    cur_x += MAX(4, moveresize_client->size_inc.width);
                else if (e->xkey.keycode == button_left)
                    cur_x -= MAX(4, moveresize_client->size_inc.width);
                else if (e->xkey.keycode == button_down)
                    cur_y += MAX(4, moveresize_client->size_inc.height);
                else if (e->xkey.keycode == button_up)
                    cur_y -= MAX(4, moveresize_client->size_inc.height);
                else
                    return;
                do_resize();
            } else if (corner == prop_atoms.net_wm_moveresize_move_keyboard) {
                if (e->xkey.keycode == button_right)
                    cur_x += 4;
                else if (e->xkey.keycode == button_left)
                    cur_x -= 4;
                else if (e->xkey.keycode == button_down)
                    cur_y += 4;
                else if (e->xkey.keycode == button_up)
                    cur_y -= 4;
                else
                    return;
                do_move();
            }
        }
    }
}
