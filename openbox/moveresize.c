#include "grab.h"
#include "framerender.h"
#include "screen.h"
#include "prop.h"
#include "client.h"
#include "frame.h"
#include "openbox.h"
#include "resist.h"
#include "popup.h"
#include "moveresize.h"
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

#define POPUP_X (10)
#define POPUP_Y (10)

static void client_dest(ObClient *c)
{
    if (moveresize_client == c)
        moveresize_end(TRUE);    
}

void moveresize_startup()
{
    XSetWindowAttributes attrib;

    popup = popup_new(FALSE);
    popup_size_to_string(popup, "W:  0000  W:  0000");

    attrib.save_under = True;

    client_add_destructor(client_dest);
}

void moveresize_shutdown()
{
    client_remove_destructor(client_dest);

    popup_free(popup);
    popup = NULL;
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

    g_assert(!moveresize_in_progress);

    moveresize_client = c;
    start_cx = c->frame->area.x;
    start_cy = c->frame->area.y;
    start_cw = c->area.width;
    start_ch = c->area.height;
    start_x = x;
    start_y = y;
    corner = cnr;
    button = b;

    /*
      have to change start_cx and start_cy if going to do this..
    if (corner == prop_atoms.net_wm_moveresize_move_keyboard ||
        corner == prop_atoms.net_wm_moveresize_size_keyboard)
        XWarpPointer(ob_display, None, c->window, 0, 0, 0, 0,
                     c->area.width / 2, c->area.height / 2);
    */

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
}

void moveresize_end(gboolean cancel)
{
    grab_keyboard(FALSE);
    grab_pointer(FALSE, None);

    popup_hide(popup);

    if (moving) {
        client_move(moveresize_client,
                    (cancel ? start_cx : cur_x),
                    (cancel ? start_cy : cur_y));
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

static void do_move(gboolean resist)
{
    Rect *a;

    if (resist)
        resist_move(moveresize_client, &cur_x, &cur_y);

    /* get where the client should be */
    frame_frame_gravity(moveresize_client->frame, &cur_x, &cur_y);
    client_move(moveresize_client, cur_x, cur_y);

    /* this would be better with a fixed width font ... XXX can do it better
       if there are 2 text boxes */
    a = screen_area(screen_desktop);
    popup_coords("X:  %4d  Y:  %4d",
                 moveresize_client->frame->area.x - a->x,
                 moveresize_client->frame->area.y - a->y);
}

static void do_resize(gboolean resist)
{
    if (resist) {
        /* resist_size needs the frame size */
        cur_x += moveresize_client->frame->size.left +
            moveresize_client->frame->size.right;
        cur_y += moveresize_client->frame->size.top +
            moveresize_client->frame->size.bottom;

        resist_size(moveresize_client, &cur_x, &cur_y, lockcorner);

        cur_x -= moveresize_client->frame->size.left +
            moveresize_client->frame->size.right;
        cur_y -= moveresize_client->frame->size.top +
            moveresize_client->frame->size.bottom;
    }
    
    client_configure(moveresize_client, lockcorner, 
                     moveresize_client->area.x, moveresize_client->area.y,
                     cur_x, cur_y, TRUE, FALSE);

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
            do_move(TRUE);
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

            do_resize(TRUE);
        }
    } else if (e->type == KeyPress) {
        if (e->xkey.keycode == ob_keycode(OB_KEY_ESCAPE))
            moveresize_end(TRUE);
        else if (e->xkey.keycode == ob_keycode(OB_KEY_RETURN))
            moveresize_end(FALSE);
        else {
            if (corner == prop_atoms.net_wm_moveresize_size_keyboard) {
                int dx = 0, dy = 0;

                if (e->xkey.keycode == ob_keycode(OB_KEY_RIGHT))
                    dx = MAX(4, moveresize_client->size_inc.width);
                else if (e->xkey.keycode == ob_keycode(OB_KEY_LEFT))
                    dx = -MAX(4, moveresize_client->size_inc.width);
                else if (e->xkey.keycode == ob_keycode(OB_KEY_DOWN))
                    dy = MAX(4, moveresize_client->size_inc.height);
                else if (e->xkey.keycode == ob_keycode(OB_KEY_UP))
                    dy = -MAX(4, moveresize_client->size_inc.height);
                else
                    return;

                cur_x += dx;
                cur_y += dy;
                XWarpPointer(ob_display, None, None, 0, 0, 0, 0, dx, dy);

                do_resize(FALSE);
            } else if (corner == prop_atoms.net_wm_moveresize_move_keyboard) {
                int dx = 0, dy = 0;

                if (e->xkey.keycode == ob_keycode(OB_KEY_RIGHT))
                    dx = 4;
                else if (e->xkey.keycode == ob_keycode(OB_KEY_LEFT))
                    dx = -4;
                else if (e->xkey.keycode == ob_keycode(OB_KEY_DOWN))
                    dy = 4;
                else if (e->xkey.keycode == ob_keycode(OB_KEY_UP))
                    dy = -4;
                else
                    return;

                cur_x += dx;
                cur_y += dy;
                XWarpPointer(ob_display, None, None, 0, 0, 0, 0, dx, dy);

                do_move(FALSE);
            }
        }
    }
}
