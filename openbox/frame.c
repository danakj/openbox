#include "frame.h"
#include "openbox.h"
#include "extensions.h"
#include "framerender.h"
#include "render2/theme.h"

#define PLATE_EVENTMASK (SubstructureRedirectMask | ButtonPressMask)
#define FRAME_EVENTMASK (EnterWindowMask | LeaveWindowMask | \
                         ButtonPressMask | ButtonReleaseMask | ExposureMask)
#define ELEMENT_EVENTMASK (ButtonPressMask | ButtonReleaseMask | \
                           ButtonMotionMask | ExposureMask)

static void layout_title(Frame *self);

void frame_startup()
{
}

void frame_shutdown()
{
}

static Window createWindow(Window parent, unsigned long mask,
			   XSetWindowAttributes *attrib)
{
    return XCreateWindow(ob_display, parent, 0, 0, 1, 1, 0,
			 RrInstanceDepth(ob_render_inst), InputOutput,
                         RrInstanceVisual(ob_render_inst), mask, attrib);
                       
}

Frame *frame_new()
{
    XSetWindowAttributes attrib;
    unsigned long mask;
    Frame *self;

    self = g_new(Frame, 1);

    self->visible = FALSE;

    /* create all of the decor windows */
    mask = CWOverrideRedirect | CWEventMask;
    attrib.event_mask = FRAME_EVENTMASK;
    attrib.override_redirect = TRUE;
    self->window = createWindow(ob_root, mask, &attrib);

    mask = 0;
    self->plate = createWindow(self->window, mask, &attrib);

    self->s_frame = RrSurfaceNew(ob_render_inst, 0, self->window, 0);
    self->s_title = RrSurfaceNewChild(0, self->s_frame, 0);
    self->s_label = RrSurfaceNewChild(0, self->s_title, 0);
    self->s_max = RrSurfaceNewChild(0, self->s_title, 0);
    self->s_close = RrSurfaceNewChild(0, self->s_title, 0);
    self->s_desk = RrSurfaceNewChild(0, self->s_title, 0);
    self->s_shade = RrSurfaceNewChild(0, self->s_title, 0);
    self->s_iconify = RrSurfaceNewChild(0, self->s_title, 0);
    self->s_icon = RrSurfaceNewChild(0, self->s_title, 0);
    self->s_handle = RrSurfaceNewChild(0, self->s_frame, 0);
    self->s_lgrip = RrSurfaceNewChild(0, self->s_handle, 0);
    self->s_rgrip = RrSurfaceNewChild(0, self->s_handle, 0);

    self->w_title = RrSurfaceWindow(self->s_title);
    self->w_label = RrSurfaceWindow(self->s_label);
    self->w_max = RrSurfaceWindow(self->s_max);
    self->w_close = RrSurfaceWindow(self->s_close);
    self->w_desk = RrSurfaceWindow(self->s_desk);
    self->w_shade = RrSurfaceWindow(self->s_shade);
    self->w_iconify = RrSurfaceWindow(self->s_iconify);
    self->w_icon = RrSurfaceWindow(self->s_icon);
    self->w_handle = RrSurfaceWindow(self->s_handle);
    self->w_lgrip = RrSurfaceWindow(self->s_lgrip);
    self->w_rgrip = RrSurfaceWindow(self->s_rgrip);

    XSelectInput(ob_display, self->w_title, ELEMENT_EVENTMASK);
    XSelectInput(ob_display, self->w_label, ELEMENT_EVENTMASK);
    XSelectInput(ob_display, self->w_max, ELEMENT_EVENTMASK);
    XSelectInput(ob_display, self->w_close, ELEMENT_EVENTMASK);
    XSelectInput(ob_display, self->w_desk, ELEMENT_EVENTMASK);
    XSelectInput(ob_display, self->w_shade, ELEMENT_EVENTMASK);
    XSelectInput(ob_display, self->w_iconify, ELEMENT_EVENTMASK);
    XSelectInput(ob_display, self->w_icon, ELEMENT_EVENTMASK);
    XSelectInput(ob_display, self->w_handle, ELEMENT_EVENTMASK);
    XSelectInput(ob_display, self->w_lgrip, ELEMENT_EVENTMASK);
    XSelectInput(ob_display, self->w_rgrip, ELEMENT_EVENTMASK);

    XDefineCursor(ob_display, self->w_lgrip, ob_cursors.bl);
    XDefineCursor(ob_display, self->w_rgrip, ob_cursors.br);

    self->focused = FALSE;

    /* the other stuff is shown based on decor settings */
    XMapWindow(ob_display, self->plate);
    RrSurfaceShow(self->s_label);
    RrSurfaceShow(self->s_lgrip);
    RrSurfaceShow(self->s_rgrip);

    self->max_press = self->close_press = self->desk_press = 
	self->iconify_press = self->shade_press = FALSE;

    return (Frame*)self;
}

