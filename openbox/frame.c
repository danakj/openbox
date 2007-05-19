/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   frame.c for the Openbox window manager
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

#include "frame.h"
#include "client.h"
#include "openbox.h"
#include "extensions.h"
#include "prop.h"
#include "config.h"
#include "framerender.h"
#include "mainloop.h"
#include "focus_cycle.h"
#include "focus_cycle_indicator.h"
#include "moveresize.h"
#include "screen.h"
#include "render/theme.h"

#define PLATE_EVENTMASK (SubstructureRedirectMask | FocusChangeMask)
#define FRAME_EVENTMASK (EnterWindowMask | LeaveWindowMask | \
                         ButtonPressMask | ButtonReleaseMask)
#define ELEMENT_EVENTMASK (ButtonPressMask | ButtonReleaseMask | \
                           ButtonMotionMask | PointerMotionMask | \
                           EnterWindowMask | LeaveWindowMask)
/* The inner window does not need enter/leave events.
   If it does get them, then it needs its own context for enter events
   because sloppy focus will focus the window when you enter the inner window
   from the frame. */
#define INNER_EVENTMASK (ButtonPressMask)

#define FRAME_ANIMATE_ICONIFY_TIME 150000 /* .15 seconds */
#define FRAME_ANIMATE_ICONIFY_STEP_TIME (G_USEC_PER_SEC / 60) /* 60 Hz */

#define FRAME_HANDLE_Y(f) (f->innersize.top + f->client->area.height + \
                           f->cbwidth_y)

static void flash_done(gpointer data);
static gboolean flash_timeout(gpointer data);

static void layout_title(ObFrame *self);
static void set_theme_statics(ObFrame *self);
static void free_theme_statics(ObFrame *self);
static gboolean frame_animate_iconify(gpointer self);

static Window createWindow(Window parent, Visual *visual,
                           gulong mask, XSetWindowAttributes *attrib)
{
    return XCreateWindow(ob_display, parent, 0, 0, 1, 1, 0,
                         (visual ? 32 : RrDepth(ob_rr_inst)), InputOutput,
                         (visual ? visual : RrVisual(ob_rr_inst)),
                         mask, attrib);
                       
}

static Visual *check_32bit_client(ObClient *c)
{
    XWindowAttributes wattrib;
    Status ret;

    ret = XGetWindowAttributes(ob_display, c->window, &wattrib);
    g_assert(ret != BadDrawable);
    g_assert(ret != BadWindow);

    if (wattrib.depth == 32)
        return wattrib.visual;
    return NULL;
}

ObFrame *frame_new(ObClient *client)
{
    XSetWindowAttributes attrib;
    gulong mask;
    ObFrame *self;
    Visual *visual;

    self = g_new0(ObFrame, 1);
    self->client = client;

    visual = check_32bit_client(client);

    /* create the non-visible decor windows */

    mask = CWEventMask;
    if (visual) {
        /* client has a 32-bit visual */
        mask |= CWColormap | CWBackPixel | CWBorderPixel;
        /* create a colormap with the visual */
        self->colormap = attrib.colormap =
            XCreateColormap(ob_display,
                            RootWindow(ob_display, ob_screen),
                            visual, AllocNone);
        attrib.background_pixel = BlackPixel(ob_display, 0);
        attrib.border_pixel = BlackPixel(ob_display, 0);
    }
    attrib.event_mask = FRAME_EVENTMASK;
    self->window = createWindow(RootWindow(ob_display, ob_screen), visual,
                                mask, &attrib);

    attrib.event_mask = INNER_EVENTMASK;
    self->inner = createWindow(self->window, visual, mask, &attrib);

    mask &= ~CWEventMask;
    self->plate = createWindow(self->inner, visual, mask, &attrib);

    /* create the visible decor windows */

    mask = CWEventMask;
    if (visual) {
        /* client has a 32-bit visual */
        mask |= CWColormap | CWBackPixel | CWBorderPixel;
        attrib.colormap = RrColormap(ob_rr_inst);
    }
    attrib.event_mask = ELEMENT_EVENTMASK;
    self->title = createWindow(self->window, NULL, mask, &attrib);

    mask |= CWCursor;
    attrib.cursor = ob_cursor(OB_CURSOR_NORTH);
    self->topresize = createWindow(self->title, NULL, mask, &attrib);
    attrib.cursor = ob_cursor(OB_CURSOR_NORTHWEST);
    self->tltresize = createWindow(self->title, NULL, mask, &attrib);
    self->tllresize = createWindow(self->title, NULL, mask, &attrib);
    attrib.cursor = ob_cursor(OB_CURSOR_NORTHEAST);
    self->trtresize = createWindow(self->title, NULL, mask, &attrib);
    self->trrresize = createWindow(self->title, NULL, mask, &attrib);

    attrib.cursor = ob_cursor(OB_CURSOR_WEST);
    self->leftresize = createWindow(self->inner, NULL, mask, &attrib);
    attrib.cursor = ob_cursor(OB_CURSOR_EAST);
    self->rightresize = createWindow(self->inner, NULL, mask, &attrib);

    mask &= ~CWCursor;
    self->label = createWindow(self->title, NULL, mask, &attrib);
    self->max = createWindow(self->title, NULL, mask, &attrib);
    self->close = createWindow(self->title, NULL, mask, &attrib);
    self->desk = createWindow(self->title, NULL, mask, &attrib);
    self->shade = createWindow(self->title, NULL, mask, &attrib);
    self->icon = createWindow(self->title, NULL, mask, &attrib);
    self->iconify = createWindow(self->title, NULL, mask, &attrib);

    mask |= CWCursor;
    attrib.cursor = ob_cursor(OB_CURSOR_SOUTH);
    self->handle = createWindow(self->window, NULL, mask, &attrib);
    attrib.cursor = ob_cursor(OB_CURSOR_SOUTHWEST);
    self->lgrip = createWindow(self->handle, NULL, mask, &attrib);
    attrib.cursor = ob_cursor(OB_CURSOR_SOUTHEAST);
    self->rgrip = createWindow(self->handle, NULL, mask, &attrib); 

    self->focused = FALSE;

    /* the other stuff is shown based on decor settings */
    XMapWindow(ob_display, self->plate);
    XMapWindow(ob_display, self->inner);
    XMapWindow(ob_display, self->lgrip);
    XMapWindow(ob_display, self->rgrip);
    XMapWindow(ob_display, self->label);

    self->max_press = self->close_press = self->desk_press = 
        self->iconify_press = self->shade_press = FALSE;
    self->max_hover = self->close_hover = self->desk_hover = 
        self->iconify_hover = self->shade_hover = FALSE;

    set_theme_statics(self);

    return (ObFrame*)self;
}

