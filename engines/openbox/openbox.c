#include "theme.h"
#include "../../kernel/openbox.h"
#include "../../kernel/screen.h"
#include "../../kernel/extensions.h"
#include "../../kernel/dispatch.h"
#include "../../kernel/themerc.h"
#include "../../kernel/frame.h"
#include "../../render/render.h"
#include "../../render/color.h"
#include "../../render/font.h"
#include "../../render/mask.h"

#include <X11/Xlib.h>
#include <glib.h>

#define LABEL_HEIGHT    (s_winfont_height)
#define TITLE_HEIGHT    (LABEL_HEIGHT + s_bevel * 2)
#define HANDLE_Y(f)     (f->innersize.top + f->frame.client->area.height + \
		         f->cbwidth)
#define BUTTON_SIZE     (LABEL_HEIGHT - 2)
#define GRIP_WIDTH      (BUTTON_SIZE * 2)
#define HANDLE_WIDTH(f) (f->width - (GRIP_WIDTH + f->bwidth) * 2)

#define PLATE_EVENTMASK (SubstructureRedirectMask | ButtonPressMask)
#define FRAME_EVENTMASK (EnterWindowMask | LeaveWindowMask)

/* style settings - geometry */
int s_bevel;
int s_handle_height;
int s_bwidth;
int s_cbwidth;
/* style settings - colors */
color_rgb *s_b_color;
color_rgb *s_cb_focused_color;
color_rgb *s_cb_unfocused_color;
color_rgb *s_title_focused_color;
color_rgb *s_title_unfocused_color;
color_rgb *s_titlebut_focused_color;
color_rgb *s_titlebut_unfocused_color;
/* style settings - fonts */
int s_winfont_height;
int s_winfont_shadow;
int s_winfont_shadow_offset;
ObFont *s_winfont;
/* style settings - masks */
pixmap_mask *s_max_mask;
pixmap_mask *s_icon_mask;
pixmap_mask *s_desk_mask;
pixmap_mask *s_close_mask;

/* global appearances */
Appearance *a_focused_unpressed_max;
Appearance *a_focused_pressed_max;
Appearance *a_unfocused_unpressed_max;
Appearance *a_unfocused_pressed_max;
Appearance *a_focused_unpressed_close;
Appearance *a_focused_pressed_close;
Appearance *a_unfocused_unpressed_close;
Appearance *a_unfocused_pressed_close;
Appearance *a_focused_unpressed_desk;
Appearance *a_focused_pressed_desk;
Appearance *a_unfocused_unpressed_desk;
Appearance *a_unfocused_pressed_desk;
Appearance *a_focused_unpressed_iconify;
Appearance *a_focused_pressed_iconify;
Appearance *a_unfocused_unpressed_iconify;
Appearance *a_unfocused_pressed_iconify;
Appearance *a_focused_grip;
Appearance *a_unfocused_grip;
Appearance *a_focused_title;
Appearance *a_unfocused_title;
Appearance *a_focused_label;
Appearance *a_unfocused_label;
Appearance *a_icon; /* always parentrelative, so no focused/unfocused */
Appearance *a_focused_handle;
Appearance *a_unfocused_handle;

typedef struct ObFrame {
    Frame frame;

    Window title;
    Window label;
    Window max;
    Window close;
    Window desk;
    Window icon;
    Window iconify;
    Window handle;
    Window lgrip;
    Window rgrip;

    Appearance *a_unfocused_title;
    Appearance *a_focused_title;
    Appearance *a_unfocused_label;
    Appearance *a_focused_label;
    Appearance *a_icon;
    Appearance *a_unfocused_handle;
    Appearance *a_focused_handle;

    Strut  innersize;

    GSList *clients;

    int width; /* title and handle */
    int label_width;
    int icon_x;        /* x-position of the window icon button */
    int label_x;       /* x-position of the window title */
    int iconify_x;     /* x-position of the window iconify button */
    int desk_x;         /* x-position of the window all-desktops button */
    int max_x;         /* x-position of the window maximize button */
    int close_x;       /* x-position of the window close button */
    int bwidth;        /* border width */
    int cbwidth;       /* client border width */

    gboolean max_press;
    gboolean close_press;
    gboolean desk_press;
    gboolean iconify_press;
} ObFrame;

