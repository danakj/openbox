/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   focus.c for the Openbox window manager
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

#include "debug.h"
#include "event.h"
#include "openbox.h"
#include "grab.h"
#include "framerender.h"
#include "client.h"
#include "config.h"
#include "frame.h"
#include "screen.h"
#include "group.h"
#include "prop.h"
#include "focus.h"
#include "stacking.h"
#include "popup.h"
#include "render/render.h"

#include <X11/Xlib.h>
#include <glib.h>
#include <assert.h>

ObClient *focus_client = NULL;
GList *focus_order = NULL;
ObClient *focus_cycle_target = NULL;

struct {
    InternalWindow top;
    InternalWindow left;
    InternalWindow right;
    InternalWindow bottom;
} focus_indicator;

RrAppearance *a_focus_indicator;
RrColor *color_white;

static ObIconPopup *focus_cycle_popup;

static void focus_cycle_destructor(ObClient *client, gpointer data)
{
    /* end cycling if the target disappears. CurrentTime is fine, time won't
       be used
    */
    if (focus_cycle_target == client)
        focus_cycle(TRUE, TRUE, TRUE, TRUE, TRUE, TRUE);
}

static Window createWindow(Window parent, gulong mask,
                           XSetWindowAttributes *attrib)
{
    return XCreateWindow(ob_display, parent, 0, 0, 1, 1, 0,
                         RrDepth(ob_rr_inst), InputOutput,
                         RrVisual(ob_rr_inst), mask, attrib);
                       
}

void focus_startup(gboolean reconfig)
{
    focus_cycle_popup = icon_popup_new(TRUE);

    if (!reconfig) {
        XSetWindowAttributes attr;

        client_add_destructor(focus_cycle_destructor, NULL);

        /* start with nothing focused */
        focus_nothing();

        focus_indicator.top.obwin.type = Window_Internal;
        focus_indicator.left.obwin.type = Window_Internal;
        focus_indicator.right.obwin.type = Window_Internal;
        focus_indicator.bottom.obwin.type = Window_Internal;

        attr.override_redirect = True;
        attr.background_pixel = BlackPixel(ob_display, ob_screen);
        focus_indicator.top.win =
            createWindow(RootWindow(ob_display, ob_screen),
                         CWOverrideRedirect | CWBackPixel, &attr);
        focus_indicator.left.win =
            createWindow(RootWindow(ob_display, ob_screen),
                         CWOverrideRedirect | CWBackPixel, &attr);
        focus_indicator.right.win =
            createWindow(RootWindow(ob_display, ob_screen),
                         CWOverrideRedirect | CWBackPixel, &attr);
        focus_indicator.bottom.win =
            createWindow(RootWindow(ob_display, ob_screen),
                         CWOverrideRedirect | CWBackPixel, &attr);

        stacking_add(INTERNAL_AS_WINDOW(&focus_indicator.top));
        stacking_add(INTERNAL_AS_WINDOW(&focus_indicator.left));
        stacking_add(INTERNAL_AS_WINDOW(&focus_indicator.right));
        stacking_add(INTERNAL_AS_WINDOW(&focus_indicator.bottom));

        color_white = RrColorNew(ob_rr_inst, 0xff, 0xff, 0xff);

        a_focus_indicator = RrAppearanceNew(ob_rr_inst, 4);
        a_focus_indicator->surface.grad = RR_SURFACE_SOLID;
        a_focus_indicator->surface.relief = RR_RELIEF_FLAT;
        a_focus_indicator->surface.primary = RrColorNew(ob_rr_inst,
                                                        0, 0, 0);
        a_focus_indicator->texture[0].type = RR_TEXTURE_LINE_ART;
        a_focus_indicator->texture[0].data.lineart.color = color_white;
        a_focus_indicator->texture[1].type = RR_TEXTURE_LINE_ART;
        a_focus_indicator->texture[1].data.lineart.color = color_white;
        a_focus_indicator->texture[2].type = RR_TEXTURE_LINE_ART;
        a_focus_indicator->texture[2].data.lineart.color = color_white;
        a_focus_indicator->texture[3].type = RR_TEXTURE_LINE_ART;
        a_focus_indicator->texture[3].data.lineart.color = color_white;
    }
}

