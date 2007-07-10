/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   moveresize.c for the Openbox window manager
   Copyright (c) 2006        Mikael Magnusson
   Copyright (c) 2003-2007   Dana Jansens

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
#include "mainloop.h"
#include "modkeys.h"
#include "popup.h"
#include "moveresize.h"
#include "config.h"
#include "event.h"
#include "debug.h"
#include "extensions.h"
#include "render/render.h"
#include "render/theme.h"

#include <X11/Xlib.h>
#include <glib.h>

/* how far windows move and resize with the keyboard arrows */
#define KEY_DIST 8

gboolean moveresize_in_progress = FALSE;
ObClient *moveresize_client = NULL;
#ifdef SYNC
XSyncAlarm moveresize_alarm = None;
#endif

static gboolean moving = FALSE; /* TRUE - moving, FALSE - resizing */

static gint start_x, start_y, start_cx, start_cy, start_cw, start_ch;
static gint cur_x, cur_y, cur_w, cur_h;
static guint button;
static guint32 corner;
static ObCorner lockcorner;
static ObDirection edge_warp_dir = -1;
static ObDirection key_resize_edge = -1;
#ifdef SYNC
static gboolean waiting_for_sync;
#endif

static ObPopup *popup = NULL;

static void do_edge_warp(gint x, gint y);
static void cancel_edge_warp();
#ifdef SYNC
static gboolean sync_timeout_func(gpointer data);
#endif

static void client_dest(ObClient *client, gpointer data)
{
    if (moveresize_client == client)
        moveresize_end(TRUE);    
}

void moveresize_startup(gboolean reconfig)
{
    popup = popup_new(FALSE);
    popup_set_text_align(popup, RR_JUSTIFY_CENTER);

    if (!reconfig)
        client_add_destroy_notify(client_dest, NULL);
}

