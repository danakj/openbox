#include "openbox.h"
#include "frame.h"
#include "extensions.h"
#include "hooks.h"

#define PLATE_EVENTMASK (SubstructureRedirectMask | ButtonPressMask)
#define FRAME_EVENTMASK (EnterWindowMask | LeaveWindowMask)

static Window createWindow(Window parent, unsigned long mask,
			   XSetWindowAttributes *attrib)
{
    /* XXX DONT USE THE DEFAULT SHIT */
    return XCreateWindow(ob_display, parent, 0, 0, 1, 1, 0,
			 DefaultDepth(ob_display, ob_screen), InputOutput,
			 DefaultVisual(ob_display, ob_screen),
			 mask, attrib);
                       
}

Frame *frame_new(Client *client)
{
    XSetWindowAttributes attrib;
    unsigned long mask;
    Frame *self;

    self = g_new(Frame, 1);

    self->client = client;
    self->visible = FALSE;

    /* create all of the decor windows */
    mask = CWOverrideRedirect | CWEventMask;
    attrib.event_mask = FRAME_EVENTMASK;
    attrib.override_redirect = TRUE;
    self->window = createWindow(ob_root, mask, &attrib);

    mask = 0;
    self->plate = createWindow(self->window, mask, &attrib);
    mask = CWEventMask;
    attrib.event_mask = (ButtonPressMask | ButtonReleaseMask |
			 ButtonMotionMask | ExposureMask);
    self->title = createWindow(self->window, mask, &attrib);
    self->label = createWindow(self->title, mask, &attrib);
    self->max = createWindow(self->title, mask, &attrib);
    self->close = createWindow(self->title, mask, &attrib);
    self->desk = createWindow(self->title, mask, &attrib);
    self->icon = createWindow(self->title, mask, &attrib);
    self->iconify = createWindow(self->title, mask, &attrib);
    self->handle = createWindow(self->window, mask, &attrib);
    mask |= CWCursor;
    attrib.cursor = ob_cursors.ll_angle;
    self->lgrip = createWindow(self->handle, mask, &attrib);
    attrib.cursor = ob_cursors.lr_angle;
    self->rgrip = createWindow(self->handle, mask, &attrib);

    /* the other stuff is shown based on decor settings */
    XMapWindow(ob_display, self->plate);
    XMapWindow(ob_display, self->lgrip);
    XMapWindow(ob_display, self->rgrip);
    XMapWindow(ob_display, self->label);


    /* XXX TEMPORARY OF COURSE!@&*(@! */

    XSetWindowBackground(ob_display, self->title, 0x3333aa);
    XSetWindowBackground(ob_display, self->handle, 0x3333aa);
    XSetWindowBackground(ob_display, self->lgrip, 0x2233aa);
    XSetWindowBackground(ob_display, self->rgrip, 0x2233aa);

    XSetWindowBorder(ob_display, self->window, 0);
    XSetWindowBorder(ob_display, self->label, 0);
    XSetWindowBorder(ob_display, self->rgrip, 0);
    XSetWindowBorder(ob_display, self->lgrip, 0);
    XSetWindowBorder(ob_display, self->plate, 0x771122);

    /* XXX /TEMPORARY OF COURSE!@&*(@! */

    /* set all the windows for the frame in the client_map */
    g_hash_table_insert(client_map, (gpointer)self->window, self->client);
    g_hash_table_insert(client_map, (gpointer)self->plate, self->client);
    g_hash_table_insert(client_map, (gpointer)self->title, self->client);
    g_hash_table_insert(client_map, (gpointer)self->label, self->client);
    g_hash_table_insert(client_map, (gpointer)self->max, self->client);
    g_hash_table_insert(client_map, (gpointer)self->close, self->client);
    g_hash_table_insert(client_map, (gpointer)self->desk, self->client);
    g_hash_table_insert(client_map, (gpointer)self->icon, self->client);
    g_hash_table_insert(client_map, (gpointer)self->iconify, self->client);
    g_hash_table_insert(client_map, (gpointer)self->handle, self->client);
    g_hash_table_insert(client_map, (gpointer)self->lgrip, self->client);
    g_hash_table_insert(client_map, (gpointer)self->rgrip, self->client);

    return self;
}