static void set_theme_statics(ObFrame *self)
{
    gint handle_height;

    if (ob_rr_theme->handle_height > 0)
        handle_height = ob_rr_theme->handle_height;
    else
        handle_height = 1;
        

    /* set colors/appearance/sizes for stuff that doesn't change */
    XResizeWindow(ob_display, self->max,
                  ob_rr_theme->button_size, ob_rr_theme->button_size);
    XResizeWindow(ob_display, self->iconify,
                  ob_rr_theme->button_size, ob_rr_theme->button_size);
    XResizeWindow(ob_display, self->icon,
                  ob_rr_theme->button_size + 2, ob_rr_theme->button_size + 2);
    XResizeWindow(ob_display, self->close,
                  ob_rr_theme->button_size, ob_rr_theme->button_size);
    XResizeWindow(ob_display, self->desk,
                  ob_rr_theme->button_size, ob_rr_theme->button_size);
    XResizeWindow(ob_display, self->shade,
                  ob_rr_theme->button_size, ob_rr_theme->button_size);
    XResizeWindow(ob_display, self->lgrip,
                  ob_rr_theme->grip_width, handle_height);
    XResizeWindow(ob_display, self->rgrip,
                  ob_rr_theme->grip_width, handle_height);
    XResizeWindow(ob_display, self->tltresize,
                  ob_rr_theme->grip_width, ob_rr_theme->paddingy + 1);
    XResizeWindow(ob_display, self->trtresize,
                  ob_rr_theme->grip_width, ob_rr_theme->paddingy + 1);
    XResizeWindow(ob_display, self->tllresize,
                  ob_rr_theme->paddingx + 1, ob_rr_theme->title_height);
    XResizeWindow(ob_display, self->trrresize,
                  ob_rr_theme->paddingx + 1, ob_rr_theme->title_height);

    /* set up the dynamic appearances */
    self->a_unfocused_title = RrAppearanceCopy(ob_rr_theme->a_unfocused_title);
    self->a_focused_title = RrAppearanceCopy(ob_rr_theme->a_focused_title);
    self->a_unfocused_label = RrAppearanceCopy(ob_rr_theme->a_unfocused_label);
    self->a_focused_label = RrAppearanceCopy(ob_rr_theme->a_focused_label);
    self->a_unfocused_handle =
        RrAppearanceCopy(ob_rr_theme->a_unfocused_handle);
    self->a_focused_handle = RrAppearanceCopy(ob_rr_theme->a_focused_handle);
    self->a_icon = RrAppearanceCopy(ob_rr_theme->a_icon);
}

static void free_theme_statics(ObFrame *self)
{
    RrAppearanceFree(self->a_unfocused_title); 
    RrAppearanceFree(self->a_focused_title);
    RrAppearanceFree(self->a_unfocused_label);
    RrAppearanceFree(self->a_focused_label);
    RrAppearanceFree(self->a_unfocused_handle);
    RrAppearanceFree(self->a_focused_handle);
    RrAppearanceFree(self->a_icon);
}

void frame_free(ObFrame *self)
{
    free_theme_statics(self);

    XDestroyWindow(ob_display, self->window);
    if (self->colormap)
        XFreeColormap(ob_display, self->colormap);

    g_free(self);
}

void frame_show(ObFrame *self)
{
    if (!self->visible) {
        self->visible = TRUE;
        XMapWindow(ob_display, self->client->window);
        XMapWindow(ob_display, self->window);
    }
}

void frame_hide(ObFrame *self)
{
    if (self->visible) {
        self->visible = FALSE;
        if (!frame_iconify_animating(self))
            XUnmapWindow(ob_display, self->window);
        /* we unmap the client itself so that we can get MapRequest
           events, and because the ICCCM tells us to! */
        XUnmapWindow(ob_display, self->client->window);
        self->client->ignore_unmaps += 1;
    }
}