void moveresize_shutdown(gboolean reconfig)
{
    if (!reconfig) {
        if (moveresize_in_progress)
            moveresize_end(FALSE);
        client_remove_destroy_notify(client_dest);
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
                       c->frame->area.y - ob_rr_theme->fbwidth);
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
    gboolean mv = (cnr == prop_atoms.net_wm_moveresize_move ||
                   cnr == prop_atoms.net_wm_moveresize_move_keyboard);

    if (moveresize_in_progress || !c->frame->visible ||
        !(mv ?
          (c->functions & OB_CLIENT_FUNC_MOVE) :
          (c->functions & OB_CLIENT_FUNC_RESIZE)))
        return;

    if (cnr == prop_atoms.net_wm_moveresize_size_topleft)
        cur = OB_CURSOR_NORTHWEST;
    else if (cnr == prop_atoms.net_wm_moveresize_size_top)
        cur = OB_CURSOR_NORTH;
    else if (cnr == prop_atoms.net_wm_moveresize_size_topright)
        cur = OB_CURSOR_NORTHEAST;
    else if (cnr == prop_atoms.net_wm_moveresize_size_right)
        cur = OB_CURSOR_EAST;
    else if (cnr == prop_atoms.net_wm_moveresize_size_bottomright)
        cur = OB_CURSOR_SOUTHEAST;
    else if (cnr == prop_atoms.net_wm_moveresize_size_bottom)
        cur = OB_CURSOR_SOUTH;
    else if (cnr == prop_atoms.net_wm_moveresize_size_bottomleft)
        cur = OB_CURSOR_SOUTHWEST;
    else if (cnr == prop_atoms.net_wm_moveresize_size_left)
        cur = OB_CURSOR_WEST;
    else if (cnr == prop_atoms.net_wm_moveresize_size_keyboard)
        cur = OB_CURSOR_SOUTHEAST;
    else if (cnr == prop_atoms.net_wm_moveresize_move)
        cur = OB_CURSOR_MOVE;
    else if (cnr == prop_atoms.net_wm_moveresize_move_keyboard)
        cur = OB_CURSOR_MOVE;
    else
        g_assert_not_reached();

    /* keep the pointer bounded to the screen for move/resize */
    if (!grab_pointer(FALSE, TRUE, cur))
        return;
    if (!grab_keyboard()) {
        ungrab_pointer();
        return;
    }

    frame_end_iconify_animation(c->frame);

    moving = mv;
    moveresize_client = c;
    start_cx = c->area.x;
    start_cy = c->area.y;
    /* these adjustments for the size_inc make resizing a terminal more
       friendly. you essentially start the resize in the middle of the
       increment instead of at 0, so you have to move half an increment
       either way instead of a full increment one and 1 px the other. */
    start_cw = c->area.width + c->size_inc.width / 2;
    start_ch = c->area.height + c->size_inc.height / 2;
    start_x = x;
    start_y = y;
    corner = cnr;
    button = b;
    key_resize_edge = -1;

    /*
      have to change start_cx and start_cy if going to do this..
    if (corner == prop_atoms.net_wm_moveresize_move_keyboard ||
        corner == prop_atoms.net_wm_moveresize_size_keyboard)
        XWarpPointer(ob_display, None, c->window, 0, 0, 0, 0,
                     c->area.width / 2, c->area.height / 2);
    */

    cur_x = start_cx;
    cur_y = start_cy;
    cur_w = start_cw;
    cur_h = start_ch;

    moveresize_in_progress = TRUE;

#ifdef SYNC
    if (config_resize_redraw && !moving && extensions_shape &&
        moveresize_client->sync_request && moveresize_client->sync_counter)
    {
        /* Initialize values for the resize syncing, and create an alarm for
           the client's xsync counter */

        XSyncValue val;
        XSyncAlarmAttributes aa;

        /* set the counter to an initial value */
        XSyncIntToValue(&val, 0);
        XSyncSetCounter(ob_display, moveresize_client->sync_counter, val);

        /* this will be incremented when we tell the client what we're
           looking for */
        moveresize_client->sync_counter_value = 0;

        /* the next sequence we're waiting for with the alarm */
        XSyncIntToValue(&val, 1);

        /* set an alarm on the counter */
        aa.trigger.counter = moveresize_client->sync_counter;
        aa.trigger.wait_value = val;
        aa.trigger.value_type = XSyncAbsolute;
        aa.trigger.test_type = XSyncPositiveTransition;
        aa.events = True;
        XSyncIntToValue(&aa.delta, 1);
        moveresize_alarm = XSyncCreateAlarm(ob_display,
                                            XSyncCACounter |
                                            XSyncCAValue |
                                            XSyncCAValueType |
                                            XSyncCATestType |
                                            XSyncCADelta |
                                            XSyncCAEvents,
                                            &aa);

        waiting_for_sync = FALSE;
    }
#endif
}

void moveresize_end(gboolean cancel)
{
    ungrab_keyboard();
    ungrab_pointer();

    popup_hide(popup);

    if (moving) {
        client_move(moveresize_client,
                    (cancel ? start_cx : cur_x),
                    (cancel ? start_cy : cur_y));
    } else {
#ifdef SYNC
        /* turn off the alarm */
        if (moveresize_alarm != None) {
            XSyncDestroyAlarm(ob_display, moveresize_alarm);
            moveresize_alarm = None;
        }

        ob_main_loop_timeout_remove(ob_main_loop, sync_timeout_func);
#endif

        client_configure(moveresize_client,
                         (cancel ? start_cx : cur_x),
                         (cancel ? start_cy : cur_y),
                         (cancel ? start_cw : cur_w),
                         (cancel ? start_ch : cur_h),
                         TRUE, TRUE, FALSE);
    }

    /* dont edge warp after its ended */
    cancel_edge_warp();

    moveresize_in_progress = FALSE;
    moveresize_client = NULL;
}

