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
#include "grab.h"
#include "debug.h"
#include "config.h"
#include "framerender.h"
#include "focus_cycle.h"
#include "focus_cycle_indicator.h"
#include "moveresize.h"
#include "screen.h"
#include "obrender/theme.h"
#include "obt/display.h"
#include "obt/xqueue.h"
#include "obt/prop.h"

#define FRAME_EVENTMASK (EnterWindowMask | LeaveWindowMask | \
                         ButtonPressMask | ButtonReleaseMask | \
                         SubstructureRedirectMask | FocusChangeMask)
#define ELEMENT_EVENTMASK (ButtonPressMask | ButtonReleaseMask | \
                           ButtonMotionMask | PointerMotionMask | \
                           EnterWindowMask | LeaveWindowMask)

#define FRAME_ANIMATE_ICONIFY_TIME 150000 /* .15 seconds */
#define FRAME_ANIMATE_ICONIFY_STEP_TIME (1000 / 60) /* 60 Hz */

#define FRAME_HANDLE_Y(f) (f->size.top + f->client->area.height + f->cbwidth_b)

static void flash_done(gpointer data);
static gboolean flash_timeout(gpointer data);

static void set_theme_statics(ObFrame *self);
static void free_theme_statics(ObFrame *self);
static gboolean frame_animate_iconify(gpointer self);
static void frame_adjust_cursors(ObFrame *self);

static Window createWindow(Window parent, Visual *visual,
                           gulong mask, XSetWindowAttributes *attrib)
{
    return XCreateWindow(obt_display, parent, 0, 0, 1, 1, 0,
                         (visual ? 32 : RrDepth(ob_rr_inst)), InputOutput,
                         (visual ? visual : RrVisual(ob_rr_inst)),
                         mask, attrib);

}

static Visual *check_32bit_client(ObClient *c)
{
    XWindowAttributes wattrib;
    Status ret;

    /* we're already running at 32 bit depth, yay. we don't need to use their
       visual */
    if (RrDepth(ob_rr_inst) == 32)
        return NULL;

    ret = XGetWindowAttributes(obt_display, c->window, &wattrib);
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

    self = g_slice_new0(ObFrame);
    self->client = client;

    visual = check_32bit_client(client);

    /* create the non-visible decor windows */

    mask = 0;
    if (visual) {
        /* client has a 32-bit visual */
        mask = CWColormap | CWBackPixel | CWBorderPixel;
        /* create a colormap with the visual */
        self->colormap = attrib.colormap =
            XCreateColormap(obt_display, obt_root(ob_screen),
                            visual, AllocNone);
        attrib.background_pixel = BlackPixel(obt_display, ob_screen);
        attrib.border_pixel = BlackPixel(obt_display, ob_screen);
    }
    self->window = createWindow(obt_root(ob_screen), visual,
                                mask, &attrib);

    /* create the visible decor windows */

    mask = 0;
    if (visual) {
        /* client has a 32-bit visual */
        mask = CWColormap | CWBackPixel | CWBorderPixel;
        attrib.colormap = RrColormap(ob_rr_inst);
    }

    self->backback = createWindow(self->window, NULL, mask, &attrib);
    self->backfront = createWindow(self->backback, NULL, mask, &attrib);
    XMapWindow(obt_display, self->backback);
    XMapWindow(obt_display, self->backfront);

    mask |= CWEventMask;
    attrib.event_mask = ELEMENT_EVENTMASK;
    /* XXX make visible decor sub-windows here */
    /* XXX map decor sub-windows that are always shown here */

    self->focused = FALSE;

    self->max_press = self->close_press = self->desk_press =
        self->iconify_press = self->shade_press = FALSE;
    self->max_hover = self->close_hover = self->desk_hover =
        self->iconify_hover = self->shade_hover = FALSE;

    /* make sure the size will be different the first time, so the extent hints
       will be set */
    STRUT_SET(self->oldsize, -1, -1, -1, -1);

    set_theme_statics(self);

    return self;
}

static void set_theme_statics(ObFrame *self)
{
    /* XXX set colors/appearance/sizes for stuff that doesn't change */
}

static void free_theme_statics(ObFrame *self)
{
}

void frame_free(ObFrame *self)
{
    free_theme_statics(self);

    XDestroyWindow(obt_display, self->window);
    if (self->colormap)
        XFreeColormap(obt_display, self->colormap);

    g_slice_free(ObFrame, self);
}

