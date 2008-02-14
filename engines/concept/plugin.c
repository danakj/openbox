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

#include "openbox/client.h"
#include "openbox/openbox.h"
#include "openbox/prop.h"
#include "openbox/grab.h"
#include "openbox/config.h"
#include "obt/mainloop.h"
#include "openbox/focus_cycle.h"
#include "openbox/focus_cycle_indicator.h"
#include "openbox/moveresize.h"
#include "openbox/screen.h"
#include "render/theme.h"

#include "plugin.h"

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

static gulong frame_animate_iconify_time_left(gpointer _self,
        const GTimeVal *now);

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

/* Create a frame */
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

    self->title = createWindow(self->window, NULL, mask, &attrib);

    self->top = createWindow(self->window, NULL, mask, &attrib);
    self->bottom = createWindow(self->window, NULL, mask, &attrib);
    self->left = createWindow(self->window, NULL, mask, &attrib);
    self->right = createWindow(self->window, NULL, mask, &attrib);

    self->top_left = createWindow(self->window, NULL, mask, &attrib);
    self->top_right = createWindow(self->window, NULL, mask, &attrib);
    self->bottom_left = createWindow(self->window, NULL, mask, &attrib);
    self->bottom_right = createWindow(self->window, NULL, mask, &attrib);

    XMapWindow(plugin.ob_display, self->background);

    self->focused = FALSE;

    self->hover_flag = OB_BUTTON_NONE;
    self->press_flag = OB_BUTTON_NONE;

    set_theme_statics(self);

    return self;
}

void set_theme_statics(gpointer _self)
{
    ObConceptFrame * self = (ObConceptFrame *) _self;
    /* do this before changing the frame's status like max_horz max_vert */

    XResizeWindow(plugin.ob_display, self->top_left, 5, 5);
    XResizeWindow(plugin.ob_display, self->top_right, 5, 5);
    XResizeWindow(plugin.ob_display, self->bottom_left, 5, 5);
    XResizeWindow(plugin.ob_display, self->bottom_right, 5, 5);
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

gint frame_hide(gpointer self)
{
    if (OBCONCEPTFRAME(self)->visible) {
        OBCONCEPTFRAME(self)->visible = FALSE;
        if (!frame_iconify_animating(self))
            XUnmapWindow(plugin.ob_display, OBCONCEPTFRAME(self)->window);
        /* we unmap the client itself so that we can get MapRequest
         events, and because the ICCCM tells us to! */
        XUnmapWindow(plugin.ob_display, OBCONCEPTFRAME(self)->client->window);
        /* We ignore 1 unmap */
        return 1;
    }
    else
        return 0;
}

void frame_adjust_theme(gpointer self)
{
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
            xrect[0].width = self->window_area.width;
            xrect[0].height = self->size.top;
            ++num;
        }

        XShapeCombineRectangles(plugin.ob_display, self->window,
                ShapeBounding, 0, 0, xrect, num,
                ShapeUnion, Unsorted);
    }
#endif
}

void frame_grab(gpointer _self, GHashTable * map)
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

    g_hash_table_insert(map, &self->title, self->client);

    g_hash_table_insert(map, &self->left, self->client);
    g_hash_table_insert(map, &self->right, self->client);
    g_hash_table_insert(map, &self->top, self->client);
    g_hash_table_insert(map, &self->bottom, self->client);

    g_hash_table_insert(map, &self->top_left, self->client);
    g_hash_table_insert(map, &self->top_right, self->client);
    g_hash_table_insert(map, &self->bottom_left, self->client);
    g_hash_table_insert(map, &self->bottom_right, self->client);

}

