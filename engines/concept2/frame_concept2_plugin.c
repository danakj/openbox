/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

 frame_default_plugin.c for the Openbox window manager
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

#include "frame_concept2_plugin.h"
#include "frame_concept2_render.h"

#include "openbox/frame.h"
#include "openbox/client.h"
#include "openbox/openbox.h"
#include "openbox/extensions.h"
#include "openbox/prop.h"
#include "openbox/grab.h"
#include "openbox/config.h"
#include "openbox/mainloop.h"
#include "openbox/focus_cycle.h"
#include "openbox/focus_cycle_indicator.h"
#include "openbox/moveresize.h"
#include "openbox/screen.h"
#include "render/theme.h"

typedef enum
{
    OB_FLAG_MAX = 1 << 0,
    OB_FLAG_CLOSE = 1 << 1,
    OB_FLAG_DESK = 1 << 2,
    OB_FLAG_SHADE = 1 << 3,
    OB_FLAG_ICONIFY = 1 << 4
} ObFrameFlags;

#define FRAME_EVENTMASK (EnterWindowMask | LeaveWindowMask | \
                         ButtonPressMask | ButtonReleaseMask | \
                         SubstructureRedirectMask | FocusChangeMask)
#define ELEMENT_EVENTMASK (ButtonPressMask | ButtonReleaseMask | \
                           ButtonMotionMask | PointerMotionMask | \
                           EnterWindowMask | LeaveWindowMask)

#define FRAME_ANIMATE_ICONIFY_TIME 150000 /* .15 seconds */
#define FRAME_ANIMATE_ICONIFY_STEP_TIME (G_USEC_PER_SEC / 60) /* 60 Hz */

#define FRAME_HANDLE_Y(f) (f->size.top + f->client->area.height + f->cbwidth_b)

Window createWindow(Window parent, Visual *visual, gulong mask,
        XSetWindowAttributes *attrib)
{
    return XCreateWindow(plugin.ob_display, parent, 0, 0, 1, 1, 0, (visual ? 32
            : RrDepth(plugin.ob_rr_inst)), InputOutput, (visual ? visual
            : RrVisual(plugin.ob_rr_inst)), mask, attrib);

}

Visual *check_32bit_client(ObClient *c)
{
    XWindowAttributes wattrib;
    Status ret;

    /* we're already running at 32 bit depth, yay. we don't need to use their
     visual */
    if (RrDepth(plugin.ob_rr_inst) == 32)
        return NULL;

    ret = XGetWindowAttributes(plugin.ob_display, c->window, &wattrib);
    g_assert(ret != BadDrawable);
    g_assert(ret != BadWindow);

    if (wattrib.depth == 32)
        return wattrib.visual;
    return NULL;
}

/* Not used */
gint init(Display * display, gint screen)
{
    plugin.ob_display = display;
    plugin.ob_screen = screen;
}

gpointer frame_new(struct _ObClient * client)
{
    XSetWindowAttributes attrib;
    gulong mask;
    ObConceptFrame *self;
    Visual *visual;

    self = g_new0(ObConceptFrame, 1);
    self->client = client;

    visual = check_32bit_client(client);

    /* create the non-visible decor windows */

    mask = 0;
    if (visual) {
        /* client has a 32-bit visual */
        mask |= CWColormap | CWBackPixel | CWBorderPixel;
        /* create a colormap with the visual */
        OBCONCEPTFRAME(self)->colormap = attrib.colormap = XCreateColormap(
                plugin.ob_display, RootWindow(plugin.ob_display,
                        plugin.ob_screen), visual, AllocNone);
        attrib.background_pixel = BlackPixel(plugin.ob_display,
                plugin.ob_screen);
        attrib.border_pixel = BlackPixel(plugin.ob_display, plugin.ob_screen);
    }
    self->window = createWindow(
            RootWindow(plugin.ob_display, plugin.ob_screen), visual, mask,
            &attrib);

    /* create the visible decor windows */

    mask = 0;
    if (visual) {
        /* client has a 32-bit visual */
        mask |= CWColormap | CWBackPixel | CWBorderPixel;
        attrib.colormap = RrColormap(plugin.ob_rr_inst);
    }

    self->background = createWindow(self->window, NULL, mask, &attrib);

    mask |= CWEventMask;
    attrib.event_mask = ELEMENT_EVENTMASK;

    self->top = createWindow(self->window, NULL, mask, &attrib);
    self->bottom = createWindow(self->window, NULL, mask, &attrib);
    self->left = createWindow(self->window, NULL, mask, &attrib);
    self->right = createWindow(self->window, NULL, mask, &attrib);

    self->left_close = createWindow(self->left, NULL, mask, &attrib);
    self->left_iconify = createWindow(self->left, NULL, mask, &attrib);
    self->left_maximize = createWindow(self->left, NULL, mask, &attrib);
    self->left_shade = createWindow(self->left, NULL, mask, &attrib);

    self->handle = createWindow(self->left, NULL, mask, &attrib);

    self->top_left = createWindow(self->window, NULL, mask, &attrib);
    self->top_right = createWindow(self->window, NULL, mask, &attrib);

    self->bottom_left = createWindow(self->window, NULL, mask, &attrib);
    self->bottom_right = createWindow(self->window, NULL, mask, &attrib);

    XMapWindow(plugin.ob_display, self->background);

    self->focused = FALSE;

    self->max_press = FALSE;
    self->close_press = FALSE;
    self->desk_press = FALSE;
    self->iconify_press = FALSE;
    self->shade_press = FALSE;
    self->max_hover = FALSE;
    self->close_hover = FALSE;
    self->desk_hover = FALSE;
    self->iconify_hover = FALSE;
    self->shade_hover = FALSE;

    set_theme_statics(self);

    return (ObFrame*)self;
}

