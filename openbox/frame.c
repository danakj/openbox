#include "frame.h"
#include "openbox.h"
#include "extensions.h"
#include "framerender.h"

#define PLATE_EVENTMASK (SubstructureRedirectMask | ButtonPressMask)
#define FRAME_EVENTMASK (EnterWindowMask | LeaveWindowMask | \
                         ButtonPressMask | ButtonReleaseMask | ExposureMask)
#define ELEMENT_EVENTMASK (ButtonPressMask | ButtonReleaseMask | \
                           ButtonMotionMask | ExposureMask)

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
			 RrInstanceDepth(ob_render_inst),
                         InputOutput, RrInstanceVisual(ob_render_inst),
			 mask, attrib);
                       
}

Frame *frame_new()
{
    struct RrColor pri,sec;
    XSetWindowAttributes attrib;
    unsigned long mask;
    Frame *self;
    FrameDecor *fd;

    self = g_new(Frame, 1);

    /* create all of the decor windows */
    mask = CWOverrideRedirect | CWEventMask;
    attrib.event_mask = FRAME_EVENTMASK;
    attrib.override_redirect = TRUE;
    self->window = createWindow(ob_root, mask, &attrib);
/* never map shapewindow */
    self->shapewindow = createWindow(ob_root, mask, &attrib);
    self->surface = RrSurfaceNew(ob_render_inst, RR_SURFACE_NONE,
                                 self->window, 0);
    RrColorSet(&pri, 1, 0, 0, 0);
    RrColorSet(&sec, 1, 0, 1, 0);

    mask = 0;
    self->plate = createWindow(self->window, mask, &attrib);
    XMapWindow(ob_display, self->plate);

    mask = CWEventMask;
    attrib.event_mask = ELEMENT_EVENTMASK;

    self->framedecors = 5;
    self->framedecor = g_new(FrameDecor, self->framedecors);
    fd = &self->framedecor[0];
    fd->obwin.type = Window_Decoration;
    fd->surface = RrSurfaceNewChild(RR_SURFACE_PLANAR, self->surface, 1);
    RrPlanarSet(fd->surface, RR_PLANAR_HORIZONTAL, &sec, &pri);
    fd->window = RrSurfaceWindow(fd->surface);
    XSelectInput(ob_display, fd->window, ELEMENT_EVENTMASK);
    fd->anchor = Decor_Top;
    RECT_SET(fd->area, 0, 0, 120, 20);
    fd->type = Decor_Titlebar;
    fd->context = Context_Titlebar;
    fd->sizetypex = Decor_Relative;
    fd->sizetypey = Decor_Absolute;
    fd->frame = self;

    fd = &self->framedecor[1];
    fd->obwin.type = Window_Decoration;
    fd->surface = RrSurfaceNewChild(RR_SURFACE_PLANAR, self->surface, 1);
    RrPlanarSet(fd->surface, RR_PLANAR_PIPECROSS, &pri, &sec);
    fd->window = RrSurfaceWindow(fd->surface);
    XSelectInput(ob_display, fd->window, ELEMENT_EVENTMASK);
    fd->anchor = Decor_Right;
    RECT_SET(fd->area, 0, 0, 5, 100);
    fd->type = Decor_Titlebar;
    fd->context = Context_Titlebar;
    fd->sizetypex = Decor_Absolute;
    fd->sizetypey = Decor_Relative;
    fd->frame = self;

    fd = &self->framedecor[2];
    fd->obwin.type = Window_Decoration;
    fd->surface = RrSurfaceNewChild(RR_SURFACE_PLANAR, self->surface, 1);
    RrPlanarSet(fd->surface, RR_PLANAR_PIPECROSS, &pri, &sec);
    fd->window = RrSurfaceWindow(fd->surface);
    XSelectInput(ob_display, fd->window, ELEMENT_EVENTMASK);
    fd->anchor = Decor_BottomLeft;
    RECT_SET(fd->area, 0, 0, 30, 30);
    fd->type = Decor_BottomLeft;
    fd->context = Context_BLCorner;
    fd->sizetypex = Decor_Absolute;
    fd->sizetypey = Decor_Absolute;
    fd->frame = self;

    fd = &self->framedecor[3];
    fd->obwin.type = Window_Decoration;
    fd->surface = RrSurfaceNewChild(RR_SURFACE_PLANAR, self->surface, 1);
    RrPlanarSet(fd->surface, RR_PLANAR_PIPECROSS, &pri, &sec);
    fd->window = RrSurfaceWindow(fd->surface);
    XSelectInput(ob_display, fd->window, ELEMENT_EVENTMASK);
    fd->anchor = Decor_BottomRight;
    RECT_SET(fd->area, 0, 0, 30, 30);
    fd->type = Decor_BottomRight;
    fd->context = Context_BRCorner;
    fd->sizetypex = Decor_Absolute;
    fd->sizetypey = Decor_Absolute;
    fd->frame = self;

    fd = &self->framedecor[4];
    fd->obwin.type = Window_Decoration;
    fd->surface = RrSurfaceNewChild(RR_SURFACE_PLANAR, self->surface, 1);
    RrPlanarSet(fd->surface, RR_PLANAR_HORIZONTAL, &pri, &sec);
    fd->window = RrSurfaceWindow(fd->surface);
    XSelectInput(ob_display, fd->window, ELEMENT_EVENTMASK);
    fd->anchor = Decor_Bottom;
    RECT_SET(fd->area, 0, 20, 100, 10);
    fd->type = Decor_Titlebar;
    fd->context = Context_Titlebar;
    fd->sizetypex = Decor_Relative;
    fd->sizetypey = Decor_Absolute;
    fd->frame = self;


    self->visible = FALSE;
    self->focused = FALSE;

    self->max_press = self->close_press = self->desk_press = 
	self->iconify_press = self->shade_press = FALSE;
    return (Frame*)self;
}