void frame_ungrab(gpointer _self, GHashTable * map)
{
    ObConceptFrame * self = (ObConceptFrame *) _self;
    XEvent ev;
    gboolean reparent = TRUE;

    /* if there was any animation going on, kill it */
    obt_main_loop_timeout_remove_data(plugin.ob_main_loop,
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

    g_hash_table_remove(map, &self->title);

    g_hash_table_remove(map, &self->left);
    g_hash_table_remove(map, &self->right);
    g_hash_table_remove(map, &self->top);
    g_hash_table_remove(map, &self->bottom);

    g_hash_table_remove(map, &self->top_left);
    g_hash_table_remove(map, &self->top_right);
    g_hash_table_remove(map, &self->bottom_left);
    g_hash_table_remove(map, &self->bottom_right);

    obt_main_loop_timeout_remove_data(plugin.ob_main_loop, flash_timeout, self,
            TRUE);
}

ObFrameContext frame_context(gpointer _self, Window win, gint x, gint y)
{
    /* Here because client can be NULL */
    ObConceptFrame *self = OBCONCEPTFRAME(_self);

    if (self->shaded)
        return OB_FRAME_CONTEXT_TITLEBAR;
    if (win == self->title)
        return OB_FRAME_CONTEXT_TITLEBAR;

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

    if (win == self->left)
        return OB_FRAME_CONTEXT_LEFT;
    if (win == self->right)
        return OB_FRAME_CONTEXT_RIGHT;

    return OB_FRAME_CONTEXT_NONE;
}

void frame_set_is_visible(gpointer self, gboolean b)
{
    OBCONCEPTFRAME(self)->visible = b;
    if (b) {
        OBCONCEPTFRAME(self)->frame_stase_flags |= OB_FRAME_STASE_IS_VISIBLE;
    }
    else {
        OBCONCEPTFRAME(self)->frame_stase_flags &= ~OB_FRAME_STASE_IS_VISIBLE;
    }
}

void frame_set_is_focus(gpointer self, gboolean b)
{
    OBCONCEPTFRAME(self)->focused = b;
    if (b) {
        OBCONCEPTFRAME(self)->frame_stase_flags |= OB_FRAME_STASE_IS_FOCUS;
    }
    else {
        OBCONCEPTFRAME(self)->frame_stase_flags &= ~OB_FRAME_STASE_IS_FOCUS;
    }
}

void frame_set_is_max_vert(gpointer self, gboolean b)
{
    OBCONCEPTFRAME(self)->max_vert = b;
    if (b) {
        OBCONCEPTFRAME(self)->frame_stase_flags |= OB_FRAME_STASE_IS_MAX_VERT;
    }
    else {
        OBCONCEPTFRAME(self)->frame_stase_flags &= ~OB_FRAME_STASE_IS_MAX_VERT;
    }
}

void frame_set_is_max_horz(gpointer self, gboolean b)
{
    OBCONCEPTFRAME(self)->max_horz = b;
    if (b) {
        OBCONCEPTFRAME(self)->frame_stase_flags |= OB_FRAME_STASE_IS_MAX_HORZ;
    }
    else {
        OBCONCEPTFRAME(self)->frame_stase_flags &= ~OB_FRAME_STASE_IS_MAX_HORZ;
    }
}

void frame_set_is_shaded(gpointer self, gboolean b)
{
    OBCONCEPTFRAME(self)->shaded = b;
    if (b) {
        OBCONCEPTFRAME(self)->frame_stase_flags |= OB_FRAME_STASE_IS_SHADED;
    }
    else {
        OBCONCEPTFRAME(self)->frame_stase_flags &= ~OB_FRAME_STASE_IS_SHADED;
    }
}

void frame_unfocus(gpointer self)
{
    OBCONCEPTFRAME(self)->focused = FALSE;
}

void frame_flash_start(gpointer _self)
{
    ObConceptFrame * self = (ObConceptFrame *) _self;
    self->flash_on = self->focused;

    if (!self->flashing)
        obt_main_loop_timeout_add(plugin.ob_main_loop, G_USEC_PER_SEC * 0.6,
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
        obt_main_loop_timeout_remove_data(plugin.ob_main_loop,
                frame_animate_iconify, self, FALSE);
        obt_main_loop_timeout_add(plugin.ob_main_loop,
        FRAME_ANIMATE_ICONIFY_STEP_TIME, frame_animate_iconify, self,
                g_direct_equal, NULL);

        /* do the first step */
        frame_animate_iconify(self);

        /* show it during the animation even if it is not "visible" */
        if (!self->visible)
            XMapWindow(plugin.ob_display, self->window);
    }
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

    XMoveResizeWindow(plugin.ob_display, self->window, self->window_area.x,
            self->window_area.y, self->window_area.width,
            self->window_area.height);
    /* we delay re-rendering until after we're done animating */
    frame_update_skin(self);
    XFlush(plugin.ob_display);
}

gboolean frame_iconify_animating(gpointer _self)
{
    ObConceptFrame * self = (ObConceptFrame *) _self;
    return self->iconify_animation_going != 0;
}

void frame_set_decorations(gpointer self, ObFrameDecorations d)
{
    OBCONCEPTFRAME(self)->decorations = d;
}

Rect frame_get_window_area(gpointer self)
{
    return OBCONCEPTFRAME(self)->window_area;
}
void frame_set_client_area(gpointer self, Rect r)
{
    OBCONCEPTFRAME(self)->client_area = r;
}

void frame_update_layout(gpointer _self, gboolean is_resize, gboolean is_fake)
{
    ObConceptFrame * self = (ObConceptFrame *) _self;

    /* do this before changing the frame's status like max_horz max_vert */
    frame_adjust_cursors(self);

    if (self->decorations && !self->shaded) {
        self->cbwidth_l = 5;
        self->cbwidth_r = 5;
        self->cbwidth_t = 5;
        self->cbwidth_b = 5;

        self->title_width = 20;

        STRUT_SET(self->size, self->cbwidth_l, self->cbwidth_t, self->cbwidth_r
                + self->title_width, self->cbwidth_b);

        RECT_SET_SIZE(self->window_area, self->client_area.width
                + self->size.left + self->size.right, self->client_area.height
                + self->size.top + self->size.bottom);

        if (!is_fake) {

            XMoveResizeWindow(plugin.ob_display, self->top, 5, 0,
                    self->window_area.width - 10, theme_config.border_width);
            XMapWindow(plugin.ob_display, self->top);

            XMoveResizeWindow(plugin.ob_display, self->bottom, 5,
                    self->window_area.height - theme_config.border_width,
                    self->window_area.width - 10, theme_config.border_width);
            XMapWindow(plugin.ob_display, self->bottom);

            XMoveResizeWindow(plugin.ob_display, self->left, 0,
                    theme_config.border_width, theme_config.border_width,
                    self->window_area.height - 2*theme_config.border_width);
            XMapWindow(plugin.ob_display, self->left);

            XMoveResizeWindow(plugin.ob_display, self->right,
                    self->window_area.width - theme_config.border_width,
                    theme_config.border_width, theme_config.border_width,
                    self->window_area.height - 2 *theme_config.border_width);
            XMapWindow(plugin.ob_display, self->right);

            XMoveWindow(plugin.ob_display, self->top_left, 0, 0);
            XMapWindow(plugin.ob_display, self->top_left);
            XMoveWindow(plugin.ob_display, self->top_right,
                    self->window_area.width - 5, 0);
            XMapWindow(plugin.ob_display, self->top_right);
            XMoveWindow(plugin.ob_display, self->bottom_left, 0,
                    self->window_area.height - theme_config.border_width);
            XMapWindow(plugin.ob_display, self->bottom_left);
            XMoveWindow(plugin.ob_display, self->bottom_right,
                    self->window_area.width - 5, self->window_area.height
                            - theme_config.border_width);
            XMapWindow(plugin.ob_display, self->bottom_right);

            XMoveResizeWindow(plugin.ob_display, self->title,
                    self->window_area.width - theme_config.border_width
                            - self->title_width, theme_config.border_width,
                    self->title_width, self->window_area.height - 2
                            *theme_config.border_width);
            XMapWindow(plugin.ob_display, self->title);

            /* find the new coordinates, done after setting the frame.size, for
             frame_client_gravity. */
            self->window_area.x = self->client_area.x;
            self->window_area.y = self->client_area.y;
            frame_client_gravity(self->client, &self->window_area.x,
                    &self->window_area.y);

            XMoveResizeWindow(plugin.ob_display, self->background,
                    theme_config.border_width, theme_config.border_width,
                    self->window_area.width - 2 * theme_config.border_width
                            - self->title_width, self->window_area.height - 2
                            * theme_config.border_width);
            XMapWindow(plugin.ob_display, self->background);

            if (!is_resize) {
                XMoveResizeWindow(plugin.ob_display, self->client->window,
                        self->size.left, self->size.top,
                        self->window_area.width - self->size.left
                                - self->size.right, self->window_area.height
                                - self->size.top - self->size.bottom);
            }
            XMoveResizeWindow(plugin.ob_display, self->window,
                    self->window_area.x, self->window_area.y,
                    self->window_area.width, self->window_area.height);

        }
    }
    else if (self->shaded) {
        self->cbwidth_l = 0;
        self->cbwidth_r = 0;
        self->cbwidth_b = 0;
        self->cbwidth_t = 0;

        self->title_width = 20;

        STRUT_SET(self->size, self->cbwidth_l, self->cbwidth_t, self->cbwidth_r
                + self->title_width, self->cbwidth_b);

        RECT_SET_SIZE(self->window_area, 30, 30);

        /* find the new coordinates, done after setting the frame.size, for
         frame_client_gravity. */
        frame_client_gravity(self->client, &self->window_area.x,
                &self->window_area.y);

        if (!is_fake) {
            XUnmapWindow(plugin.ob_display, self->top);
            XUnmapWindow(plugin.ob_display, self->bottom);
            XUnmapWindow(plugin.ob_display, self->left);
            XUnmapWindow(plugin.ob_display, self->right);
            XUnmapWindow(plugin.ob_display, self->top_left);
            XUnmapWindow(plugin.ob_display, self->top_right);
            XUnmapWindow(plugin.ob_display, self->bottom_left);
            XUnmapWindow(plugin.ob_display, self->bottom_right);

            XUnmapWindow(plugin.ob_display, self->title);

            XMoveResizeWindow(plugin.ob_display, self->background, 0, 0, 30, 30);
            XMapWindow(plugin.ob_display, self->background);

            XMoveWindow(plugin.ob_display, self->window, 35, 35);
            XResizeWindow(plugin.ob_display, self->window, 30, 30);
        }
    }
    else // No decord :)
    {
        self->cbwidth_l = 0;
        self->cbwidth_r = 0;
        self->cbwidth_b = 0;
        self->cbwidth_t = 0;
        STRUT_SET(self->size, self->cbwidth_l, self->cbwidth_t,
                self->cbwidth_r, self->cbwidth_b);

        RECT_SET_SIZE(self->window_area, self->client->area.width
                + self->size.left + self->size.right, self->client->area.height
                + self->size.top + self->size.bottom);

        /* find the new coordinates, done after setting the frame.size, for
         frame_client_gravity. */
        self->window_area.x = self->client_area.x;
        self->window_area.y = self->client_area.y;
        frame_client_gravity(self->client, &self->window_area.x,
                &self->window_area.y);

        if (!is_fake) {
            XUnmapWindow(plugin.ob_display, self->top);
            XUnmapWindow(plugin.ob_display, self->bottom);
            XUnmapWindow(plugin.ob_display, self->left);
            XUnmapWindow(plugin.ob_display, self->right);
            XUnmapWindow(plugin.ob_display, self->top_left);
            XUnmapWindow(plugin.ob_display, self->top_right);
            XUnmapWindow(plugin.ob_display, self->bottom_left);
            XUnmapWindow(plugin.ob_display, self->bottom_right);

            XUnmapWindow(plugin.ob_display, self->title);

            XMoveResizeWindow(plugin.ob_display, self->background, 0, 0,
                    self->window_area.width, self->window_area.height);
            XMapWindow(plugin.ob_display, self->background);
            if (!is_resize) {
                XMoveResizeWindow(plugin.ob_display, self->client->window,
                        self->size.left, self->size.top,
                        self->window_area.width, self->window_area.height);
            }
            XMoveResizeWindow(plugin.ob_display, self->window,
                    self->window_area.x, self->window_area.y,
                    self->window_area.width, self->window_area.height);
        }
    }
}

void frame_update_skin(gpointer _self)
{
    ObConceptFrame * self = (ObConceptFrame *) _self;
    if (plugin.frame_iconify_animating(self))
        return; /* delay redrawing until the animation is done */

    if (!self->visible)
        return;
    self->need_render = FALSE;

    gulong border_px, corner_px;

    if (self->focused) {
        border_px = RrColorPixel(theme_config.focus_border_color);
        corner_px = RrColorPixel(theme_config.focus_corner_color);
        XSetWindowBackgroundPixmap(plugin.ob_display, self->left,
                theme_config.px_focus_left);
        XClearWindow(plugin.ob_display, self->left);
        XSetWindowBackgroundPixmap(plugin.ob_display, self->right,
                theme_config.px_focus_right);
        XClearWindow(plugin.ob_display, self->right);

        XSetWindowBackgroundPixmap(plugin.ob_display, self->top,
                theme_config.px_focus_top);
        XClearWindow(plugin.ob_display, self->top);
        XSetWindowBackgroundPixmap(plugin.ob_display, self->bottom,
                theme_config.px_focus_bottom);
        XClearWindow(plugin.ob_display, self->bottom);

        XSetWindowBackgroundPixmap(plugin.ob_display, self->top_left,
                theme_config.px_focus_topleft);
        XClearWindow(plugin.ob_display, self->top_left);
        XSetWindowBackgroundPixmap(plugin.ob_display, self->top_right,
                theme_config.px_focus_topright);
        XClearWindow(plugin.ob_display, self->top_right);

        XSetWindowBackgroundPixmap(plugin.ob_display, self->bottom_left,
                theme_config.px_focus_bottomleft);
        XClearWindow(plugin.ob_display, self->bottom_left);
        XSetWindowBackgroundPixmap(plugin.ob_display, self->bottom_right,
                theme_config.px_focus_bottomright);
        XClearWindow(plugin.ob_display, self->bottom_right);

        XSetWindowBackground(plugin.ob_display, self->title, 0x00ffff);
        XClearWindow(plugin.ob_display, self->title);
        XSetWindowBackground(plugin.ob_display, self->background, 0);
        XClearWindow(plugin.ob_display, self->background);
    }
    else {
        border_px = RrColorPixel(theme_config.unfocus_border_color);
        corner_px = RrColorPixel(theme_config.unfocus_corner_color);
        XSetWindowBackgroundPixmap(plugin.ob_display, self->left,
                theme_config.px_unfocus_left);
        XClearWindow(plugin.ob_display, self->left);
        XSetWindowBackgroundPixmap(plugin.ob_display, self->right,
                theme_config.px_unfocus_right);
        XClearWindow(plugin.ob_display, self->right);

        XSetWindowBackgroundPixmap(plugin.ob_display, self->top,
                theme_config.px_unfocus_top);
        XClearWindow(plugin.ob_display, self->top);
        XSetWindowBackgroundPixmap(plugin.ob_display, self->bottom,
                theme_config.px_unfocus_bottom);
        XClearWindow(plugin.ob_display, self->bottom);

        XSetWindowBackgroundPixmap(plugin.ob_display, self->top_left,
                theme_config.px_unfocus_topleft);
        XClearWindow(plugin.ob_display, self->top_left);
        XSetWindowBackgroundPixmap(plugin.ob_display, self->top_right,
                theme_config.px_unfocus_topright);
        XClearWindow(plugin.ob_display, self->top_right);

        XSetWindowBackgroundPixmap(plugin.ob_display, self->bottom_left,
                theme_config.px_unfocus_bottomleft);
        XClearWindow(plugin.ob_display, self->bottom_left);
        XSetWindowBackgroundPixmap(plugin.ob_display, self->bottom_right,
                theme_config.px_unfocus_bottomright);
        XClearWindow(plugin.ob_display, self->bottom_right);

        XSetWindowBackground(plugin.ob_display, self->title, 0x00ffff);
        XClearWindow(plugin.ob_display, self->title);
        XSetWindowBackground(plugin.ob_display, self->background, 0);
        XClearWindow(plugin.ob_display, self->background);
    }
    XFlush(plugin.ob_display);
}

void frame_set_hover_flag(gpointer self, ObFrameButton button)
{
    if (OBCONCEPTFRAME(self)->hover_flag != button) {
        OBCONCEPTFRAME(self)->hover_flag = button;
        frame_update_skin(self);
    }
}

void frame_set_press_flag(gpointer self, ObFrameButton button)
{
    if (OBCONCEPTFRAME(self)->press_flag != button) {
        OBCONCEPTFRAME(self)->press_flag = button;
        frame_update_skin(self);
    }
}

Window frame_get_window(gpointer self)
{
    return OBCONCEPTFRAME(self)->window;
}

Strut frame_get_size(gpointer self)
{
    return OBCONCEPTFRAME(self)->size;
}

gint frame_get_decorations(gpointer self)
{
    return OBCONCEPTFRAME(self)->decorations;
}

gboolean frame_is_visible(gpointer self)
{
    return OBCONCEPTFRAME(self)->visible;
}

gboolean frame_is_max_horz(gpointer self)
{
    return OBCONCEPTFRAME(self)->max_horz;
}

gboolean frame_is_max_vert(gpointer self)
{
    return OBCONCEPTFRAME(self)->max_vert;
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
        Rect
                *a =
                        screen_physical_area_monitor(screen_find_monitor(&self->window_area));
        iconx = self->window_area.x + self->window_area.width / 2 + 32;
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
        x = self->window_area.x;
        y = self->window_area.y;
        w = self->window_area.width;
        h = self->window_area.height;
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

        dx = self->window_area.x - iconx;
        dy = self->window_area.y - icony;
        dw = self->window_area.width - iconw;
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

/* change the cursor */
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
    frame_update_skin(self);
}

void frame_adjust_focus(gpointer _self, gboolean hilite)
{
    ObConceptFrame * self = (ObConceptFrame *) _self;
    self->focused = hilite;
    self->need_render = TRUE;
    frame_update_skin(self);
    XFlush(plugin.ob_display);
}

void frame_adjust_title(gpointer _self)
{
    ObConceptFrame * self = (ObConceptFrame *) _self;
    self->need_render = TRUE;
    frame_update_skin(self);
}

void frame_adjust_icon(gpointer _self)
{
    ObConceptFrame * self = (ObConceptFrame *) _self;
    self->need_render = TRUE;
    frame_update_skin(self);
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

ObFramePlugin plugin = { 0, //gpointer handler;
        "libconcept.la", //gchar * filename;
        "Concept", //gchar * name;
        init, //gint (*init) (Display * display, gint screen);
        0, frame_new, //gpointer (*frame_new) (struct _ObClient *c);
        frame_free, //void (*frame_free) (gpointer self);
        frame_show, //void (*frame_show) (gpointer self);
        frame_hide, //void (*frame_hide) (gpointer self);
        frame_adjust_theme, //void (*frame_adjust_theme) (gpointer self);
        frame_adjust_shape, //void (*frame_adjust_shape) (gpointer self);
        frame_grab, //void (*frame_adjust_area) (gpointer self, gboolean moved, gboolean resized, gboolean fake);
        frame_ungrab, frame_context, //void (*frame_adjust_state) (gpointer self);
        frame_set_is_visible, /* */
        frame_set_is_focus, /* */
        frame_set_is_max_vert, /* */
        frame_set_is_max_horz, /* */
        frame_set_is_shaded, /* */
        frame_flash_start, /* */
        frame_flash_stop, /* */
        frame_begin_iconify_animation, /* */
        frame_end_iconify_animation, /* */
        frame_iconify_animating, /* */

        frame_set_decorations, /* */
        /* This give the window area */
        frame_get_window_area, /* */
        frame_set_client_area, /* */
        /* Draw the frame */
        frame_update_layout, /* */
        frame_update_skin, /* */

        frame_set_hover_flag, /* */
        frame_set_press_flag, /* */

        frame_get_window, /* */
 
        frame_get_size, /* */
        frame_get_decorations, /* */

        frame_is_visible, /* */
        frame_is_max_horz, frame_is_max_vert, /* */
        
        NULL, /* */

        load_theme_config, /* */

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