static void do_move(gboolean keyboard, gint keydist)
{
    gint resist;

    if (keyboard) resist = keydist - 1; /* resist for one key press */
    else resist = config_resist_win;
    resist_move_windows(moveresize_client, resist, &cur_x, &cur_y);
    if (!keyboard) resist = config_resist_edge;
    resist_move_monitors(moveresize_client, resist, &cur_x, &cur_y);

    client_configure(moveresize_client, cur_x, cur_y, cur_w, cur_h,
                     TRUE, FALSE, FALSE);
    if (config_resize_popup_show == 2) /* == "Always" */
        popup_coords(moveresize_client, "%d x %d",
                     moveresize_client->frame->area.x,
                     moveresize_client->frame->area.y);
}


static void do_resize()
{
    gint x, y, w, h, lw, lh;

    /* see if it is actually going to resize */
    x = 0;
    y = 0;
    w = cur_w;
    h = cur_h;
    client_try_configure(moveresize_client, &x, &y, &w, &h,
                         &lw, &lh, TRUE);
    if (w == moveresize_client->area.width &&
        h == moveresize_client->area.height)
    {
        return;
    }

#ifdef SYNC
    if (config_resize_redraw && extensions_sync &&
        moveresize_client->sync_request && moveresize_client->sync_counter)
    {
        XEvent ce;
        XSyncValue val;

        /* are we already waiting for the sync counter to catch up? */
        if (waiting_for_sync)
            return;

        /* increment the value we're waiting for */
        ++moveresize_client->sync_counter_value;
        XSyncIntToValue(&val, moveresize_client->sync_counter_value);

        /* tell the client what we're waiting for */
        ce.xclient.type = ClientMessage;
        ce.xclient.message_type = prop_atoms.wm_protocols;
        ce.xclient.display = ob_display;
        ce.xclient.window = moveresize_client->window;
        ce.xclient.format = 32;
        ce.xclient.data.l[0] = prop_atoms.net_wm_sync_request;
        ce.xclient.data.l[1] = event_curtime;
        ce.xclient.data.l[2] = XSyncValueLow32(val);
        ce.xclient.data.l[3] = XSyncValueHigh32(val);
        ce.xclient.data.l[4] = 0l;
        XSendEvent(ob_display, moveresize_client->window, FALSE,
                   NoEventMask, &ce);

        waiting_for_sync = TRUE;

        ob_main_loop_timeout_remove(ob_main_loop, sync_timeout_func);
        ob_main_loop_timeout_add(ob_main_loop, G_USEC_PER_SEC / 2,
                                 sync_timeout_func,
                                 NULL, NULL, NULL);
    }
#endif

    client_configure(moveresize_client, cur_x, cur_y, cur_w, cur_h,
                     TRUE, FALSE, FALSE);

    /* this would be better with a fixed width font ... XXX can do it better
       if there are 2 text boxes */
    if (config_resize_popup_show == 2 || /* == "Always" */
            (config_resize_popup_show == 1 && /* == "Nonpixel" */
             moveresize_client->size_inc.width > 1 &&
             moveresize_client->size_inc.height > 1))
        popup_coords(moveresize_client, "%d x %d",
                     moveresize_client->logical_size.width,
                     moveresize_client->logical_size.height);
}

#ifdef SYNC
static gboolean sync_timeout_func(gpointer data)
{
    waiting_for_sync = FALSE; /* we timed out waiting for our sync... */
    do_resize(); /* ...so let any pending resizes through */

    return FALSE; /* don't repeat */
}
#endif

static void calc_resize(gboolean keyboard, gint keydist, gint *dw, gint *dh,
                        ObCorner cor)
{
    gint resist, ow, oh, nw, nh;

    /* resist_size_* needs the frame size */
    ow = cur_w +
        moveresize_client->frame->size.left +
        moveresize_client->frame->size.right;
    oh = cur_h +
        moveresize_client->frame->size.top +
        moveresize_client->frame->size.bottom;
    nw = ow + *dw;
    nh = oh + *dh;

    if (keyboard) resist = keydist - 1; /* resist for one key press */
    else resist = config_resist_win;
    resist_size_windows(moveresize_client, resist, &nw, &nh, cor);
    if (!keyboard) resist = config_resist_edge;
    resist_size_monitors(moveresize_client, resist, &nw, &nh, cor);

    *dw = nw - ow;
    *dh = nh - oh;
}