static void layout_title(ObFrame *self);
static void render(ObFrame *self);
static void render_label(ObFrame *self);
static void render_max(ObFrame *self);
static void render_icon(ObFrame *self);
static void render_iconify(ObFrame *self);
static void render_desk(ObFrame *self);
static void render_close(ObFrame *self);

static void frame_mouse_press(const ObEvent *e, ObFrame *self);
static void frame_mouse_release(const ObEvent *e, ObFrame *self);

gboolean startup()
{
    g_quark_from_string("none");
    g_quark_from_string("root");
    g_quark_from_string("client");
    g_quark_from_string("titlebar");
    g_quark_from_string("handle");
    g_quark_from_string("frame");
    g_quark_from_string("blcorner");
    g_quark_from_string("brcorner");
    g_quark_from_string("maximize");
    g_quark_from_string("alldesktops");
    g_quark_from_string("iconify");
    g_quark_from_string("icon");
    g_quark_from_string("close");

    s_b_color = s_cb_unfocused_color = s_cb_focused_color = 
        s_title_unfocused_color = s_title_focused_color = 
        s_titlebut_unfocused_color = s_titlebut_focused_color = NULL;
    s_winfont = NULL;
    s_max_mask = s_icon_mask = s_desk_mask = s_close_mask = NULL;

    a_focused_unpressed_max = appearance_new(Surface_Planar, 1);
    a_focused_pressed_max = appearance_new(Surface_Planar, 1);
    a_unfocused_unpressed_max = appearance_new(Surface_Planar, 1);
    a_unfocused_pressed_max = appearance_new(Surface_Planar, 1);
    a_focused_unpressed_close = NULL;
    a_focused_pressed_close = NULL;
    a_unfocused_unpressed_close = NULL;
    a_unfocused_pressed_close = NULL;
    a_focused_unpressed_desk = NULL;
    a_focused_pressed_desk = NULL;
    a_unfocused_unpressed_desk = NULL;
    a_unfocused_pressed_desk = NULL;
    a_focused_unpressed_iconify = NULL;
    a_focused_pressed_iconify = NULL;
    a_unfocused_unpressed_iconify = NULL;
    a_unfocused_pressed_iconify = NULL;
    a_focused_grip = appearance_new(Surface_Planar, 0);
    a_unfocused_grip = appearance_new(Surface_Planar, 0);
    a_focused_title = appearance_new(Surface_Planar, 0);
    a_unfocused_title = appearance_new(Surface_Planar, 0);
    a_focused_label = appearance_new(Surface_Planar, 1);
    a_unfocused_label = appearance_new(Surface_Planar, 1);
    a_icon = appearance_new(Surface_Planar, 0);//1);
    a_focused_handle = appearance_new(Surface_Planar, 0);
    a_unfocused_handle = appearance_new(Surface_Planar, 0);

    return load();
}