void frame_adjust_theme(ObFrame *self)
{
    free_theme_statics(self);
    set_theme_statics(self);
}

void frame_adjust_shape(ObFrame *self)
{
#ifdef SHAPE
    gint num;
    XRectangle xrect[2];

    if (!self->client->shaped) {
        /* clear the shape on the frame window */
        XShapeCombineMask(ob_display, self->window, ShapeBounding,
                          self->innersize.left,
                          self->innersize.top,
                          None, ShapeSet);
    } else {
        /* make the frame's shape match the clients */
        XShapeCombineShape(ob_display, self->window, ShapeBounding,
                           self->innersize.left,
                           self->innersize.top,
                           self->client->window,
                           ShapeBounding, ShapeSet);

        num = 0;
        if (self->decorations & OB_FRAME_DECOR_TITLEBAR) {
            xrect[0].x = -ob_rr_theme->fbwidth;
            xrect[0].y = -ob_rr_theme->fbwidth;
            xrect[0].width = self->width + self->rbwidth * 2;
            xrect[0].height = ob_rr_theme->title_height +
                self->bwidth * 2;
            ++num;
        }

        if (self->decorations & OB_FRAME_DECOR_HANDLE) {
            xrect[1].x = -ob_rr_theme->fbwidth;
            xrect[1].y = FRAME_HANDLE_Y(self);
            xrect[1].width = self->width + self->rbwidth * 2;
            xrect[1].height = ob_rr_theme->handle_height +
                self->bwidth * 2;
            ++num;
        }

        XShapeCombineRectangles(ob_display, self->window,
                                ShapeBounding, 0, 0, xrect, num,
                                ShapeUnion, Unsorted);
    }
#endif
}

