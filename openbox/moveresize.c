#include "grab.h"
#include "framerender.h"
#include "prop.h"
#include "client.h"
#include "dispatch.h"
#include "openbox.h"

#include <X11/Xlib.h>
#include <glib.h>

gboolean moveresize_in_progress = FALSE;
static gboolean moving = FALSE; /* TRUE - moving, FALSE - resizing */

static Window coords = None;
static int start_x, start_y, start_cx, start_cy, start_cw, start_ch;
static int cur_x, cur_y;
static Client *client;
static guint button;
static guint32 corner;
static Corner lockcorner;

#define POPUP_X (10)
#define POPUP_Y (10)

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

    client = c;
    start_cx = c->frame->area.x;
    start_cy = c->frame->area.y;
    start_cw = c->area.width;
    start_ch = c->area.height;
    start_x = x;
    start_y = y;
    button = b;
    corner = cnr;

    moveresize_in_progress = TRUE;
    moving = (corner == prop_atoms.net_wm_moveresize_move ||
              corner == prop_atoms.net_wm_moveresize_move_keyboard);

    if (corner == prop_atoms.net_wm_moveresize_size_topleft)
        cur = ob_cursors.tl;
    else if (corner == prop_atoms.net_wm_moveresize_size_top)
        cur = ob_cursors.tl;
    else if (corner == prop_atoms.net_wm_moveresize_size_topright)
        cur = ob_cursors.tr;
    else if (corner == prop_atoms.net_wm_moveresize_size_right)
        cur = ob_cursors.tr;
    else if (corner == prop_atoms.net_wm_moveresize_size_bottomright)
        cur = ob_cursors.br;
    else if (corner == prop_atoms.net_wm_moveresize_size_bottom)
        cur = ob_cursors.br;
    else if (corner == prop_atoms.net_wm_moveresize_size_bottomleft)
        cur = ob_cursors.bl;
    else if (corner == prop_atoms.net_wm_moveresize_size_left)
        cur = ob_cursors.bl;
    else if (corner == prop_atoms.net_wm_moveresize_size_keyboard)
        cur = ob_cursors.br;
    else if (corner == prop_atoms.net_wm_moveresize_move)
        cur = ob_cursors.move;
    else if (corner == prop_atoms.net_wm_moveresize_move_keyboard)
        cur = ob_cursors.move;
    else
        g_assert_not_reached();

    grab_keyboard(TRUE);
    grab_pointer(TRUE, cur);
}

void moveresize_event(XEvent *e)
{
    g_assert(moveresize_in_progress);

    if (e->type == MotionNotify) {
        if (moving) {
            cur_x = start_cx + e->xmotion.x_root - start_x;
            cur_y = start_cy + e->xmotion.y_root - start_y;

            dispatch_move(client, &cur_x, &cur_y);

            popup_coords("X:  %d  Y:  %d", cur_x, cur_y);

            /* get where the client should be */
            frame_frame_gravity(client->frame, &cur_x, &cur_y);
            client_configure(client, Corner_TopLeft, cur_x, cur_y,
                             start_cw, start_ch, TRUE, FALSE);
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

            /* dispatch_resize needs the frame size */
            cur_x += client->frame->size.left + client->frame->size.right;
            cur_y += client->frame->size.top + client->frame->size.bottom;

            dispatch_resize(client, &cur_x, &cur_y, lockcorner);

            cur_x -= client->frame->size.left + client->frame->size.right;
            cur_y -= client->frame->size.top + client->frame->size.bottom;
    
            client_configure(client, lockcorner, client->area.x,
                             client->area.y, cur_x, cur_y, TRUE, FALSE);

            popup_coords("W:  %d  H:  %d", client->logical_size.width,
                         client->logical_size.height);
        }
    } else if (e->type == ButtonRelease) {
        if (e->xbutton.button == button) {
            grab_keyboard(FALSE);
            grab_pointer(FALSE, None);

            XDestroyWindow(ob_display, coords);
            coords = None;

            moveresize_in_progress = FALSE;

            if (moving) {
                client_configure(client, Corner_TopLeft, cur_x, cur_y,
                                 start_cw, start_ch, TRUE, TRUE);
            } else {
                client_configure(client, lockcorner, client->area.x,
                                 client->area.y, cur_x, cur_y, TRUE, TRUE);
            }
        }
    } else if (e->type == KeyPress) {
    }
}