void shutdown()
{
    if (s_b_color != NULL) color_free(s_b_color);
    if (s_cb_unfocused_color != NULL) color_free(s_cb_unfocused_color);
    if (s_cb_focused_color != NULL) color_free(s_cb_focused_color);
    if (s_title_unfocused_color != NULL) color_free(s_title_unfocused_color);
    if (s_title_focused_color != NULL) color_free(s_title_focused_color);
    if (s_titlebut_unfocused_color != NULL)
        color_free(s_titlebut_unfocused_color);
    if (s_titlebut_focused_color != NULL)
        color_free(s_titlebut_focused_color);

    if (s_max_mask != NULL) pixmap_mask_free(s_max_mask);
    if (s_desk_mask != NULL) pixmap_mask_free(s_desk_mask);
    if (s_icon_mask != NULL) pixmap_mask_free(s_icon_mask);
    if (s_close_mask != NULL) pixmap_mask_free(s_close_mask);

    if (s_winfont != NULL) font_close(s_winfont);

    appearance_free(a_focused_unpressed_max);
    appearance_free(a_focused_pressed_max);
    appearance_free(a_unfocused_unpressed_max);
    appearance_free(a_unfocused_pressed_max);
    if (a_focused_unpressed_close != NULL)
	appearance_free(a_focused_unpressed_close);
    if (a_focused_pressed_close != NULL)
	appearance_free(a_focused_pressed_close);
    if (a_unfocused_unpressed_close != NULL)
	appearance_free(a_unfocused_unpressed_close);
    if (a_unfocused_pressed_close != NULL)
	appearance_free(a_unfocused_pressed_close);
    if (a_focused_unpressed_desk != NULL)
	appearance_free(a_focused_unpressed_desk);
    if (a_focused_pressed_desk != NULL)
	appearance_free(a_focused_pressed_desk);
    if (a_unfocused_unpressed_desk != NULL)
	appearance_free(a_unfocused_unpressed_desk);
    if (a_unfocused_pressed_desk != NULL)
	appearance_free(a_unfocused_pressed_desk);
    if (a_focused_unpressed_iconify != NULL)
	appearance_free(a_focused_unpressed_iconify);
    if (a_focused_pressed_iconify != NULL)
	appearance_free(a_focused_pressed_iconify);
    if (a_unfocused_unpressed_iconify != NULL)
	appearance_free(a_unfocused_unpressed_iconify);
    if (a_unfocused_pressed_iconify != NULL)
	appearance_free(a_unfocused_pressed_iconify);
    appearance_free(a_focused_grip);
    appearance_free(a_unfocused_grip);
    appearance_free(a_focused_title);
    appearance_free(a_unfocused_title);
    appearance_free(a_focused_label);
    appearance_free(a_unfocused_label);
    appearance_free(a_icon);
    appearance_free(a_focused_handle);
    appearance_free(a_unfocused_handle);
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
    ObFrame *self;

    self = g_new(ObFrame, 1);

    self->frame.visible = FALSE;

    /* create all of the decor windows */
    mask = CWOverrideRedirect | CWEventMask;
    attrib.event_mask = FRAME_EVENTMASK;
    attrib.override_redirect = TRUE;
    self->frame.window = createWindow(ob_root, mask, &attrib);

    mask = 0;
    self->frame.plate = createWindow(self->frame.window, mask, &attrib);

    mask = CWEventMask;
    attrib.event_mask = (ButtonPressMask | ButtonReleaseMask |
			 ButtonMotionMask | ExposureMask);
    self->title = createWindow(self->frame.window, mask, &attrib);
    self->label = createWindow(self->title, mask, &attrib);
    self->max = createWindow(self->title, mask, &attrib);
    self->close = createWindow(self->title, mask, &attrib);
    self->desk = createWindow(self->title, mask, &attrib);
    self->icon = createWindow(self->title, mask, &attrib);
    self->iconify = createWindow(self->title, mask, &attrib);
    self->handle = createWindow(self->frame.window, mask, &attrib);
    mask |= CWCursor;
    attrib.cursor = ob_cursors.ll_angle;
    self->lgrip = createWindow(self->handle, mask, &attrib);
    attrib.cursor = ob_cursors.lr_angle;
    self->rgrip = createWindow(self->handle, mask, &attrib);

    /* the other stuff is shown based on decor settings */
    XMapWindow(ob_display, self->frame.plate);
    XMapWindow(ob_display, self->lgrip);
    XMapWindow(ob_display, self->rgrip);
    XMapWindow(ob_display, self->label);

    /* set colors/appearance/sizes for stuff that doesn't change */
    XSetWindowBorder(ob_display, self->frame.window, s_b_color->pixel);
    XSetWindowBorder(ob_display, self->label, s_b_color->pixel);
    XSetWindowBorder(ob_display, self->rgrip, s_b_color->pixel);
    XSetWindowBorder(ob_display, self->lgrip, s_b_color->pixel);

    XResizeWindow(ob_display, self->max, BUTTON_SIZE, BUTTON_SIZE);
    XResizeWindow(ob_display, self->iconify, BUTTON_SIZE, BUTTON_SIZE);
    XResizeWindow(ob_display, self->icon, BUTTON_SIZE, BUTTON_SIZE);
    XResizeWindow(ob_display, self->close, BUTTON_SIZE, BUTTON_SIZE);
    XResizeWindow(ob_display, self->desk, BUTTON_SIZE, BUTTON_SIZE);
    XResizeWindow(ob_display, self->lgrip, GRIP_WIDTH, s_handle_height);
    XResizeWindow(ob_display, self->rgrip, GRIP_WIDTH, s_handle_height);

    /* set up the dynamic appearances */
    self->a_unfocused_title = appearance_copy(a_unfocused_title);
    self->a_focused_title = appearance_copy(a_focused_title);
    self->a_unfocused_label = appearance_copy(a_unfocused_label);
    self->a_focused_label = appearance_copy(a_focused_label);
    self->a_unfocused_handle = appearance_copy(a_unfocused_handle);
    self->a_focused_handle = appearance_copy(a_focused_handle);
    self->a_icon = appearance_copy(a_icon);

    self->max_press = self->close_press = self->desk_press = 
	self->iconify_press = FALSE;

    dispatch_register(Event_X_ButtonPress, (EventHandler)frame_mouse_press,
                      self);
    dispatch_register(Event_X_ButtonRelease, (EventHandler)frame_mouse_release,
                      self);

    return (Frame*)self;
}

