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
#include "obt/prop.h"
#include "openbox/grab.h"
#include "openbox/config.h"
#include "obt/mainloop.h"
#include "openbox/focus_cycle.h"
#include "openbox/focus_cycle_indicator.h"
#include "openbox/moveresize.h"
#include "openbox/screen.h"
#include "render/theme.h"

#include "plugin.h"
#include "render.h"

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

#define FRAME_HANDLE_Y(f) (f->size.top + f->client_area.height + f->cbwidth_b)

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
    ObDefaultFrame *self;
    Visual *visual;

    self = g_new0(ObDefaultFrame, 1);
    self->client = client;

    visual = check_32bit_client(client);

    /* create the non-visible decor windows */

    mask = 0;
    if (visual) {
        /* client has a 32-bit visual */
        mask |= CWColormap | CWBackPixel | CWBorderPixel;
        /* create a colormap with the visual */
        OBDEFAULTFRAME(self)->colormap = attrib.colormap = XCreateColormap(
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

    self->backback = createWindow(self->window, NULL, mask, &attrib);
    self->backfront = createWindow(self->backback, NULL, mask, &attrib);

    mask |= CWEventMask;
    attrib.event_mask = ELEMENT_EVENTMASK;
    self->innerleft = createWindow(self->window, NULL, mask, &attrib);
    self->innertop = createWindow(self->window, NULL, mask, &attrib);
    self->innerright = createWindow(self->window, NULL, mask, &attrib);
    self->innerbottom = createWindow(self->window, NULL, mask, &attrib);

    self->innerblb = createWindow(self->innerbottom, NULL, mask, &attrib);
    self->innerbrb = createWindow(self->innerbottom, NULL, mask, &attrib);
    self->innerbll = createWindow(self->innerleft, NULL, mask, &attrib);
    self->innerbrr = createWindow(self->innerright, NULL, mask, &attrib);

    self->title = createWindow(self->window, NULL, mask, &attrib);
    self->titleleft = createWindow(self->window, NULL, mask, &attrib);
    self->titletop = createWindow(self->window, NULL, mask, &attrib);
    self->titletopleft = createWindow(self->window, NULL, mask, &attrib);
    self->titletopright = createWindow(self->window, NULL, mask, &attrib);
    self->titleright = createWindow(self->window, NULL, mask, &attrib);
    self->titlebottom = createWindow(self->window, NULL, mask, &attrib);

    self->topresize = createWindow(self->title, NULL, mask, &attrib);
    self->tltresize = createWindow(self->title, NULL, mask, &attrib);
    self->tllresize = createWindow(self->title, NULL, mask, &attrib);
    self->trtresize = createWindow(self->title, NULL, mask, &attrib);
    self->trrresize = createWindow(self->title, NULL, mask, &attrib);

    self->left = createWindow(self->window, NULL, mask, &attrib);
    self->right = createWindow(self->window, NULL, mask, &attrib);

    self->label = createWindow(self->title, NULL, mask, &attrib);
    self->max = createWindow(self->title, NULL, mask, &attrib);
    self->close = createWindow(self->title, NULL, mask, &attrib);
    self->desk = createWindow(self->title, NULL, mask, &attrib);
    self->shade = createWindow(self->title, NULL, mask, &attrib);
    self->icon = createWindow(self->title, NULL, mask, &attrib);
    self->iconify = createWindow(self->title, NULL, mask, &attrib);

    self->handle = createWindow(self->window, NULL, mask, &attrib);
    self->lgrip = createWindow(self->handle, NULL, mask, &attrib);
    self->rgrip = createWindow(self->handle, NULL, mask, &attrib);

    self->handleleft = createWindow(self->handle, NULL, mask, &attrib);
    self->handleright = createWindow(self->handle, NULL, mask, &attrib);

    self->handletop = createWindow(self->window, NULL, mask, &attrib);
    self->handlebottom = createWindow(self->window, NULL, mask, &attrib);
    self->lgripleft = createWindow(self->window, NULL, mask, &attrib);
    self->lgriptop = createWindow(self->window, NULL, mask, &attrib);
    self->lgripbottom = createWindow(self->window, NULL, mask, &attrib);
    self->rgripright = createWindow(self->window, NULL, mask, &attrib);
    self->rgriptop = createWindow(self->window, NULL, mask, &attrib);
    self->rgripbottom = createWindow(self->window, NULL, mask, &attrib);

    self->stitle = g_strdup("");
    self->focused = FALSE;

    /* the other stuff is shown based on decor settings */
    XMapWindow(plugin.ob_display, self->label);
    XMapWindow(plugin.ob_display, self->backback);
    XMapWindow(plugin.ob_display, self->backfront);

    self->hover_flag = OB_BUTTON_NONE;
    self->press_flag = OB_BUTTON_NONE;

    set_theme_statics(self);

    return self;
}

void set_theme_statics(gpointer _self)
{
    ObDefaultFrame * self = (ObDefaultFrame *) _self;
    /* set colors/appearance/sizes for stuff that doesn't change */
    XResizeWindow(plugin.ob_display, self->max, theme_config.button_size,
            theme_config.button_size);
    XResizeWindow(plugin.ob_display, self->iconify, theme_config.button_size,
            theme_config.button_size);
    XResizeWindow(plugin.ob_display, self->icon, theme_config.button_size + 2,
            theme_config.button_size + 2);
    XResizeWindow(plugin.ob_display, self->close, theme_config.button_size,
            theme_config.button_size);
    XResizeWindow(plugin.ob_display, self->desk, theme_config.button_size,
            theme_config.button_size);
    XResizeWindow(plugin.ob_display, self->shade, theme_config.button_size,
            theme_config.button_size);
    XResizeWindow(plugin.ob_display, self->tltresize, theme_config.grip_width,
            theme_config.paddingy + 1);
    XResizeWindow(plugin.ob_display, self->trtresize, theme_config.grip_width,
            theme_config.paddingy + 1);
    XResizeWindow(plugin.ob_display, self->tllresize,
            theme_config.paddingx + 1, theme_config.title_height);
    XResizeWindow(plugin.ob_display, self->trrresize,
            theme_config.paddingx + 1, theme_config.title_height);

    /* set up the dynamic appearances */
    self->a_unfocused_title = RrAppearanceCopy(theme_config.a_unfocused_title);
    self->a_focused_title = RrAppearanceCopy(theme_config.a_focused_title);
    self->a_unfocused_label = RrAppearanceCopy(theme_config.a_unfocused_label);
    self->a_focused_label = RrAppearanceCopy(theme_config.a_focused_label);
    self->a_unfocused_handle
            = RrAppearanceCopy(theme_config.a_unfocused_handle);
    self->a_focused_handle = RrAppearanceCopy(theme_config.a_focused_handle);
    self->a_icon = RrAppearanceCopy(theme_config.a_icon);
}

void free_theme_statics(gpointer _self)
{
    ObDefaultFrame * self = (ObDefaultFrame *) _self;
    RrAppearanceFree(self->a_unfocused_title);
    RrAppearanceFree(self->a_focused_title);
    RrAppearanceFree(self->a_unfocused_label);
    RrAppearanceFree(self->a_focused_label);
    RrAppearanceFree(self->a_unfocused_handle);
    RrAppearanceFree(self->a_focused_handle);
    RrAppearanceFree(self->a_icon);
}

void frame_free(gpointer self)
{
    free_theme_statics(OBDEFAULTFRAME(self));
    XDestroyWindow(plugin.ob_display, OBDEFAULTFRAME(self)->window);
    if (OBDEFAULTFRAME(self)->colormap)
        XFreeColormap(plugin.ob_display, OBDEFAULTFRAME(self)->colormap);
    
    g_free(OBDEFAULTFRAME(self)->stitle);
    g_free(self);
}

void frame_show(gpointer _self)
{
    ObDefaultFrame * self = (ObDefaultFrame *) _self;
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
    if (OBDEFAULTFRAME(self)->visible) {
        OBDEFAULTFRAME(self)->visible = FALSE;
        if (!frame_iconify_animating(self))
            XUnmapWindow(plugin.ob_display, OBDEFAULTFRAME(self)->window);
        /* we unmap the client itself so that we can get MapRequest
         events, and because the ICCCM tells us to! */
        XUnmapWindow(plugin.ob_display, OBDEFAULTFRAME(self)->client->window);
        return 1;
    }
    else {
        return 0;
    }
}

void frame_adjust_theme(gpointer self)
{
    free_theme_statics(self);
    set_theme_statics(self);
}

void frame_adjust_shape(gpointer _self)
{
#ifdef SHAPE
    ObDefaultFrame * self = (ObDefaultFrame *) _self;
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
        if (self->decorations & OB_FRAME_DECOR_TITLEBAR)
        {
            xrect[0].x = 0;
            xrect[0].y = 0;
            xrect[0].width = self->area.width;
            xrect[0].height = self->size.top;
            ++num;
        }

        if (self->decorations & OB_FRAME_DECOR_HANDLE &&
                theme_config.handle_height> 0)
        {
            xrect[1].x = 0;
            xrect[1].y = FRAME_HANDLE_Y(self);
            xrect[1].width = self->area.width;
            xrect[1].height = theme_config.handle_height +
            self->bwidth * 2;
            ++num;
        }

        XShapeCombineRectangles(plugin.ob_display, self->window,
                ShapeBounding, 0, 0, xrect, num,
                ShapeUnion, Unsorted);
    }
#endif
}