void frame_free(Frame *self)
{
    /* remove all the windows for the frame from the client_map */
    g_hash_table_remove(client_map, (gpointer)self->window);
    g_hash_table_remove(client_map, (gpointer)self->plate);
    g_hash_table_remove(client_map, (gpointer)self->title);
    g_hash_table_remove(client_map, (gpointer)self->label);
    g_hash_table_remove(client_map, (gpointer)self->max);
    g_hash_table_remove(client_map, (gpointer)self->close);
    g_hash_table_remove(client_map, (gpointer)self->desk);
    g_hash_table_remove(client_map, (gpointer)self->icon);
    g_hash_table_remove(client_map, (gpointer)self->iconify);
    g_hash_table_remove(client_map, (gpointer)self->handle);
    g_hash_table_remove(client_map, (gpointer)self->lgrip);
    g_hash_table_remove(client_map, (gpointer)self->rgrip);

    XDestroyWindow(ob_display, self->window);

    g_free(self);
}

void frame_grab_client(Frame *self)
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
    if (ob_state == State_Starting)
	self->client->ignore_unmaps += 2;

    /* select the event mask on the client's parent (to receive config/map
       req's) the ButtonPress is to catch clicks on the client border */
    XSelectInput(ob_display, self->plate, PLATE_EVENTMASK);

    /* map the client so it maps when the frame does */
    XMapWindow(ob_display, self->client->window);

    frame_adjust_size(self);
    frame_adjust_position(self);
}

void frame_release_client(Frame *self)
{
    XEvent ev;

    /* check if the app has already reparented its window away */
    if (XCheckTypedWindowEvent(ob_display, self->client->window,
			       ReparentNotify, &ev)) {
	XPutBackEvent(ob_display, &ev);
	/* re-map the window since the unmanaging process unmaps it */
	XMapWindow(ob_display, self->client->window);
    } else {
	/* according to the ICCCM - if the client doesn't reparent itself,
	   then we will reparent the window to root for them */
	XReparentWindow(ob_display, self->client->window, ob_root,
			self->client->area.x, self->client->area.y);
    }
}

void frame_show(Frame *self)
{
    if (!self->visible) {
	self->visible = TRUE;
	XMapWindow(ob_display, self->window);
	LOGICALHOOK(WindowShow, g_quark_try_string("client"), self->client);
    }
}

void frame_hide(Frame *self)
{
    if (self->visible) {
	self->visible = FALSE;
	self->client->ignore_unmaps++;
	XUnmapWindow(ob_display, self->window);
	LOGICALHOOK(WindowHide, g_quark_try_string("client"), self->client);
    }
}