static void frame_free(ObFrame *self)
{
    appearance_free(self->a_unfocused_title); 
    appearance_free(self->a_focused_title);
    appearance_free(self->a_unfocused_label);
    appearance_free(self->a_focused_label);
    appearance_free(self->a_unfocused_handle);
    appearance_free(self->a_focused_handle);
    appearance_free(self->a_icon);

    XDestroyWindow(ob_display, self->frame.window);

    dispatch_register(0, (EventHandler)frame_mouse_press, self);
    dispatch_register(0, (EventHandler)frame_mouse_release, self);

    g_free(self);
}

void frame_show(ObFrame *self)
{
    if (!self->frame.visible) {
	self->frame.visible = TRUE;
	XMapWindow(ob_display, self->frame.window);
    }
}

void frame_hide(ObFrame *self)
{
    if (self->frame.visible) {
	self->frame.visible = FALSE;
	self->frame.client->ignore_unmaps++;
	XUnmapWindow(ob_display, self->frame.window);
    }
}

void frame_adjust_shape(ObFrame *self)
{
#ifdef SHAPE
    int num;
    XRectangle xrect[2];

    if (!self->frame.client->shaped) {
	/* clear the shape on the frame window */
	XShapeCombineMask(ob_display, self->frame.window, ShapeBounding,
			  self->innersize.left,
			  self->innersize.top,
			  None, ShapeSet);
    } else {
	/* make the frame's shape match the clients */
	XShapeCombineShape(ob_display, self->frame.window, ShapeBounding,
			   self->innersize.left,
			   self->innersize.top,
			   self->frame.client->window,
			   ShapeBounding, ShapeSet);

	num = 0;
	if (self->frame.client->decorations & Decor_Titlebar) {
	    xrect[0].x = -s_bevel;
	    xrect[0].y = -s_bevel;
	    xrect[0].width = self->width + self->bwidth * 2;
	    xrect[0].height = TITLE_HEIGHT +
		self->bwidth * 2;
	    ++num;
	}

	if (self->frame.client->decorations & Decor_Handle) {
	    xrect[1].x = -s_bevel;
	    xrect[1].y = HANDLE_Y(self);
	    xrect[1].width = self->width + self->bwidth * 2;
	    xrect[1].height = s_handle_height +
		self->bwidth * 2;
	    ++num;
	}

	XShapeCombineRectangles(ob_display, self->frame.window,
				ShapeBounding, 0, 0, xrect, num,
				ShapeUnion, Unsorted);
    }
#endif
}