void frame_grab(gpointer _self, GHashTable * window_map)
{
    ObDefaultFrame * self = (ObDefaultFrame *) _self;
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
    g_hash_table_insert(window_map, &self->window, self->client);
    g_hash_table_insert(window_map, &self->backback, self->client);
    g_hash_table_insert(window_map, &self->backfront, self->client);
    g_hash_table_insert(window_map, &self->innerleft, self->client);
    g_hash_table_insert(window_map, &self->innertop, self->client);
    g_hash_table_insert(window_map, &self->innerright, self->client);
    g_hash_table_insert(window_map, &self->innerbottom, self->client);
    g_hash_table_insert(window_map, &self->title, self->client);
    g_hash_table_insert(window_map, &self->label, self->client);
    g_hash_table_insert(window_map, &self->max, self->client);
    g_hash_table_insert(window_map, &self->close, self->client);
    g_hash_table_insert(window_map, &self->desk, self->client);
    g_hash_table_insert(window_map, &self->shade, self->client);
    g_hash_table_insert(window_map, &self->icon, self->client);
    g_hash_table_insert(window_map, &self->iconify, self->client);
    g_hash_table_insert(window_map, &self->handle, self->client);
    g_hash_table_insert(window_map, &self->lgrip, self->client);
    g_hash_table_insert(window_map, &self->rgrip, self->client);
    g_hash_table_insert(window_map, &self->topresize, self->client);
    g_hash_table_insert(window_map, &self->tltresize, self->client);
    g_hash_table_insert(window_map, &self->tllresize, self->client);
    g_hash_table_insert(window_map, &self->trtresize, self->client);
    g_hash_table_insert(window_map, &self->trrresize, self->client);
    g_hash_table_insert(window_map, &self->left, self->client);
    g_hash_table_insert(window_map, &self->right, self->client);
    g_hash_table_insert(window_map, &self->titleleft, self->client);
    g_hash_table_insert(window_map, &self->titletop, self->client);
    g_hash_table_insert(window_map, &self->titletopleft, self->client);
    g_hash_table_insert(window_map, &self->titletopright, self->client);
    g_hash_table_insert(window_map, &self->titleright, self->client);
    g_hash_table_insert(window_map, &self->titlebottom, self->client);
    g_hash_table_insert(window_map, &self->handleleft, self->client);
    g_hash_table_insert(window_map, &self->handletop, self->client);
    g_hash_table_insert(window_map, &self->handleright, self->client);
    g_hash_table_insert(window_map, &self->handlebottom, self->client);
    g_hash_table_insert(window_map, &self->lgripleft, self->client);
    g_hash_table_insert(window_map, &self->lgriptop, self->client);
    g_hash_table_insert(window_map, &self->lgripbottom, self->client);
    g_hash_table_insert(window_map, &self->rgripright, self->client);
    g_hash_table_insert(window_map, &self->rgriptop, self->client);
    g_hash_table_insert(window_map, &self->rgripbottom, self->client);
}

void frame_ungrab(gpointer _self, GHashTable * window_map)
{
    ObDefaultFrame * self = (ObDefaultFrame *) _self;
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
                plugin.ob_display, plugin.ob_screen), self->client_area.x,
                self->client_area.y);
    }

    /* remove all the windows for the frame from the window_map */
    g_hash_table_remove(window_map, &self->window);
    g_hash_table_remove(window_map, &self->backback);
    g_hash_table_remove(window_map, &self->backfront);
    g_hash_table_remove(window_map, &self->innerleft);
    g_hash_table_remove(window_map, &self->innertop);
    g_hash_table_remove(window_map, &self->innerright);
    g_hash_table_remove(window_map, &self->innerbottom);
    g_hash_table_remove(window_map, &self->title);
    g_hash_table_remove(window_map, &self->label);
    g_hash_table_remove(window_map, &self->max);
    g_hash_table_remove(window_map, &self->close);
    g_hash_table_remove(window_map, &self->desk);
    g_hash_table_remove(window_map, &self->shade);
    g_hash_table_remove(window_map, &self->icon);
    g_hash_table_remove(window_map, &self->iconify);
    g_hash_table_remove(window_map, &self->handle);
    g_hash_table_remove(window_map, &self->lgrip);
    g_hash_table_remove(window_map, &self->rgrip);
    g_hash_table_remove(window_map, &self->topresize);
    g_hash_table_remove(window_map, &self->tltresize);
    g_hash_table_remove(window_map, &self->tllresize);
    g_hash_table_remove(window_map, &self->trtresize);
    g_hash_table_remove(window_map, &self->trrresize);
    g_hash_table_remove(window_map, &self->left);
    g_hash_table_remove(window_map, &self->right);
    g_hash_table_remove(window_map, &self->titleleft);
    g_hash_table_remove(window_map, &self->titletop);
    g_hash_table_remove(window_map, &self->titletopleft);
    g_hash_table_remove(window_map, &self->titletopright);
    g_hash_table_remove(window_map, &self->titleright);
    g_hash_table_remove(window_map, &self->titlebottom);
    g_hash_table_remove(window_map, &self->handleleft);
    g_hash_table_remove(window_map, &self->handletop);
    g_hash_table_remove(window_map, &self->handleright);
    g_hash_table_remove(window_map, &self->handlebottom);
    g_hash_table_remove(window_map, &self->lgripleft);
    g_hash_table_remove(window_map, &self->lgriptop);
    g_hash_table_remove(window_map, &self->lgripbottom);
    g_hash_table_remove(window_map, &self->rgripright);
    g_hash_table_remove(window_map, &self->rgriptop);
    g_hash_table_remove(window_map, &self->rgripbottom);

    obt_main_loop_timeout_remove_data(plugin.ob_main_loop, flash_timeout, self,
            TRUE);
}