void focus_shutdown(gboolean reconfig)
{
    icon_popup_free(focus_cycle_popup);

    if (!reconfig) {
        client_remove_destructor(focus_cycle_destructor);

        /* reset focus to root */
        XSetInputFocus(ob_display, PointerRoot, RevertToNone, CurrentTime);

        RrColorFree(color_white);

        RrAppearanceFree(a_focus_indicator);

        XDestroyWindow(ob_display, focus_indicator.top.win);
        XDestroyWindow(ob_display, focus_indicator.left.win);
        XDestroyWindow(ob_display, focus_indicator.right.win);
        XDestroyWindow(ob_display, focus_indicator.bottom.win);
    }
}

static void push_to_top(ObClient *client)
{
    focus_order = g_list_remove(focus_order, client);
    focus_order = g_list_prepend(focus_order, client);
}

void focus_set_client(ObClient *client)
{
    Window active;

    ob_debug_type(OB_DEBUG_FOCUS,
                  "focus_set_client 0x%lx\n", client ? client->window : 0);

    /* uninstall the old colormap, and install the new one */
    screen_install_colormap(focus_client, FALSE);
    screen_install_colormap(client, TRUE);

    /* in the middle of cycling..? kill it. CurrentTime is fine, time won't
       be used.
    */
    if (focus_cycle_target)
        focus_cycle(TRUE, TRUE, TRUE, TRUE, TRUE, TRUE);

    focus_client = client;

    if (client != NULL) {
        /* move to the top of the list */
        push_to_top(client);
        /* remove hiliting from the window when it gets focused */
        client_hilite(client, FALSE);
    }

    /* set the NET_ACTIVE_WINDOW hint, but preserve it on shutdown */
    if (ob_state() != OB_STATE_EXITING) {
        active = client ? client->window : None;
        PROP_SET32(RootWindow(ob_display, ob_screen),
                   net_active_window, window, active);
    }
}

ObClient* focus_fallback_target(gboolean allow_refocus, ObClient *old)
{
    GList *it;
    ObClient *target = NULL;
    ObClient *desktop = NULL;

    ob_debug_type(OB_DEBUG_FOCUS, "trying pointer stuff\n");
    if (config_focus_follow && !config_focus_last)
    {
        if ((target = client_under_pointer()))
            if (allow_refocus || target != old)
                if (client_normal(target) && client_can_focus(target)) {
                    ob_debug_type(OB_DEBUG_FOCUS, "found in pointer stuff\n");
                    return target;
                }
    }

#if 0
        /* try for group relations */
        if (old->group) {
            GSList *sit;

            for (it = focus_order[screen_desktop]; it; it = g_list_next(it))
                for (sit = old->group->members; sit; sit = g_slist_next(sit))
                    if (sit->data == it->data)
                        if (sit->data != old && client_normal(sit->data))
                            if (client_can_focus(sit->data))
                                return sit->data;
        }
#endif

    ob_debug_type(OB_DEBUG_FOCUS, "trying omnipresentness\n");
    if (allow_refocus && old && old->desktop == DESKTOP_ALL &&
        client_normal(old))
    {
        return old;
    }


    ob_debug_type(OB_DEBUG_FOCUS, "trying  the focus order\n");
    for (it = focus_order; it; it = g_list_next(it))
        if (allow_refocus || it->data != old) {
            ObClient *c = it->data;
            /* fallback focus to a window if:
               1. it is actually focusable, cuz if it's not then we're sending
               focus off to nothing
               2. it is validated. if the window is about to disappear, then
               don't try focus it.
               3. it is visible on the current desktop. this ignores
               omnipresent windows, which are problematic in their own rite.
               4. it's not iconic
               5. it is a normal type window, don't fall back onto a dock or
               a splashscreen or a desktop window (save the desktop as a
               backup fallback though)
            */
            if (client_can_focus(c) && !c->iconic)
            {
                if (c->desktop == screen_desktop && client_normal(c)) {
                    ob_debug_type(OB_DEBUG_FOCUS, "found in focus order\n");
                    return it->data;
                } else if ((c->desktop == screen_desktop ||
                            c->desktop == DESKTOP_ALL) &&
                           c->type == OB_CLIENT_TYPE_DESKTOP && 
                           desktop == NULL)
                    desktop = c;
            }
        }

    /* as a last resort fallback to the desktop window if there is one.
       (if there's more than one, then the one most recently focused.)
    */
    ob_debug_type(OB_DEBUG_FOCUS, "found desktop: \n", !!desktop);
    return desktop;   
}