void frame_adjust_area(ObFrame *self, gboolean moved,
                       gboolean resized, gboolean fake)
{
    Strut oldsize;

    oldsize = self->size;

    if (resized) {
        self->decorations = self->client->decorations;
        self->max_horz = self->client->max_horz;

        if (self->decorations & OB_FRAME_DECOR_BORDER) {
            self->bwidth = ob_rr_theme->fbwidth;
            self->cbwidth_x = ob_rr_theme->cbwidthx;
            self->cbwidth_y = ob_rr_theme->cbwidthy;
        } else {
            self->bwidth = self->cbwidth_x = self->cbwidth_y = 0;
        }
        self->rbwidth = self->bwidth;

        if (self->max_horz)
            self->bwidth = self->cbwidth_x = 0;

        STRUT_SET(self->innersize,
                  self->cbwidth_x,
                  self->cbwidth_y,
                  self->cbwidth_x,
                  self->cbwidth_y);
        self->width = self->client->area.width + self->cbwidth_x * 2 -
            (self->max_horz ? self->rbwidth * 2 : 0);
        self->width = MAX(self->width, 1); /* no lower than 1 */

        /* set border widths */
        if (!fake) {
            XSetWindowBorderWidth(ob_display, self->window, self->bwidth);
            XSetWindowBorderWidth(ob_display, self->inner, self->bwidth);
            XSetWindowBorderWidth(ob_display, self->title,  self->rbwidth);
            XSetWindowBorderWidth(ob_display, self->handle, self->rbwidth);
            XSetWindowBorderWidth(ob_display, self->lgrip,  self->rbwidth);
            XSetWindowBorderWidth(ob_display, self->rgrip,  self->rbwidth);
            XSetWindowBorderWidth(ob_display, self->leftresize, self->bwidth);
            XSetWindowBorderWidth(ob_display, self->rightresize, self->bwidth);
        }

        if (self->decorations & OB_FRAME_DECOR_TITLEBAR)
            self->innersize.top += ob_rr_theme->title_height + self->rbwidth +
                (self->rbwidth - self->bwidth);
        if (self->decorations & OB_FRAME_DECOR_HANDLE &&
            ob_rr_theme->handle_height > 0)
            self->innersize.bottom += ob_rr_theme->handle_height +
                self->rbwidth + (self->rbwidth - self->bwidth);
  
        /* position/size and map/unmap all the windows */

        if (!fake) {
            if (self->decorations & OB_FRAME_DECOR_TITLEBAR) {
                XMoveResizeWindow(ob_display, self->title,
                                  -self->bwidth, -self->bwidth,
                                  self->width, ob_rr_theme->title_height);
                XMapWindow(ob_display, self->title);

                if (self->decorations & OB_FRAME_DECOR_GRIPS) {
                    XMoveResizeWindow(ob_display, self->topresize,
                                      ob_rr_theme->grip_width + self->bwidth,
                                      0,
                                      self->width - (ob_rr_theme->grip_width +
                                                     self->bwidth) * 2,
                                      ob_rr_theme->paddingy + 1);

                    XMoveWindow(ob_display, self->tltresize, 0, 0);
                    XMoveWindow(ob_display, self->tllresize, 0, 0);
                    XMoveWindow(ob_display, self->trtresize,
                                self->width - ob_rr_theme->grip_width, 0);
                    XMoveWindow(ob_display, self->trrresize,
                                self->width - ob_rr_theme->paddingx - 1, 0);

                    XMapWindow(ob_display, self->topresize);
                    XMapWindow(ob_display, self->tltresize);
                    XMapWindow(ob_display, self->tllresize);
                    XMapWindow(ob_display, self->trtresize);
                    XMapWindow(ob_display, self->trrresize);
                } else {
                    XUnmapWindow(ob_display, self->topresize);
                    XUnmapWindow(ob_display, self->tltresize);
                    XUnmapWindow(ob_display, self->tllresize);
                    XUnmapWindow(ob_display, self->trtresize);
                    XUnmapWindow(ob_display, self->trrresize);
                }
            } else
                XUnmapWindow(ob_display, self->title);
        }

        if ((self->decorations & OB_FRAME_DECOR_TITLEBAR))
            /* layout the title bar elements */
            layout_title(self);

        if (!fake) {
            if (self->decorations & OB_FRAME_DECOR_HANDLE)
            {
                gint handle_height;

                if (ob_rr_theme->handle_height > 0)
                    handle_height = ob_rr_theme->handle_height;
                else
                    handle_height = 1;

                XMoveResizeWindow(ob_display, self->handle,
                                  -self->bwidth, FRAME_HANDLE_Y(self),
                                  self->width, handle_height);
                XMapWindow(ob_display, self->handle);

                if (self->decorations & OB_FRAME_DECOR_GRIPS) {
                    XMoveWindow(ob_display, self->lgrip,
                                -self->rbwidth, -self->rbwidth);
                    XMoveWindow(ob_display, self->rgrip,
                                -self->rbwidth + self->width -
                                ob_rr_theme->grip_width, -self->rbwidth);
                    XMapWindow(ob_display, self->lgrip);
                    XMapWindow(ob_display, self->rgrip);
                } else {
                    XUnmapWindow(ob_display, self->lgrip);
                    XUnmapWindow(ob_display, self->rgrip);
                }
            } else
                XUnmapWindow(ob_display, self->handle);

            if (self->decorations & OB_FRAME_DECOR_GRIPS) {
                XMoveResizeWindow(ob_display, self->leftresize,
                                  -(ob_rr_theme->fbwidth * 2) - 1,
                                  0,
                                  1,
                                  self->client->area.height +
                                  self->cbwidth_y * 2);
                XMoveResizeWindow(ob_display, self->rightresize,
                                  self->client->area.width +
                                  self->cbwidth_x * 2,
                                  0,
                                  1,
                                  self->client->area.height +
                                  self->cbwidth_y * 2);

                XMapWindow(ob_display, self->leftresize);
                XMapWindow(ob_display, self->rightresize);
            } else {
                XUnmapWindow(ob_display, self->leftresize);
                XUnmapWindow(ob_display, self->rightresize);
            }

            /* move and resize the inner border window which contains the plate
             */
            XMoveResizeWindow(ob_display, self->inner,
                              self->innersize.left - self->cbwidth_x -
                              self->bwidth,
                              self->innersize.top - self->cbwidth_y -
                              self->bwidth,
                              self->client->area.width +
                              self->cbwidth_x * 2,
                              self->client->area.height +
                              self->cbwidth_y * 2);

            /* move the plate */
            XMoveWindow(ob_display, self->plate,
                        self->cbwidth_x, self->cbwidth_y);

            /* when the client has StaticGravity, it likes to move around. */
            XMoveWindow(ob_display, self->client->window, 0, 0);
        }

        STRUT_SET(self->size,
                  self->innersize.left + self->bwidth,
                  self->innersize.top + self->bwidth,
                  self->innersize.right + self->bwidth,
                  self->innersize.bottom + self->bwidth);
    }

    /* shading can change without being moved or resized */
    RECT_SET_SIZE(self->area,
                  self->client->area.width +
                  self->size.left + self->size.right,
                  (self->client->shaded ?
                   ob_rr_theme->title_height + self->rbwidth * 2:
                   self->client->area.height +
                   self->size.top + self->size.bottom));

    if (moved || resized) {
        /* find the new coordinates, done after setting the frame.size, for
           frame_client_gravity. */
        self->area.x = self->client->area.x;
        self->area.y = self->client->area.y;
        frame_client_gravity(self, &self->area.x, &self->area.y,
                             self->client->area.width,
                             self->client->area.height);
    }

    if (!fake) {
        if (!frame_iconify_animating(self))
            /* move and resize the top level frame.
               shading can change without being moved or resized.
               
               but don't do this during an iconify animation. it will be
               reflected afterwards.
            */
            XMoveResizeWindow(ob_display, self->window,
                              self->area.x, self->area.y,
                              self->area.width - self->bwidth * 2,
                              self->area.height - self->bwidth * 2);

        if (resized) {
            framerender_frame(self);
            frame_adjust_shape(self);
        }

        if (!STRUT_EQUAL(self->size, oldsize)) {
            gulong vals[4];
            vals[0] = self->size.left;
            vals[1] = self->size.right;
            vals[2] = self->size.top;
            vals[3] = self->size.bottom;
            PROP_SETA32(self->client->window, net_frame_extents,
                        cardinal, vals, 4);
            PROP_SETA32(self->client->window, kde_net_wm_frame_strut,
                        cardinal, vals, 4);
        }

        /* if this occurs while we are focus cycling, the indicator needs to
           match the changes */
        if (focus_cycle_target == self->client)
            focus_cycle_draw_indicator(self->client);
    }
    if (resized && (self->decorations & OB_FRAME_DECOR_TITLEBAR))
        XResizeWindow(ob_display, self->label, self->label_width,
                      ob_rr_theme->label_height);
}