ObFrameContext frame_context(gpointer _self, Window win, gint x, gint y)
{
    ObDefaultFrame * self = OBDEFAULTFRAME(_self);

    /* when the user clicks in the corners of the titlebar and the client
     is fully maximized, then treat it like they clicked in the
     button that is there */
    if (self->max_horz && self->max_vert && (win == self->title || win
            == self->titletop || win == self->titleleft || win
            == self->titletopleft || win == self->titleright || win
            == self->titletopright)) {
        /* get the mouse coords in reference to the whole frame */
        gint fx = x;
        gint fy = y;

        /* these windows are down a border width from the top of the frame */
        if (win == self->title || win == self->titleleft || win
                == self->titleright)
            fy += self->bwidth;

        /* title is a border width in from the edge */
        if (win == self->title)
            fx += self->bwidth;
        /* titletop is a bit to the right */
        else if (win == self->titletop)
            fx += theme_config.grip_width + self->bwidth;
        /* titletopright is way to the right edge */
        else if (win == self->titletopright)
            fx += self->area.width - (theme_config.grip_width + self->bwidth);
        /* titleright is even more way to the right edge */
        else if (win == self->titleright)
            fx += self->area.width - self->bwidth;

        /* figure out if we're over the area that should be considered a
         button */
        if (fy < self->bwidth + theme_config.paddingy + 1
                + theme_config.button_size) {
            if (fx < (self->bwidth + theme_config.paddingx + 1
                    + theme_config.button_size)) {
                if (self->leftmost != OB_FRAME_CONTEXT_NONE)
                    return self->leftmost;
            }
            else if (fx >= (self->area.width - (self->bwidth
                    + theme_config.paddingx + 1 + theme_config.button_size))) {
                if (self->rightmost != OB_FRAME_CONTEXT_NONE)
                    return self->rightmost;
            }
        }

        /* there is no resizing maximized windows so make them the titlebar
         context */
        return OB_FRAME_CONTEXT_TITLEBAR;
    }
    else if (self->max_vert
            && (win == self->titletop || win == self->topresize))
        /* can't resize vertically when max vert */
        return OB_FRAME_CONTEXT_TITLEBAR;
    else if (self->shaded && (win == self->titletop || win == self->topresize))
        /* can't resize vertically when shaded */
        return OB_FRAME_CONTEXT_TITLEBAR;

    if (win == self->window)
        return OB_FRAME_CONTEXT_FRAME;
    if (win == self->label)
        return OB_FRAME_CONTEXT_TITLEBAR;
    if (win == self->handle)
        return OB_FRAME_CONTEXT_BOTTOM;
    if (win == self->handletop)
        return OB_FRAME_CONTEXT_BOTTOM;
    if (win == self->handlebottom)
        return OB_FRAME_CONTEXT_BOTTOM;
    if (win == self->handleleft)
        return OB_FRAME_CONTEXT_BLCORNER;
    if (win == self->lgrip)
        return OB_FRAME_CONTEXT_BLCORNER;
    if (win == self->lgripleft)
        return OB_FRAME_CONTEXT_BLCORNER;
    if (win == self->lgriptop)
        return OB_FRAME_CONTEXT_BLCORNER;
    if (win == self->lgripbottom)
        return OB_FRAME_CONTEXT_BLCORNER;
    if (win == self->handleright)
        return OB_FRAME_CONTEXT_BRCORNER;
    if (win == self->rgrip)
        return OB_FRAME_CONTEXT_BRCORNER;
    if (win == self->rgripright)
        return OB_FRAME_CONTEXT_BLCORNER;
    if (win == self->rgriptop)
        return OB_FRAME_CONTEXT_BLCORNER;
    if (win == self->rgripbottom)
        return OB_FRAME_CONTEXT_BLCORNER;
    if (win == self->title)
        return OB_FRAME_CONTEXT_TITLEBAR;
    if (win == self->titlebottom)
        return OB_FRAME_CONTEXT_TITLEBAR;
    if (win == self->titleleft)
        return OB_FRAME_CONTEXT_TLCORNER;
    if (win == self->titletopleft)
        return OB_FRAME_CONTEXT_TLCORNER;
    if (win == self->titleright)
        return OB_FRAME_CONTEXT_TRCORNER;
    if (win == self->titletopright)
        return OB_FRAME_CONTEXT_TRCORNER;
    if (win == self->titletop)
        return OB_FRAME_CONTEXT_TOP;
    if (win == self->topresize)
        return OB_FRAME_CONTEXT_TOP;
    if (win == self->tltresize)
        return OB_FRAME_CONTEXT_TLCORNER;
    if (win == self->tllresize)
        return OB_FRAME_CONTEXT_TLCORNER;
    if (win == self->trtresize)
        return OB_FRAME_CONTEXT_TRCORNER;
    if (win == self->trrresize)
        return OB_FRAME_CONTEXT_TRCORNER;
    if (win == self->left)
        return OB_FRAME_CONTEXT_LEFT;
    if (win == self->right)
        return OB_FRAME_CONTEXT_RIGHT;
    if (win == self->innertop)
        return OB_FRAME_CONTEXT_TITLEBAR;
    if (win == self->innerleft)
        return OB_FRAME_CONTEXT_LEFT;
    if (win == self->innerbottom)
        return OB_FRAME_CONTEXT_BOTTOM;
    if (win == self->innerright)
        return OB_FRAME_CONTEXT_RIGHT;
    if (win == self->max)
        return OB_FRAME_CONTEXT_MAXIMIZE;
    if (win == self->iconify)
        return OB_FRAME_CONTEXT_ICONIFY;
    if (win == self->close)
        return OB_FRAME_CONTEXT_CLOSE;
    if (win == self->icon)
        return OB_FRAME_CONTEXT_ICON;
    if (win == self->desk)
        return OB_FRAME_CONTEXT_ALLDESKTOPS;
    if (win == self->shade)
        return OB_FRAME_CONTEXT_SHADE;

    return OB_FRAME_CONTEXT_NONE;
}

void frame_set_is_visible(gpointer self, gboolean b)
{
    OBDEFAULTFRAME(self)->visible = b;
}

void frame_set_is_focus(gpointer self, gboolean b)
{
    OBDEFAULTFRAME(self)->focused = b;
}

void frame_set_is_max_vert(gpointer self, gboolean b)
{
    OBDEFAULTFRAME(self)->max_vert = b;
}

void frame_set_is_max_horz(gpointer self, gboolean b)
{
    OBDEFAULTFRAME(self)->max_horz = b;
}

void frame_set_is_shaded(gpointer self, gboolean b)
{
    OBDEFAULTFRAME(self)->shaded = b;
}

void frame_unfocus(gpointer self)
{
    OBDEFAULTFRAME(self)->focused = FALSE;
}

void frame_flash_start(gpointer _self)
{
    ObDefaultFrame * self = (ObDefaultFrame *) _self;
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
    ObDefaultFrame * self = (ObDefaultFrame *) _self;
    self->flashing = FALSE;
}

void frame_begin_iconify_animation(gpointer _self, gboolean iconifying)
{
    ObDefaultFrame * self = (ObDefaultFrame *) _self;
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
            time = time - frame_animate_iconify_time_left(_self, &now);
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
    ObDefaultFrame * self = (ObDefaultFrame *) _self;
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
    frame_update_skin(self);
    XFlush(plugin.ob_display);
}

gboolean frame_iconify_animating(gpointer _self)
{
    ObDefaultFrame * self = (ObDefaultFrame *) _self;
    return self->iconify_animation_going != 0;
}

void frame_set_decorations(gpointer self, ObFrameDecorations d)
{
    OBDEFAULTFRAME(self)->decorations = d;
}