void focus_fallback(gboolean allow_refocus)
{
    ObClient *new;
    ObClient *old = focus_client;

    /* unfocus any focused clients.. they can be focused by Pointer events
       and such, and then when I try focus them, I won't get a FocusIn event
       at all for them.
    */
    focus_nothing();

    if ((new = focus_fallback_target(allow_refocus, old)))
        client_focus(new);
}

void focus_nothing()
{
    /* Install our own colormap */
    if (focus_client != NULL) {
        screen_install_colormap(focus_client, FALSE);
        screen_install_colormap(NULL, TRUE);
    }

    focus_client = NULL;

    /* when nothing will be focused, send focus to the backup target */
    XSetInputFocus(ob_display, screen_support_win, RevertToPointerRoot,
                   event_curtime);
}

static void popup_cycle(ObClient *c, gboolean show)
{
    if (!show) {
        icon_popup_hide(focus_cycle_popup);
    } else {
        Rect *a;
        ObClient *p = c;
        gchar *title = NULL;

        a = screen_physical_area_monitor(0);
        icon_popup_position(focus_cycle_popup, CenterGravity,
                            a->x + a->width / 2, a->y + a->height / 2);
/*        icon_popup_size(focus_cycle_popup, a->height/2, a->height/16);
        icon_popup_show(focus_cycle_popup, c->title,
                        client_icon(c, a->height/16, a->height/16));
*/
        /* XXX the size and the font extents need to be related on some level
         */
        icon_popup_size(focus_cycle_popup, POPUP_WIDTH, POPUP_HEIGHT);

        /* use the transient's parent's title/icon */
        while (p->transient_for && p->transient_for != OB_TRAN_GROUP)
            p = p->transient_for;

        if (p != c && !strcmp("", (c->iconic ? c->icon_title : c->title)))
            title = g_strdup(p->iconic ? p->icon_title : p->title);
            /*title = g_strconcat((c->iconic ? c->icon_title : c->title),
                                " - ",
                                (p->iconic ? p->icon_title : p->title),
                                NULL);
            */
        icon_popup_show(focus_cycle_popup,
                        (title ? title :
                         (c->iconic ? c->icon_title : c->title)),
                        client_icon(p, 48, 48));
        g_free(title);
    }
}