void frame_adjust_client_area(ObFrame *self)
{
    /* resize the plate */
    XResizeWindow(ob_display, self->plate,
                  self->client->area.width, self->client->area.height);
}

void frame_adjust_state(ObFrame *self)
{
    framerender_frame(self);
}

void frame_adjust_focus(ObFrame *self, gboolean hilite)
{
    self->focused = hilite;
    framerender_frame(self);
    XFlush(ob_display);
}

void frame_adjust_title(ObFrame *self)
{
    framerender_frame(self);
}

void frame_adjust_icon(ObFrame *self)
{
    framerender_frame(self);
}

void frame_grab_client(ObFrame *self)
{
    /* reparent the client to the frame */
    XReparentWindow(ob_display, self->client->window, self->plate, 0, 0);

    /*
      When reparenting the client window, it is usually not mapped yet, since
      this occurs from a MapRequest. However, in the case where Openbox is
      starting up, the window is already mapped, so we'll see unmap events for
      it. There are 2 unmap events generated that we see, one with the 'event'
      member set the root window, and one set to the client, but both get
      handled and need to be ignored.
    */
    if (ob_state() == OB_STATE_STARTING)
        self->client->ignore_unmaps += 2;

    /* select the event mask on the client's parent (to receive config/map
       req's) the ButtonPress is to catch clicks on the client border */
    XSelectInput(ob_display, self->plate, PLATE_EVENTMASK);

    /* map the client so it maps when the frame does */
    XMapWindow(ob_display, self->client->window);

    /* set all the windows for the frame in the window_map */
    g_hash_table_insert(window_map, &self->window, self->client);
    g_hash_table_insert(window_map, &self->plate, self->client);
    g_hash_table_insert(window_map, &self->inner, self->client);
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
    g_hash_table_insert(window_map, &self->leftresize, self->client);
    g_hash_table_insert(window_map, &self->rightresize, self->client);
}

void frame_release_client(ObFrame *self)
{
    XEvent ev;
    gboolean reparent = TRUE;

    /* if there was any animation going on, kill it */
    ob_main_loop_timeout_remove_data(ob_main_loop, frame_animate_iconify,
                                     self, FALSE);

    /* check if the app has already reparented its window away */
    while (XCheckTypedWindowEvent(ob_display, self->client->window,
                                  ReparentNotify, &ev))
    {
        /* This check makes sure we don't catch our own reparent action to
           our frame window. This doesn't count as the app reparenting itself
           away of course.

           Reparent events that are generated by us are just discarded here.
           They are of no consequence to us anyhow.
        */
        if (ev.xreparent.parent != self->plate) {
            reparent = FALSE;
            XPutBackEvent(ob_display, &ev);
            break;
        }
    }

    if (reparent) {
        /* according to the ICCCM - if the client doesn't reparent itself,
           then we will reparent the window to root for them */
        XReparentWindow(ob_display, self->client->window,
                        RootWindow(ob_display, ob_screen),
                        self->client->area.x,
                        self->client->area.y);
    }

    /* remove all the windows for the frame from the window_map */
    g_hash_table_remove(window_map, &self->window);
    g_hash_table_remove(window_map, &self->plate);
    g_hash_table_remove(window_map, &self->inner);
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
    g_hash_table_remove(window_map, &self->leftresize);
    g_hash_table_remove(window_map, &self->rightresize);

    ob_main_loop_timeout_remove_data(ob_main_loop, flash_timeout, self, TRUE);
}

/* is there anything present between us and the label? */
static gboolean is_button_present(ObFrame *self, const gchar *lc, gint dir) {
    for (; *lc != '\0' && lc >= config_title_layout; lc += dir) {
        if (*lc == ' ') continue; /* it was invalid */
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
        if (*lc == 'L') return FALSE;
    }
    return FALSE;
}