void set_theme_statics(gpointer _self)
{
    ObConceptFrame * self = (ObConceptFrame *) _self;
    /* do this before changing the frame's status like max_horz max_vert */

    XResizeWindow(plugin.ob_display, self->top_left, 15,
            theme_config.border_width);
    XResizeWindow(plugin.ob_display, self->top_right, 15,
            theme_config.border_width);
    XResizeWindow(plugin.ob_display, self->bottom_left, 15,
            theme_config.border_width);
    XResizeWindow(plugin.ob_display, self->bottom_right, 15,
            theme_config.border_width);

    XResizeWindow(plugin.ob_display, self->left_close, theme_config.left_width,
            15);
    XResizeWindow(plugin.ob_display, self->left_iconify,
            theme_config.left_width, 15);
    XResizeWindow(plugin.ob_display, self->left_maximize,
            theme_config.left_width, 15);
    XResizeWindow(plugin.ob_display, self->left_shade, theme_config.left_width,
            15);
    XResizeWindow(plugin.ob_display, self->handle, theme_config.left_width, 15);

    XMoveWindow(plugin.ob_display, self->left_close, 0, 0);
    XMoveWindow(plugin.ob_display, self->left_iconify, 0, 15);
    XMoveWindow(plugin.ob_display, self->left_maximize, 0, 30);
    XMoveWindow(plugin.ob_display, self->left_shade, 0, 45);
    XMoveWindow(plugin.ob_display, self->handle, 0, 60);

}

void free_theme_statics(gpointer _self)
{
    ObConceptFrame * self = (ObConceptFrame *) _self;
}

void frame_free(gpointer self)
{
    free_theme_statics(OBCONCEPTFRAME(self));
    XDestroyWindow(plugin.ob_display, OBCONCEPTFRAME(self)->window);
    if (OBCONCEPTFRAME(self)->colormap)
        XFreeColormap(plugin.ob_display, OBCONCEPTFRAME(self)->colormap);
    g_free(self);
}

void frame_show(gpointer _self)
{
    ObConceptFrame * self = (ObConceptFrame *) _self;
    if (!self->visible) {
        self->visible = TRUE;
        frame_update_skin(self);
        /* Grab the server to make sure that the frame window is mapped before
         the client gets its MapNotify, i.e. to make sure the client is
         _visible_ when it gets MapNotify. */
        grab_server(TRUE);
        XMapWindow(plugin.ob_display, self->client->window);
        XMapWindow(plugin.ob_display, self->window);
        grab_server(FALSE);
    }
}

void frame_hide(gpointer _self)
{
    ObConceptFrame * self = (ObConceptFrame *) _self;
    if (self->visible) {
        self->visible = FALSE;
        if (!frame_iconify_animating(self))
            XUnmapWindow(plugin.ob_display, self->window);
        /* we unmap the client itself so that we can get MapRequest
         events, and because the ICCCM tells us to! */
        XUnmapWindow(plugin.ob_display, self->client->window);
        self->client->ignore_unmaps += 1;
    }
}

void frame_adjust_theme(gpointer _self)
{
    ObConceptFrame * self = (ObConceptFrame *) _self;
    free_theme_statics(self);
    set_theme_statics(self);
}

void frame_adjust_shape(gpointer _self)
{
#ifdef SHAPE
    ObConceptFrame * self = (ObConceptFrame *) _self;
    gint num;
    XRectangle xrect[2];

    if (!self->client->shaped)
    {
        /* clear the shape on the frame window */
        XShapeCombineMask(plugin.ob_display, self->window, ShapeBounding,
                self->size.left,
                self->size.top,
                None, ShapeSet);
    }
    else
    {
        /* make the frame's shape match the clients */
        XShapeCombineShape(plugin.ob_display, self->window, ShapeBounding,
                self->size.left,
                self->size.top,
                self->client->window,
                ShapeBounding, ShapeSet);

        num = 0;
        if (self->decorations)
        {
            xrect[0].x = 0;
            xrect[0].y = 0;
            xrect[0].width = self->area.width;
            xrect[0].height = self->size.top;
            ++num;
        }

        XShapeCombineRectangles(plugin.ob_display, self->window,
                ShapeBounding, 0, 0, xrect, num,
                ShapeUnion, Unsorted);
    }
#endif
}