static void frame_free(Frame *self)
{
    int i;

    for (i = 0; i < self->framedecors; i++)
        RrSurfaceFree(self->framedecor[i].surface);
    RrSurfaceFree(self->surface);
    XDestroyWindow(ob_display, self->plate);
    XDestroyWindow(ob_display, self->window);
    XDestroyWindow(ob_display, self->shapewindow);
    g_free(self->framedecor);
    g_free(self);
}

void frame_show(Frame *self)
{
    if (!self->visible) {
	self->visible = TRUE;
        RrSurfaceShow(self->surface);
        XSync(ob_display, FALSE);
    }
}

void frame_hide(Frame *self)
{
    if (self->visible) {
	self->visible = FALSE;
	self->client->ignore_unmaps++;
        RrSurfaceHide(self->surface);
    }
}

void frame_adjust_shape(Frame *self)
{
#ifdef SHAPE
    int i;
    FrameDecor *dec;

    /* make the pixmap's shape match the clients */
printf("resize shape window to %x, %x\n", self->area.width, self->area.height);
    XResizeWindow(ob_display, self->shapewindow, self->area.width,
                      self->area.height);
    XShapeCombineShape(ob_display, self->shapewindow, ShapeBounding,
                       self->size.left,
                       self->size.top,
                       self->client->window,
                       ShapeBounding, ShapeSet);
    for (i = 0; i < self->framedecors; i++) {
        dec = &self->framedecor[i];
        if (dec->type & self->client->decorations)
            XShapeCombineShape(ob_display, self->shapewindow, ShapeBounding,
                              dec->xoff,
                              dec->yoff,
                              dec->window,
                              ShapeBounding, ShapeUnion);
    }

    XShapeCombineShape(ob_display, self->window, ShapeBounding,
                       0,
                       0,
                       self->shapewindow,
                       ShapeBounding, ShapeSet);

#endif
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
    int i;
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

    for (i = 0; i < self->framedecors; i++)
        g_hash_table_insert(window_map, &self->framedecor[i].window, 
                            &self->framedecor[i]);
}