Rect frame_get_window_area(gpointer self)
{
    return OBDEFAULTFRAME(self)->area;
}
void frame_set_client_area(gpointer self, Rect r)
{
    OBDEFAULTFRAME(self)->client_area = r;
}

void frame_update_layout(gpointer _self, gboolean is_resize, gboolean is_fake)
{
    ObDefaultFrame * self = (ObDefaultFrame *) _self;
    Strut oldsize;

    oldsize = self->size;
    self->area = self->client_area;

    /* do this before changing the frame's status like max_horz max_vert */
    frame_adjust_cursors(self);

    if (self->decorations & OB_FRAME_DECOR_BORDER
            || (plugin.config_theme_keepborder)) {
        self->bwidth = theme_config.fbwidth;
    }
    else {
        self->bwidth = 0;
    }

    if (self->decorations & OB_FRAME_DECOR_BORDER) {
        self->cbwidth_l = theme_config.cbwidthx;
        self->cbwidth_r = theme_config.cbwidthx;
        self->cbwidth_t = theme_config.cbwidthy;
        self->cbwidth_b = theme_config.cbwidthy;
    }
    else {
        self->cbwidth_l = 0;
        self->cbwidth_t = 0;
        self->cbwidth_r = 0;
        self->cbwidth_b = 0;
    }

    if (self->max_horz) {
        self->cbwidth_l = 0;
        self->cbwidth_r = 0;
        self->width = self->client_area.width;
        if (self->max_vert)
            self->cbwidth_b = 0;
    }
    else {
        self->width = self->client_area.width + self->cbwidth_l
                + self->cbwidth_r;
    }

    /* some elements are sized based of the width, so don't let them have
     negative values */
    self->width = MAX(self->width, (theme_config.grip_width + self->bwidth) * 2
            + 1);

    STRUT_SET(self->size, self->cbwidth_l
            + (!self->max_horz ? self->bwidth : 0), self->cbwidth_t
            + self->bwidth, self->cbwidth_r + (!self->max_horz ? self->bwidth
            : 0), self->cbwidth_b
            + (!self->max_horz || !self->max_vert ? self->bwidth : 0));

    if (self->decorations & OB_FRAME_DECOR_TITLEBAR)
        self->size.top += theme_config.title_height + self->bwidth;
    if (self->decorations & OB_FRAME_DECOR_HANDLE && theme_config.handle_height
            > 0) {
        self->size.bottom += theme_config.handle_height + self->bwidth;
    }

    /* position/size and map/unmap all the windows */
    if (!is_fake) {
        gint innercornerheight = theme_config.grip_width - self->size.bottom;

        if (self->cbwidth_l) {
            XMoveResizeWindow(plugin.ob_display, self->innerleft,
                    self->size.left - self->cbwidth_l, self->size.top,
                    self->cbwidth_l, self->client_area.height);

            XMapWindow(plugin.ob_display, self->innerleft);
        }
        else
            XUnmapWindow(plugin.ob_display, self->innerleft);

        if (self->cbwidth_l && innercornerheight > 0) {
            XMoveResizeWindow(plugin.ob_display, self->innerbll, 0,
                    self->client_area.height - (theme_config.grip_width
                            - self->size.bottom), self->cbwidth_l,
                    theme_config.grip_width - self->size.bottom);

            XMapWindow(plugin.ob_display, self->innerbll);
        }
        else
            XUnmapWindow(plugin.ob_display, self->innerbll);

        if (self->cbwidth_r) {
            XMoveResizeWindow(plugin.ob_display, self->innerright,
                    self->size.left + self->client_area.width, self->size.top,
                    self->cbwidth_r, self->client_area.height);

            XMapWindow(plugin.ob_display, self->innerright);
        }
        else
            XUnmapWindow(plugin.ob_display, self->innerright);

        if (self->cbwidth_r && innercornerheight > 0) {
            XMoveResizeWindow(plugin.ob_display, self->innerbrr, 0,
                    self->client_area.height - (theme_config.grip_width
                            - self->size.bottom), self->cbwidth_r,
                    theme_config.grip_width - self->size.bottom);

            XMapWindow(plugin.ob_display, self->innerbrr);
        }
        else
            XUnmapWindow(plugin.ob_display, self->innerbrr);

        if (self->cbwidth_t) {
            XMoveResizeWindow(plugin.ob_display, self->innertop,
                    self->size.left - self->cbwidth_l, self->size.top
                            - self->cbwidth_t, self->client_area.width
                            + self->cbwidth_l + self->cbwidth_r,
                    self->cbwidth_t);

            XMapWindow(plugin.ob_display, self->innertop);
        }
        else
            XUnmapWindow(plugin.ob_display, self->innertop);

        if (self->cbwidth_b) {
            XMoveResizeWindow(plugin.ob_display, self->innerbottom,
                    self->size.left - self->cbwidth_l, self->size.top
                            + self->client_area.height, self->client_area.width
                            + self->cbwidth_l + self->cbwidth_r,
                    self->cbwidth_b);

            XMoveResizeWindow(plugin.ob_display, self->innerblb, 0, 0,
                    theme_config.grip_width + self->bwidth, self->cbwidth_b);
            XMoveResizeWindow(plugin.ob_display, self->innerbrb,
                    self->client_area.width + self->cbwidth_l + self->cbwidth_r
                            - (theme_config.grip_width + self->bwidth), 0,
                    theme_config.grip_width + self->bwidth, self->cbwidth_b);

            XMapWindow(plugin.ob_display, self->innerbottom);
            XMapWindow(plugin.ob_display, self->innerblb);
            XMapWindow(plugin.ob_display, self->innerbrb);
        }
        else {
            XUnmapWindow(plugin.ob_display, self->innerbottom);
            XUnmapWindow(plugin.ob_display, self->innerblb);
            XUnmapWindow(plugin.ob_display, self->innerbrb);
        }

        if (self->bwidth) {
            gint titlesides;

            /* height of titleleft and titleright */
            titlesides = (!self->max_horz ? theme_config.grip_width : 0);

            XMoveResizeWindow(plugin.ob_display, self->titletop,
                    theme_config.grip_width + self->bwidth, 0,
                    /* width + bwidth*2 - bwidth*2 - grips*2 */
                    self->width - theme_config.grip_width * 2, self->bwidth);
            XMoveResizeWindow(plugin.ob_display, self->titletopleft, 0, 0,
                    theme_config.grip_width + self->bwidth, self->bwidth);
            XMoveResizeWindow(plugin.ob_display, self->titletopright,
                    self->client_area.width + self->size.left
                            + self->size.right - theme_config.grip_width
                            - self->bwidth, 0, theme_config.grip_width
                            + self->bwidth, self->bwidth);

            if (titlesides > 0) {
                XMoveResizeWindow(plugin.ob_display, self->titleleft, 0,
                        self->bwidth, self->bwidth, titlesides);
                XMoveResizeWindow(plugin.ob_display, self->titleright,
                        self->client_area.width + self->size.left
                                + self->size.right - self->bwidth,
                        self->bwidth, self->bwidth, titlesides);

                XMapWindow(plugin.ob_display, self->titleleft);
                XMapWindow(plugin.ob_display, self->titleright);
            }
            else {
                XUnmapWindow(plugin.ob_display, self->titleleft);
                XUnmapWindow(plugin.ob_display, self->titleright);
            }

            XMapWindow(plugin.ob_display, self->titletop);
            XMapWindow(plugin.ob_display, self->titletopleft);
            XMapWindow(plugin.ob_display, self->titletopright);

            if (self->decorations & OB_FRAME_DECOR_TITLEBAR) {
                XMoveResizeWindow(plugin.ob_display, self->titlebottom,
                        (self->max_horz ? 0 : self->bwidth),
                        theme_config.title_height + self->bwidth, self->width,
                        self->bwidth);

                XMapWindow(plugin.ob_display, self->titlebottom);
            }
            else
                XUnmapWindow(plugin.ob_display, self->titlebottom);
        }
        else {
            XUnmapWindow(plugin.ob_display, self->titlebottom);

            XUnmapWindow(plugin.ob_display, self->titletop);
            XUnmapWindow(plugin.ob_display, self->titletopleft);
            XUnmapWindow(plugin.ob_display, self->titletopright);
            XUnmapWindow(plugin.ob_display, self->titleleft);
            XUnmapWindow(plugin.ob_display, self->titleright);
        }

        if (self->decorations & OB_FRAME_DECOR_TITLEBAR) {
            XMoveResizeWindow(plugin.ob_display, self->title,
                    (self->max_horz ? 0 : self->bwidth), self->bwidth,
                    self->width, theme_config.title_height);

            XMapWindow(plugin.ob_display, self->title);

            if (self->decorations & OB_FRAME_DECOR_GRIPS) {
                XMoveResizeWindow(plugin.ob_display, self->topresize,
                        theme_config.grip_width, 0, self->width
                                - theme_config.grip_width *2,
                        theme_config.paddingy + 1);

                XMoveWindow(plugin.ob_display, self->tltresize, 0, 0);
                XMoveWindow(plugin.ob_display, self->tllresize, 0, 0);
                XMoveWindow(plugin.ob_display, self->trtresize, self->width
                        - theme_config.grip_width, 0);
                XMoveWindow(plugin.ob_display, self->trrresize, self->width
                        - theme_config.paddingx - 1, 0);

                XMapWindow(plugin.ob_display, self->topresize);
                XMapWindow(plugin.ob_display, self->tltresize);
                XMapWindow(plugin.ob_display, self->tllresize);
                XMapWindow(plugin.ob_display, self->trtresize);
                XMapWindow(plugin.ob_display, self->trrresize);
            }
            else {
                XUnmapWindow(plugin.ob_display, self->topresize);
                XUnmapWindow(plugin.ob_display, self->tltresize);
                XUnmapWindow(plugin.ob_display, self->tllresize);
                XUnmapWindow(plugin.ob_display, self->trtresize);
                XUnmapWindow(plugin.ob_display, self->trrresize);
            }
        }
        else
            XUnmapWindow(plugin.ob_display, self->title);
    }

    if ((self->decorations & OB_FRAME_DECOR_TITLEBAR))
        /* layout the title bar elements */
        layout_title(self);

    if (!is_fake) {
        gint sidebwidth = self->max_horz ? 0 : self->bwidth;

        if (self->bwidth && self->size.bottom) {
            XMoveResizeWindow(plugin.ob_display, self->handlebottom,
                    theme_config.grip_width + self->bwidth + sidebwidth,
                    self->size.top + self->client_area.height
                            + self->size.bottom - self->bwidth, self->width
                            - (theme_config.grip_width + sidebwidth) * 2,
                    self->bwidth);

            if (sidebwidth) {
                XMoveResizeWindow(plugin.ob_display, self->lgripleft, 0,
                        self->size.top + self->client_area.height
                                + self->size.bottom
                                - (!self->max_horz ? theme_config.grip_width
                                        : self->size.bottom - self->cbwidth_b),
                        self->bwidth,
                        (!self->max_horz ? theme_config.grip_width
                                : self->size.bottom - self->cbwidth_b));
                XMoveResizeWindow(plugin.ob_display, self->rgripright,
                        self->size.left + self->client_area.width
                                + self->size.right - self->bwidth,
                        self->size.top + self->client_area.height
                                + self->size.bottom
                                - (!self->max_horz ? theme_config.grip_width
                                        : self->size.bottom - self->cbwidth_b),
                        self->bwidth,
                        (!self->max_horz ? theme_config.grip_width
                                : self->size.bottom - self->cbwidth_b));

                XMapWindow(plugin.ob_display, self->lgripleft);
                XMapWindow(plugin.ob_display, self->rgripright);
            }
            else {
                XUnmapWindow(plugin.ob_display, self->lgripleft);
                XUnmapWindow(plugin.ob_display, self->rgripright);
            }

            XMoveResizeWindow(plugin.ob_display, self->lgripbottom, sidebwidth,
                    self->size.top + self->client_area.height
                            + self->size.bottom - self->bwidth,
                    theme_config.grip_width + self->bwidth, self->bwidth);
            XMoveResizeWindow(plugin.ob_display, self->rgripbottom,
                    self->size.left + self->client_area.width
                            + self->size.right - self->bwidth - sidebwidth
                            - theme_config.grip_width, self->size.top
                            + self->client_area.height + self->size.bottom
                            - self->bwidth, theme_config.grip_width
                            + self->bwidth, self->bwidth);

            XMapWindow(plugin.ob_display, self->handlebottom);
            XMapWindow(plugin.ob_display, self->lgripbottom);
            XMapWindow(plugin.ob_display, self->rgripbottom);

            if (self->decorations & OB_FRAME_DECOR_HANDLE
                    && theme_config.handle_height > 0) {
                XMoveResizeWindow(plugin.ob_display, self->handletop,
                        theme_config.grip_width + self->bwidth + sidebwidth, 
                        FRAME_HANDLE_Y(self), self->width - (theme_config.grip_width
                                + sidebwidth) * 2, self->bwidth);
                XMapWindow(plugin.ob_display, self->handletop);

                if (self->decorations & OB_FRAME_DECOR_GRIPS) {
                    XMoveResizeWindow(plugin.ob_display, self->handleleft,
                            theme_config.grip_width, 0, self->bwidth,
                            theme_config.handle_height);
                    XMoveResizeWindow(plugin.ob_display, self->handleright,
                            self->width - theme_config.grip_width
                                    - self->bwidth, 0, self->bwidth,
                            theme_config.handle_height);

                    XMoveResizeWindow(plugin.ob_display, self->lgriptop,
                            sidebwidth, 
                            FRAME_HANDLE_Y(self), theme_config.grip_width
                                    + self->bwidth, self->bwidth);
                    XMoveResizeWindow(plugin.ob_display, self->rgriptop,
                            self->size.left + self->client_area.width
                                    + self->size.right - self->bwidth
                                    - sidebwidth - theme_config.grip_width, 
                            FRAME_HANDLE_Y(self), theme_config.grip_width
                                    + self->bwidth, self->bwidth);

                    XMapWindow(plugin.ob_display, self->handleleft);
                    XMapWindow(plugin.ob_display, self->handleright);
                    XMapWindow(plugin.ob_display, self->lgriptop);
                    XMapWindow(plugin.ob_display, self->rgriptop);
                }
                else {
                    XUnmapWindow(plugin.ob_display, self->handleleft);
                    XUnmapWindow(plugin.ob_display, self->handleright);
                    XUnmapWindow(plugin.ob_display, self->lgriptop);
                    XUnmapWindow(plugin.ob_display, self->rgriptop);
                }
            }
            else {
                XUnmapWindow(plugin.ob_display, self->handleleft);
                XUnmapWindow(plugin.ob_display, self->handleright);
                XUnmapWindow(plugin.ob_display, self->lgriptop);
                XUnmapWindow(plugin.ob_display, self->rgriptop);

                XUnmapWindow(plugin.ob_display, self->handletop);
            }
        }
        else {
            XUnmapWindow(plugin.ob_display, self->handleleft);
            XUnmapWindow(plugin.ob_display, self->handleright);
            XUnmapWindow(plugin.ob_display, self->lgriptop);
            XUnmapWindow(plugin.ob_display, self->rgriptop);

            XUnmapWindow(plugin.ob_display, self->handletop);

            XUnmapWindow(plugin.ob_display, self->handlebottom);
            XUnmapWindow(plugin.ob_display, self->lgripleft);
            XUnmapWindow(plugin.ob_display, self->rgripright);
            XUnmapWindow(plugin.ob_display, self->lgripbottom);
            XUnmapWindow(plugin.ob_display, self->rgripbottom);
        }

        if (self->decorations & OB_FRAME_DECOR_HANDLE
                && theme_config.handle_height > 0) {
            XMoveResizeWindow(plugin.ob_display, self->handle, sidebwidth, 
            FRAME_HANDLE_Y(self) + self->bwidth, self->width,
                    theme_config.handle_height);
            XMapWindow(plugin.ob_display, self->handle);

            if (self->decorations & OB_FRAME_DECOR_GRIPS) {
                XMoveResizeWindow(plugin.ob_display, self->lgrip, 0, 0,
                        theme_config.grip_width, theme_config.handle_height);
                XMoveResizeWindow(plugin.ob_display, self->rgrip, self->width
                        - theme_config.grip_width, 0, theme_config.grip_width,
                        theme_config.handle_height);

                XMapWindow(plugin.ob_display, self->lgrip);
                XMapWindow(plugin.ob_display, self->rgrip);
            }
            else {
                XUnmapWindow(plugin.ob_display, self->lgrip);
                XUnmapWindow(plugin.ob_display, self->rgrip);
            }
        }
        else {
            XUnmapWindow(plugin.ob_display, self->lgrip);
            XUnmapWindow(plugin.ob_display, self->rgrip);

            XUnmapWindow(plugin.ob_display, self->handle);
        }

        if (self->bwidth && !self->max_horz && (self->client_area.height
                + self->size.top + self->size.bottom) > theme_config.grip_width
                * 2) {
            XMoveResizeWindow(plugin.ob_display, self->left, 0, self->bwidth
                    + theme_config.grip_width, self->bwidth,
                    self->client_area.height + self->size.top
                            + self->size.bottom - theme_config.grip_width * 2);

            XMapWindow(plugin.ob_display, self->left);
        }
        else
            XUnmapWindow(plugin.ob_display, self->left);

        if (self->bwidth && !self->max_horz && (self->client_area.height
                + self->size.top + self->size.bottom) > theme_config.grip_width
                * 2) {
            XMoveResizeWindow(plugin.ob_display, self->right,
                    self->client_area.width + self->cbwidth_l + self->cbwidth_r
                            + self->bwidth, self->bwidth
                            + theme_config.grip_width, self->bwidth,
                    self->client_area.height + self->size.top
                            + self->size.bottom - theme_config.grip_width * 2);

            XMapWindow(plugin.ob_display, self->right);
        }
        else
            XUnmapWindow(plugin.ob_display, self->right);

        XMoveResizeWindow(plugin.ob_display, self->backback, self->size.left,
                self->size.top, self->client_area.width,
                self->client_area.height);
    }

    /* shading can change without being moved or resized */
    RECT_SET_SIZE(self->area, self->client_area.width + self->size.left
            + self->size.right, (self->shaded ? theme_config.title_height
            + self->bwidth * 2 : self->client_area.height + self->size.top
            + self->size.bottom));

    if ((is_resize) && !is_fake) {
        /* find the new coordinates, done after setting the frame.size, for
         frame_client_gravity. */
        self->area.x = self->client_area.x;
        self->area.y = self->client_area.y;
        frame_client_gravity(OBDEFAULTFRAME(_self)->client, &self->area.x, &self->area.y);
    }

    if (!is_fake) {
        if (!frame_iconify_animating(self))
            /* move and resize the top level frame.
             shading can change without being moved or resized.

             but don't do this during an iconify animation. it will be
             reflected afterwards.
             */
            XMoveResizeWindow(plugin.ob_display, self->window, self->area.x,
                    self->area.y, self->area.width, self->area.height);

        /* when the client has StaticGravity, it likes to move around.
         also this correctly positions the client when it maps.
         this also needs to be run when the frame's decorations sizes change!
         */
        if (!is_resize)
            XMoveResizeWindow(plugin.ob_display, self->client->window,
                    self->size.left, self->size.top, self->client_area.width,
                    self->client_area.height);

        if (is_resize) {
            self->need_render = TRUE;
            frame_update_skin(self);
            frame_adjust_shape(self);
        }

        if (!STRUT_EQUAL(self->size, oldsize)) {
            gulong vals[4];
            vals[0] = self->size.left;
            vals[1] = self->size.right;
            vals[2] = self->size.top;
            vals[3] = self->size.bottom;
            OBT_PROP_SETA32(self->client->window, NET_FRAME_EXTENTS, CARDINAL,
                    vals, 4);
            OBT_PROP_SETA32(self->client->window, KDE_NET_WM_FRAME_STRUT,
                    CARDINAL, vals, 4);
        }

        /* if this occurs while we are focus cycling, the indicator needs to
         match the changes */
        if (plugin.focus_cycle_target == self->client)
            focus_cycle_draw_indicator(self->client);
    }
    if (is_resize && (self->decorations & OB_FRAME_DECOR_TITLEBAR))
        XResizeWindow(plugin.ob_display, self->label, self->label_width,
                theme_config.label_height);
}