static gboolean edge_warp_delay_func(gpointer data)
{
    guint d;

    d = screen_find_desktop(screen_desktop, edge_warp_dir, TRUE, FALSE);
    if (d != screen_desktop) screen_set_desktop(d, TRUE);

    edge_warp_dir = -1;

    return FALSE; /* don't repeat */
}

static void do_edge_warp(gint x, gint y)
{
    guint i;
    ObDirection dir;

    if (!config_mouse_screenedgetime) return;

    dir = -1;

    for (i = 0; i < screen_num_monitors; ++i) {
        Rect *a = screen_physical_area_monitor(i);
        if (x == RECT_LEFT(*a)) dir = OB_DIRECTION_WEST;
        if (x == RECT_RIGHT(*a)) dir = OB_DIRECTION_EAST;
        if (y == RECT_TOP(*a)) dir = OB_DIRECTION_NORTH;
        if (y == RECT_BOTTOM(*a)) dir = OB_DIRECTION_SOUTH;

        /* try check for xinerama boundaries */
        if ((x + 1 == RECT_LEFT(*a) || x - 1 == RECT_RIGHT(*a)) &&
            (dir == OB_DIRECTION_WEST || dir == OB_DIRECTION_EAST))
        {
            dir = -1;
        }
        if ((y + 1 == RECT_TOP(*a) || y - 1 == RECT_BOTTOM(*a)) &&
            (dir == OB_DIRECTION_NORTH || dir == OB_DIRECTION_SOUTH))
        {
            dir = -1;
        }
        g_free(a);
    }

    if (dir != edge_warp_dir) {
        if (dir == (ObDirection)-1)
            cancel_edge_warp();
        else
            ob_main_loop_timeout_add(ob_main_loop,
                                     config_mouse_screenedgetime * 1000,
                                     edge_warp_delay_func,
                                     NULL, NULL, NULL);
        edge_warp_dir = dir;
    }
}

static void cancel_edge_warp()
{
    ob_main_loop_timeout_remove(ob_main_loop, edge_warp_delay_func);
}

static void move_with_keys(gint keycode, gint state)
{
    gint dx = 0, dy = 0, ox = cur_x, oy = cur_y;
    gint opx, px, opy, py;
    gint dist = 0;

    /* shift means jump to edge */
    if (state & modkeys_key_to_mask(OB_MODKEY_KEY_SHIFT)) {
        gint x, y;
        ObDirection dir;

        if (keycode == ob_keycode(OB_KEY_RIGHT))
            dir = OB_DIRECTION_EAST;
        else if (keycode == ob_keycode(OB_KEY_LEFT))
            dir = OB_DIRECTION_WEST;
        else if (keycode == ob_keycode(OB_KEY_DOWN))
            dir = OB_DIRECTION_SOUTH;
        else /* if (keycode == ob_keycode(OB_KEY_UP)) */
            dir = OB_DIRECTION_NORTH;

        client_find_move_directional(moveresize_client, dir, &x, &y);
        dx = x - moveresize_client->area.x;
        dy = y - moveresize_client->area.y;
    } else {
        /* control means fine grained */
        if (state & modkeys_key_to_mask(OB_MODKEY_KEY_CONTROL))
            dist = 1;
        else
            dist = KEY_DIST;

        if (keycode == ob_keycode(OB_KEY_RIGHT))
            dx = dist;
        else if (keycode == ob_keycode(OB_KEY_LEFT))
            dx = -dist;
        else if (keycode == ob_keycode(OB_KEY_DOWN))
            dy = dist;
        else /* if (keycode == ob_keycode(OB_KEY_UP)) */
            dy = -dist;
    }

    screen_pointer_pos(&opx, &opy);
    XWarpPointer(ob_display, None, None, 0, 0, 0, 0, dx, dy);
    /* steal the motion events this causes */
    XSync(ob_display, FALSE);
    {
        XEvent ce;
        while (XCheckTypedEvent(ob_display, MotionNotify, &ce));
    }
    screen_pointer_pos(&px, &py);

    cur_x += dx;
    cur_y += dy;
    do_move(TRUE, dist);

    /* because the cursor moves even though the window does
       not nessesarily (resistance), this adjusts where the curor
       thinks it started so that it keeps up with where the window
       actually is */
    start_x += (px - opx) - (cur_x - ox);
    start_y += (py - opy) - (cur_y - oy);
}