static void frame_free(Frame *self)
{
    RrSurfaceFree(self->s_rgrip);
    RrSurfaceFree(self->s_lgrip);
    RrSurfaceFree(self->s_handle);
    RrSurfaceFree(self->s_icon);
    RrSurfaceFree(self->s_iconify);
    RrSurfaceFree(self->s_shade);
    RrSurfaceFree(self->s_desk);
    RrSurfaceFree(self->s_close);
    RrSurfaceFree(self->s_max);
    RrSurfaceFree(self->s_label);
    RrSurfaceFree(self->s_title);
    RrSurfaceFree(self->s_frame);

    XDestroyWindow(ob_display, self->plate);
    XDestroyWindow(ob_display, self->window);

    g_free(self);
}

void frame_show(Frame *self)
{
    if (!self->visible) {
	self->visible = TRUE;
        RrSurfaceShow(self->s_frame);
    }
}

void frame_hide(Frame *self)
{
    if (self->visible) {
	self->visible = FALSE;
	self->client->ignore_unmaps++;
        RrSurfaceHide(self->s_frame);
    }
}

void frame_adjust_shape(Frame *self)
{
#ifdef SHAPE
    int num;
    XRectangle xrect[2];

    if (!self->client->shaped) {
	/* clear the shape on the frame window */
	XShapeCombineMask(ob_display, self->window, ShapeBounding,
			  self->size.left,
			  self->size.top,
			  None, ShapeSet);
    } else {
	/* make the frame's shape match the clients */
	XShapeCombineShape(ob_display, self->window, ShapeBounding,
			   self->size.left,
			   self->size.top,
			   self->client->window,
			   ShapeBounding, ShapeSet);

	num = 0;
	if (self->client->decorations & Decor_Titlebar) {
	    xrect[0].x = 0;
	    xrect[0].y = 0;
	    xrect[0].width = self->area.width;
	    xrect[0].height = self->size.top - ob_theme->bwidth;
	    ++num;
	}

	if (self->client->decorations & Decor_Handle) {
	    xrect[1].x = 0;
	    xrect[1].y = FRAME_HANDLE_Y(self);
	    xrect[1].width = self->area.width;
	    xrect[1].height = self->size.bottom - ob_theme->bwidth;
	    ++num;
	}

	XShapeCombineRectangles(ob_display, self->window,
				ShapeBounding, 0, 0, xrect, num,
				ShapeUnion, Unsorted);
    }
#endif
}

