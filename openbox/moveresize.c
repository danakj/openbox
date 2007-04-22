/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   moveresize.c for the Openbox window manager
   Copyright (c) 2006        Mikael Magnusson
   Copyright (c) 2003        Ben Jansens

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   See the COPYING file for a copy of the GNU General Public License.
*/

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

static gint start_x, start_y, start_cx, start_cy, start_cw, start_ch;
static gint cur_x, cur_y;
static guint button;
static guint32 corner;
static ObCorner lockcorner;

static ObPopup *popup = NULL;

static void client_dest(ObClient *client, gpointer data)
{
    if (moveresize_client == client)
        moveresize_end(TRUE);    
}

void moveresize_startup(gboolean reconfig)
{
    popup = popup_new(FALSE);

    if (!reconfig)
        client_add_destructor(client_dest, NULL);
}

void moveresize_shutdown(gboolean reconfig)
{
    if (!reconfig) {
        if (moveresize_in_progress)
            moveresize_end(FALSE);
        client_remove_destructor(client_dest);
    }

    popup_free(popup);
    popup = NULL;
}

static void popup_coords(ObClient *c, const gchar *format, gint a, gint b)
{
    gchar *text;

    text = g_strdup_printf(format, a, b);
    if (config_resize_popup_pos == 1) /* == "Top" */
        popup_position(popup, SouthGravity,
                       c->frame->area.x
                     + c->frame->area.width/2,
                       c->frame->area.y);
    else /* == "Center" */
        popup_position(popup, CenterGravity,
                       c->frame->area.x + c->frame->size.left +
                       c->area.width / 2,
                       c->frame->area.y + c->frame->size.top +
                       c->area.height / 2);
    popup_show(popup, text);
    g_free(text);
}

void moveresize_start(ObClient *c, gint x, gint y, guint b, guint32 cnr)
{
    ObCursor cur;

    moving = (cnr == prop_atoms.net_wm_moveresize_move ||
              cnr == prop_atoms.net_wm_moveresize_move_keyboard);

    if (moveresize_in_progress || !c->frame->visible ||
        !(moving ?
          (c->functions & OB_CLIENT_FUNC_MOVE) :
          (c->functions & OB_CLIENT_FUNC_RESIZE)))
        return;

    moveresize_client = c;
    start_cx = c->frame->area.x;
    start_cy = c->frame->area.y;
    /* these adjustments for the size_inc make resizing a terminal more
       friendly. you essentially start the resize in the middle of the
       increment instead of at 0, so you have to move half an increment
       either way instead of a full increment one and 1 px the other. and this
       is one large mother fucking comment. */
    start_cw = c->area.width + c->size_inc.width / 2;
    start_ch = c->area.height + c->size_inc.height / 2;
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

    if (moving) {
        cur_x = start_cx;
        cur_y = start_cy;
    } else {
        cur_x = start_cw;
        cur_y = start_ch;
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

    grab_pointer(TRUE, FALSE, cur);
    grab_keyboard(TRUE);
}

void moveresize_end(gboolean cancel)
{
    grab_keyboard(FALSE);
    grab_pointer(FALSE, FALSE, OB_CURSOR_NONE);

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
    if (resist) {
        resist_move_windows(moveresize_client, &cur_x, &cur_y);
        resist_move_monitors(moveresize_client, &cur_x, &cur_y);
    }

    /* get where the client should be */
    frame_frame_gravity(moveresize_client->frame, &cur_x, &cur_y);
    client_configure(moveresize_client, OB_CORNER_TOPLEFT, cur_x, cur_y,
                     moveresize_client->area.width,
                     moveresize_client->area.height, TRUE, FALSE);
    if (config_resize_popup_show == 2) /* == "Always" */
        popup_coords(moveresize_client, "%d x %d",
                moveresize_client->frame->area.x,
                moveresize_client->frame->area.y);
}

static void do_resize(gboolean resist)
{
    /* resist_size_* needs the frame size */
    cur_x += moveresize_client->frame->size.left +
        moveresize_client->frame->size.right;
    cur_y += moveresize_client->frame->size.top +
        moveresize_client->frame->size.bottom;

    if (resist) {
        resist_size_windows(moveresize_client, &cur_x, &cur_y, lockcorner);
        resist_size_monitors(moveresize_client, &cur_x, &cur_y, lockcorner);
    }

    cur_x -= moveresize_client->frame->size.left +
        moveresize_client->frame->size.right;
    cur_y -= moveresize_client->frame->size.top +
        moveresize_client->frame->size.bottom;
 
    client_configure(moveresize_client, lockcorner, 
                     moveresize_client->area.x, moveresize_client->area.y,
                     cur_x, cur_y, TRUE, FALSE);

    /* this would be better with a fixed width font ... XXX can do it better
       if there are 2 text boxes */
    if (config_resize_popup_show == 2 || /* == "Always" */
            (config_resize_popup_show == 1 && /* == "Nonpixel" */
                (moveresize_client->size_inc.width > 1 ||
                 moveresize_client->size_inc.height > 1))
        )
        popup_coords(moveresize_client, "%d x %d",
                     moveresize_client->logical_size.width,
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
                gint dx = 0, dy = 0, ox = cur_x, oy = cur_y;

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
                /* steal the motion events this causes */
                XSync(ob_display, FALSE);
                {
                    XEvent ce;
                    while (XCheckTypedEvent(ob_display, MotionNotify, &ce));
                }

                do_resize(FALSE);

                /* because the cursor moves even though the window does
                   not nessesarily (resistance), this adjusts where the curor
                   thinks it started so that it keeps up with where the window
                   actually is */
                start_x += dx - (cur_x - ox);
                start_y += dy - (cur_y - oy);
            } else if (corner == prop_atoms.net_wm_moveresize_move_keyboard) {
                gint dx = 0, dy = 0, ox = cur_x, oy = cur_y;
                gint opx, px, opy, py;

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
                screen_pointer_pos(&opx, &opy);
                XWarpPointer(ob_display, None, None, 0, 0, 0, 0, dx, dy);
                /* steal the motion events this causes */
                XSync(ob_display, FALSE);
                {
                    XEvent ce;
                    while (XCheckTypedEvent(ob_display, MotionNotify, &ce));
                }
                screen_pointer_pos(&px, &py);

                do_move(FALSE);

                /* because the cursor moves even though the window does
                   not nessesarily (resistance), this adjusts where the curor
                   thinks it started so that it keeps up with where the window
                   actually is */
                start_x += (px - opx) - (cur_x - ox);
                start_y += (py - opy) - (cur_y - oy);
            }
        }
    }
}