void frame_adjust_size(ObFrame *self)
{
    if (self->frame.client->decorations & Decor_Border) {
	self->bwidth = s_bwidth;
	self->cbwidth = s_cbwidth;
    } else {
	self->bwidth = self->cbwidth = 0;
    }
    STRUT_SET(self->innersize, self->cbwidth, self->cbwidth,
	      self->cbwidth, self->cbwidth);
    self->width = self->frame.client->area.width + self->cbwidth * 2;
    g_assert(self->width > 0);

    /* set border widths */
    XSetWindowBorderWidth(ob_display, self->frame.plate,  self->cbwidth);
    XSetWindowBorderWidth(ob_display, self->frame.window, self->bwidth);
    XSetWindowBorderWidth(ob_display, self->title,  self->bwidth);
    XSetWindowBorderWidth(ob_display, self->handle, self->bwidth);
    XSetWindowBorderWidth(ob_display, self->lgrip,  self->bwidth);
    XSetWindowBorderWidth(ob_display, self->rgrip,  self->bwidth);
  
    /* position/size and map/unmap all the windows */

    if (self->frame.client->decorations & Decor_Titlebar) {
	XMoveResizeWindow(ob_display, self->title,
			  -self->bwidth, -self->bwidth,
			  self->width, TITLE_HEIGHT);
	self->innersize.top += TITLE_HEIGHT + self->bwidth;
	XMapWindow(ob_display, self->title);

	/* layout the title bar elements */
	layout_title(self);
    } else {
	XUnmapWindow(ob_display, self->title);
	/* make all the titlebar stuff not render */
	self->frame.client->decorations &= ~(Decor_Icon | Decor_Iconify |
			       Decor_Maximize | Decor_Close |
			       Decor_AllDesktops);
    }

    if (self->frame.client->decorations & Decor_Handle) {
	XMoveResizeWindow(ob_display, self->handle,
			  -self->bwidth, HANDLE_Y(self),
			  self->width, s_handle_height);
	XMoveWindow(ob_display, self->lgrip,
		    -self->bwidth, -self->bwidth);
	XMoveWindow(ob_display, self->rgrip,
		    -self->bwidth + self->width -
		    GRIP_WIDTH, -self->bwidth);
	self->innersize.bottom += s_handle_height +
	    self->bwidth;
	XMapWindow(ob_display, self->handle);
    } else
	XUnmapWindow(ob_display, self->handle);
  
    XResizeWindow(ob_display, self->frame.window, self->width,
		  (self->frame.client->shaded ? TITLE_HEIGHT :
		   self->innersize.top + self->innersize.bottom +
		   self->frame.client->area.height));

    /* do this in two steps because clients whose gravity is set to
       'Static' don't end up getting moved at all with an XMoveResizeWindow */
    XMoveWindow(ob_display, self->frame.plate,
		self->innersize.left - self->cbwidth,
		self->innersize.top - self->cbwidth);
    XResizeWindow(ob_display, self->frame.plate,
		  self->frame.client->area.width,
		  self->frame.client->area.height);

    STRUT_SET(self->frame.size,
	      self->innersize.left + self->bwidth,
	      self->innersize.top + self->bwidth,
	      self->innersize.right + self->bwidth,
	      self->innersize.bottom + self->bwidth);

    RECT_SET_SIZE(self->frame.area,
		  self->frame.client->area.width +
		  self->frame.size.left + self->frame.size.right,
		  self->frame.client->area.height +
		  self->frame.size.top + self->frame.size.bottom);

    render(self);
     
    frame_adjust_shape(self);
}

void frame_adjust_position(ObFrame *self)
{
    self->frame.area.x = self->frame.client->area.x;
    self->frame.area.y = self->frame.client->area.y;
    frame_client_gravity((Frame*)self,
			 &self->frame.area.x, &self->frame.area.y);
    XMoveWindow(ob_display, self->frame.window,
		self->frame.area.x, self->frame.area.y);
}

void frame_adjust_state(ObFrame *self)
{
    render_max(self);
    render_desk(self);
}

void frame_adjust_focus(ObFrame *self)
{
    render(self);
}

void frame_adjust_title(ObFrame *self)
{
    render_label(self);
}

void frame_adjust_icon(ObFrame *self)
{
    render_icon(self);
}