void focus_cycle_draw_indicator()
{
    if (!focus_cycle_target) {
        XUnmapWindow(ob_display, focus_indicator.top.win);
        XUnmapWindow(ob_display, focus_indicator.left.win);
        XUnmapWindow(ob_display, focus_indicator.right.win);
        XUnmapWindow(ob_display, focus_indicator.bottom.win);

        /* kill enter events cause by this unmapping */
        event_ignore_queued_enters();
    } else {
        /*
          if (focus_cycle_target)
              frame_adjust_focus(focus_cycle_target->frame, FALSE);
          frame_adjust_focus(focus_cycle_target->frame, TRUE);
        */
        gint x, y, w, h;
        gint wt, wl, wr, wb;

        wt = wl = wr = wb = MAX(3,
                                MAX(1, MAX(ob_rr_theme->paddingx,
                                           ob_rr_theme->paddingy)) * 2 +
                                ob_rr_theme->fbwidth * 2);

        x = focus_cycle_target->frame->area.x;
        y = focus_cycle_target->frame->area.y;
        w = focus_cycle_target->frame->area.width;
        h = wt;

        XMoveResizeWindow(ob_display, focus_indicator.top.win,
                          x, y, w, h);
        a_focus_indicator->texture[0].data.lineart.x1 = 0;
        a_focus_indicator->texture[0].data.lineart.y1 = h-1;
        a_focus_indicator->texture[0].data.lineart.x2 = 0;
        a_focus_indicator->texture[0].data.lineart.y2 = 0;
        a_focus_indicator->texture[1].data.lineart.x1 = 0;
        a_focus_indicator->texture[1].data.lineart.y1 = 0;
        a_focus_indicator->texture[1].data.lineart.x2 = w-1;
        a_focus_indicator->texture[1].data.lineart.y2 = 0;
        a_focus_indicator->texture[2].data.lineart.x1 = w-1;
        a_focus_indicator->texture[2].data.lineart.y1 = 0;
        a_focus_indicator->texture[2].data.lineart.x2 = w-1;
        a_focus_indicator->texture[2].data.lineart.y2 = h-1;
        a_focus_indicator->texture[3].data.lineart.x1 = (wl-1);
        a_focus_indicator->texture[3].data.lineart.y1 = h-1;
        a_focus_indicator->texture[3].data.lineart.x2 = w - wr;
        a_focus_indicator->texture[3].data.lineart.y2 = h-1;
        RrPaint(a_focus_indicator, focus_indicator.top.win,
                w, h);

        x = focus_cycle_target->frame->area.x;
        y = focus_cycle_target->frame->area.y;
        w = wl;
        h = focus_cycle_target->frame->area.height;

        XMoveResizeWindow(ob_display, focus_indicator.left.win,
                          x, y, w, h);
        a_focus_indicator->texture[0].data.lineart.x1 = w-1;
        a_focus_indicator->texture[0].data.lineart.y1 = 0;
        a_focus_indicator->texture[0].data.lineart.x2 = 0;
        a_focus_indicator->texture[0].data.lineart.y2 = 0;
        a_focus_indicator->texture[1].data.lineart.x1 = 0;
        a_focus_indicator->texture[1].data.lineart.y1 = 0;
        a_focus_indicator->texture[1].data.lineart.x2 = 0;
        a_focus_indicator->texture[1].data.lineart.y2 = h-1;
        a_focus_indicator->texture[2].data.lineart.x1 = 0;
        a_focus_indicator->texture[2].data.lineart.y1 = h-1;
        a_focus_indicator->texture[2].data.lineart.x2 = w-1;
        a_focus_indicator->texture[2].data.lineart.y2 = h-1;
        a_focus_indicator->texture[3].data.lineart.x1 = w-1;
        a_focus_indicator->texture[3].data.lineart.y1 = wt-1;
        a_focus_indicator->texture[3].data.lineart.x2 = w-1;
        a_focus_indicator->texture[3].data.lineart.y2 = h - wb;
        RrPaint(a_focus_indicator, focus_indicator.left.win,
                w, h);

        x = focus_cycle_target->frame->area.x +
            focus_cycle_target->frame->area.width - wr;
        y = focus_cycle_target->frame->area.y;
        w = wr;
        h = focus_cycle_target->frame->area.height ;

        XMoveResizeWindow(ob_display, focus_indicator.right.win,
                          x, y, w, h);
        a_focus_indicator->texture[0].data.lineart.x1 = 0;
        a_focus_indicator->texture[0].data.lineart.y1 = 0;
        a_focus_indicator->texture[0].data.lineart.x2 = w-1;
        a_focus_indicator->texture[0].data.lineart.y2 = 0;
        a_focus_indicator->texture[1].data.lineart.x1 = w-1;
        a_focus_indicator->texture[1].data.lineart.y1 = 0;
        a_focus_indicator->texture[1].data.lineart.x2 = w-1;
        a_focus_indicator->texture[1].data.lineart.y2 = h-1;
        a_focus_indicator->texture[2].data.lineart.x1 = w-1;
        a_focus_indicator->texture[2].data.lineart.y1 = h-1;
        a_focus_indicator->texture[2].data.lineart.x2 = 0;
        a_focus_indicator->texture[2].data.lineart.y2 = h-1;
        a_focus_indicator->texture[3].data.lineart.x1 = 0;
        a_focus_indicator->texture[3].data.lineart.y1 = wt-1;
        a_focus_indicator->texture[3].data.lineart.x2 = 0;
        a_focus_indicator->texture[3].data.lineart.y2 = h - wb;
        RrPaint(a_focus_indicator, focus_indicator.right.win,
                w, h);

        x = focus_cycle_target->frame->area.x;
        y = focus_cycle_target->frame->area.y +
            focus_cycle_target->frame->area.height - wb;
        w = focus_cycle_target->frame->area.width;
        h = wb;

        XMoveResizeWindow(ob_display, focus_indicator.bottom.win,
                          x, y, w, h);
        a_focus_indicator->texture[0].data.lineart.x1 = 0;
        a_focus_indicator->texture[0].data.lineart.y1 = 0;
        a_focus_indicator->texture[0].data.lineart.x2 = 0;
        a_focus_indicator->texture[0].data.lineart.y2 = h-1;
        a_focus_indicator->texture[1].data.lineart.x1 = 0;
        a_focus_indicator->texture[1].data.lineart.y1 = h-1;
        a_focus_indicator->texture[1].data.lineart.x2 = w-1;
        a_focus_indicator->texture[1].data.lineart.y2 = h-1;
        a_focus_indicator->texture[2].data.lineart.x1 = w-1;
        a_focus_indicator->texture[2].data.lineart.y1 = h-1;
        a_focus_indicator->texture[2].data.lineart.x2 = w-1;
        a_focus_indicator->texture[2].data.lineart.y2 = 0;
        a_focus_indicator->texture[3].data.lineart.x1 = wl-1;
        a_focus_indicator->texture[3].data.lineart.y1 = 0;
        a_focus_indicator->texture[3].data.lineart.x2 = w - wr;
        a_focus_indicator->texture[3].data.lineart.y2 = 0;
        RrPaint(a_focus_indicator, focus_indicator.bottom.win,
                w, h);

        XMapWindow(ob_display, focus_indicator.top.win);
        XMapWindow(ob_display, focus_indicator.left.win);
        XMapWindow(ob_display, focus_indicator.right.win);
        XMapWindow(ob_display, focus_indicator.bottom.win);
    }
}