void frame_adjust_area(gpointer _self, gboolean moved, gboolean resized,
        gboolean fake)
{
    ObConceptFrame * self = (ObConceptFrame *) _self;

    /* do this before changing the frame's status like max_horz max_vert */
    frame_adjust_cursors(self);

    /* Copy client status */
    self->functions = self->client->functions;
    self->decorations = self->client->decorations;
    self->max_horz = self->client->max_horz;
    self->max_vert = self->client->max_vert;
    self->shaded = self->client->shaded;

    if (self->decorations && !self->shaded) {
        self->cbwidth_l = theme_config.left_width;
        self->cbwidth_r = theme_config.border_width;
        self->cbwidth_t = theme_config.border_width;
        self->cbwidth_b = theme_config.border_width;

        if (self->max_horz) {
            self->cbwidth_l = theme_config.left_width;
            self->cbwidth_r = 0;
        }

        if (self->max_vert) {
            self->cbwidth_b = 0;
            self->cbwidth_t = 0;
        }

        STRUT_SET(self->size, self->cbwidth_l, self->cbwidth_t,
                self->cbwidth_r, self->cbwidth_b);

        RECT_SET_SIZE(self->area, self->client->area.width + self->size.left
                + self->size.right, self->client->area.height + self->size.top
                + self->size.bottom);

        self->width = self->area.width;

        if (!fake) {

            XMoveResizeWindow(plugin.ob_display, self->top, 15, 0,
                    self->area.width - 30, theme_config.border_width);
            XMapWindow(plugin.ob_display, self->top);

            XMoveResizeWindow(plugin.ob_display, self->bottom, 15,
                    self->area.height - theme_config.border_width,
                    self->area.width - 30, theme_config.border_width);
            XMapWindow(plugin.ob_display, self->bottom);

            XMoveResizeWindow(plugin.ob_display, self->left, 0,
                    theme_config.border_width, theme_config.left_width,
                    self->area.height - 2*theme_config.border_width);
            XMapWindow(plugin.ob_display, self->left);

            XMoveResizeWindow(plugin.ob_display, self->right, self->area.width
                    - theme_config.border_width, theme_config.border_width,
                    theme_config.border_width, self->area.height - 2
                            *theme_config.border_width);
            XMapWindow(plugin.ob_display, self->right);

            XMoveWindow(plugin.ob_display, self->top_left, 0, 0);
            XMapWindow(plugin.ob_display, self->top_left);
            XMoveWindow(plugin.ob_display, self->top_right, self->area.width
                    - 15, 0);
            XMapWindow(plugin.ob_display, self->top_right);
            XMoveWindow(plugin.ob_display, self->bottom_left, 0,
                    self->area.height - theme_config.border_width);
            XMapWindow(plugin.ob_display, self->bottom_left);
            XMoveWindow(plugin.ob_display, self->bottom_right, self->area.width
                    - 15, self->area.height - theme_config.border_width);
            XMapWindow(plugin.ob_display, self->bottom_right);

            XMapWindow(plugin.ob_display, self->left_close);
            XMapWindow(plugin.ob_display, self->left_iconify);
            XMapWindow(plugin.ob_display, self->left_maximize);
            XMapWindow(plugin.ob_display, self->left_shade);
            XMapWindow(plugin.ob_display, self->handle);
            /* find the new coordinates, done after setting the frame.size, for
             frame_client_gravity. */
            self->area.x = self->client->area.x;
            self->area.y = self->client->area.y;
            frame_client_gravity(self, &self->area.x, &self->area.y);

            XMoveResizeWindow(plugin.ob_display, self->background,
                    theme_config.border_width, theme_config.border_width,
                    self->area.width - 2 * theme_config.border_width,
                    self->area.height - 2 * theme_config.border_width);
            XMapWindow(plugin.ob_display, self->background);
        }

        XMoveWindow(plugin.ob_display, self->client->window, self->size.left,
                self->size.top);
        XMoveResizeWindow(plugin.ob_display, self->window, self->area.x,
                self->area.y, self->area.width, self->area.height);

    }
    else if (self->shaded) {
        self->cbwidth_l = theme_config.left_width;
        self->cbwidth_r = 0;
        self->cbwidth_b = 0;
        self->cbwidth_t = 0;
        STRUT_SET(self->size, self->cbwidth_l, self->cbwidth_t,
                self->cbwidth_r, self->cbwidth_b);

        RECT_SET_SIZE(self->area, theme_config.left_width, self->area.height);

        /* find the new coordinates, done after setting the frame.size, for
         frame_client_gravity. */
        self->area.x = self->client->area.x;
        self->area.y = self->client->area.y;
        //frame_client_gravity(self, &self->area.x, &self->area.y);
        self->width = self->area.width;
        if (!fake) {
            XUnmapWindow(plugin.ob_display, self->top);
            XUnmapWindow(plugin.ob_display, self->bottom);
            XUnmapWindow(plugin.ob_display, self->right);
            XUnmapWindow(plugin.ob_display, self->top_left);
            XUnmapWindow(plugin.ob_display, self->top_right);
            XUnmapWindow(plugin.ob_display, self->bottom_left);
            XUnmapWindow(plugin.ob_display, self->bottom_right);

            XMoveResizeWindow(plugin.ob_display, self->left, 0, 0,
                    theme_config.left_width, self->area.height);
            XMapWindow(plugin.ob_display, self->left);
        }

        XMoveWindow(plugin.ob_display, self->client->window,
                theme_config.left_width, 0);
        XMoveResizeWindow(plugin.ob_display, self->window, self->area.x,
                self->area.y, self->area.width, self->area.height);
    }
    else {
        self->cbwidth_l = 0;
        self->cbwidth_r = 0;
        self->cbwidth_b = 0;
        self->cbwidth_t = 0;
        STRUT_SET(self->size, self->cbwidth_l, self->cbwidth_t,
                self->cbwidth_r, self->cbwidth_b);

        RECT_SET_SIZE(self->area, self->client->area.width + self->size.left
                + self->size.right, self->client->area.height + self->size.top
                + self->size.bottom);

        /* find the new coordinates, done after setting the frame.size, for
         frame_client_gravity. */
        self->area.x = self->client->area.x;
        self->area.y = self->client->area.y;
        frame_client_gravity(self, &self->area.x, &self->area.y);

        self->width = self->area.width;
        if (!fake) {
            XUnmapWindow(plugin.ob_display, self->top);
            XUnmapWindow(plugin.ob_display, self->bottom);
            XUnmapWindow(plugin.ob_display, self->left);
            XUnmapWindow(plugin.ob_display, self->right);
            XUnmapWindow(plugin.ob_display, self->top_left);
            XUnmapWindow(plugin.ob_display, self->top_right);
            XUnmapWindow(plugin.ob_display, self->bottom_left);
            XUnmapWindow(plugin.ob_display, self->bottom_right);

            XUnmapWindow(plugin.ob_display, self->handle);

            XMoveResizeWindow(plugin.ob_display, self->background, 0, 0,
                    self->area.width, self->area.height);
            XMapWindow(plugin.ob_display, self->background);
        }

        XMoveWindow(plugin.ob_display, self->client->window, self->size.left,
                self->size.top);
        XMoveResizeWindow(plugin.ob_display, self->window, self->area.x,
                self->area.y, self->area.width, self->area.height);
    }
}