void frame_grab_client(ObFrame *self, Client *client)
{
    self->frame.client = client;

    /* reparent the client to the frame */
    XReparentWindow(ob_display, client->window, self->frame.plate, 0, 0);
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
    XSelectInput(ob_display, self->frame.plate, PLATE_EVENTMASK);

    /* map the client so it maps when the frame does */
    XMapWindow(ob_display, client->window);

    frame_adjust_size(self);
    frame_adjust_position(self);

    /* set all the windows for the frame in the client_map */
    g_hash_table_insert(client_map, (gpointer)self->frame.window, client);
    g_hash_table_insert(client_map, (gpointer)self->frame.plate, client);
    g_hash_table_insert(client_map, (gpointer)self->title, client);
    g_hash_table_insert(client_map, (gpointer)self->label, client);
    g_hash_table_insert(client_map, (gpointer)self->max, client);
    g_hash_table_insert(client_map, (gpointer)self->close, client);
    g_hash_table_insert(client_map, (gpointer)self->desk, client);
    g_hash_table_insert(client_map, (gpointer)self->icon, client);
    g_hash_table_insert(client_map, (gpointer)self->iconify, client);
    g_hash_table_insert(client_map, (gpointer)self->handle, client);
    g_hash_table_insert(client_map, (gpointer)self->lgrip, client);
    g_hash_table_insert(client_map, (gpointer)self->rgrip, client);
}

void frame_release_client(ObFrame *self, Client *client)
{
    XEvent ev;

    g_assert(self->frame.client == client);

    /* check if the app has already reparented its window away */
    if (XCheckTypedWindowEvent(ob_display, client->window,
			       ReparentNotify, &ev)) {
	XPutBackEvent(ob_display, &ev);
	/* re-map the window since the unmanaging process unmaps it */
	XMapWindow(ob_display, client->window);
    } else {
	/* according to the ICCCM - if the client doesn't reparent itself,
	   then we will reparent the window to root for them */
	XReparentWindow(ob_display, client->window, ob_root,
			client->area.x,
			client->area.y);
    }

    /* remove all the windows for the frame from the client_map */
    g_hash_table_remove(client_map, (gpointer)self->frame.window);
    g_hash_table_remove(client_map, (gpointer)self->frame.plate);
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

    frame_free(self);
}

static void layout_title(ObFrame *self)
{
    const char *lc;
    int x;
    gboolean n, d, i, l, m ,c;

    n = d = i = l = m = c = FALSE;

    /* figure out whats being shown, and the width of the label */
    self->label_width = self->width - (s_bevel + 1) * 2;
    for (lc = themerc_titlebar_layout; *lc != '\0'; ++lc) {
	switch (*lc) {
	case 'N':
	    if (!(self->frame.client->decorations & Decor_Icon)) break;
	    n = TRUE;
	    self->label_width -= BUTTON_SIZE + s_bevel + 1;
	    break;
	case 'D':
	    if (!(self->frame.client->decorations & Decor_AllDesktops)) break;
	    d = TRUE;
	    self->label_width -= BUTTON_SIZE + s_bevel + 1;
	    break;
	case 'I':
	    if (!(self->frame.client->decorations & Decor_Iconify)) break;
	    i = TRUE;
	    self->label_width -= BUTTON_SIZE + s_bevel + 1;
	    break;
	case 'L':
	    l = TRUE;
	    break;
	case 'M':
	    if (!(self->frame.client->decorations & Decor_Maximize)) break;
	    m = TRUE;
	    self->label_width -= BUTTON_SIZE + s_bevel + 1;
	    break;
	case 'C':
	    if (!(self->frame.client->decorations & Decor_Close)) break;
	    c = TRUE;
	    self->label_width -= BUTTON_SIZE + s_bevel + 1;
	    break;
	}
    }
    if (self->label_width < 1) self->label_width = 1;

    XResizeWindow(ob_display, self->label, self->label_width,
                  LABEL_HEIGHT);
  
    if (!n) {
	self->frame.client->decorations &= ~Decor_Icon;
	XUnmapWindow(ob_display, self->icon);
	self->icon_x = -1;
    }
    if (!d) {
	self->frame.client->decorations &= ~Decor_AllDesktops;
	XUnmapWindow(ob_display, self->desk);
	self->desk_x = -1;
    }
    if (!i) {
	self->frame.client->decorations &= ~Decor_Iconify;
	XUnmapWindow(ob_display, self->iconify);
	self->icon_x = -1;
    }
    if (!l) {
	XUnmapWindow(ob_display, self->label);
	self->label_x = -1;
    }
    if (!m) {
	self->frame.client->decorations &= ~Decor_Maximize;
	XUnmapWindow(ob_display, self->max);
	self->max_x = -1;
    }
    if (!c) {
	self->frame.client->decorations &= ~Decor_Close;
	XUnmapWindow(ob_display, self->close);
	self->close_x = -1;
    }

    x = s_bevel + 1;
    for (lc = themerc_titlebar_layout; *lc != '\0'; ++lc) {
	switch (*lc) {
	case 'N':
	    if (!n) break;
	    self->icon_x = x;
	    XMapWindow(ob_display, self->icon);
	    XMoveWindow(ob_display, self->icon, x, s_bevel + 1);
	    x += BUTTON_SIZE + s_bevel + 1;
	    break;
	case 'D':
	    if (!d) break;
	    self->desk_x = x;
	    XMapWindow(ob_display, self->desk);
	    XMoveWindow(ob_display, self->desk, x, s_bevel + 1);
	    x += BUTTON_SIZE + s_bevel + 1;
	    break;
	case 'I':
	    if (!i) break;
	    self->iconify_x = x;
	    XMapWindow(ob_display, self->iconify);
	    XMoveWindow(ob_display, self->iconify, x, s_bevel + 1);
	    x += BUTTON_SIZE + s_bevel + 1;
	    break;
	case 'L':
	    if (!l) break;
	    self->label_x = x;
	    XMapWindow(ob_display, self->label);
	    XMoveWindow(ob_display, self->label, x, s_bevel);
	    x += self->label_width + s_bevel + 1;
	    break;
	case 'M':
	    if (!m) break;
	    self->max_x = x;
	    XMapWindow(ob_display, self->max);
	    XMoveWindow(ob_display, self->max, x, s_bevel + 1);
	    x += BUTTON_SIZE + s_bevel + 1;
	    break;
	case 'C':
	    if (!c) break;
	    self->close_x = x;
	    XMapWindow(ob_display, self->close);
	    XMoveWindow(ob_display, self->close, x, s_bevel + 1);
	    x += BUTTON_SIZE + s_bevel + 1;
	    break;
	}
    }
}