void frame_adjust_size(Frame *self)
{
    self->decorations = self->client->decorations;

    /* XXX set shit from the style */
    self->geom.font_height = 10;
    self->geom.bevel = 1;
    self->geom.button_size = self->geom.font_height - 2;
    self->geom.handle_height = 2;
    self->geom.grip_width = self->geom.button_size * 2;
    XResizeWindow(ob_display, self->lgrip, self->geom.grip_width,
		  self->geom.handle_height);
    XResizeWindow(ob_display, self->rgrip, self->geom.grip_width,
		  self->geom.handle_height);
	  
     
     
	  
    if (self->decorations & Decor_Border) {
	self->geom.bwidth = 1;/*XXX style->frameBorderWidth(); */
	self->geom.cbwidth = 1; /*XXX style->clientBorderWidth(); */
    } else {
	self->geom.bwidth = self->geom.cbwidth = 0;
    }
    STRUT_SET(self->innersize, self->geom.cbwidth, self->geom.cbwidth,
	      self->geom.cbwidth, self->geom.cbwidth);
    self->geom.width = self->client->area.width + self->geom.cbwidth * 2;
    g_assert(self->geom.width > 0);

    /* set border widths */
    XSetWindowBorderWidth(ob_display, self->plate,  self->geom.cbwidth);
    XSetWindowBorderWidth(ob_display, self->window, self->geom.bwidth);
    XSetWindowBorderWidth(ob_display, self->title,  self->geom.bwidth);
    XSetWindowBorderWidth(ob_display, self->handle, self->geom.bwidth);
    XSetWindowBorderWidth(ob_display, self->lgrip,  self->geom.bwidth);
    XSetWindowBorderWidth(ob_display, self->rgrip,  self->geom.bwidth);
  
    /* position/size and map/unmap all the windows */

    if (self->decorations & Decor_Titlebar) {
	self->geom.title_height = self->geom.font_height +
	    self->geom.bevel * 2;
	XMoveResizeWindow(ob_display, self->title,
			  -self->geom.bwidth, -self->geom.bwidth,
			  self->geom.width, self->geom.title_height);
	self->innersize.top += self->geom.title_height + self->geom.bwidth;
	XMapWindow(ob_display, self->title);

	/* layout the title bar elements */
	/*XXX layoutTitle(); */
    } else {
	XUnmapWindow(ob_display, self->title);
	/* make all the titlebar stuff not render */
	self->decorations &= ~(Decor_Icon | Decor_Iconify |
			       Decor_Maximize | Decor_Close |
			       Decor_AllDesktops);
    }

    if (self->decorations & Decor_Handle) {
	self->geom.handle_y = self->innersize.top +
	    self->client->area.height + self->geom.cbwidth;
	XMoveResizeWindow(ob_display, self->handle,
			  -self->geom.bwidth, self->geom.handle_y,
			  self->geom.width, self->geom.handle_height);
	XMoveWindow(ob_display, self->lgrip,
		    -self->geom.bwidth, -self->geom.bwidth);
	XMoveWindow(ob_display, self->rgrip,
		    -self->geom.bwidth + self->geom.width -
		    self->geom.grip_width, -self->geom.bwidth);
	self->innersize.bottom += self->geom.handle_height +
	    self->geom.bwidth;
	XMapWindow(ob_display, self->handle);
    } else
	XUnmapWindow(ob_display, self->handle);
  
    XResizeWindow(ob_display, self->window, self->geom.width,
		  (self->client->shaded ? self->geom.title_height :
		   self->innersize.top + self->innersize.bottom +
		   self->client->area.height));

    /* do this in two steps because clients whose gravity is set to
       'Static' don't end up getting moved at all with an XMoveResizeWindow */
    XMoveWindow(ob_display, self->plate,
		self->innersize.left - self->geom.cbwidth,
		self->innersize.top - self->geom.cbwidth);
    XResizeWindow(ob_display, self->plate, self->client->area.width,
		  self->client->area.height);

    STRUT_SET(self->size,
	      self->innersize.left + self->geom.bwidth,
	      self->innersize.right + self->geom.bwidth,
	      self->innersize.top + self->geom.bwidth,
	      self->innersize.bottom + self->geom.bwidth);

    RECT_SET_SIZE(self->area,
		  self->client->area.width +
		  self->size.left + self->size.right,
		  self->client->area.height +
		  self->size.top + self->size.bottom);

    /*
    // render all the elements
    int screen = _client->screen();
    bool focus = _client->focused();
    if (_decorations & Client::Decor_Titlebar) {
    render(screen, otk::Size(geom.width, geom.title_height()), _title,
    &_title_sur, *(focus ? style->titlebarFocusBackground() :
    style->titlebarUnfocusBackground()), false);
    
    renderLabel();
    renderMax();
    renderDesk();
    renderIconify();
    renderIcon();
    renderClose();
    }

    if (_decorations & Client::Decor_Handle) {
    render(screen, otk::Size(geom.width, geom.handle_height), _handle,
    &_handle_sur, *(focus ? style->handleFocusBackground() :
    style->handleUnfocusBackground()));
    render(screen, otk::Size(geom.grip_width(), geom.handle_height), _lgrip,
    &_grip_sur, *(focus ? style->gripFocusBackground() :
    style->gripUnfocusBackground()));
    if ((focus ? style->gripFocusBackground() :
    style->gripUnfocusBackground())->parentRelative())
    XSetWindowBackgroundPixmap(**otk::display, _rgrip, ParentRelative);
    else {
    XSetWindowBackgroundPixmap(**otk::display, _rgrip, _grip_sur->pixmap());
    }
    XClearWindow(**otk::display, _rgrip);
    }

    XSetWindowBorder(**otk::display, _plate,
    focus ? style->clientBorderFocusColor()->pixel() :
    style->clientBorderUnfocusColor()->pixel());

    */
     
    frame_adjust_shape(self);
}