void frame_adjust_cursors(gpointer _self)
{
    ObConceptFrame * self = (ObConceptFrame *) _self;

    XSetWindowAttributes a;
    a.cursor = ob_cursor(OB_CURSOR_NORTH);
    XChangeWindowAttributes(plugin.ob_display, self->top, CWCursor, &a);

    a.cursor = ob_cursor(OB_CURSOR_SOUTH);
    XChangeWindowAttributes(plugin.ob_display, self->bottom, CWCursor, &a);

    a.cursor = ob_cursor(OB_CURSOR_WEST);
    XChangeWindowAttributes(plugin.ob_display, self->left, CWCursor, &a);

    a.cursor = ob_cursor(OB_CURSOR_EAST);
    XChangeWindowAttributes(plugin.ob_display, self->right, CWCursor, &a);

    a.cursor = ob_cursor(OB_CURSOR_NORTHWEST);
    XChangeWindowAttributes(plugin.ob_display, self->top_left, CWCursor, &a);

    a.cursor = ob_cursor(OB_CURSOR_NORTHEAST);
    XChangeWindowAttributes(plugin.ob_display, self->top_right, CWCursor, &a);

    a.cursor = ob_cursor(OB_CURSOR_SOUTHWEST);
    XChangeWindowAttributes(plugin.ob_display, self->bottom_left, CWCursor, &a);

    a.cursor = ob_cursor(OB_CURSOR_SOUTHEAST);
    XChangeWindowAttributes(plugin.ob_display, self->bottom_right, CWCursor, &a);

    a.cursor = ob_cursor(OB_CURSOR_POINTER);
    XChangeWindowAttributes(plugin.ob_display, self->left_close, CWCursor, &a);
    XChangeWindowAttributes(plugin.ob_display, self->left_iconify, CWCursor, &a);
    XChangeWindowAttributes(plugin.ob_display, self->left_maximize, CWCursor,
            &a);
    XChangeWindowAttributes(plugin.ob_display, self->left_shade, CWCursor, &a);
    XChangeWindowAttributes(plugin.ob_display, self->handle, CWCursor, &a);

}

void frame_adjust_client_area(gpointer _self)
{
    ObConceptFrame * self = (ObConceptFrame *) _self;
    /* adjust the window which is there to prevent flashing on unmap */
    XMoveResizeWindow(plugin.ob_display, self->background, 0, 0,
            self->client->area.width, self->client->area.height);
}

void frame_adjust_state(gpointer _self)
{
    ObConceptFrame * self = (ObConceptFrame *) _self;
    self->need_render = TRUE;
    framerender_frame(self);
}

void frame_adjust_focus(gpointer _self, gboolean hilite)
{
    ObConceptFrame * self = (ObConceptFrame *) _self;
    self->focused = hilite;
    self->need_render = TRUE;
    framerender_frame(self);
    XFlush(plugin.ob_display);
}

void frame_adjust_title(gpointer _self)
{
    ObConceptFrame * self = (ObConceptFrame *) _self;
    self->need_render = TRUE;
    framerender_frame(self);
}

void frame_adjust_icon(gpointer _self)
{
    ObConceptFrame * self = (ObConceptFrame *) _self;
    self->need_render = TRUE;
    framerender_frame(self);
}

