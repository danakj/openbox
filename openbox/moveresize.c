#include "grab.h"
#include "framerender.h"
#include "screen.h"
#include "prop.h"
#include "client.h"
#include "frame.h"
#include "dispatch.h"
#include "openbox.h"
#include "popup.h"
#include "config.h"
#include "render/render.h"
#include "render/theme.h"

#include <X11/Xlib.h>
#include <glib.h>

gboolean moveresize_in_progress = FALSE;
ObClient *moveresize_client = NULL;

static gboolean moving = FALSE; /* TRUE - moving, FALSE - resizing */

static int start_x, start_y, start_cx, start_cy, start_cw, start_ch;
static int cur_x, cur_y;
static guint button;
static guint32 corner;
static ObCorner lockcorner;

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

    popup = popup_new(FALSE);
    popup_size_to_string(popup, "W:  0000  W:  0000");

    attrib.save_under = True;
    opaque_window.win = XCreateWindow(ob_display,
                                      RootWindow(ob_display, ob_screen),
                                      0, 0, 1, 1, 0,
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
    area = screen_physical_area_monitor(0);
    popup_position(popup, NorthWestGravity,
                   POPUP_X + area->x, POPUP_Y + area->y);
    popup_show(popup, text, NULL);
    g_free(text);
}

void moveresize_start(ObClient *c, int x, int y, guint b, guint32 cnr)
{
    ObCursor cur;
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
        cur = OB_CURSOR_NORTHWEST;
    else if (corner == prop_atoms.net_wm_moveresize_size_top)
        cur = OB_CURSOR_NORTH;
    else if (corner == prop_atoms.net_wm_moveresize_size_topright)
        cur = OB_CURSOR_NORTHEAST;
    else if (corner == prop_atoms.net_wm_moveresize_size_right)
        cur = OB_CURSOR_EAST;
    else if (corner == prop_atoms.net_wm_moveresize_size_bottomright)
        cur = OB_CURSOR_SOUTHEAST;
    else if (corner == prop_atoms.net_wm_moveresize_size_bottom)
        cur = OB_CURSOR_SOUTH;
    else if (corner == prop_atoms.net_wm_moveresize_size_bottomleft)
        cur = OB_CURSOR_SOUTHWEST;
    else if (corner == prop_atoms.net_wm_moveresize_size_left)
        cur = OB_CURSOR_WEST;
    else if (corner == prop_atoms.net_wm_moveresize_size_keyboard)
        cur = OB_CURSOR_SOUTHEAST;
    else if (corner == prop_atoms.net_wm_moveresize_move)
        cur = OB_CURSOR_MOVE;
    else if (corner == prop_atoms.net_wm_moveresize_move_keyboard)
        cur = OB_CURSOR_MOVE;
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
        client_configure(moveresize_client, OB_CORNER_TOPLEFT,
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
    client_configure(moveresize_client, OB_CORNER_TOPLEFT, cur_x, cur_y,
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
                lockcorner = OB_CORNER_BOTTOMRIGHT;
            } else if (corner == prop_atoms.net_wm_moveresize_size_top) {
                cur_x = start_cw;
                cur_y = start_ch - (e->xmotion.y_root - start_y);
                lockcorner = OB_CORNER_BOTTOMRIGHT;
            } else if (corner == prop_atoms.net_wm_moveresize_size_topright) {
                cur_x = start_cw + (e->xmotion.x_root - start_x);
                cur_y = start_ch - (e->xmotion.y_root - start_y);
                lockcorner = OB_CORNER_BOTTOMLEFT;
            } else if (corner == prop_atoms.net_wm_moveresize_size_right) { 
                cur_x = start_cw + (e->xmotion.x_root - start_x);
                cur_y = start_ch;
                lockcorner = OB_CORNER_BOTTOMLEFT;
            } else if (corner ==
                       prop_atoms.net_wm_moveresize_size_bottomright) {
                cur_x = start_cw + (e->xmotion.x_root - start_x);
                cur_y = start_ch + (e->xmotion.y_root - start_y);
                lockcorner = OB_CORNER_TOPLEFT;
            } else if (corner == prop_atoms.net_wm_moveresize_size_bottom) {
                cur_x = start_cw;
                cur_y = start_ch + (e->xmotion.y_root - start_y);
                lockcorner = OB_CORNER_TOPLEFT;
            } else if (corner ==
                       prop_atoms.net_wm_moveresize_size_bottomleft) {
                cur_x = start_cw - (e->xmotion.x_root - start_x);
                cur_y = start_ch + (e->xmotion.y_root - start_y);
                lockcorner = OB_CORNER_TOPRIGHT;
            } else if (corner == prop_atoms.net_wm_moveresize_size_left) {
                cur_x = start_cw - (e->xmotion.x_root - start_x);
                cur_y = start_ch;
                lockcorner = OB_CORNER_TOPRIGHT;
            } else if (corner == prop_atoms.net_wm_moveresize_size_keyboard) {
                cur_x = start_cw + (e->xmotion.x_root - start_x);
                cur_y = start_ch + (e->xmotion.y_root - start_y);
                lockcorner = OB_CORNER_TOPLEFT;
            } else
                g_assert_not_reached();

            do_resize();
        }
    } else if (e->type == KeyPress) {
        if (e->xkey.keycode == ob_keycode(OB_KEY_ESCAPE))
            moveresize_end(TRUE);
        else if (e->xkey.keycode == ob_keycode(OB_KEY_RETURN))
            moveresize_end(FALSE);
        else {
            if (corner == prop_atoms.net_wm_moveresize_size_keyboard) {
                if (e->xkey.keycode == ob_keycode(OB_KEY_RIGHT))
                    cur_x += MAX(4, moveresize_client->size_inc.width);
                else if (e->xkey.keycode == ob_keycode(OB_KEY_LEFT))
                    cur_x -= MAX(4, moveresize_client->size_inc.width);
                else if (e->xkey.keycode == ob_keycode(OB_KEY_DOWN))
                    cur_y += MAX(4, moveresize_client->size_inc.height);
                else if (e->xkey.keycode == ob_keycode(OB_KEY_UP))
                    cur_y -= MAX(4, moveresize_client->size_inc.height);
                else
                    return;
                do_resize();
            } else if (corner == prop_atoms.net_wm_moveresize_move_keyboard) {
                if (e->xkey.keycode == ob_keycode(OB_KEY_RIGHT))
                    cur_x += 4;
                else if (e->xkey.keycode == ob_keycode(OB_KEY_LEFT))
                    cur_x -= 4;
                else if (e->xkey.keycode == ob_keycode(OB_KEY_DOWN))
                    cur_y += 4;
                else if (e->xkey.keycode == ob_keycode(OB_KEY_UP))
                    cur_y -= 4;
                else
                    return;
                do_move();
            }
        }
    }
}