void frame_adjust_area(Frame *self, gboolean moved, gboolean resized)
{
    if (resized) {
        if (self->client->decorations & Decor_Border) {
            self->bwidth = ob_theme->bwidth;
            self->cbwidth = ob_theme->cbwidth;
        } else {
            self->bwidth = self->cbwidth = 0;
        }
        STRUT_SET(self->size,
                  self->bwidth + self->cbwidth,
                  self->bwidth + self->cbwidth,
                  self->bwidth + self->cbwidth,
                  self->bwidth + self->cbwidth);
        self->width = self->client->area.width +
            (self->bwidth + self->cbwidth) * 2;
        g_assert(self->width > 0);

        /* position/size and map/unmap all the windows */

        /* they all default off, they're turned on in layout_title */
        self->icon_x = -1;
        self->desk_x = -1;
        self->shade_x = -1;
        self->iconify_x = -1;
        self->label_x = -1;
        self->max_x = -1;
        self->close_x = -1;

        if (self->client->decorations & Decor_Titlebar) {
            RrSurfaceSetArea(self->s_title, 0, 0,
                             self->width, RrThemeTitleHeight(ob_theme));
            self->size.top += RrThemeTitleHeight(ob_theme) - self->bwidth;
            RrSurfaceShow(self->s_title);

            /* layout the title bar elements */
            layout_title(self);
        } else
            RrSurfaceHide(self->s_title);

        if (self->client->decorations & Decor_Handle) {
            RrSurfaceSetArea(self->s_handle, 0, FRAME_HANDLE_Y(self),
                             self->width, ob_theme->handle_height);
            RrSurfaceSetArea(self->s_lgrip, 0, 0,
                             RrThemeGripWidth(ob_theme),
                             ob_theme->handle_height);
            RrSurfaceSetArea(self->s_rgrip,
                             self->width - RrThemeGripWidth(ob_theme), 0,
                             RrThemeGripWidth(ob_theme),
                             ob_theme->handle_height);
            self->size.bottom += ob_theme->handle_height - ob_theme->bwidth;
            RrSurfaceShow(self->s_handle);
        } else
            RrSurfaceHide(self->s_handle);
    }

    if (resized) {
        /* move and resize the plate */
        XMoveResizeWindow(ob_display, self->plate,
                          self->size.left, self->size.top,
                          self->client->area.width,
                          self->client->area.height);
        /* when the client has StaticGravity, it likes to move around. */
        XMoveWindow(ob_display, self->client->window, 0, 0);
    }

    /* shading can change without being moved or resized */
    RECT_SET_SIZE(self->area,
		  self->client->area.width +
		  self->size.left + self->size.right,
		  (self->client->shaded ? RrThemeTitleHeight(ob_theme) :
                   self->client->area.height +
                   self->size.top + self->size.bottom));

    if (moved) {
        /* find the new coordinates, done after setting the frame.size, for
           frame_client_gravity. */
        self->area.x = self->client->area.x;
        self->area.y = self->client->area.y;
        frame_client_gravity((Frame*)self,
                             &self->area.x, &self->area.y);
    }

    /* move and resize the top level frame.
       shading can change without being moved or resized */
    RrSurfaceSetArea(self->s_frame, self->area.x, self->area.y,
                     self->area.width, self->area.height);

    if (resized) {
        framerender_frame(self);

        frame_adjust_shape(self);
    }
}

void frame_adjust_state(Frame *self)
{
    framerender_frame(self);
}

void frame_adjust_focus(Frame *self, gboolean hilite)
{
    self->focused = hilite;
    framerender_frame(self);
}

void frame_adjust_title(Frame *self)
{
    framerender_frame(self);
}

void frame_adjust_icon(Frame *self)
{
    framerender_frame(self);
}