static void resize_with_keys(gint keycode, gint state)
{
    gint dw = 0, dh = 0, pdx = 0, pdy = 0, opx, opy, px, py;
    gint dist = 0;
    ObDirection dir;
    ObCorner cor;

    /* pick the edge if it needs to move */
    if (keycode == ob_keycode(OB_KEY_RIGHT)) {
        dir = OB_DIRECTION_EAST;
        if (key_resize_edge != OB_DIRECTION_WEST &&
            key_resize_edge != OB_DIRECTION_EAST)
        {
            key_resize_edge = OB_DIRECTION_EAST;
            return;
        }
    }
    if (keycode == ob_keycode(OB_KEY_LEFT)) {
        dir = OB_DIRECTION_WEST;
        if (key_resize_edge != OB_DIRECTION_WEST &&
            key_resize_edge != OB_DIRECTION_EAST)
        {
            key_resize_edge = OB_DIRECTION_WEST;
            return;
        }
    }
    if (keycode == ob_keycode(OB_KEY_UP)) {
        dir = OB_DIRECTION_NORTH;
        if (key_resize_edge != OB_DIRECTION_NORTH &&
            key_resize_edge != OB_DIRECTION_SOUTH)
        {
            key_resize_edge = OB_DIRECTION_NORTH;
            return;
        }
    }
    if (keycode == ob_keycode(OB_KEY_DOWN)) {
        dir = OB_DIRECTION_SOUTH;
        if (key_resize_edge != OB_DIRECTION_NORTH &&
            key_resize_edge != OB_DIRECTION_SOUTH)
        {
            key_resize_edge = OB_DIRECTION_SOUTH;
            return;
        }
    }

    /* shift means jump to edge */
    if (state & modkeys_key_to_mask(OB_MODKEY_KEY_SHIFT)) {
        gint x, y, w, h;

        if (keycode == ob_keycode(OB_KEY_RIGHT))
            dir = OB_DIRECTION_EAST;
        else if (keycode == ob_keycode(OB_KEY_LEFT))
            dir = OB_DIRECTION_WEST;
        else if (keycode == ob_keycode(OB_KEY_DOWN))
            dir = OB_DIRECTION_SOUTH;
        else /* if (keycode == ob_keycode(OB_KEY_UP)) */
            dir = OB_DIRECTION_NORTH;

        client_find_resize_directional(moveresize_client, key_resize_edge,
                                       key_resize_edge == dir,
                                       &x, &y, &w, &h);
        dw = w - moveresize_client->area.width;
        dh = h - moveresize_client->area.height;
    } else {
        gint distw, disth;

        /* control means fine grained */
        if (moveresize_client->size_inc.width > 1)
            distw = moveresize_client->size_inc.width;
        else if (state & modkeys_key_to_mask(OB_MODKEY_KEY_CONTROL))
            distw = 1;
        else
            distw = KEY_DIST;
        if (moveresize_client->size_inc.height > 1)
            disth = moveresize_client->size_inc.height;
        else if (state & modkeys_key_to_mask(OB_MODKEY_KEY_CONTROL))
            disth = 1;
        else
            disth = KEY_DIST;

        if (key_resize_edge == OB_DIRECTION_WEST) {
            if (dir == OB_DIRECTION_WEST)
                dw = (dist = distw);
            else
                dw = -(dist = distw);
        }
        else if (key_resize_edge == OB_DIRECTION_EAST) {
            if (dir == OB_DIRECTION_EAST)
                dw = (dist = distw);
            else
                dw = -(dist = distw);
        }
        else if (key_resize_edge == OB_DIRECTION_NORTH) {
            if (dir == OB_DIRECTION_NORTH)
                dh = (dist = disth);
            else
                dh = -(dist = disth);
        }
        else /*if (key_resize_edge == OB_DIRECTION_SOUTH)*/ {
            if (dir == OB_DIRECTION_SOUTH)
                dh = (dist = disth);
            else
                dh = -(dist = disth);
        }
    }

    /* which corner is locked, for resistance */
    if (key_resize_edge == OB_DIRECTION_WEST)
        cor = OB_CORNER_TOPRIGHT;
    else if (key_resize_edge == OB_DIRECTION_EAST)
        cor = OB_CORNER_TOPLEFT;
    else if (key_resize_edge == OB_DIRECTION_NORTH)
        cor = OB_CORNER_BOTTOMLEFT;
    else if (key_resize_edge == OB_DIRECTION_SOUTH)
        cor = OB_CORNER_TOPLEFT;

    calc_resize(TRUE, dist, &dw, &dh, cor);
    if (key_resize_edge == OB_DIRECTION_WEST)
        cur_x -= dw;
    else if (key_resize_edge == OB_DIRECTION_NORTH)
        cur_y -= dh;
    cur_w += dw;
    cur_h += dh;

    /* how to move the pointer to keep up with the change */
    if (key_resize_edge == OB_DIRECTION_WEST)
        pdx = -dw;
    else if (key_resize_edge == OB_DIRECTION_EAST)
        pdx = dw;
    else if (key_resize_edge == OB_DIRECTION_NORTH)
        pdy = -dh;
    else if (key_resize_edge == OB_DIRECTION_SOUTH)
        pdy = dh;
    
    screen_pointer_pos(&opx, &opy);
    XWarpPointer(ob_display, None, None, 0, 0, 0, 0, pdx, pdy);
    /* steal the motion events this causes */
    XSync(ob_display, FALSE);
    {
        XEvent ce;
        while (XCheckTypedEvent(ob_display, MotionNotify, &ce));
    }
    screen_pointer_pos(&px, &py);

    do_resize();

    /* because the cursor moves even though the window does
       not nessesarily (resistance), this adjusts where the cursor
       thinks it started so that it keeps up with where the window
       actually is */
    start_x += (px - opx) - dw;
    start_y += (py - opy) - dh;

}