void frame_show(ObFrame *self)
{
    if (!self->visible) {
        self->visible = TRUE;
        framerender_frame(self);
        /* Grab the server to make sure that the frame window is mapped before
           the client gets its MapNotify, i.e. to make sure the client is
           _visible_ when it gets MapNotify. */
        grab_server(TRUE);
        XMapWindow(obt_display, self->client->window);
        XMapWindow(obt_display, self->window);
        grab_server(FALSE);
    }
}

void frame_hide(ObFrame *self)
{
    if (self->visible) {
        self->visible = FALSE;
        if (!frame_iconify_animating(self))
            XUnmapWindow(obt_display, self->window);
        /* we unmap the client itself so that we can get MapRequest
           events, and because the ICCCM tells us to! */
        XUnmapWindow(obt_display, self->client->window);
        self->client->ignore_unmaps += 1;
    }
}

void frame_adjust_theme(ObFrame *self)
{
    free_theme_statics(self);
    set_theme_statics(self);
}

#ifdef SHAPE
void frame_adjust_shape_kind(ObFrame *self, int kind)
{
    gint num;
    XRectangle xrect[2];

    if (!((kind == ShapeBounding && self->client->shaped) ||
          (kind == ShapeInput && self->client->shaped_input))) {
        /* clear the shape on the frame window */
        XShapeCombineMask(obt_display, self->window, kind,
                          self->size.left,
                          self->size.top,
                          None, ShapeSet);
    } else {
        /* make the frame's shape match the clients */
        XShapeCombineShape(obt_display, self->window, kind,
                           self->size.left,
                           self->size.top,
                           self->client->window,
                           kind, ShapeSet);

        num = 0;
        if (self->decorations & OB_FRAME_DECOR_TITLEBAR) {
            xrect[0].x = 0;
            xrect[0].y = 0;
            xrect[0].width = self->area.width;
            xrect[0].height = self->size.top;
            ++num;
        }

        if (self->decorations & OB_FRAME_DECOR_HANDLE &&
            ob_rr_theme->handle_height > 0)
        {
            xrect[1].x = 0;
            xrect[1].y = FRAME_HANDLE_Y(self);
            xrect[1].width = self->area.width;
            xrect[1].height = ob_rr_theme->handle_height +
                self->bwidth * 2;
            ++num;
        }

        XShapeCombineRectangles(obt_display, self->window,
                                ShapeBounding, 0, 0, xrect, num,
                                ShapeUnion, Unsorted);
    }
}
#endif

void frame_adjust_shape(ObFrame *self)
{
#ifdef SHAPE
  frame_adjust_shape_kind(self, ShapeBounding);
  frame_adjust_shape_kind(self, ShapeInput);
#endif
}