static gboolean valid_focus_target(ObClient *ft)
{
    /* we don't use client_can_focus here, because that doesn't let you
       focus an iconic window, but we want to be able to, so we just check
       if the focus flags on the window allow it, and its on the current
       desktop */
    if ((ft->type == OB_CLIENT_TYPE_NORMAL ||
         ft->type == OB_CLIENT_TYPE_DIALOG ||
         (!client_has_group_siblings(ft) &&
          (ft->type == OB_CLIENT_TYPE_TOOLBAR ||
           ft->type == OB_CLIENT_TYPE_MENU ||
           ft->type == OB_CLIENT_TYPE_UTILITY))) &&
        ((ft->can_focus || ft->focus_notify) &&
         !ft->skip_pager &&
         (ft->desktop == screen_desktop || ft->desktop == DESKTOP_ALL)) &&
        ft == client_focus_target(ft))
        return TRUE;
/*
    {
        GSList *it;

        for (it = ft->transients; it; it = g_slist_next(it)) {
            ObClient *c = it->data;

            if (c->frame->visible)
                return FALSE;
        }
        return TRUE;
    }
*/

    return FALSE;
}

void focus_cycle(gboolean forward, gboolean linear, gboolean interactive,
                 gboolean dialog, gboolean done, gboolean cancel)
{
    static ObClient *first = NULL;
    static ObClient *t = NULL;
    static GList *order = NULL;
    GList *it, *start, *list;
    ObClient *ft = NULL;

    if (interactive) {
        if (cancel) {
            focus_cycle_target = NULL;
            goto done_cycle;
        } else if (done)
            goto done_cycle;

        if (!focus_order)
            goto done_cycle;

        if (!first) first = focus_client;

        if (linear) list = client_list;
        else        list = focus_order;
    } else {
        if (!focus_order)
            goto done_cycle;
        list = client_list;
    }
    if (!focus_cycle_target) focus_cycle_target = focus_client;

    start = it = g_list_find(list, focus_cycle_target);
    if (!start) /* switched desktops or something? */
        start = it = forward ? g_list_last(list) : g_list_first(list);
    if (!start) goto done_cycle;

    do {
        if (forward) {
            it = it->next;
            if (it == NULL) it = g_list_first(list);
        } else {
            it = it->prev;
            if (it == NULL) it = g_list_last(list);
        }
        ft = it->data;
        if (valid_focus_target(ft)) {
            if (interactive) {
                if (ft != focus_cycle_target) { /* prevents flicker */
                    focus_cycle_target = ft;
                    focus_cycle_draw_indicator();
                }
                popup_cycle(ft, dialog);
                return;
            } else if (ft != focus_cycle_target) {
                focus_cycle_target = ft;
                done = TRUE;
                break;
            }
        }
    } while (it != start);

done_cycle:
    if (done && focus_cycle_target)
        client_activate(focus_cycle_target, FALSE, TRUE);

    t = NULL;
    first = NULL;
    focus_cycle_target = NULL;
    g_list_free(order);
    order = NULL;

    if (interactive) {
        focus_cycle_draw_indicator();
        popup_cycle(ft, FALSE);
    }

    return;
}