void frame_grab_client(gpointer _self, GHashTable * map)
{
    ObConceptFrame * self = (ObConceptFrame *) _self;
    /* DO NOT map the client window here. we used to do that, but it is bogus.
     we need to set up the client's dimensions and everything before we
     send a mapnotify or we create race conditions.
     */

    /* reparent the client to the frame */
    XReparentWindow(plugin.ob_display, self->client->window, self->window, 0, 0);

    /*
     When reparenting the client window, it is usually not mapped yet, since
     this occurs from a MapRequest. However, in the case where Openbox is
     starting up, the window is already mapped, so we'll see an unmap event
     for it.
     */
    if (ob_state() == OB_STATE_STARTING)
        ++self->client->ignore_unmaps;

    /* select the event mask on the client's parent (to receive config/map
     req's) the ButtonPress is to catch clicks on the client border */
    XSelectInput(plugin.ob_display, self->window, FRAME_EVENTMASK);

    /* set all the windows for the frame in the window_map */
    g_hash_table_insert(map, &self->window, self->client);
    g_hash_table_insert(map, &self->left, self->client);
    g_hash_table_insert(map, &self->right, self->client);

    g_hash_table_insert(map, &self->top, self->client);
    g_hash_table_insert(map, &self->bottom, self->client);

    g_hash_table_insert(map, &self->top_left, self->client);
    g_hash_table_insert(map, &self->top_right, self->client);

    g_hash_table_insert(map, &self->bottom_left, self->client);
    g_hash_table_insert(map, &self->bottom_right, self->client);

    g_hash_table_insert(map, &self->left_close, self->client);
    g_hash_table_insert(map, &self->left_iconify, self->client);
    g_hash_table_insert(map, &self->left_maximize, self->client);
    g_hash_table_insert(map, &self->left_shade, self->client);

    g_hash_table_insert(map, &self->handle, self->client);

}

void frame_release_client(gpointer _self, GHashTable * map)
{
    ObConceptFrame * self = (ObConceptFrame *) _self;
    XEvent ev;
    gboolean reparent = TRUE;

    /* if there was any animation going on, kill it */
    ob_main_loop_timeout_remove_data(plugin.ob_main_loop,
            frame_animate_iconify, self, FALSE);

    /* check if the app has already reparented its window away */
    while (XCheckTypedWindowEvent(plugin.ob_display, self->client->window,
            ReparentNotify, &ev)) {
        /* This check makes sure we don't catch our own reparent action to
         our frame window. This doesn't count as the app reparenting itself
         away of course.

         Reparent events that are generated by us are just discarded here.
         They are of no consequence to us anyhow.
         */
        if (ev.xreparent.parent != self->window) {
            reparent = FALSE;
            XPutBackEvent(plugin.ob_display, &ev);
            break;
        }
    }

    if (reparent) {
        /* according to the ICCCM - if the client doesn't reparent itself,
         then we will reparent the window to root for them */
        XReparentWindow(plugin.ob_display, self->client->window, RootWindow(
                plugin.ob_display, plugin.ob_screen), self->client->area.x,
                self->client->area.y);
    }

    /* remove all the windows for the frame from the window_map */
    g_hash_table_remove(map, &self->window);

    g_hash_table_remove(map, &self->left);
    g_hash_table_remove(map, &self->right);

    g_hash_table_remove(map, &self->top);
    g_hash_table_remove(map, &self->bottom);

    g_hash_table_remove(map, &self->top_left);
    g_hash_table_remove(map, &self->top_right);

    g_hash_table_remove(map, &self->bottom_left);
    g_hash_table_remove(map, &self->bottom_right);

    g_hash_table_remove(map, &self->left_close);
    g_hash_table_remove(map, &self->left_iconify);
    g_hash_table_remove(map, &self->left_maximize);
    g_hash_table_remove(map, &self->left_shade);

    g_hash_table_remove(map, &self->handle);

    ob_main_loop_timeout_remove_data(plugin.ob_main_loop, flash_timeout, self,
            TRUE);
}

/* is there anything present between us and the label? */
static gboolean is_button_present(ObConceptFrame *_self, const gchar *lc,
        gint dir)
{
    ObConceptFrame * self = (ObConceptFrame *) _self;
    for (; *lc != '\0' && lc >= plugin.config_title_layout; lc += dir) {
        if (*lc == ' ')
            continue; /* it was invalid */
        if (*lc == 'N' && self->decorations & OB_FRAME_DECOR_ICON)
            return TRUE;
        if (*lc == 'D' && self->decorations & OB_FRAME_DECOR_ALLDESKTOPS)
            return TRUE;
        if (*lc == 'S' && self->decorations & OB_FRAME_DECOR_SHADE)
            return TRUE;
        if (*lc == 'I' && self->decorations & OB_FRAME_DECOR_ICONIFY)
            return TRUE;
        if (*lc == 'M' && self->decorations & OB_FRAME_DECOR_MAXIMIZE)
            return TRUE;
        if (*lc == 'C' && self->decorations & OB_FRAME_DECOR_CLOSE)
            return TRUE;
        if (*lc == 'L')
            return FALSE;
    }
    return FALSE;
}