void frame_adjust_area(ObFrame *self, gboolean moved,
                       gboolean resized, gboolean fake)
{
    /* XXX fake should not exist !! it is used in two cases:
       1) when "fake managing a window" just to report the window's frame.size
       2) when trying out a move/resize to see what the result would be.
          again, this is just to find out the frame's theoretical geometry,
          and actually changing it is potentially problematic.  there should
          be a separate function that returns the frame's geometry that this
          can use, and outsiders can use it to "test" a configuration.
    */

    /* XXX this notion of "resized" doesn't really make sense anymore, it is
       more of a "the frame might be changing more than it's position.  it
       also occurs from state changes in the client, and a different name for
       it would make sense.  basically, if resized is FALSE then the client
       can only have moved and had no other change take place. if moved is
       also FALSE then it didn't change at all !

       resized is only false when reconfiguring (move/resizing) a window
       and not changing its size, and its maximized/shaded states and decor
       match the frame's.

       moved and resized should be something decided HERE, not by the caller?
       would need to keep the client's old position/size/gravity/etc all
       mirrored here, thats maybe a lot of state, maybe not more than was here
       already.. not all states affect decor.
    */

    if (resized) {
        /* do this before changing the frame's status like max_horz max_vert,
           as it compares them to the client's to see if things need to
           change */
        frame_adjust_cursors(self);

        self->functions = self->client->functions;
        self->decorations = self->client->decorations;
        self->max_horz = self->client->max_horz;
        self->max_vert = self->client->max_vert;
        self->shaded = self->client->shaded;

        if (self->decorations & OB_FRAME_DECOR_BORDER)
            self->bwidth = ob_rr_theme->fbwidth;
        else
            self->bwidth = 0;

        if (self->decorations & OB_FRAME_DECOR_BORDER &&
            !self->client->undecorated)
        {
            self->cbwidth_b = 0;
        } else
            self->cbwidth_b = 0;

        if (self->max_horz) {
            /* horz removes some decor? */;
            if (self->max_vert)
                /* vert also removes more decor? */;
        } else
            /* only vert or not max at all */;

        /* XXX set the size of the frame around the client... */
        STRUT_SET(self->size,
                  self->bwidth, self->bwidth, self->bwidth, self->bwidth);
        /* ... which may depend on what decor is being shown */
        if (self->decorations & OB_FRAME_DECOR_TITLEBAR)
            ;
        if (self->decorations & OB_FRAME_DECOR_HANDLE)
            ;

        /* XXX set the size of the frame, which may depend on states such as
           being shaded. */
        RECT_SET_SIZE(self->area,
                      self->client->area.width +
                      self->size.left + self->size.right,
                      self->client->area.height +
                      self->size.top + self->size.bottom);

        if (!fake) {
            XMoveResizeWindow(obt_display, self->backback,
                              self->size.left, self->size.top,
                              self->client->area.width,
                              self->client->area.height);

            /* XXX set up all the decor sub-windows if not fake.  when it's
               fake that means we want to calc stuff but not change anything
               visibly. */
        }
    }

    if ((moved || resized) && !fake) {
        /* XXX the geometry (such as frame.size strut) of the frame is
           recalculated when !fake, but the position of the frame is not
           changed.

           i don't know why this does not also happen for fakes.
        */

        /* find the new coordinates for the frame, which must be done after
           setting the frame.size (frame_client_gravity uses it) */
        self->area.x = self->client->area.x;
        self->area.y = self->client->area.y;
        frame_client_gravity(self, &self->area.x, &self->area.y);
    }

    if (!fake) {
        /* XXX not sure why this happens even if moved and resized are
           FALSE. */

        /* actually move/resize the frame window if !fake and not animating */
        if (!frame_iconify_animating(self)) {
            /* move and resize the top level frame.
               shading can change without being moved or resized.

               but don't do this during an iconify animation. it will be
               reflected afterwards.
            */
            XMoveResizeWindow(obt_display, self->window,
                              self->area.x,
                              self->area.y,
                              self->area.width,
                              self->area.height);

            /* when the client has StaticGravity, it likes to move around.
               also this correctly positions the client when it maps.
               this also needs to be run when the frame's decorations sizes
               change!
            */
            XMoveWindow(obt_display, self->client->window,
                        self->size.left, self->size.top);
        }

        if (resized) {
            /* mark the frame that it needs to be repainted */
            self->need_render = TRUE;
            /* draw the frame */
            framerender_frame(self);
            /* adjust the shape masks inside the frame to match the client's */
            frame_adjust_shape(self);
        }

        /* set hints to tell apps what their frame size is */
        if (!STRUT_EQUAL(self->size, self->oldsize)) {
            gulong vals[4];
            vals[0] = self->size.left;
            vals[1] = self->size.right;
            vals[2] = self->size.top;
            vals[3] = self->size.bottom;
            OBT_PROP_SETA32(self->client->window, NET_FRAME_EXTENTS,
                            CARDINAL, vals, 4);
            OBT_PROP_SETA32(self->client->window, KDE_NET_WM_FRAME_STRUT,
                            CARDINAL, vals, 4);
            self->oldsize = self->size;
        }

        /* if this occurs while we are focus cycling, the indicator needs to
           match the changes */
        if (focus_cycle_target == self->client)
            focus_cycle_update_indicator(self->client);
    }
}