void frame_grab_client(Frame *self, Client *client)
{
    self->client = client;

    /* reparent the client to the frame */
    XReparentWindow(ob_display, client->window, self->plate, 0, 0);
    /*
      When reparenting the client window, it is usually not mapped yet, since
      this occurs from a MapRequest. However, in the case where Openbox is
      starting up, the window is already mapped, so we'll see unmap events for
      it. There are 2 unmap events generated that we see, one with the 'event'
      member set the root window, and one set to the client, but both get
      handled and need to be ignored.
    */
    if (ob_state == State_Starting)
	client->ignore_unmaps += 2;

    /* select the event mask on the client's parent (to receive config/map
       req's) the ButtonPress is to catch clicks on the client border */
    XSelectInput(ob_display, self->plate, PLATE_EVENTMASK);

    /* map the client so it maps when the frame does */
    XMapWindow(ob_display, client->window);

    frame_adjust_area(self, TRUE, TRUE);

    /* set all the windows for the frame in the window_map */
    g_hash_table_insert(window_map, &self->window, client);
    g_hash_table_insert(window_map, &self->plate, client);
    g_hash_table_insert(window_map, &self->w_title, client);
    g_hash_table_insert(window_map, &self->w_label, client);
    g_hash_table_insert(window_map, &self->w_max, client);
    g_hash_table_insert(window_map, &self->w_close, client);
    g_hash_table_insert(window_map, &self->w_desk, client);
    g_hash_table_insert(window_map, &self->w_shade, client);
    g_hash_table_insert(window_map, &self->w_iconify, client);
    g_hash_table_insert(window_map, &self->w_icon, client);
    g_hash_table_insert(window_map, &self->w_handle, client);
    g_hash_table_insert(window_map, &self->w_lgrip, client);
    g_hash_table_insert(window_map, &self->w_rgrip, client);
}

void frame_release_client(Frame *self, Client *client)
{
    XEvent ev;

    g_assert(self->client == client);

    /* check if the app has already reparented its window away */
    if (XCheckTypedWindowEvent(ob_display, client->window,
			       ReparentNotify, &ev)) {
	XPutBackEvent(ob_display, &ev);

	/* re-map the window since the unmanaging process unmaps it */

        /* XXX ... um no it doesnt it unmaps its parent, the window itself
           retains its mapped state, no?! XXX
           XMapWindow(ob_display, client->window); */
    } else {
	/* according to the ICCCM - if the client doesn't reparent itself,
	   then we will reparent the window to root for them */
	XReparentWindow(ob_display, client->window, ob_root,
			client->area.x,
			client->area.y);
    }

    /* remove all the windows for the frame from the window_map */
    g_hash_table_remove(window_map, &self->window);
    g_hash_table_remove(window_map, &self->plate);
    g_hash_table_remove(window_map, &self->w_title);
    g_hash_table_remove(window_map, &self->w_label);
    g_hash_table_remove(window_map, &self->w_max);
    g_hash_table_remove(window_map, &self->w_close);
    g_hash_table_remove(window_map, &self->w_desk);
    g_hash_table_remove(window_map, &self->w_shade);
    g_hash_table_remove(window_map, &self->w_icon);
    g_hash_table_remove(window_map, &self->w_iconify);
    g_hash_table_remove(window_map, &self->w_handle);
    g_hash_table_remove(window_map, &self->w_lgrip);
    g_hash_table_remove(window_map, &self->w_rgrip);

    frame_free(self);
}