static void render(ObFrame *self)
{
    if (self->frame.client->focused) {
        XSetWindowBorder(ob_display, self->frame.plate,
                         s_cb_focused_color->pixel);
    } else {
        XSetWindowBorder(ob_display, self->frame.plate,
                         s_cb_unfocused_color->pixel);
    }

    if (self->frame.client->decorations & Decor_Titlebar) {
        paint(self->title, (self->frame.client->focused ?
                            self->a_focused_title :
                            self->a_unfocused_title),
              0, 0, self->width, TITLE_HEIGHT);
        render_label(self);
        render_max(self);
        render_icon(self);
        render_iconify(self);
        render_desk(self);
        render_close(self);
    }

    if (self->frame.client->decorations & Decor_Handle) {
        paint(self->handle, (self->frame.client->focused ?
                             self->a_focused_handle :
                             self->a_unfocused_handle),
              GRIP_WIDTH + self->bwidth, 0,
              HANDLE_WIDTH(self), s_handle_height);
        paint(self->lgrip, (self->frame.client->focused ?
                            a_focused_grip :
                            a_unfocused_grip),
              0, 0, GRIP_WIDTH, s_handle_height);
        paint(self->rgrip, (self->frame.client->focused ?
                            a_focused_grip :
                            a_unfocused_grip),
              0, 0, GRIP_WIDTH, s_handle_height);
    }
}

static void render_label(ObFrame *self)
{
    Appearance *a;

    if (self->label_x < 0) return;

    a = (self->frame.client->focused ?
         self->a_focused_label : self->a_unfocused_label);

    /* set the texture's text! */
    a->texture[0].data.text.string = self->frame.client->title;

    paint(self->label, a, 0, 0, self->label_width, LABEL_HEIGHT);
}

static void render_icon(ObFrame *self)
{
    if (self->icon_x < 0) return;

    /* XXX set the texture's icon picture! */
    paint(self->icon, self->a_icon, 0, 0, BUTTON_SIZE, BUTTON_SIZE);
}