static void frame_adjust_cursors(ObFrame *self)
{
    if ((self->functions & OB_CLIENT_FUNC_RESIZE) !=
        (self->client->functions & OB_CLIENT_FUNC_RESIZE) ||
        self->max_horz != self->client->max_horz ||
        self->max_vert != self->client->max_vert ||
        self->shaded != self->client->shaded)
    {
        gboolean r = (self->client->functions & OB_CLIENT_FUNC_RESIZE) &&
            !(self->client->max_horz && self->client->max_vert);
        gboolean topbot = !self->client->max_vert;
        gboolean sh = self->client->shaded;
        XSetWindowAttributes a;

        /* these ones turn off when max vert, and some when shaded */
        a.cursor = ob_cursor(r && topbot && !sh ?
                             OB_CURSOR_NORTH : OB_CURSOR_NONE);
        /* XXX set north cursors on decor sub-windows
           XChangeWindowAttributes(obt_display, ..., CWCursor, &a); */
        a.cursor = ob_cursor(r && topbot ? OB_CURSOR_SOUTH : OB_CURSOR_NONE);
        /* XXX set south cursors on decor sub-windows
           XChangeWindowAttributes(obt_display, ..., CWCursor, &a); */

        /* these ones change when shaded */
        a.cursor = ob_cursor(r ? (sh ? OB_CURSOR_WEST : OB_CURSOR_NORTHWEST) :
                             OB_CURSOR_NONE);
        /* XXX set northwest cursors on decor sub-windows
           XChangeWindowAttributes(obt_display, ..., CWCursor, &a); */
        a.cursor = ob_cursor(r ? (sh ? OB_CURSOR_EAST : OB_CURSOR_NORTHEAST) :
                             OB_CURSOR_NONE);
        /* XXX set northeast cursors on decor sub-windows
           XChangeWindowAttributes(obt_display, ..., CWCursor, &a); */

        /* these ones are pretty static */
        a.cursor = ob_cursor(r ? OB_CURSOR_WEST : OB_CURSOR_NONE);
        /* XXX set west cursors on decor sub-windows
           XChangeWindowAttributes(obt_display, ..., CWCursor, &a); */
        a.cursor = ob_cursor(r ? OB_CURSOR_EAST : OB_CURSOR_NONE);
        /* XXX set east cursors on decor sub-windows
           XChangeWindowAttributes(obt_display, ..., CWCursor, &a); */
        a.cursor = ob_cursor(r ? OB_CURSOR_SOUTHWEST : OB_CURSOR_NONE);
        /* XXX set southwest cursors on decor sub-windows
           XChangeWindowAttributes(obt_display, ..., CWCursor, &a); */
        a.cursor = ob_cursor(r ? OB_CURSOR_SOUTHEAST : OB_CURSOR_NONE);
        /* XXX set southeast cursors on decor sub-windows
           XChangeWindowAttributes(obt_display, ..., CWCursor, &a); */
    }
}

void frame_adjust_client_area(ObFrame *self)
{
    /* adjust the window which is there to prevent flashing on unmap */
    XMoveResizeWindow(obt_display, self->backfront, 0, 0,
                      self->client->area.width,
                      self->client->area.height);
}

void frame_adjust_state(ObFrame *self)
{
    self->need_render = TRUE;
    framerender_frame(self);
}

void frame_adjust_focus(ObFrame *self, gboolean hilite)
{
    ob_debug_type(OB_DEBUG_FOCUS,
                  "Frame for 0x%x has focus: %d\n",
                  self->client->window, hilite);
    self->focused = hilite;
    self->need_render = TRUE;
    framerender_frame(self);
    XFlush(obt_display);
}

void frame_adjust_title(ObFrame *self)
{
    self->need_render = TRUE;
    framerender_frame(self);
}

void frame_adjust_icon(ObFrame *self)
{
    self->need_render = TRUE;
    framerender_frame(self);
}

void frame_grab_client(ObFrame *self)
{
    /* DO NOT map the client window here. we used to do that, but it is bogus.
       we need to set up the client's dimensions and everything before we
       send a mapnotify or we create race conditions.
    */

    /* reparent the client to the frame */
    XReparentWindow(obt_display, self->client->window, self->window, 0, 0);

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
    XSelectInput(obt_display, self->window, FRAME_EVENTMASK);

    /* set all the windows for the frame in the window_map */
    window_add(&self->window, CLIENT_AS_WINDOW(self->client));
    window_add(&self->backback, CLIENT_AS_WINDOW(self->client));
    window_add(&self->backfront, CLIENT_AS_WINDOW(self->client));
    /* XXX add any decor sub-windows that may receive events */
}

static gboolean find_reparent(XEvent *e, gpointer data)
{
    const ObFrame *self = data;

    /* Find ReparentNotify events for the window that aren't being reparented into the
       frame, thus the client reparenting itself off the frame. */
    return e->type == ReparentNotify && e->xreparent.window == self->client->window &&
        e->xreparent.parent != self->window;
}