static void layout_title(ObFrame *self)
{
    gchar *lc;
    gint i;

    const gint bwidth = ob_rr_theme->button_size + ob_rr_theme->paddingx + 1;
    /* position of the left most button */
    const gint left = ob_rr_theme->paddingx + 1;
    /* position of the right most button */
    const gint right = self->width - bwidth;

    /* turn them all off */
    self->icon_on = self->desk_on = self->shade_on = self->iconify_on =
        self->max_on = self->close_on = self->label_on = FALSE;
    self->label_width = self->width - (ob_rr_theme->paddingx + 1) * 2;
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
            lc = config_title_layout;
            firstcon = &self->leftmost;
        } else {
            x = right;
            lc = config_title_layout + strlen(config_title_layout)-1;
            firstcon = &self->rightmost;
        }

        /* stop at the end of the string (or the label, which calls break) */
        for (; *lc != '\0' && lc >= config_title_layout; lc+=i) {
            if (*lc == 'L') {
                if (i > 0) {
                    self->label_on = TRUE;
                    self->label_x = x;
                }
                break; /* break the for loop, do other side of label */
            } else if (*lc == 'N') {
                if (firstcon) *firstcon = OB_FRAME_CONTEXT_ICON;
                if ((self->icon_on = is_button_present(self, lc, i))) {
                    /* icon is bigger than buttons */
                    self->label_width -= bwidth + 2;
                    self->icon_x = x;
                    x += i * (bwidth + 2);
                }
            } else if (*lc == 'D') {
                if (firstcon) *firstcon = OB_FRAME_CONTEXT_ALLDESKTOPS;
                if ((self->desk_on = is_button_present(self, lc, i))) {
                    self->label_width -= bwidth;
                    self->desk_x = x;
                    x += i * bwidth;
                }
            } else if (*lc == 'S') {
                if (firstcon) *firstcon = OB_FRAME_CONTEXT_SHADE;
                if ((self->shade_on = is_button_present(self, lc, i))) {
                    self->label_width -= bwidth;
                    self->shade_x = x;
                    x += i * bwidth;
                }
            } else if (*lc == 'I') {
                if (firstcon) *firstcon = OB_FRAME_CONTEXT_ICONIFY;
                if ((self->iconify_on = is_button_present(self, lc, i))) {
                    self->label_width -= bwidth;
                    self->iconify_x = x;
                    x += i * bwidth;
                }
            } else if (*lc == 'M') {
                if (firstcon) *firstcon = OB_FRAME_CONTEXT_MAXIMIZE;
                if ((self->max_on = is_button_present(self, lc, i))) {
                    self->label_width -= bwidth;
                    self->max_x = x;
                    x += i * bwidth;
                }
            } else if (*lc == 'C') {
                if (firstcon) *firstcon = OB_FRAME_CONTEXT_CLOSE;
                if ((self->close_on = is_button_present(self, lc, i))) {
                    self->label_width -= bwidth;
                    self->close_x = x;
                    x += i * bwidth;
                }
            } else
                continue; /* don't set firstcon */
            firstcon = NULL;
        }
    }

    /* position and map the elements */
    if (self->icon_on) {
        XMapWindow(ob_display, self->icon);
        XMoveWindow(ob_display, self->icon, self->icon_x,
                    ob_rr_theme->paddingy);
    } else
        XUnmapWindow(ob_display, self->icon);

    if (self->desk_on) {
        XMapWindow(ob_display, self->desk);
        XMoveWindow(ob_display, self->desk, self->desk_x,
                    ob_rr_theme->paddingy + 1);
    } else
        XUnmapWindow(ob_display, self->desk);

    if (self->shade_on) {
        XMapWindow(ob_display, self->shade);
        XMoveWindow(ob_display, self->shade, self->shade_x,
                    ob_rr_theme->paddingy + 1);
    } else
        XUnmapWindow(ob_display, self->shade);

    if (self->iconify_on) {
        XMapWindow(ob_display, self->iconify);
        XMoveWindow(ob_display, self->iconify, self->iconify_x,
                    ob_rr_theme->paddingy + 1);
    } else
        XUnmapWindow(ob_display, self->iconify);

    if (self->max_on) {
        XMapWindow(ob_display, self->max);
        XMoveWindow(ob_display, self->max, self->max_x,
                    ob_rr_theme->paddingy + 1);
    } else
        XUnmapWindow(ob_display, self->max);

    if (self->close_on) {
        XMapWindow(ob_display, self->close);
        XMoveWindow(ob_display, self->close, self->close_x,
                    ob_rr_theme->paddingy + 1);
    } else
        XUnmapWindow(ob_display, self->close);

    if (self->label_on) {
        self->label_width = MAX(1, self->label_width); /* no lower than 1 */
        XMapWindow(ob_display, self->label);
        XMoveWindow(ob_display, self->label, self->label_x,
                    ob_rr_theme->paddingy);
    } else
        XUnmapWindow(ob_display, self->label);
}

ObFrameContext frame_context_from_string(const gchar *name)
{
    if (!g_ascii_strcasecmp("Desktop", name))
        return OB_FRAME_CONTEXT_DESKTOP;
    else if (!g_ascii_strcasecmp("Root", name))
        return OB_FRAME_CONTEXT_ROOT;
    else if (!g_ascii_strcasecmp("Client", name))
        return OB_FRAME_CONTEXT_CLIENT;
    else if (!g_ascii_strcasecmp("Titlebar", name))
        return OB_FRAME_CONTEXT_TITLEBAR;
    else if (!g_ascii_strcasecmp("Frame", name))
        return OB_FRAME_CONTEXT_FRAME;
    else if (!g_ascii_strcasecmp("TLCorner", name))
        return OB_FRAME_CONTEXT_TLCORNER;
    else if (!g_ascii_strcasecmp("TRCorner", name))
        return OB_FRAME_CONTEXT_TRCORNER;
    else if (!g_ascii_strcasecmp("BLCorner", name))
        return OB_FRAME_CONTEXT_BLCORNER;
    else if (!g_ascii_strcasecmp("BRCorner", name))
        return OB_FRAME_CONTEXT_BRCORNER;
    else if (!g_ascii_strcasecmp("Top", name))
        return OB_FRAME_CONTEXT_TOP;
    else if (!g_ascii_strcasecmp("Bottom", name))
        return OB_FRAME_CONTEXT_BOTTOM;
    else if (!g_ascii_strcasecmp("Left", name))
        return OB_FRAME_CONTEXT_LEFT;
    else if (!g_ascii_strcasecmp("Right", name))
        return OB_FRAME_CONTEXT_RIGHT;
    else if (!g_ascii_strcasecmp("Maximize", name))
        return OB_FRAME_CONTEXT_MAXIMIZE;
    else if (!g_ascii_strcasecmp("AllDesktops", name))
        return OB_FRAME_CONTEXT_ALLDESKTOPS;
    else if (!g_ascii_strcasecmp("Shade", name))
        return OB_FRAME_CONTEXT_SHADE;
    else if (!g_ascii_strcasecmp("Iconify", name))
        return OB_FRAME_CONTEXT_ICONIFY;
    else if (!g_ascii_strcasecmp("Icon", name))
        return OB_FRAME_CONTEXT_ICON;
    else if (!g_ascii_strcasecmp("Close", name))
        return OB_FRAME_CONTEXT_CLOSE;
    else if (!g_ascii_strcasecmp("MoveResize", name))
        return OB_FRAME_CONTEXT_MOVE_RESIZE;
    return OB_FRAME_CONTEXT_NONE;
}