static void render_max(ObFrame *self)
{
    gboolean press = self->max_press ||
	self->frame.client->max_vert || self->frame.client->max_horz;
    
    if (self->max_x < 0) return;

    paint(self->max, (self->frame.client->focused ?
		      (press ?
		       a_focused_pressed_max :
		       a_focused_unpressed_max) :
		      (press ?
		       a_unfocused_pressed_max :
		       a_unfocused_unpressed_max)),
	  0, 0, BUTTON_SIZE, BUTTON_SIZE);
}

static void render_iconify(ObFrame *self)
{
    if (self->iconify_x < 0) return;

    paint(self->iconify, (self->frame.client->focused ?
			  (self->iconify_press ?
			   a_focused_pressed_iconify :
			   a_focused_unpressed_iconify) :
			  (self->iconify_press ?
			   a_unfocused_pressed_iconify :
			   a_unfocused_unpressed_iconify)),
	  0, 0, BUTTON_SIZE, BUTTON_SIZE);
}

static void render_desk(ObFrame *self)
{
    gboolean press = self->desk_press ||
	self->frame.client->desktop == DESKTOP_ALL;
    
    if (self->desk_x < 0) return;

    paint(self->desk, (self->frame.client->focused ?
		       (press ?
			a_focused_pressed_desk :
			a_focused_unpressed_desk) :
		       (press ?
			a_unfocused_pressed_desk :
			a_unfocused_unpressed_desk)),
	  0, 0, BUTTON_SIZE, BUTTON_SIZE);
}

static void render_close(ObFrame *self)
{
    if (self->close_x < 0) return;

    paint(self->close, (self->frame.client->focused ?
			  (self->close_press ?
			   a_focused_pressed_close :
			   a_focused_unpressed_close) :
			  (self->close_press ?
			   a_unfocused_pressed_close :
			   a_unfocused_unpressed_close)),
	  0, 0, BUTTON_SIZE, BUTTON_SIZE);
}

GQuark get_context(Client *client, Window win)
{
    ObFrame *self;

    if (win == ob_root) return g_quark_try_string("root");
    if (client == NULL) return g_quark_try_string("none");
    if (win == client->window) return g_quark_try_string("client");

    self = (ObFrame*) client->frame;
    if (win == self->frame.window) return g_quark_try_string("frame");
    if (win == self->frame.plate)  return g_quark_try_string("client");
    if (win == self->title)  return g_quark_try_string("titlebar");
    if (win == self->label)  return g_quark_try_string("titlebar");
    if (win == self->handle) return g_quark_try_string("handle");
    if (win == self->lgrip)  return g_quark_try_string("blcorner");
    if (win == self->rgrip)  return g_quark_try_string("brcorner");
    if (win == self->max)  return g_quark_try_string("maximize");
    if (win == self->iconify)  return g_quark_try_string("iconify");
    if (win == self->close)  return g_quark_try_string("close");
    if (win == self->icon)  return g_quark_try_string("icon");
    if (win == self->desk)  return g_quark_try_string("alldesktops");

    return g_quark_try_string("none");
}

static void frame_mouse_press(const ObEvent *e, ObFrame *self)
{
    Window win = e->data.x.e->xbutton.window;
    if (win == self->max) {
        self->max_press = TRUE;
        render_max(self);
    } else if (win == self->close) {
        self->close_press = TRUE;
        render_close(self);
    } else if (win == self->iconify) {
        self->iconify_press = TRUE;
        render_iconify(self);
    } else if (win == self->desk) { 
        self->desk_press = TRUE;
        render_desk(self);
    }
}

static void frame_mouse_release(const ObEvent *e, ObFrame *self)
{
    Window win = e->data.x.e->xbutton.window;
    if (win == self->max) {
        self->max_press = FALSE;
        render_max(self);
    } else if (win == self->close) {
        self->close_press = FALSE; 
        render_close(self);
    } else if (win == self->iconify) {
        self->iconify_press = FALSE;
        render_iconify(self);
    } else if (win == self->desk) {
        self->desk_press = FALSE;
        render_desk(self);
    }
}