void frame_release_client(ObFrame *self)
{
    /* if there was any animation going on, kill it */
    if (self->iconify_animation_timer)
        g_source_remove(self->iconify_animation_timer);

    /* check if the app has already reparented its window away */
    if (!xqueue_exists_local(find_reparent, self)) {
        /* according to the ICCCM - if the client doesn't reparent itself,
           then we will reparent the window to root for them */
        XReparentWindow(obt_display, self->client->window, obt_root(ob_screen),
                        self->client->area.x, self->client->area.y);
    }

    /* remove all the windows for the frame from the window_map */
    window_remove(self->window);
    window_remove(self->backback);
    window_remove(self->backfront);
    /* XXX add any decor sub-windows that may receive events */

    if (self->flash_timer) g_source_remove(self->flash_timer);
}

gboolean frame_next_context_from_string(gchar *names, ObFrameContext *cx)
{
    gchar *p, *n;

    if (!*names) /* empty string */
        return FALSE;

    /* find the first space */
    for (p = names; *p; p = g_utf8_next_char(p)) {
        const gunichar c = g_utf8_get_char(p);
        if (g_unichar_isspace(c)) break;
    }

    if (p == names) {
        /* leading spaces in the string */
        n = g_utf8_next_char(names);
        if (!frame_next_context_from_string(n, cx))
            return FALSE;
    } else {
        n = p;
        if (*p) {
            /* delete the space with null zero(s) */
            while (n < g_utf8_next_char(p))
                *(n++) = '\0';
        }

        *cx = frame_context_from_string(names);

        /* find the next non-space */
        for (; *n; n = g_utf8_next_char(n)) {
            const gunichar c = g_utf8_get_char(n);
            if (!g_unichar_isspace(c)) break;
        }
    }

    /* delete everything we just read (copy everything at n to the start of
       the string */
    for (p = names; *n; ++p, ++n)
        *p = *n;
    *p = *n;

    return TRUE;
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
    else if (!g_ascii_strcasecmp("Dock", name))
        return OB_FRAME_CONTEXT_DOCK;

    return OB_FRAME_CONTEXT_NONE;
}

ObFrameContext frame_context(ObClient *client, Window win, gint x, gint y)
{
    ObFrame *self;
    ObWindow *obwin;

    if (moveresize_in_progress)
        return OB_FRAME_CONTEXT_MOVE_RESIZE;

    if (win == obt_root(ob_screen))
        return OB_FRAME_CONTEXT_ROOT;
    if ((obwin = window_find(win))) {
        if (WINDOW_IS_DOCK(obwin)) {
          return OB_FRAME_CONTEXT_DOCK;
        }
    }
    if (client == NULL) return OB_FRAME_CONTEXT_NONE;
    if (win == client->window) {
        /* conceptually, this is the desktop, as far as users are
           concerned */
        if (client->type == OB_CLIENT_TYPE_DESKTOP)
            return OB_FRAME_CONTEXT_DESKTOP;
        return OB_FRAME_CONTEXT_CLIENT;
    }

    self = client->frame;

    /* when the user clicks in the corners of the titlebar and the client
       is fully maximized, then treat it like they clicked in the
       button that is there */
    if (self->max_horz && self->max_vert &&
        (win == 0
         /* XXX some windows give a different content when fully maxd */))
    {
        /* XXX i.e. the topright corner was considered to be == the rightmost
           titlebar button */
        return OB_FRAME_CONTEXT_TITLEBAR;
    }
    else if (self->max_vert &&
             (win == 0
              /* XXX some windows give a different content when vert maxd */))
        /* XXX i.e. the top stopped existing and became the titlebar cuz you
           couldnt resize it anymore (this is changing) */
        return OB_FRAME_CONTEXT_TITLEBAR;
    else if (self->shaded &&
             (win == 0
              /* XXX some windows give a different context when shaded */))
        /* XXX i.e. the top/bottom became the titlebar cuz you
           can't resize vertically when shaded (still true) */
        return OB_FRAME_CONTEXT_TITLEBAR;

    if (win == self->window)            return OB_FRAME_CONTEXT_FRAME;

    /* XXX add all the decor sub-windows, or calculate position of the mouse
       to determine the context */

    /* XXX if its not in any other context then its not in an input context */
    return OB_FRAME_CONTEXT_NONE;
}