void frame_release_client(Frame *self, Client *client)
{
    int i;
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

    for (i = 0; i < self->framedecors; i++)
        g_hash_table_remove(window_map, &self->framedecor[i].window);
    frame_free(self);
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
    ObWindow *obwin;
    if (win == ob_root) return Context_Root;
    if (client == NULL) return Context_None;
    if (win == client->window) return Context_Client;

    if (client->frame->window == win)
        return Context_Frame;
    if (client->frame->plate == win)
        return Context_Client;

    obwin = g_hash_table_lookup(window_map, &win);
    g_assert(obwin && WINDOW_IS_DECORATION(obwin));
    return WINDOW_AS_DECORATION(obwin)->context;
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

void decor_calculate_size(FrameDecor *d, Rect *r)
{
    r->x = d->area.x;
    r->y = d->area.y;

    switch (d->sizetypex) {
    case Decor_Absolute:
       r->width = d->area.width;
    break;
    case Decor_Relative:
       r->width = d->frame->client->area.width * d->area.width / 100;
    break;
    }

    switch (d->sizetypey) {
    case Decor_Absolute:
       r->height = d->area.height;
    break;
    case Decor_Relative:
       r->height = d->frame->client->area.height * d->area.height / 100;
    break;
    }
g_print("area of decoration is %d, %d, %d, %d\n", r->x, r->y, r->width, 
r->height);
}

void frame_adjust_area(Frame *self, gboolean moved, gboolean resized)
{
    FrameDecor *dec;
    Rect *cr, area;
    int i, le = 0, re = 0, te = 0, be = 0, temp, x, y;

    if (resized)
        for (i = 0; i < self->framedecors; i++) {
            dec = &self->framedecor[i];
            cr = &self->client->area;
            decor_calculate_size(dec, &area);
            if (dec->type & self->client->decorations)
                switch (dec->anchor) {
                case Decor_TopLeft:
                    temp = area.x + area.width;
                    if (temp > le) le = temp;
                    temp = area.y + area.height;
                    if (temp > te) te = temp;
                break;

                case Decor_Top:
                    temp = area.y + area.height;
                    if (temp > te) te = temp;
                    if (area.width > cr->width) {
                        temp = (area.width - cr->width)/2;
                        if (temp > re) re = temp;
                        if (temp > le) le = temp;
                    }
                break;

                case Decor_TopRight:
                    temp = area.x + area.width;
                    if (temp > re) re = temp;
                    temp = area.y + area.height;
                    if (temp > te) te = temp;
                break;

                case Decor_Left:
                    temp = area.x + area.width;
                    if (temp > le) le = temp;
                    if (area.height > cr->height) {
                        temp = (area.height - cr->height)/2;
                        if (temp > be) be = temp;
                        if (temp > te) te = temp;
                    }
                break;

                case Decor_Right:
                    temp = area.x + area.width;
                    if (temp > re) re = temp;
                    if (area.height > cr->height) {
                        temp = (area.height - cr->height)/2;
                        if (temp > be) be = temp;
                        if (temp > te) te = temp;
                    }
                break;

                case Decor_BottomLeft:
                    temp = area.x + area.width;
                    if (temp > le) le = temp;
                    temp = area.y + area.height;
                    if (temp > be) be = temp;
                break;

                case Decor_Bottom:
                    temp = area.y + area.height;
                    if (temp > be) be = temp;
                    if (area.width > cr->width) {
                        temp = (area.width - cr->width)/2;
                        if (temp > re) re = temp;
                        if (temp > le) le = temp;
                    }
                break;

                case Decor_BottomRight:
                    temp = area.x + area.width;
                    if (temp > re) re = temp;
                    temp = area.y + area.height;
                    if (temp > be) te = temp;
                break;
            }
        }
g_print("frame extends by %d, %d, %d, %d\n", le, te, le, be);
    if (resized) {
        /*
        if (self->client->decorations & Decor_Border) {
            self->bwidth = theme_bwidth;
            self->cbwidth = theme_cbwidth;
        } else {
            self->bwidth = self->cbwidth = 0;
        }
        */
        STRUT_SET(self->size, le, te, re, be);
    }

    if (resized) {
        /* move and resize the plate */
        XMoveResizeWindow(ob_display, self->plate,
                          self->size.left,
                          self->size.top,
                          self->client->area.width,
                          self->client->area.height);

        /* when the client has StaticGravity, it likes to move around. */
        XMoveWindow(ob_display, self->client->window, 0, 0);
    }

    /* shading can change without being moved or resized */
    RECT_SET_SIZE(self->area,
		  self->client->area.width +
		  self->size.left + self->size.right,
		  (self->client->shaded ? self->size.top : 
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

    if (resized)
        for (i = 0; i < self->framedecors; i++) {
            dec = &self->framedecor[i];
            cr = &self->client->area;
            decor_calculate_size(dec, &area);
            if (!(dec->type & self->client->decorations))
                RrSurfaceHide(dec->surface);
            else
                switch (dec->anchor) {
                    case Decor_TopLeft:
                    x = self->size.left - area.x - area.width;
                    y = self->size.top - area.y - area.height;
                    dec->xoff = x;
                    dec->yoff = y;
                    RrSurfaceSetArea(dec->surface, x, y,
                                      area.width,
                                      area.height);
                    break;

                    case Decor_Top:
                    x = cr->width/2 + self->size.left - area.x
                      - area.width/2;
                    y = self->size.top - area.y - area.height;
                    dec->xoff = x;
                    dec->yoff = y;
                    RrSurfaceSetArea(dec->surface, x, y,
                                      area.width,
                                      area.height);
                    break;

                    case Decor_TopRight:
                    x = self->size.left + cr->width
                      + area.x;
                    y = self->size.top - area.y - area.height;
                    dec->xoff = x;
                    dec->yoff = y;
                    RrSurfaceSetArea(dec->surface, x, y,
                                      area.width,
                                      area.height);
                    break;

                    case Decor_Left:
                    x = self->size.left - area.x
                      - area.width;
                    y = cr->height/2 + self->size.top - area.y
                      - area.height/2;
                    dec->xoff = x;
                    dec->yoff = y;
                    RrSurfaceSetArea(dec->surface, x, y,
                                      area.width,
                                      area.height);
                    break;

                    case Decor_Right:
                    x = self->size.left + cr->width + area.x;
                    y = cr->height/2 + self->size.top - area.y
                      - area.height/2;
                    dec->xoff = x;
                    dec->yoff = y;
                    RrSurfaceSetArea(dec->surface, x, y,
                                      area.width,
                                      area.height);
                    break;

                    case Decor_BottomLeft:
                    x = self->size.left - area.x - area.width;
                    y = self->size.top + cr->height
                      - area.y;
                    dec->xoff = x;
                    dec->yoff = y;
                    RrSurfaceSetArea(dec->surface, x, y,
                                      area.width,
                                      area.height);
                    break;

                    case Decor_Bottom:
                    x = cr->width/2 + self->size.left - area.x
                      - area.width/2;
                    y = self->size.top + cr->height
                      + area.y;
                    dec->xoff = x;
                    dec->yoff = y;
                    RrSurfaceSetArea(dec->surface, x, y,
                                      area.width,
                                      area.height);
                    break;

                    case Decor_BottomRight:
                    x = self->size.left + cr->width + area.x;
                    y = self->size.top + cr->height - area.y;
                    dec->xoff = x;
                    dec->yoff = y;
                    RrSurfaceSetArea(dec->surface, x, y,
                                      area.width, area.height);
                    break;
                }
        }
    /* move and resize the top level frame.
       shading can change without being moved or resized */
    RrSurfaceSetArea(self->surface,
                      self->area.x, self->area.y,
                      self->area.width,
                      self->area.height);

    if (resized) {
        framerender_frame(self);

        frame_adjust_shape(self);
    }
}