void frame_adjust_position(Frame *self)
{
    self->area.x = self->client->area.x;
    self->area.y = self->client->area.y;
    frame_client_gravity(self, &self->area.x, &self->area.y);
    XMoveWindow(ob_display, self->window, self->area.x, self->area.y);
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
			   self->client->window, ShapeBounding, ShapeSet);

	num = 0;
	if (self->decorations & Decor_Titlebar) {
	    xrect[0].x = -self->geom.bevel;
	    xrect[0].y = -self->geom.bevel;
	    xrect[0].width = self->geom.width + self->geom.bwidth * 2;
	    xrect[0].height = self->geom.title_height +
		self->geom.bwidth * 2;
	    ++num;
	}

	if (self->decorations & Decor_Handle) {
	    xrect[1].x = -self->geom.bevel;
	    xrect[1].y = self->geom.handle_y;
	    xrect[1].width = self->geom.width + self->geom.bwidth * 2;
	    xrect[1].height = self->geom.handle_height +
		self->geom.bwidth * 2;
	    ++num;
	}

	XShapeCombineRectangles(ob_display, self->window,
				ShapeBounding, 0, 0, xrect, num,
				ShapeUnion, Unsorted);
    }
#endif
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
	x += self->size.left;
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

void frame_adjust_state(Frame *self)
{
    /* XXX do shit.. buttons? */
}

void frame_adjust_focus(Frame *self)
{
    /* XXX optimizations later... */
    frame_adjust_size(self);
}

void frame_adjust_title(Frame *self)
{
    /* XXX optimizations later... */
    frame_adjust_size(self);
}

void frame_adjust_icon(Frame *self)
{
    /* XXX render icon */
}

GQuark frame_get_context(Client *client, Window win)
{
    Frame *self;

    if (win == ob_root) return g_quark_try_string("root");
    if (client == NULL) return g_quark_try_string("none");
    if (win == client->window) return g_quark_try_string("client");

    self = client->frame;
    if (win == self->window) return g_quark_try_string("frame");
    if (win == self->plate)  return g_quark_try_string("frame");
    if (win == self->title)  return g_quark_try_string("titlebar");
    if (win == self->label)  return g_quark_try_string("titlebar");
    if (win == self->handle) return g_quark_try_string("handle");
    if (win == self->lgrip)  return g_quark_try_string("blcorner");
    if (win == self->rgrip)  return g_quark_try_string("brcorner");

    return g_quark_try_string("none");
}

void frame_startup(void)
{
    g_quark_from_string("none");
    g_quark_from_string("root");
    g_quark_from_string("client");
    g_quark_from_string("titlebar");
    g_quark_from_string("handle");
    g_quark_from_string("frame");
    g_quark_from_string("blcorner");
    g_quark_from_string("brcorner");
    g_quark_from_string("tlcorner");
    g_quark_from_string("trcorner");
    g_quark_from_string("foo");
}