ObFrameContext frame_context(gpointer _self, Window win, gint x, gint y)
{
    /* Here because client can be NULL */
    ObConceptFrame *self = OBCONCEPTFRAME(_self);

    if (win == self->window)
        return OB_FRAME_CONTEXT_FRAME;

    if (win == self->bottom)
        return OB_FRAME_CONTEXT_BOTTOM;

    if (win == self->bottom_left)
        return OB_FRAME_CONTEXT_BLCORNER;

    if (win == self->bottom_right)
        return OB_FRAME_CONTEXT_BRCORNER;

    if (win == self->top)
        return OB_FRAME_CONTEXT_TOP;

    if (win == self->top_left)
        return OB_FRAME_CONTEXT_TLCORNER;

    if (win == self->top_right)
        return OB_FRAME_CONTEXT_TRCORNER;

    if (win == self->left_close)
        return OB_FRAME_CONTEXT_CLOSE;
    if (win == self->left_iconify)
        return OB_FRAME_CONTEXT_ICONIFY;
    if (win == self->left_maximize)
        return OB_FRAME_CONTEXT_MAXIMIZE;
    if (win == self->left_shade)
        return OB_FRAME_CONTEXT_SHADE;
    if (win == self->handle)
        return OB_FRAME_CONTEXT_TITLEBAR;

    if (win == self->left)
        return OB_FRAME_CONTEXT_LEFT;
    if (win == self->right)
        return OB_FRAME_CONTEXT_RIGHT;

    return OB_FRAME_CONTEXT_NONE;
}

void frame_client_gravity(gpointer _self, gint *x, gint *y)
{
    ObConceptFrame * self = OBCONCEPTFRAME(_self);
    /* horizontal */
    switch (self->client->gravity) {
    default:
    case NorthWestGravity:
    case SouthWestGravity:
    case WestGravity:
        break;

    case NorthGravity:
    case SouthGravity:
    case CenterGravity:
        /* the middle of the client will be the middle of the frame */
        *x -= (self->size.right - self->size.left) / 2;
        break;

    case NorthEastGravity:
    case SouthEastGravity:
    case EastGravity:
        /* the right side of the client will be the right side of the frame */
        *x -= self->size.right + self->size.left - self->client->border_width
                * 2;
        break;

    case ForgetGravity:
    case StaticGravity:
        /* the client's position won't move */
        *x -= self->size.left - self->client->border_width;
        break;
    }

    /* vertical */
    switch (self->client->gravity) {
    default:
    case NorthWestGravity:
    case NorthEastGravity:
    case NorthGravity:
        break;

    case CenterGravity:
    case EastGravity:
    case WestGravity:
        /* the middle of the client will be the middle of the frame */
        *y -= (self->size.bottom - self->size.top) / 2;
        break;

    case SouthWestGravity:
    case SouthEastGravity:
    case SouthGravity:
        /* the bottom of the client will be the bottom of the frame */
        *y -= self->size.bottom + self->size.top - self->client->border_width
                * 2;
        break;

    case ForgetGravity:
    case StaticGravity:
        /* the client's position won't move */
        *y -= self->size.top - self->client->border_width;
        break;
    }
}

void frame_frame_gravity(gpointer _self, gint *x, gint *y)
{
    ObConceptFrame * self = (ObConceptFrame *) _self;
    /* horizontal */
    switch (self->client->gravity) {
    default:
    case NorthWestGravity:
    case WestGravity:
    case SouthWestGravity:
        break;
    case NorthGravity:
    case CenterGravity:
    case SouthGravity:
        /* the middle of the client will be the middle of the frame */
        *x += (self->size.right - self->size.left) / 2;
        break;
    case NorthEastGravity:
    case EastGravity:
    case SouthEastGravity:
        /* the right side of the client will be the right side of the frame */
        *x += self->size.right + self->size.left - self->client->border_width
                * 2;
        break;
    case StaticGravity:
    case ForgetGravity:
        /* the client's position won't move */
        *x += self->size.left - self->client->border_width;
        break;
    }

    /* vertical */
    switch (self->client->gravity) {
    default:
    case NorthWestGravity:
    case NorthGravity:
    case NorthEastGravity:
        break;
    case WestGravity:
    case CenterGravity:
    case EastGravity:
        /* the middle of the client will be the middle of the frame */
        *y += (self->size.bottom - self->size.top) / 2;
        break;
    case SouthWestGravity:
    case SouthGravity:
    case SouthEastGravity:
        /* the bottom of the client will be the bottom of the frame */
        *y += self->size.bottom + self->size.top - self->client->border_width
                * 2;
        break;
    case StaticGravity:
    case ForgetGravity:
        /* the client's position won't move */
        *y += self->size.top - self->client->border_width;
        break;
    }
}

void frame_rect_to_frame(gpointer _self, Rect *r)
{
    ObConceptFrame * self = (ObConceptFrame *) _self;
    r->width += self->size.left + self->size.right;
    r->height += self->size.top + self->size.bottom;
    frame_client_gravity(self, &r->x, &r->y);
}

