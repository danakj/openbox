#include "grab.h"
#include "framerender.h"
#include "screen.h"
#include "prop.h"
#include "client.h"
#include "dispatch.h"
#include "openbox.h"
#include "popup.h"
#include "config.h"
#include "render/render.h"
#include "render/theme.h"

#include <X11/Xlib.h>
#include <glib.h>

gboolean moveresize_in_progress = FALSE;
Client *moveresize_client = NULL;

static gboolean moving = FALSE; /* TRUE - moving, FALSE - resizing */

static int start_x, start_y, start_cx, start_cy, start_cw, start_ch;
static int cur_x, cur_y;
static guint button;
static guint32 corner;
static Corner lockcorner;

static guint button_return, button_escape, button_left, button_right,
    button_up, button_down;

static Popup *popup = NULL;
static InternalWindow opaque_window = { { Window_Internal }, None };
static GC opaque_gc = None;
static gboolean first_draw = FALSE;

#define POPUP_X (10)
#define POPUP_Y (10)

void moveresize_startup()
{
    XSetWindowAttributes attrib;
    XGCValues gcv;

    button_return = XKeysymToKeycode(ob_display, XStringToKeysym("Return"));
    button_escape = XKeysymToKeycode(ob_display, XStringToKeysym("Escape"));
    button_left = XKeysymToKeycode(ob_display, XStringToKeysym("Left"));
    button_right = XKeysymToKeycode(ob_display, XStringToKeysym("Right"));
    button_up = XKeysymToKeycode(ob_display, XStringToKeysym("Up"));
    button_down = XKeysymToKeycode(ob_display, XStringToKeysym("Down"));

    popup = popup_new(FALSE);
    popup_size_to_string(popup, "W:  0000  W:  0000");

    attrib.save_under = True;
    opaque_window.win = XCreateWindow(ob_display, ob_root, 0, 0, 1, 1, 0,
                                      RrDepth(ob_rr_inst), InputOutput,
                                      RrVisual(ob_rr_inst),
                                      CWSaveUnder, &attrib);
    stacking_add(INTERNAL_AS_WINDOW(&opaque_window));
    stacking_raise(INTERNAL_AS_WINDOW(&opaque_window));

    /* a GC to invert stuff */
    gcv.function = GXxor;
    gcv.line_width = ob_rr_theme->bwidth;
    gcv.foreground = (WhitePixel(ob_display, ob_screen) ^
                      BlackPixel(ob_display, ob_screen));
    opaque_gc = XCreateGC(ob_display, opaque_window.win,
                          GCFunction | GCForeground | GCLineWidth, &gcv);
}

void moveresize_shutdown()
{
    popup_free(popup);
    popup = NULL;
    stacking_remove(&opaque_window);
    XFreeGC(ob_display, opaque_gc);
    XDestroyWindow(ob_display, opaque_window.win);
}

static void popup_coords(char *format, int a, int b)
{
    char *text;
    Rect *area;

    text = g_strdup_printf(format, a, b);
    area = screen_physical_area_xinerama(0);
    popup_position(popup, NorthWestGravity,
                   POPUP_X + area->x, POPUP_Y + area->y);
    popup_show(popup, text, NULL);
    g_free(text);
}

void moveresize_start(Client *c, int x, int y, guint b, guint32 cnr)
{
    Cursor cur;
    Rect *a;

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

    a = screen_physical_area();

    XMoveResizeWindow(ob_display, opaque_window.win,
                      a->x, a->y, a->width, a->height);
    stacking_raise(INTERNAL_AS_WINDOW(&opaque_window));
    if (corner == prop_atoms.net_wm_moveresize_move ||
        corner == prop_atoms.net_wm_moveresize_move_keyboard) {
        if (!config_opaque_move)
            XMapWindow(ob_display, opaque_window.win);
    } else {
        if (!config_opaque_resize)
            XMapWindow(ob_display, opaque_window.win);
    }
    first_draw = TRUE;
}

void moveresize_end(gboolean cancel)
{
    XUnmapWindow(ob_display, opaque_window.win);

    grab_keyboard(FALSE);
    grab_pointer(FALSE, None);

    popup_hide(popup);

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
    int oldx, oldy, oldw, oldh;

    dispatch_move(moveresize_client, &cur_x, &cur_y);

    oldx = moveresize_client->frame->area.x;
    oldy = moveresize_client->frame->area.y;
    oldw = moveresize_client->frame->area.width;
    oldh = moveresize_client->frame->area.height;
    /* get where the client should be */
    frame_frame_gravity(moveresize_client->frame, &cur_x, &cur_y);
    client_configure(moveresize_client, Corner_TopLeft, cur_x, cur_y,
                     start_cw, start_ch, TRUE, FALSE);
    /* draw the new one */
    if (moveresize_client->frame->area.x != oldx ||
        moveresize_client->frame->area.y != oldy ||
        moveresize_client->frame->area.width != oldw ||
        moveresize_client->frame->area.height != oldh) {
        if (!config_opaque_move)
            XDrawRectangle(ob_display, opaque_window.win, opaque_gc,
                           moveresize_client->frame->area.x,
                           moveresize_client->frame->area.y,
                           moveresize_client->frame->area.width - 1,
                           moveresize_client->frame->area.height - 1);
        /* erase the old one */
        if (!config_opaque_move && !first_draw)
            XDrawRectangle(ob_display, opaque_window.win, opaque_gc,
                           oldx, oldy, oldw - 1, oldh - 1);
        first_draw = FALSE;
    }

    /* this would be better with a fixed width font ... XXX can do it better
       if there are 2 text boxes */
    popup_coords("X:  %4d  Y:  %4d", moveresize_client->frame->area.x,
                 moveresize_client->frame->area.y);
}

static void do_resize()
{
    int oldx, oldy, oldw, oldh;

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
    
    oldx = moveresize_client->frame->area.x;
    oldy = moveresize_client->frame->area.y;
    oldw = moveresize_client->frame->area.width;
    oldh = moveresize_client->frame->area.height;
    client_configure(moveresize_client, lockcorner, 
                     moveresize_client->area.x, moveresize_client->area.y,
                     cur_x, cur_y, TRUE, FALSE);
    /* draw the new one */
    if (!config_opaque_resize)
        XDrawRectangle(ob_display, opaque_window.win, opaque_gc,
                       moveresize_client->frame->area.x,
                       moveresize_client->frame->area.y,
                       moveresize_client->frame->area.width - 1,
                       moveresize_client->frame->area.height - 1);
    /* erase the old one */
    if (!config_opaque_resize && !first_draw)
        XDrawRectangle(ob_display, opaque_window.win, opaque_gc,
                       oldx, oldy, oldw - 1, oldh - 1);
    first_draw = FALSE;

    /* this would be better with a fixed width font ... XXX can do it better
       if there are 2 text boxes */
    popup_coords("W:  %4d  H:  %4d", moveresize_client->logical_size.width,
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