static void layout_title(Frame *self)
{
    char *lc;
    int x, y;
    gboolean n, d, i, l, m, c, s;

    n = d = i = l = m = c = s = FALSE;

    /* figure out whats being shown, and the width of the label */
    self->label_width = self->width - (ob_theme->bevel + 1) * 2;
    for (lc = ob_theme->title_layout; *lc != '\0'; ++lc) {
	switch (*lc) {
	case 'N':
	    if (!(self->client->decorations & Decor_Icon)) break;
            if (n) { *lc = ' '; break; } /* rm duplicates */
	    n = TRUE;
	    self->label_width -= RrThemeButtonSize(ob_theme) + 2 +
                ob_theme->bevel + 1;
	    break;
	case 'D':
	    if (!(self->client->decorations & Decor_AllDesktops)) break;
            if (d) { *lc = ' '; break; } /* rm duplicates */
	    d = TRUE;
	    self->label_width -= RrThemeButtonSize(ob_theme) +
                ob_theme->bevel + 1;
	    break;
	case 'S':
	    if (!(self->client->decorations & Decor_Shade)) break;
            if (s) { *lc = ' '; break; } /* rm duplicates */
	    s = TRUE;
	    self->label_width -= RrThemeButtonSize(ob_theme) +
                ob_theme->bevel + 1;
	    break;
	case 'I':
	    if (!(self->client->decorations & Decor_Iconify)) break;
            if (i) { *lc = ' '; break; } /* rm duplicates */
	    i = TRUE;
	    self->label_width -= RrThemeButtonSize(ob_theme) +
                ob_theme->bevel + 1;
	    break;
	case 'L':
            if (l) { *lc = ' '; break; } /* rm duplicates */
	    l = TRUE;
	    break;
	case 'M':
	    if (!(self->client->decorations & Decor_Maximize)) break;
            if (m) { *lc = ' '; break; } /* rm duplicates */
	    m = TRUE;
	    self->label_width -= RrThemeButtonSize(ob_theme) +
                ob_theme->bevel + 1;
	    break;
	case 'C':
	    if (!(self->client->decorations & Decor_Close)) break;
            if (c) { *lc = ' '; break; } /* rm duplicates */
	    c = TRUE;
	    self->label_width -= RrThemeButtonSize(ob_theme) +
                ob_theme->bevel + 1;
	    break;
	}
    }
    if (self->label_width < 1) self->label_width = 1;

    if (!n) RrSurfaceHide(self->s_icon);
    if (!d) RrSurfaceHide(self->s_desk);
    if (!s) RrSurfaceHide(self->s_shade);
    if (!i) RrSurfaceHide(self->s_iconify);
    if (!l) RrSurfaceHide(self->s_label);
    if (!m) RrSurfaceHide(self->s_max);
    if (!c) RrSurfaceHide(self->s_close);


    x = ob_theme->bevel + 1;
    y = ob_theme->bevel + ob_theme->bwidth + 1;
    for (lc = ob_theme->title_layout; *lc != '\0'; ++lc) {
	switch (*lc) {
	case 'N':
	    if (!n) break;
	    self->icon_x = x;
            RrSurfaceSetArea(self->s_icon, x, y - 1,
                             RrThemeButtonSize(ob_theme) + 2,
                             RrThemeButtonSize(ob_theme) + 2);
            RrSurfaceShow(self->s_icon);
	    x += RrThemeButtonSize(ob_theme) + 2 + ob_theme->bevel + 1;
	    break;
	case 'D':
	    if (!d) break;
	    self->desk_x = x;
            RrSurfaceSetArea(self->s_desk, x, y,
                             RrThemeButtonSize(ob_theme),
                             RrThemeButtonSize(ob_theme));
            RrSurfaceShow(self->s_desk);
	    x += RrThemeButtonSize(ob_theme) + ob_theme->bevel + 1;
	    break;
	case 'S':
	    if (!s) break;
	    self->shade_x = x;
            RrSurfaceSetArea(self->s_shade, x, y,
                             RrThemeButtonSize(ob_theme),
                             RrThemeButtonSize(ob_theme));
            RrSurfaceShow(self->s_shade);
	    x += RrThemeButtonSize(ob_theme) + ob_theme->bevel + 1;
	    break;
	case 'I':
	    if (!i) break;
	    self->iconify_x = x;
            RrSurfaceSetArea(self->s_iconify, x, y,
                             RrThemeButtonSize(ob_theme),
                             RrThemeButtonSize(ob_theme));
            RrSurfaceShow(self->s_iconify);
	    x += RrThemeButtonSize(ob_theme) + ob_theme->bevel + 1;
	    break;
	case 'L':
	    if (!l) break;
	    self->label_x = x;
            RrSurfaceSetArea(self->s_label, x, y - 1,
                             self->label_width, RrThemeLabelHeight(ob_theme));
            RrSurfaceShow(self->s_label);
	    x += self->label_width + ob_theme->bevel + 1;
	    break;
	case 'M':
	    if (!m) break;
	    self->max_x = x;
            RrSurfaceSetArea(self->s_max, x, y,
                             RrThemeButtonSize(ob_theme),
                             RrThemeButtonSize(ob_theme));
            RrSurfaceShow(self->s_max);
	    x += RrThemeButtonSize(ob_theme) + ob_theme->bevel + 1;
	    break;
	case 'C':
	    if (!c) break;
	    self->close_x = x;
            RrSurfaceSetArea(self->s_close, x, y,
                             RrThemeButtonSize(ob_theme),
                             RrThemeButtonSize(ob_theme));
            RrSurfaceShow(self->s_close);
	    x += RrThemeButtonSize(ob_theme) + ob_theme->bevel + 1;
	    break;
	}
    }
}