gboolean moveresize_event(XEvent *e)
{
    gboolean used = FALSE;

    if (!moveresize_in_progress) return FALSE;

    if (e->type == ButtonPress) {
        if (!button) {
            start_x = e->xbutton.x_root;
            start_y = e->xbutton.y_root;
            button = e->xbutton.button; /* this will end it now */
        }
        used = e->xbutton.button == button;
    } else if (e->type == ButtonRelease) {
        if (!button || e->xbutton.button == button) {
            moveresize_end(FALSE);
            used = TRUE;
        }
    } else if (e->type == MotionNotify) {
        if (moving) {
            cur_x = start_cx + e->xmotion.x_root - start_x;
            cur_y = start_cy + e->xmotion.y_root - start_y;
            do_move(FALSE, 0);
            do_edge_warp(e->xmotion.x_root, e->xmotion.y_root);
        } else {
            gint dw, dh;

            if (corner == prop_atoms.net_wm_moveresize_size_topleft) {
                dw = -(e->xmotion.x_root - start_x);
                dh = -(e->xmotion.y_root - start_y);
                lockcorner = OB_CORNER_BOTTOMRIGHT;
            } else if (corner == prop_atoms.net_wm_moveresize_size_top) {
                dw = 0;
                dh = (e->xmotion.y_root - start_y);
                lockcorner = OB_CORNER_BOTTOMRIGHT;
            } else if (corner == prop_atoms.net_wm_moveresize_size_topright) {
                dw = (e->xmotion.x_root - start_x);
                dh = -(e->xmotion.y_root - start_y);
                lockcorner = OB_CORNER_BOTTOMLEFT;
            } else if (corner == prop_atoms.net_wm_moveresize_size_right) { 
                dw = (e->xmotion.x_root - start_x);
                dh = 0;
                lockcorner = OB_CORNER_BOTTOMLEFT;
            } else if (corner ==
                       prop_atoms.net_wm_moveresize_size_bottomright) {
                dw = (e->xmotion.x_root - start_x);
                dh = (e->xmotion.y_root - start_y);
                lockcorner = OB_CORNER_TOPLEFT;
            } else if (corner == prop_atoms.net_wm_moveresize_size_bottom) {
                dw = 0;
                dh = (e->xmotion.y_root - start_y);
                lockcorner = OB_CORNER_TOPLEFT;
            } else if (corner ==
                       prop_atoms.net_wm_moveresize_size_bottomleft) {
                dw = -(e->xmotion.x_root - start_x);
                dh = (e->xmotion.y_root - start_y);
                lockcorner = OB_CORNER_TOPRIGHT;
            } else if (corner == prop_atoms.net_wm_moveresize_size_left) {
                dw = -(e->xmotion.x_root - start_x);
                dh = 0;
                lockcorner = OB_CORNER_TOPRIGHT;
            } else if (corner == prop_atoms.net_wm_moveresize_size_keyboard) {
                dw = (e->xmotion.x_root - start_x);
                dh = (e->xmotion.y_root - start_y);
                lockcorner = OB_CORNER_TOPLEFT;
            } else
                g_assert_not_reached();

            dw -= cur_w - start_cw;
            dh -= cur_h - start_ch;

            calc_resize(FALSE, 0, &dw, &dh, lockcorner);
            cur_w += dw;
            cur_h += dh;

            if (corner == prop_atoms.net_wm_moveresize_size_topleft ||
                corner == prop_atoms.net_wm_moveresize_size_left ||
                corner == prop_atoms.net_wm_moveresize_size_bottomleft)
            {
                cur_x -= dw;
            }
            if (corner == prop_atoms.net_wm_moveresize_size_topleft ||
                corner == prop_atoms.net_wm_moveresize_size_top ||
                corner == prop_atoms.net_wm_moveresize_size_topright)
            {
                cur_y -= dh;
            }

            do_resize();
        }
        used = TRUE;
    } else if (e->type == KeyPress) {
        if (e->xkey.keycode == ob_keycode(OB_KEY_ESCAPE)) {
            moveresize_end(TRUE);
            used = TRUE;
        } else if (e->xkey.keycode == ob_keycode(OB_KEY_RETURN)) {
            moveresize_end(FALSE);
            used = TRUE;
        } else if (e->xkey.keycode == ob_keycode(OB_KEY_RIGHT) ||
                   e->xkey.keycode == ob_keycode(OB_KEY_LEFT) ||
                   e->xkey.keycode == ob_keycode(OB_KEY_DOWN) ||
                   e->xkey.keycode == ob_keycode(OB_KEY_UP))
        {
            if (corner == prop_atoms.net_wm_moveresize_size_keyboard) {
                resize_with_keys(e->xkey.keycode, e->xkey.state);
                used = TRUE;
            } else if (corner == prop_atoms.net_wm_moveresize_move_keyboard) {
                move_with_keys(e->xkey.keycode, e->xkey.state);
                used = TRUE;
            }
        }
    }
#ifdef SYNC
    else if (e->type == extensions_sync_event_basep + XSyncAlarmNotify)
    {
        waiting_for_sync = FALSE; /* we got our sync... */
        do_resize(); /* ...so try resize if there is more change pending */
        used = TRUE;
    }
#endif
    return used;
}