void frame_rect_to_client(gpointer _self, Rect *r)
{
    ObConceptFrame * self = (ObConceptFrame *) _self;
    r->width -= self->size.left + self->size.right;
    r->height -= self->size.top + self->size.bottom;
    frame_frame_gravity(self, &r->x, &r->y);
}

void flash_done(gpointer data)
{
    ObConceptFrame *self = data;

    if (self->focused != self->flash_on)
        frame_adjust_focus(self, self->focused);
}

gboolean flash_timeout(gpointer data)
{
    ObConceptFrame *self = data;
    GTimeVal now;

    g_get_current_time(&now);
    if (now.tv_sec > self->flash_end.tv_sec
            || (now.tv_sec == self->flash_end.tv_sec && now.tv_usec
                    >= self->flash_end.tv_usec))
        self->flashing = FALSE;

    if (!self->flashing)
        return FALSE; /* we are done */

    self->flash_on = !self->flash_on;
    if (!self->focused) {
        frame_adjust_focus(self, self->flash_on);
        self->focused = FALSE;
    }

    return TRUE; /* go again */
}

void frame_flash_start(gpointer _self)
{
    ObConceptFrame * self = (ObConceptFrame *) _self;
    self->flash_on = self->focused;

    if (!self->flashing)
        ob_main_loop_timeout_add(plugin.ob_main_loop, G_USEC_PER_SEC * 0.6,
                flash_timeout, self, g_direct_equal, flash_done);
    g_get_current_time(&self->flash_end);
    g_time_val_add(&self->flash_end, G_USEC_PER_SEC * 5);

    self->flashing = TRUE;
}

void frame_flash_stop(gpointer _self)
{
    ObConceptFrame * self = (ObConceptFrame *) _self;
    self->flashing = FALSE;
}

static gulong frame_animate_iconify_time_left(gpointer _self,
        const GTimeVal *now)
{
    ObConceptFrame * self = (ObConceptFrame *) _self;
    glong sec, usec;
    sec = self->iconify_animation_end.tv_sec - now->tv_sec;
    usec = self->iconify_animation_end.tv_usec - now->tv_usec;
    if (usec < 0) {
        usec += G_USEC_PER_SEC;
        sec--;
    }
    /* no negative values */
    return MAX(sec * G_USEC_PER_SEC + usec, 0);
}

gboolean frame_animate_iconify(gpointer p)
{
    ObConceptFrame *self = p;
    gint x, y, w, h;
    gint iconx, icony, iconw;
    GTimeVal now;
    gulong time;
    gboolean iconifying;

    if (self->client->icon_geometry.width == 0) {
        /* there is no icon geometry set so just go straight down */
        Rect *a =
                screen_physical_area_monitor(screen_find_monitor(&self->area));
        iconx = self->area.x + self->area.width / 2 + 32;
        icony = a->y + a->width;
        iconw = 64;
        g_free(a);
    }
    else {
        iconx = self->client->icon_geometry.x;
        icony = self->client->icon_geometry.y;
        iconw = self->client->icon_geometry.width;
    }

    iconifying = self->iconify_animation_going > 0;

    /* how far do we have left to go ? */
    g_get_current_time(&now);
    time = frame_animate_iconify_time_left(self, &now);

    if (time == 0 || iconifying) {
        /* start where the frame is supposed to be */
        x = self->area.x;
        y = self->area.y;
        w = self->area.width;
        h = self->area.height;
    }
    else {
        /* start at the icon */
        x = iconx;
        y = icony;
        w = iconw;
        h = self->size.top; /* just the titlebar */
    }

    if (time > 0) {
        glong dx, dy, dw;
        glong elapsed;

        dx = self->area.x - iconx;
        dy = self->area.y - icony;
        dw = self->area.width - self->bwidth * 2 - iconw;
        /* if restoring, we move in the opposite direction */
        if (!iconifying) {
            dx = -dx;
            dy = -dy;
            dw = -dw;
        }

        elapsed = FRAME_ANIMATE_ICONIFY_TIME - time;
        x = x - (dx * elapsed) / FRAME_ANIMATE_ICONIFY_TIME;
        y = y - (dy * elapsed) / FRAME_ANIMATE_ICONIFY_TIME;
        w = w - (dw * elapsed) / FRAME_ANIMATE_ICONIFY_TIME;
        h = self->size.top; /* just the titlebar */
    }

    if (time == 0)
        frame_end_iconify_animation(self);
    else {
        XMoveResizeWindow(plugin.ob_display, self->window, x, y, w, h);
        XFlush(plugin.ob_display);
    }

    return time > 0; /* repeat until we're out of time */
}

void frame_end_iconify_animation(gpointer _self)
{
    ObConceptFrame * self = (ObConceptFrame *) _self;
    /* see if there is an animation going */
    if (self->iconify_animation_going == 0)
        return;

    if (!self->visible)
        XUnmapWindow(plugin.ob_display, self->window);
    else {
        /* Send a ConfigureNotify when the animation is done, this fixes
         KDE's pager showing the window in the wrong place.  since the
         window is mapped at a different location and is then moved, we
         need to send the synthetic configurenotify, since apps may have
         read the position when the client mapped, apparently. */
        client_reconfigure(self->client, TRUE);
    }

    /* we're not animating any more ! */
    self->iconify_animation_going = 0;

    XMoveResizeWindow(plugin.ob_display, self->window, self->area.x,
            self->area.y, self->area.width, self->area.height);
    /* we delay re-rendering until after we're done animating */
    framerender_frame(self);
    XFlush(plugin.ob_display);
}