Context frame_context_from_string(char *name)
{
    if (!g_ascii_strcasecmp("root", name))
        return Context_Root;
    else if (!g_ascii_strcasecmp("client", name))
        return Context_Client;
    else if (!g_ascii_strcasecmp("titlebar", name))
        return Context_Titlebar;
    else if (!g_ascii_strcasecmp("handle", name))
        return Context_Handle;
    else if (!g_ascii_strcasecmp("frame", name))
        return Context_Frame;
    else if (!g_ascii_strcasecmp("blcorner", name))
        return Context_BLCorner;
    else if (!g_ascii_strcasecmp("tlcorner", name))
        return Context_TLCorner;
    else if (!g_ascii_strcasecmp("brcorner", name))
        return Context_BRCorner;
    else if (!g_ascii_strcasecmp("trcorner", name))
        return Context_TRCorner;
    else if (!g_ascii_strcasecmp("maximize", name))
        return Context_Maximize;
    else if (!g_ascii_strcasecmp("alldesktops", name))
        return Context_AllDesktops;
    else if (!g_ascii_strcasecmp("shade", name))
        return Context_Shade;
    else if (!g_ascii_strcasecmp("iconify", name))
        return Context_Iconify;
    else if (!g_ascii_strcasecmp("icon", name))
        return Context_Icon;
    else if (!g_ascii_strcasecmp("close", name))
        return Context_Close;
    return Context_None;
}

Context frame_context(Client *client, Window win)
{
    Frame *self;

    if (win == ob_root) return Context_Root;
    if (client == NULL) return Context_None;
    if (win == client->window) return Context_Client;

    self = client->frame;
    if (win == self->window)   return Context_Frame;
    if (win == self->plate)    return Context_Client;
    if (win == self->w_title)  return Context_Titlebar;
    if (win == self->w_label)  return Context_Titlebar;
    if (win == self->w_handle) return Context_Handle;
    if (win == self->w_lgrip)  return Context_BLCorner;
    if (win == self->w_rgrip)  return Context_BRCorner;
    if (win == self->w_max)    return Context_Maximize;
    if (win == self->w_iconify)return Context_Iconify;
    if (win == self->w_close)  return Context_Close;
    if (win == self->w_icon)   return Context_Icon;
    if (win == self->w_desk)   return Context_AllDesktops;
    if (win == self->w_shade)  return Context_Shade;

    return Context_None;
}

void frame_client_gravity(Frame *self, int *x, int *y)
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
	*x -= (self->size.left + self->size.right) / 2;
	break;

    case NorthEastGravity:
    case SouthEastGravity:
    case EastGravity:
	*x -= self->size.left + self->size.right;
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
	*y -= (self->size.top + self->size.bottom) / 2;
	break;

    case SouthWestGravity:
    case SouthEastGravity:
    case SouthGravity:
	*y -= self->size.top + self->size.bottom;
	break;

    case ForgetGravity:
    case StaticGravity:
	*y -= self->size.top;
	break;
    }
}

void frame_frame_gravity(Frame *self, int *x, int *y)
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
	*x += (self->size.left + self->size.right) / 2;
	break;
    case NorthEastGravity:
    case EastGravity:
    case SouthEastGravity:
	*x += self->size.left + self->size.right;
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
    case WestGravity:
    case SouthWestGravity:
	break;
    case NorthGravity:
    case CenterGravity:
    case SouthGravity:
	*y += (self->size.top + self->size.bottom) / 2;
	break;
    case NorthEastGravity:
    case EastGravity:
    case SouthEastGravity:
	*y += self->size.top + self->size.bottom;
	break;
    case StaticGravity:
    case ForgetGravity:
	*y += self->size.top;
	break;
    }
}
