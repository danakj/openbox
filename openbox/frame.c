#include "frame.h"
#include "openbox.h"
#include "extensions.h"
#include "framerender.h"
#include "render/theme.h"

#define PLATE_EVENTMASK (SubstructureRedirectMask | ButtonPressMask)
#define FRAME_EVENTMASK (EnterWindowMask | LeaveWindowMask | \
                         ButtonPressMask | ButtonReleaseMask)
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
			 render_depth, InputOutput, render_visual,
			 mask, attrib);
                       
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

    mask = CWEventMask;
    attrib.event_mask = ELEMENT_EVENTMASK;
    self->title = createWindow(self->window, mask, &attrib);
    self->label = createWindow(self->title, mask, &attrib);
    self->max = createWindow(self->title, mask, &attrib);
    self->close = createWindow(self->title, mask, &attrib);
    self->desk = createWindow(self->title, mask, &attrib);
    self->shade = createWindow(self->title, mask, &attrib);
    self->icon = createWindow(self->title, mask, &attrib);
    self->iconify = createWindow(self->title, mask, &attrib);
    self->handle = createWindow(self->window, mask, &attrib);
    mask |= CWCursor;
    attrib.cursor = ob_cursors.bl;
    self->lgrip = createWindow(self->handle, mask, &attrib);
    attrib.cursor = ob_cursors.br;
    self->rgrip = createWindow(self->handle, mask, &attrib);

    self->focused = FALSE;

    /* the other stuff is shown based on decor settings */
    XMapWindow(ob_display, self->plate);
    XMapWindow(ob_display, self->lgrip);
    XMapWindow(ob_display, self->rgrip);
    XMapWindow(ob_display, self->label);

    /* set colors/appearance/sizes for stuff that doesn't change */
    XSetWindowBorder(ob_display, self->window, theme_b_color->pixel);
    XSetWindowBorder(ob_display, self->label, theme_b_color->pixel);
    XSetWindowBorder(ob_display, self->rgrip, theme_b_color->pixel);
    XSetWindowBorder(ob_display, self->lgrip, theme_b_color->pixel);

    XResizeWindow(ob_display, self->max, theme_button_size, theme_button_size);
    XResizeWindow(ob_display, self->iconify,
                  theme_button_size, theme_button_size);
    XResizeWindow(ob_display, self->icon,
                  theme_button_size + 2, theme_button_size + 2);
    XResizeWindow(ob_display, self->close,
                  theme_button_size, theme_button_size);
    XResizeWindow(ob_display, self->desk,
                  theme_button_size, theme_button_size);
    XResizeWindow(ob_display, self->shade,
                  theme_button_size, theme_button_size);
    XResizeWindow(ob_display, self->lgrip,
                  theme_grip_width, theme_handle_height);
    XResizeWindow(ob_display, self->rgrip,
                  theme_grip_width, theme_handle_height);

    /* set up the dynamic appearances */
    self->a_unfocused_title = appearance_copy(theme_a_unfocused_title);
    self->a_focused_title = appearance_copy(theme_a_focused_title);
    self->a_unfocused_label = appearance_copy(theme_a_unfocused_label);
    self->a_focused_label = appearance_copy(theme_a_focused_label);
    self->a_unfocused_handle = appearance_copy(theme_a_unfocused_handle);
    self->a_focused_handle = appearance_copy(theme_a_focused_handle);
    self->a_icon = appearance_copy(theme_a_icon);

    self->max_press = self->close_press = self->desk_press = 
	self->iconify_press = self->shade_press = FALSE;

    return (Frame*)self;
}

static void frame_free(Frame *self)
{
    appearance_free(self->a_unfocused_title); 
    appearance_free(self->a_focused_title);
    appearance_free(self->a_unfocused_label);
    appearance_free(self->a_focused_label);
    appearance_free(self->a_unfocused_handle);
    appearance_free(self->a_focused_handle);
    appearance_free(self->a_icon);

    XDestroyWindow(ob_display, self->window);

    g_free(self);
}

void frame_show(Frame *self)
{
    if (!self->visible) {
	self->visible = TRUE;
	XMapWindow(ob_display, self->window);
    }
}