void frame_set_hover_flag(gpointer self, ObFrameButton button)
{
    if (OBDEFAULTFRAME(self)->hover_flag != button) {
        OBDEFAULTFRAME(self)->hover_flag = button;
        frame_update_skin(self);
    }
}

void frame_set_press_flag(gpointer self, ObFrameButton button)
{
    if (OBDEFAULTFRAME(self)->press_flag != button) {
        OBDEFAULTFRAME(self)->press_flag = button;
        frame_update_skin(self);
    }
}

Window frame_get_window(gpointer self)
{
    return OBDEFAULTFRAME(self)->window;
}

Strut frame_get_size(gpointer self)
{
    return OBDEFAULTFRAME(self)->size;
}

gint frame_get_decorations(gpointer self)
{
    return OBDEFAULTFRAME(self)->decorations;
}

void frame_update_title (gpointer self, const gchar * src)
{
    g_free(OBDEFAULTFRAME(self)->stitle);
    OBDEFAULTFRAME(self)->stitle = g_strdup(src);
}

gboolean frame_is_visible(gpointer self)
{
    return OBDEFAULTFRAME(self)->visible;
}

gboolean frame_is_max_horz(gpointer self)
{
    return OBDEFAULTFRAME(self)->max_horz;
}

gboolean frame_is_max_vert(gpointer self)
{
    return OBDEFAULTFRAME(self)->max_vert;
}