ObFrameContext frame_context(ObClient *client, Window win, gint x, gint y)
{
    ObFrame *self;

    if (moveresize_in_progress)
        return OB_FRAME_CONTEXT_MOVE_RESIZE;

    if (win == RootWindow(ob_display, ob_screen))
        return OB_FRAME_CONTEXT_ROOT ;
    if (client == NULL) return OB_FRAME_CONTEXT_NONE;
    if (win == client->window) {
        /* conceptually, this is the desktop, as far as users are
           concerned */
        if (client->type == OB_CLIENT_TYPE_DESKTOP)
            return OB_FRAME_CONTEXT_DESKTOP;
        return OB_FRAME_CONTEXT_CLIENT;
    }

    self = client->frame;
    if (win == self->inner || win == self->plate) {
        /* conceptually, this is the desktop, as far as users are
           concerned */
        if (client->type == OB_CLIENT_TYPE_DESKTOP)
            return OB_FRAME_CONTEXT_DESKTOP;
        return OB_FRAME_CONTEXT_CLIENT;
    }

    if (win == self->title) {
        /* when the user clicks in the corners of the titlebar and the client
           is fully maximized, then treat it like they clicked in the
           button that is there */
        if (self->client->max_horz && self->client->max_vert &&
            y < ob_rr_theme->paddingy + 1 + ob_rr_theme->button_size)
        {
            if (x < ((ob_rr_theme->paddingx + 1) * 2 +
                     ob_rr_theme->button_size)) {
                if (self->leftmost != OB_FRAME_CONTEXT_NONE)
                    return self->leftmost;
            }
            else if (x > (self->width -
                          (ob_rr_theme->paddingx + 1 +
                           ob_rr_theme->button_size)))
            {
                if (self->rightmost != OB_FRAME_CONTEXT_NONE)
                    return self->rightmost;
            }
        }
        return OB_FRAME_CONTEXT_TITLEBAR;
    }

    if (win == self->window)    return OB_FRAME_CONTEXT_FRAME;
    if (win == self->label)     return OB_FRAME_CONTEXT_TITLEBAR;
    if (win == self->handle)    return OB_FRAME_CONTEXT_BOTTOM;
    if (win == self->lgrip)     return OB_FRAME_CONTEXT_BLCORNER;
    if (win == self->rgrip)     return OB_FRAME_CONTEXT_BRCORNER;
    if (win == self->topresize) return OB_FRAME_CONTEXT_TOP;
    if (win == self->tltresize) return OB_FRAME_CONTEXT_TLCORNER;
    if (win == self->tllresize) return OB_FRAME_CONTEXT_TLCORNER;
    if (win == self->trtresize) return OB_FRAME_CONTEXT_TRCORNER;
    if (win == self->trrresize) return OB_FRAME_CONTEXT_TRCORNER;
    if (win == self->leftresize) return OB_FRAME_CONTEXT_LEFT;
    if (win == self->rightresize) return OB_FRAME_CONTEXT_RIGHT;
    if (win == self->max)       return OB_FRAME_CONTEXT_MAXIMIZE;
    if (win == self->iconify)   return OB_FRAME_CONTEXT_ICONIFY;
    if (win == self->close)     return OB_FRAME_CONTEXT_CLOSE;
    if (win == self->icon)      return OB_FRAME_CONTEXT_ICON;
    if (win == self->desk)      return OB_FRAME_CONTEXT_ALLDESKTOPS;
    if (win == self->shade)     return OB_FRAME_CONTEXT_SHADE;

    return OB_FRAME_CONTEXT_NONE;
}

void frame_client_gravity(ObFrame *self, gint *x, gint *y, gint w, gint h)
{
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
        *x -= (self->size.left + w) / 2;
        break;

    case NorthEastGravity:
    case SouthEastGravity:
    case EastGravity:
        *x -= (self->size.left + self->size.right + w) - 1;
        break;

    case ForgetGravity:
    case StaticGravity:
        *x -= self->size.left;
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
        *y -= (self->size.top + h) / 2;
        break;

    case SouthWestGravity:
    case SouthEastGravity:
    case SouthGravity:
        *y -= (self->size.top + self->size.bottom + h) - 1;
        break;

    case ForgetGravity:
    case StaticGravity:
        *y -= self->size.top;
        break;
    }
}