void focus_directional_cycle(ObDirection dir, gboolean interactive,
                             gboolean dialog, gboolean done, gboolean cancel)
{
    static ObClient *first = NULL;
    ObClient *ft = NULL;

    if (!interactive)
        return;

    if (cancel) {
        focus_cycle_target = NULL;
        goto done_cycle;
    } else if (done)
        goto done_cycle;

    if (!focus_order)
        goto done_cycle;

    if (!first) first = focus_client;
    if (!focus_cycle_target) focus_cycle_target = focus_client;

    if (focus_cycle_target)
        ft = client_find_directional(focus_cycle_target, dir);
    else {
        GList *it;

        for (it = focus_order; it; it = g_list_next(it))
            if (valid_focus_target(it->data))
                ft = it->data;
    }
        
    if (ft) {
        if (ft != focus_cycle_target) {/* prevents flicker */
            focus_cycle_target = ft;
            focus_cycle_draw_indicator();
        }
    }
    if (focus_cycle_target) {
        popup_cycle(focus_cycle_target, dialog);
        if (dialog)
            return;
    }


done_cycle:
    if (done && focus_cycle_target)
        client_activate(focus_cycle_target, FALSE, TRUE);

    first = NULL;
    focus_cycle_target = NULL;

    focus_cycle_draw_indicator();
    popup_cycle(ft, FALSE);

    return;
}

void focus_order_add_new(ObClient *c)
{
    if (c->iconic)
        focus_order_to_top(c);
    else {
        g_assert(!g_list_find(focus_order, c));
        /* if there are any iconic windows, put this above them in the order,
           but if there are not, then put it under the currently focused one */
        if (focus_order && ((ObClient*)focus_order->data)->iconic)
            focus_order = g_list_insert(focus_order, c, 0);
        else
            focus_order = g_list_insert(focus_order, c, 1);
    }
}

void focus_order_remove(ObClient *c)
{
    focus_order = g_list_remove(focus_order, c);
}

void focus_order_to_top(ObClient *c)
{
    focus_order = g_list_remove(focus_order, c);
    if (!c->iconic) {
        focus_order = g_list_prepend(focus_order, c);
    } else {
        GList *it;

        /* insert before first iconic window */
        for (it = focus_order;
             it && !((ObClient*)it->data)->iconic; it = g_list_next(it));
        focus_order = g_list_insert_before(focus_order, it, c);
    }
}

void focus_order_to_bottom(ObClient *c)
{
    focus_order = g_list_remove(focus_order, c);
    if (c->iconic) {
        focus_order = g_list_append(focus_order, c);
    } else {
        GList *it;

        /* insert before first iconic window */
        for (it = focus_order;
             it && !((ObClient*)it->data)->iconic; it = g_list_next(it));
        focus_order = g_list_insert_before(focus_order, it, c);
    }
}

ObClient *focus_order_find_first(guint desktop)
{
    GList *it;
    for (it = focus_order; it; it = g_list_next(it)) {
        ObClient *c = it->data;
        if (c->desktop == desktop || c->desktop == DESKTOP_ALL)
            return c;
    }
    return NULL;
}