void frame_client_gravity(ObFrame *self, gint *x, gint *y)
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
        /* the middle of the client will be the middle of the frame */
        *x -= (self->size.right - self->size.left) / 2;
        break;

    case NorthEastGravity:
    case SouthEastGravity:
    case EastGravity:
        /* the right side of the client will be the right side of the frame */
        *x -= self->size.right + self->size.left -
            self->client->border_width * 2;
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
        *y -= self->size.bottom + self->size.top -
            self->client->border_width * 2;
        break;

    case ForgetGravity:
    case StaticGravity:
        /* the client's position won't move */
        *y -= self->size.top - self->client->border_width;
        break;
    }
}

void frame_frame_gravity(ObFrame *self, gint *x, gint *y)
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
        /* the middle of the client will be the middle of the frame */
        *x += (self->size.right - self->size.left) / 2;
        break;
    case NorthEastGravity:
    case EastGravity:
    case SouthEastGravity:
        /* the right side of the client will be the right side of the frame */
        *x += self->size.right + self->size.left -
            self->client->border_width * 2;
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
        *y += self->size.bottom + self->size.top -
            self->client->border_width * 2;
        break;
    case StaticGravity:
    case ForgetGravity:
        /* the client's position won't move */
        *y += self->size.top - self->client->border_width;
        break;
    }
}

void frame_rect_to_frame(ObFrame *self, Rect *r)
{
    r->width += self->size.left + self->size.right;
    r->height += self->size.top + self->size.bottom;
    frame_client_gravity(self, &r->x, &r->y);
}

void frame_rect_to_client(ObFrame *self, Rect *r)
{
    r->width -= self->size.left + self->size.right;
    r->height -= self->size.top + self->size.bottom;
    frame_frame_gravity(self, &r->x, &r->y);
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

    XFlush(obt_display);
    return TRUE; /* go again */
}

void frame_flash_start(ObFrame *self)
{
    self->flash_on = self->focused;

    if (!self->flashing)
        self->flash_timer = g_timeout_add_full(G_PRIORITY_DEFAULT,
                                               600, flash_timeout, self,
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
        const Rect *a;

        a = screen_physical_area_monitor(screen_find_monitor(&self->area));
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

    if ((time > 0 && iconifying) || (time == 0 && !iconifying)) {
        /* start where the frame is supposed to be */
        x = self->area.x;
        y = self->area.y;
        w = self->area.width;
        h = self->area.height;
    } else {
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
        if (!iconifying) { dx = -dx; dy = -dy; dw = -dw; }

        elapsed = FRAME_ANIMATE_ICONIFY_TIME - time;
        x = x - (dx * elapsed) / FRAME_ANIMATE_ICONIFY_TIME;
        y = y - (dy * elapsed) / FRAME_ANIMATE_ICONIFY_TIME;
        w = w - (dw * elapsed) / FRAME_ANIMATE_ICONIFY_TIME;
        h = self->size.top; /* just the titlebar */
    }

    XMoveResizeWindow(obt_display, self->window, x, y, w, h);

    if (time == 0)
        frame_end_iconify_animation(self);

    XFlush(obt_display);
    return time > 0; /* repeat until we're out of time */
}

void frame_end_iconify_animation(ObFrame *self)
{
    /* see if there is an animation going */
    if (self->iconify_animation_going == 0) return;

    if (!self->visible)
        XUnmapWindow(obt_display, self->window);
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

    XMoveResizeWindow(obt_display, self->window,
                      self->area.x, self->area.y,
                      self->area.width, self->area.height);
    /* we delay re-rendering until after we're done animating */
    framerender_frame(self);
    XFlush(obt_display);
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
        if (self->iconify_animation_timer)
            g_source_remove(self->iconify_animation_timer);
        self->iconify_animation_timer =
            g_timeout_add_full(G_PRIORITY_DEFAULT,
                               FRAME_ANIMATE_ICONIFY_STEP_TIME,
                               frame_animate_iconify, self, NULL);
                               

        /* do the first step */
        frame_animate_iconify(self);

        /* show it during the animation even if it is not "visible" */
        if (!self->visible)
            XMapWindow(obt_display, self->window);
    }
}