void frame_frame_gravity(ObFrame *self, gint *x, gint *y, gint w, gint h)
{
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
        *x += (self->size.left + w) / 2;
        break;
    case NorthEastGravity:
    case EastGravity:
    case SouthEastGravity:
        *x += (self->size.left + self->size.right + w) - 1;
        break;
    case StaticGravity:
    case ForgetGravity:
        *x += self->size.left;
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
        *y += (self->size.top + h) / 2;
        break;
    case SouthWestGravity:
    case SouthGravity:
    case SouthEastGravity:
        *y += (self->size.top + self->size.bottom + h) - 1;
        break;
    case StaticGravity:
    case ForgetGravity:
        *y += self->size.top;
        break;
    }
}

static void flash_done(gpointer data)
{
    ObFrame *self = data;

    if (self->focused != self->flash_on)
        frame_adjust_focus(self, self->focused);
}

static gboolean flash_timeout(gpointer data)
{
    ObFrame *self = data;
    GTimeVal now;

    g_get_current_time(&now);
    if (now.tv_sec > self->flash_end.tv_sec ||
        (now.tv_sec == self->flash_end.tv_sec &&
         now.tv_usec >= self->flash_end.tv_usec))
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

void frame_flash_start(ObFrame *self)
{
    self->flash_on = self->focused;

    if (!self->flashing)
        ob_main_loop_timeout_add(ob_main_loop,
                                 G_USEC_PER_SEC * 0.6,
                                 flash_timeout,
                                 self,
                                 g_direct_equal,
                                 flash_done);
    g_get_current_time(&self->flash_end);
    g_time_val_add(&self->flash_end, G_USEC_PER_SEC * 5);
    
    self->flashing = TRUE;
}

void frame_flash_stop(ObFrame *self)
{
    self->flashing = FALSE;
}

static gulong frame_animate_iconify_time_left(ObFrame *self,
                                              const GTimeVal *now)
{
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

static gboolean frame_animate_iconify(gpointer p)
{
    ObFrame *self = p;
    gint x, y, w, h;
    gint iconx, icony, iconw;
    GTimeVal now;
    gulong time;
    gboolean iconifying;

    if (self->client->icon_geometry.width == 0) {
        /* there is no icon geometry set so just go straight down */
        Rect *a = screen_physical_area();
        iconx = self->area.x + self->area.width / 2 + 32;
        icony = a->y + a->width;
        iconw = 64;
    } else {
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
        w = self->area.width - self->bwidth * 2;
        h = self->area.height - self->bwidth * 2;
    } else {
        /* start at the icon */
        x = iconx;
        y = icony;
        w = iconw;
        h = self->innersize.top; /* just the titlebar */
    }

    if (time > 0) {
        glong dx, dy, dw;
        glong elapsed;

        dx = self->area.x - iconx;
        dy = self->area.y - icony;
        dw = self->area.width - self->bwidth * 2 - iconw;
         /* if restoring, we move in the opposite direction */
        if (!iconifying) { dx = -dx; dy = -dy; dw = -dw; }

        elapsed = FRAME_ANIMATE_ICONIFY_TIME - time;
        x = x - (dx * elapsed) / FRAME_ANIMATE_ICONIFY_TIME;
        y = y - (dy * elapsed) / FRAME_ANIMATE_ICONIFY_TIME;
        w = w - (dw * elapsed) / FRAME_ANIMATE_ICONIFY_TIME;
        h = self->innersize.top; /* just the titlebar */
    }

    if (time == 0)
        frame_end_iconify_animation(self);
    else {
        XMoveResizeWindow(ob_display, self->window, x, y, w, h);
        XFlush(ob_display);
    }

    return time > 0; /* repeat until we're out of time */
}

void frame_end_iconify_animation(ObFrame *self)
{
    /* see if there is an animation going */
    if (self->iconify_animation_going == 0) return;

    if (!self->visible)
        XUnmapWindow(ob_display, self->window);
    else
        /* Send a ConfigureNotify when the animation is done, this fixes
           KDE's pager showing the window in the wrong place. */
        client_reconfigure(self->client);

    /* we're not animating any more ! */
    self->iconify_animation_going = 0;

    XMoveResizeWindow(ob_display, self->window,
                      self->area.x, self->area.y,
                      self->area.width - self->bwidth * 2,
                      self->area.height - self->bwidth * 2);
    XFlush(ob_display);
}

void frame_begin_iconify_animation(ObFrame *self, gboolean iconifying)
{
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
        } else
            /* animation was already going in the same direction */
            set_end = FALSE;
    } else
        new_anim = TRUE;
    self->iconify_animation_going = iconifying ? 1 : -1;

    /* set the ending time */
    if (set_end) {
        self->iconify_animation_end.tv_sec = now.tv_sec;
        self->iconify_animation_end.tv_usec = now.tv_usec;
        g_time_val_add(&self->iconify_animation_end, time);
    }

    if (new_anim) {
        ob_main_loop_timeout_remove_data(ob_main_loop, frame_animate_iconify,
                                         self, FALSE);
        ob_main_loop_timeout_add(ob_main_loop,
                                 FRAME_ANIMATE_ICONIFY_STEP_TIME,
                                 frame_animate_iconify, self,
                                 g_direct_equal, NULL);

        /* do the first step */
        frame_animate_iconify(self);

        /* show it during the animation even if it is not "visible" */
        if (!self->visible)
            XMapWindow(ob_display, self->window);
    }
}