gulong frame_animate_iconify_time_left(gpointer _self, const GTimeVal *now)
{
    ObDefaultFrame * self = (ObDefaultFrame *) _self;
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
    ObDefaultFrame *self = p;
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

void frame_adjust_cursors(gpointer _self)
{
    ObDefaultFrame * self = (ObDefaultFrame *) _self;
    if ((self->functions & OB_CLIENT_FUNC_RESIZE) != (self->functions
            & OB_CLIENT_FUNC_RESIZE) || self->max_horz != self->max_horz
            || self->max_vert != self->max_vert || self->shaded != self->shaded) {
        gboolean r = (self->functions & OB_CLIENT_FUNC_RESIZE)
                && !(self->max_horz && self->max_vert);
        gboolean topbot = !self->max_vert;
        gboolean sh = self->shaded;
        XSetWindowAttributes a;

        /* these ones turn off when max vert, and some when shaded */
        a.cursor = ob_cursor(r && topbot && !sh ? OB_CURSOR_NORTH
                : OB_CURSOR_NONE);
        XChangeWindowAttributes(plugin.ob_display, self->topresize, CWCursor,
                &a);
        XChangeWindowAttributes(plugin.ob_display, self->titletop, CWCursor, &a);
        a.cursor = ob_cursor(r && topbot ? OB_CURSOR_SOUTH : OB_CURSOR_NONE);
        XChangeWindowAttributes(plugin.ob_display, self->handle, CWCursor, &a);
        XChangeWindowAttributes(plugin.ob_display, self->handletop, CWCursor,
                &a);
        XChangeWindowAttributes(plugin.ob_display, self->handlebottom,
                CWCursor, &a);
        XChangeWindowAttributes(plugin.ob_display, self->innerbottom, CWCursor,
                &a);

        /* these ones change when shaded */
        a.cursor = ob_cursor(r ? (sh ? OB_CURSOR_WEST : OB_CURSOR_NORTHWEST)
                : OB_CURSOR_NONE);
        XChangeWindowAttributes(plugin.ob_display, self->titleleft, CWCursor,
                &a);
        XChangeWindowAttributes(plugin.ob_display, self->tltresize, CWCursor,
                &a);
        XChangeWindowAttributes(plugin.ob_display, self->tllresize, CWCursor,
                &a);
        XChangeWindowAttributes(plugin.ob_display, self->titletopleft,
                CWCursor, &a);
        a.cursor = ob_cursor(r ? (sh ? OB_CURSOR_EAST : OB_CURSOR_NORTHEAST)
                : OB_CURSOR_NONE);
        XChangeWindowAttributes(plugin.ob_display, self->titleright, CWCursor,
                &a);
        XChangeWindowAttributes(plugin.ob_display, self->trtresize, CWCursor,
                &a);
        XChangeWindowAttributes(plugin.ob_display, self->trrresize, CWCursor,
                &a);
        XChangeWindowAttributes(plugin.ob_display, self->titletopright,
                CWCursor, &a);

        /* these ones are pretty static */
        a.cursor = ob_cursor(r ? OB_CURSOR_WEST : OB_CURSOR_NONE);
        XChangeWindowAttributes(plugin.ob_display, self->left, CWCursor, &a);
        XChangeWindowAttributes(plugin.ob_display, self->innerleft, CWCursor,
                &a);
        a.cursor = ob_cursor(r ? OB_CURSOR_EAST : OB_CURSOR_NONE);
        XChangeWindowAttributes(plugin.ob_display, self->right, CWCursor, &a);
        XChangeWindowAttributes(plugin.ob_display, self->innerright, CWCursor,
                &a);
        a.cursor = ob_cursor(r ? OB_CURSOR_SOUTHWEST : OB_CURSOR_NONE);
        XChangeWindowAttributes(plugin.ob_display, self->lgrip, CWCursor, &a);
        XChangeWindowAttributes(plugin.ob_display, self->handleleft, CWCursor,
                &a);
        XChangeWindowAttributes(plugin.ob_display, self->lgripleft, CWCursor,
                &a);
        XChangeWindowAttributes(plugin.ob_display, self->lgriptop, CWCursor, &a);
        XChangeWindowAttributes(plugin.ob_display, self->lgripbottom, CWCursor,
                &a);
        XChangeWindowAttributes(plugin.ob_display, self->innerbll, CWCursor, &a);
        XChangeWindowAttributes(plugin.ob_display, self->innerblb, CWCursor, &a);
        a.cursor = ob_cursor(r ? OB_CURSOR_SOUTHEAST : OB_CURSOR_NONE);
        XChangeWindowAttributes(plugin.ob_display, self->rgrip, CWCursor, &a);
        XChangeWindowAttributes(plugin.ob_display, self->handleright, CWCursor,
                &a);
        XChangeWindowAttributes(plugin.ob_display, self->rgripright, CWCursor,
                &a);
        XChangeWindowAttributes(plugin.ob_display, self->rgriptop, CWCursor, &a);
        XChangeWindowAttributes(plugin.ob_display, self->rgripbottom, CWCursor,
                &a);
        XChangeWindowAttributes(plugin.ob_display, self->innerbrr, CWCursor, &a);
        XChangeWindowAttributes(plugin.ob_display, self->innerbrb, CWCursor, &a);
    }
}

void frame_adjust_client_area(gpointer _self)
{
    ObDefaultFrame * self = (ObDefaultFrame *) _self;
    /* adjust the window which is there to prevent flashing on unmap */
    XMoveResizeWindow(plugin.ob_display, self->backfront, 0, 0,
            self->client_area.width, self->client_area.height);
}

void frame_adjust_state(gpointer _self)
{
    ObDefaultFrame * self = (ObDefaultFrame *) _self;
    self->need_render = TRUE;
    frame_update_skin(self);
}

void frame_adjust_focus(gpointer _self, gboolean hilite)
{
    ObDefaultFrame * self = (ObDefaultFrame *) _self;
    self->focused = hilite;
    self->need_render = TRUE;
    frame_update_skin(self);
    XFlush(plugin.ob_display);
}

void frame_adjust_title(gpointer _self)
{
    ObDefaultFrame * self = (ObDefaultFrame *) _self;
    self->need_render = TRUE;
    frame_update_skin(self);
}

void frame_adjust_icon(gpointer _self)
{
    ObDefaultFrame * self = (ObDefaultFrame *) _self;
    self->need_render = TRUE;
    frame_update_skin(self);
}

/* is there anything present between us and the label? */
static gboolean is_button_present(ObDefaultFrame *_self, const gchar *lc,
        gint dir)
{
    ObDefaultFrame * self = (ObDefaultFrame *) _self;
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
    ObDefaultFrame *self = data;

    if (self->focused != self->flash_on)
        frame_adjust_focus(self, self->focused);
}

gboolean flash_timeout(gpointer data)
{
    ObDefaultFrame *self = data;
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

void layout_title(ObDefaultFrame * self)
{
    gchar *lc;
    gint i;

    const gint bwidth = theme_config.button_size + theme_config.paddingx + 1;
    /* position of the left most button */
    const gint left = theme_config.paddingx + 1;
    /* position of the right most button */
    const gint right = self->width;

    /* turn them all off */
    self->icon_on = self->desk_on = self->shade_on = self->iconify_on
            = self->max_on = self->close_on = self->label_on = FALSE;
    self->label_width = self->width - (theme_config.paddingx + 1) * 2;
    self->leftmost = self->rightmost = OB_FRAME_CONTEXT_NONE;

    /* figure out what's being show, find each element's position, and the
     width of the label

     do the ones before the label, then after the label,
     i will be +1 the first time through when working to the left,
     and -1 the second time through when working to the right */
    for (i = 1; i >= -1; i-=2) {
        gint x;
        ObFrameContext *firstcon;

        if (i > 0) {
            x = left;
            lc = plugin.config_title_layout;
            firstcon = &self->leftmost;
        }
        else {
            x = right;
            lc = plugin.config_title_layout
                    + strlen(plugin.config_title_layout)-1;
            firstcon = &self->rightmost;
        }

        /* stop at the end of the string (or the label, which calls break) */
        for (; *lc != '\0' && lc >= plugin.config_title_layout; lc+=i) {
            if (*lc == 'L') {
                if (i > 0) {
                    self->label_on = TRUE;
                    self->label_x = x;
                }
                break; /* break the for loop, do other side of label */
            }
            else if (*lc == 'N') {
                if (firstcon)
                    *firstcon = OB_FRAME_CONTEXT_ICON;
                if ((self->icon_on = is_button_present(self, lc, i))) {
                    /* icon is bigger than buttons */
                    self->label_width -= bwidth + 2;
                    if (i > 0)
                        self->icon_x = x;
                    x += i * (bwidth + 2);
                    if (i < 0)
                        self->icon_x = x;
                }
            }
            else if (*lc == 'D') {
                if (firstcon)
                    *firstcon = OB_FRAME_CONTEXT_ALLDESKTOPS;
                if ((self->desk_on = is_button_present(self, lc, i))) {
                    self->label_width -= bwidth;
                    if (i > 0)
                        self->desk_x = x;
                    x += i * bwidth;
                    if (i < 0)
                        self->desk_x = x;
                }
            }
            else if (*lc == 'S') {
                if (firstcon)
                    *firstcon = OB_FRAME_CONTEXT_SHADE;
                if ((self->shade_on = is_button_present(self, lc, i))) {
                    self->label_width -= bwidth;
                    if (i > 0)
                        self->shade_x = x;
                    x += i * bwidth;
                    if (i < 0)
                        self->shade_x = x;
                }
            }
            else if (*lc == 'I') {
                if (firstcon)
                    *firstcon = OB_FRAME_CONTEXT_ICONIFY;
                if ((self->iconify_on = is_button_present(self, lc, i))) {
                    self->label_width -= bwidth;
                    if (i > 0)
                        self->iconify_x = x;
                    x += i * bwidth;
                    if (i < 0)
                        self->iconify_x = x;
                }
            }
            else if (*lc == 'M') {
                if (firstcon)
                    *firstcon = OB_FRAME_CONTEXT_MAXIMIZE;
                if ((self->max_on = is_button_present(self, lc, i))) {
                    self->label_width -= bwidth;
                    if (i > 0)
                        self->max_x = x;
                    x += i * bwidth;
                    if (i < 0)
                        self->max_x = x;
                }
            }
            else if (*lc == 'C') {
                if (firstcon)
                    *firstcon = OB_FRAME_CONTEXT_CLOSE;
                if ((self->close_on = is_button_present(self, lc, i))) {
                    self->label_width -= bwidth;
                    if (i > 0)
                        self->close_x = x;
                    x += i * bwidth;
                    if (i < 0)
                        self->close_x = x;
                }
            }
            else
                continue; /* don't set firstcon */
            firstcon = NULL;
        }
    }

    /* position and map the elements */
    if (self->icon_on) {
        XMapWindow(plugin.ob_display, self->icon);
        XMoveWindow(plugin.ob_display, self->icon, self->icon_x,
                theme_config.paddingy);
    }
    else
        XUnmapWindow(plugin.ob_display, self->icon);

    if (self->desk_on) {
        XMapWindow(plugin.ob_display, self->desk);
        XMoveWindow(plugin.ob_display, self->desk, self->desk_x,
                theme_config.paddingy + 1);
    }
    else
        XUnmapWindow(plugin.ob_display, self->desk);

    if (self->shade_on) {
        XMapWindow(plugin.ob_display, self->shade);
        XMoveWindow(plugin.ob_display, self->shade, self->shade_x,
                theme_config.paddingy + 1);
    }
    else
        XUnmapWindow(plugin.ob_display, self->shade);

    if (self->iconify_on) {
        XMapWindow(plugin.ob_display, self->iconify);
        XMoveWindow(plugin.ob_display, self->iconify, self->iconify_x,
                theme_config.paddingy + 1);
    }
    else
        XUnmapWindow(plugin.ob_display, self->iconify);

    if (self->max_on) {
        XMapWindow(plugin.ob_display, self->max);
        XMoveWindow(plugin.ob_display, self->max, self->max_x,
                theme_config.paddingy + 1);
    }
    else
        XUnmapWindow(plugin.ob_display, self->max);

    if (self->close_on) {
        XMapWindow(plugin.ob_display, self->close);
        XMoveWindow(plugin.ob_display, self->close, self->close_x,
                theme_config.paddingy + 1);
    }
    else
        XUnmapWindow(plugin.ob_display, self->close);

    if (self->label_on) {
        self->label_width = MAX(1, self->label_width); /* no lower than 1 */
        XMapWindow(plugin.ob_display, self->label);
        XMoveWindow(plugin.ob_display, self->label, self->label_x,
                theme_config.paddingy);
    }
    else
        XUnmapWindow(plugin.ob_display, self->label);
}

void trigger_none (gpointer self) {}
void trigger_iconify(gpointer self) {}
void trigger_uniconnity(gpointer self) {}
void trigger_iconify_toggle(gpointer self) {}
void trigger_shade(gpointer self) {}
void trigger_unshade(gpointer self) {}
void trigger_shade_toggle(gpointer self) {}
void trigger_max(gpointer self) {}
void trigger_unmax(gpointer self) {}
void trigger_max_troggle(gpointer self) {}
void trigger_max_vert(gpointer self) {OBDEFAULTFRAME(self)->max_vert = TRUE;}
void trigger_unmax_vert(gpointer self) {OBDEFAULTFRAME(self)->max_vert = FALSE;}
void trigger_max_toggle(gpointer self) {}
void trigger_max_horz(gpointer self) {OBDEFAULTFRAME(self)->max_horz = TRUE;}
void trigger_unmax_horz(gpointer self) {OBDEFAULTFRAME(self)->max_horz = FALSE;}
void trigger_max_horz_toggle(gpointer self) {}
void trigger_plugin1(gpointer self) {}
void trigger_plugin2(gpointer self) {}
void trigger_plugin3(gpointer self) {}
void trigger_plugin4(gpointer self) {}
void trigger_plugin5(gpointer self) {}
void trigger_plugin6(gpointer self) {}
void trigger_plugin7(gpointer self) {}
void trigger_plugin8(gpointer self) {}
void trigger_plugin9(gpointer self) {}

void frame_trigger(gpointer self, ObFrameTrigger trigger_name)
{

    static void (*trigger_func[64])(gpointer) = { 
            trigger_none,
            trigger_iconify,
            trigger_uniconnity,
            trigger_iconify_toggle,
            trigger_shade,
            trigger_unshade,
            trigger_shade_toggle,
            trigger_max,
            trigger_unmax,
            trigger_max_troggle,
            trigger_max_vert,
            trigger_unmax_vert,
            trigger_max_toggle,
            trigger_max_horz,
            trigger_unmax_horz,
            trigger_max_horz_toggle,
            trigger_plugin1,
            trigger_plugin2,
            trigger_plugin3,
            trigger_plugin4,
            trigger_plugin5,
            trigger_plugin6,
            trigger_plugin7,
            trigger_plugin8,
            trigger_plugin9,
            NULL,
    };
    
    void (*call_trigger_func)(gpointer) = trigger_func[trigger_name];
    if(!call_trigger_func)
    {
        call_trigger_func (self);
    }
}

ObFramePlugin plugin = { 0, //gpointer handler;
        "libdefault.la", //gchar * filename;
        "Default", //gchar * name;
        init, //gint (*init) (Display * display, gint screen);
        0, /* */
        frame_new, //gpointer (*frame_new) (struct _ObClient *c);
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
        
        frame_update_title, /* */
        /* This give the window area */
        frame_get_window_area, /* */
        frame_set_client_area, /* */
        /* Draw the frame */
        frame_update_layout, /* */
        frame_update_skin, /* */

        frame_set_hover_flag, /* */
        frame_set_press_flag, /* */

        frame_get_window,/* */

        frame_get_size, /* */
        frame_get_decorations, /* */

        frame_is_visible, /* */
        frame_is_max_horz, /* */
        frame_is_max_vert, /* */

        frame_trigger, /* */

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