void frame_hide(Frame *self)
{
    if (self->visible) {
	self->visible = FALSE;
	self->client->ignore_unmaps++;
	XUnmapWindow(ob_display, self->window);
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
	if (self->client->decorations & Decor_Titlebar) {
	    xrect[0].x = -theme_bevel;
	    xrect[0].y = -theme_bevel;
	    xrect[0].width = self->width + self->bwidth * 2;
	    xrect[0].height = theme_title_height +
		self->bwidth * 2;
	    ++num;
	}

	if (self->client->decorations & Decor_Handle) {
	    xrect[1].x = -theme_bevel;
	    xrect[1].y = FRAME_HANDLE_Y(self);
	    xrect[1].width = self->width + self->bwidth * 2;
	    xrect[1].height = theme_handle_height +
		self->bwidth * 2;
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
            self->bwidth = theme_bwidth;
            self->cbwidth = theme_cbwidth;
        } else {
            self->bwidth = self->cbwidth = 0;
        }
        STRUT_SET(self->innersize, self->cbwidth, self->cbwidth,
                  self->cbwidth, self->cbwidth);
        self->width = self->client->area.width + self->cbwidth * 2;
        g_assert(self->width > 0);

        /* set border widths */
        XSetWindowBorderWidth(ob_display, self->plate,  self->cbwidth);
        XSetWindowBorderWidth(ob_display, self->window, self->bwidth);
        XSetWindowBorderWidth(ob_display, self->title,  self->bwidth);
        XSetWindowBorderWidth(ob_display, self->handle, self->bwidth);
        XSetWindowBorderWidth(ob_display, self->lgrip,  self->bwidth);
        XSetWindowBorderWidth(ob_display, self->rgrip,  self->bwidth);
  
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
            XMoveResizeWindow(ob_display, self->title,
                              -self->bwidth, -self->bwidth,
                              self->width, theme_title_height);
            self->innersize.top += theme_title_height + self->bwidth;
            XMapWindow(ob_display, self->title);

            /* layout the title bar elements */
            layout_title(self);
        } else
            XUnmapWindow(ob_display, self->title);

        if (self->client->decorations & Decor_Handle) {
            XMoveResizeWindow(ob_display, self->handle,
                              -self->bwidth, FRAME_HANDLE_Y(self),
                              self->width, theme_handle_height);
            XMoveWindow(ob_display, self->lgrip,
                        -self->bwidth, -self->bwidth);
            XMoveWindow(ob_display, self->rgrip,
                        -self->bwidth + self->width -
                        theme_grip_width, -self->bwidth);
            self->innersize.bottom += theme_handle_height +
                self->bwidth;
            XMapWindow(ob_display, self->handle);

            /* XXX make a subwindow with these dimentions?
               theme_grip_width + self->bwidth, 0,
               self->width - (theme_grip_width + self->bwidth) * 2,
               theme_handle_height);
            */
        } else
            XUnmapWindow(ob_display, self->handle);
    }

    if (resized) {
        /* move and resize the plate */
        XMoveResizeWindow(ob_display, self->plate,
                          self->innersize.left - self->cbwidth,
                          self->innersize.top - self->cbwidth,
                          self->client->area.width,
                          self->client->area.height);
        /* when the client has StaticGravity, it likes to move around. */
        XMoveWindow(ob_display, self->client->window, 0, 0);
    }

    if (resized) {
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
		  (self->client->shaded ? theme_title_height + self->bwidth*2:
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
    XMoveResizeWindow(ob_display, self->window,
                      self->area.x, self->area.y,
                      self->width,
                      self->area.height - self->bwidth * 2);

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
    g_hash_table_insert(window_map, &self->title, client);
    g_hash_table_insert(window_map, &self->label, client);
    g_hash_table_insert(window_map, &self->max, client);
    g_hash_table_insert(window_map, &self->close, client);
    g_hash_table_insert(window_map, &self->desk, client);
    g_hash_table_insert(window_map, &self->shade, client);
    g_hash_table_insert(window_map, &self->icon, client);
    g_hash_table_insert(window_map, &self->iconify, client);
    g_hash_table_insert(window_map, &self->handle, client);
    g_hash_table_insert(window_map, &self->lgrip, client);
    g_hash_table_insert(window_map, &self->rgrip, client);
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

    frame_free(self);
}

static void layout_title(Frame *self)
{
    char *lc;
    int x;
    gboolean n, d, i, l, m, c, s;

    n = d = i = l = m = c = s = FALSE;

    /* figure out whats being shown, and the width of the label */
    self->label_width = self->width - (theme_bevel + 1) * 2;
    for (lc = theme_title_layout; *lc != '\0'; ++lc) {
	switch (*lc) {
	case 'N':
	    if (!(self->client->decorations & Decor_Icon)) break;
            if (n) { *lc = ' '; break; } /* rm duplicates */
	    n = TRUE;
	    self->label_width -= theme_button_size + 2 + theme_bevel + 1;
	    break;
	case 'D':
	    if (!(self->client->decorations & Decor_AllDesktops)) break;
            if (d) { *lc = ' '; break; } /* rm duplicates */
	    d = TRUE;
	    self->label_width -= theme_button_size + theme_bevel + 1;
	    break;
	case 'S':
	    if (!(self->client->decorations & Decor_Shade)) break;
            if (s) { *lc = ' '; break; } /* rm duplicates */
	    s = TRUE;
	    self->label_width -= theme_button_size + theme_bevel + 1;
	    break;
	case 'I':
	    if (!(self->client->decorations & Decor_Iconify)) break;
            if (i) { *lc = ' '; break; } /* rm duplicates */
	    i = TRUE;
	    self->label_width -= theme_button_size + theme_bevel + 1;
	    break;
	case 'L':
            if (l) { *lc = ' '; break; } /* rm duplicates */
	    l = TRUE;
	    break;
	case 'M':
	    if (!(self->client->decorations & Decor_Maximize)) break;
            if (m) { *lc = ' '; break; } /* rm duplicates */
	    m = TRUE;
	    self->label_width -= theme_button_size + theme_bevel + 1;
	    break;
	case 'C':
	    if (!(self->client->decorations & Decor_Close)) break;
            if (c) { *lc = ' '; break; } /* rm duplicates */
	    c = TRUE;
	    self->label_width -= theme_button_size + theme_bevel + 1;
	    break;
	}
    }
    if (self->label_width < 1) self->label_width = 1;

    XResizeWindow(ob_display, self->label, self->label_width,
                  theme_label_height);
  
    if (!n) XUnmapWindow(ob_display, self->icon);
    if (!d) XUnmapWindow(ob_display, self->desk);
    if (!s) XUnmapWindow(ob_display, self->shade);
    if (!i) XUnmapWindow(ob_display, self->iconify);
    if (!l) XUnmapWindow(ob_display, self->label);
    if (!m) XUnmapWindow(ob_display, self->max);
    if (!c) XUnmapWindow(ob_display, self->close);

    x = theme_bevel + 1;
    for (lc = theme_title_layout; *lc != '\0'; ++lc) {
	switch (*lc) {
	case 'N':
	    if (!n) break;
	    self->icon_x = x;
	    XMapWindow(ob_display, self->icon);
	    XMoveWindow(ob_display, self->icon, x, theme_bevel);
	    x += theme_button_size + 2 + theme_bevel + 1;
	    break;
	case 'D':
	    if (!d) break;
	    self->desk_x = x;
	    XMapWindow(ob_display, self->desk);
	    XMoveWindow(ob_display, self->desk, x, theme_bevel + 1);
	    x += theme_button_size + theme_bevel + 1;
	    break;
	case 'S':
	    if (!s) break;
	    self->shade_x = x;
	    XMapWindow(ob_display, self->shade);
	    XMoveWindow(ob_display, self->shade, x, theme_bevel + 1);
	    x += theme_button_size + theme_bevel + 1;
	    break;
	case 'I':
	    if (!i) break;
	    self->iconify_x = x;
	    XMapWindow(ob_display, self->iconify);
	    XMoveWindow(ob_display, self->iconify, x, theme_bevel + 1);
	    x += theme_button_size + theme_bevel + 1;
	    break;
	case 'L':
	    if (!l) break;
	    self->label_x = x;
	    XMapWindow(ob_display, self->label);
	    XMoveWindow(ob_display, self->label, x, theme_bevel);
	    x += self->label_width + theme_bevel + 1;
	    break;
	case 'M':
	    if (!m) break;
	    self->max_x = x;
	    XMapWindow(ob_display, self->max);
	    XMoveWindow(ob_display, self->max, x, theme_bevel + 1);
	    x += theme_button_size + theme_bevel + 1;
	    break;
	case 'C':
	    if (!c) break;
	    self->close_x = x;
	    XMapWindow(ob_display, self->close);
	    XMoveWindow(ob_display, self->close, x, theme_bevel + 1);
	    x += theme_button_size + theme_bevel + 1;
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
    if (win == self->window) return Context_Frame;
    if (win == self->plate)  return Context_Client;
    if (win == self->title)  return Context_Titlebar;
    if (win == self->label)  return Context_Titlebar;
    if (win == self->handle) return Context_Handle;
    if (win == self->lgrip)  return Context_BLCorner;
    if (win == self->rgrip)  return Context_BRCorner;
    if (win == self->max)    return Context_Maximize;
    if (win == self->iconify)return Context_Iconify;
    if (win == self->close)  return Context_Close;
    if (win == self->icon)   return Context_Icon;
    if (win == self->desk)   return Context_AllDesktops;
    if (win == self->shade)  return Context_Shade;

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