void frame_begin_iconify_animation(gpointer _self, gboolean iconifying)
{
    ObConceptFrame * self = (ObConceptFrame *) _self;
    gulong time;
    gboolean new_anim = FALSE;
    gboolean set_end = TRUE;
    GTimeVal now;

    /* if there is no titlebar, just don't animate for now
     XXX it would be nice tho.. */
    if (!(self->decorations & OB_FRAME_DECOR_TITLEBAR))
        return;

    /* get the current time */
    g_get_current_time(&now);

    /* get how long until the end */
    time = FRAME_ANIMATE_ICONIFY_TIME;
    if (self->iconify_animation_going) {
        if (!!iconifying != (self->iconify_animation_going > 0)) {
            /* animation was already going on in the opposite direction */
            time = time - frame_animate_iconify_time_left(self, &now);
        }
        else
            /* animation was already going in the same direction */
            set_end = FALSE;
    }
    else
        new_anim = TRUE;
    self->iconify_animation_going = iconifying ? 1 : -1;

    /* set the ending time */
    if (set_end) {
        self->iconify_animation_end.tv_sec = now.tv_sec;
        self->iconify_animation_end.tv_usec = now.tv_usec;
        g_time_val_add(&self->iconify_animation_end, time);
    }

    if (new_anim) {
        ob_main_loop_timeout_remove_data(plugin.ob_main_loop,
                frame_animate_iconify, self, FALSE);
        ob_main_loop_timeout_add(plugin.ob_main_loop,
        FRAME_ANIMATE_ICONIFY_STEP_TIME, frame_animate_iconify, self,
                g_direct_equal, NULL);

        /* do the first step */
        frame_animate_iconify(self);

        /* show it during the animation even if it is not "visible" */
        if (!self->visible)
            XMapWindow(plugin.ob_display, self->window);
    }
}

gboolean frame_iconify_animating(gpointer _self)
{
    ObConceptFrame * self = (ObConceptFrame *) _self;
    return self->iconify_animation_going != 0;
}

ObFramePlugin plugin = { 0, //gpointer handler;
        "libdefault.la", //gchar * filename;
        "Default", //gchar * name;
        init, //gint (*init) (Display * display, gint screen);
        0, // release
        frame_new, //gpointer (*frame_new) (struct _ObClient *c);
        frame_free, //void (*frame_free) (gpointer self);
        frame_show, //void (*frame_show) (gpointer self);
        frame_hide, //void (*frame_hide) (gpointer self);
        frame_adjust_theme, //void (*frame_adjust_theme) (gpointer self);
        frame_adjust_shape, //void (*frame_adjust_shape) (gpointer self);
        frame_adjust_area, //void (*frame_adjust_area) (gpointer self, gboolean moved, gboolean resized, gboolean fake);
        frame_adjust_client_area, //void (*frame_adjust_client_area) (gpointer self);
        frame_adjust_state, //void (*frame_adjust_state) (gpointer self);
        frame_adjust_focus, //void (*frame_adjust_focus) (gpointer self, gboolean hilite);
        frame_adjust_title, //void (*frame_adjust_title) (gpointer self);
        frame_adjust_icon, //void (*frame_adjust_icon) (gpointer self);
        frame_grab_client, //void (*frame_grab_client) (gpointer self);
        frame_release_client, //void (*frame_release_client) (gpointer self);
        frame_context, //ObFrameContext (*frame_context) (struct _ObClient *self, Window win, gint x, gint y);
        frame_client_gravity, //void (*frame_client_gravity) (gpointer self, gint *x, gint *y);
        frame_frame_gravity, //void (*frame_frame_gravity) (gpointer self, gint *x, gint *y);
        frame_rect_to_frame, //void (*frame_rect_to_frame) (gpointer self, Rect *r);
        frame_rect_to_client, //void (*frame_rect_to_client) (gpointer self, Rect *r);
        frame_flash_start, //void (*frame_flash_start) (gpointer self);
        frame_flash_stop, //void (*frame_flash_stop) (gpointer self);
        frame_begin_iconify_animation, //void (*frame_begin_iconify_animation) (gpointer self, gboolean iconifying);
        frame_end_iconify_animation, //void (*frame_end_iconify_animation) (gpointer self);
        frame_iconify_animating, // gboolean (*frame_iconify_animating)(gpointer p);
        load_theme_config,

        /* This fields are fill by openbox. */
        0, //Display * ob_display;
        0, //gint ob_screen;
        0, //RrInstance *ob_rr_inst;
        0, //gboolean config_theme_keepborder;
        0, //struct _ObClient *focus_cycle_target;
        0, //gchar *config_title_layout;
        FALSE, //gboolean moveresize_in_progress;
        0, //struct _ObMainLoop *ob_main_loop;
};

ObFramePlugin * get_info()
{
    return &plugin;
}
