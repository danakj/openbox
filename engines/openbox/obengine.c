#include "obtheme.h"
#include "obrender.h"
#include "obengine.h"
#include "../../kernel/openbox.h"
#include "../../kernel/extensions.h"
#include "../../kernel/dispatch.h"
#include "../../kernel/config.h"

#ifdef HAVE_SYS_STAT_H
#  include <sys/stat.h>
#  include <sys/types.h>
#endif
#include <X11/Xlib.h>
#include <glib.h>

#define PLATE_EVENTMASK (SubstructureRedirectMask | ButtonPressMask)
#define FRAME_EVENTMASK (EnterWindowMask | LeaveWindowMask)
#define ELEMENT_EVENTMASK (ButtonPressMask | ButtonReleaseMask | \
                           ButtonMotionMask | ExposureMask)

/* style settings - geometry */
int ob_s_bevel;
int ob_s_handle_height;
int ob_s_bwidth;
int ob_s_cbwidth;
/* style settings - colors */
color_rgb *ob_s_b_color;
color_rgb *ob_s_cb_focused_color;
color_rgb *ob_s_cb_unfocused_color;
color_rgb *ob_s_title_focused_color;
color_rgb *ob_s_title_unfocused_color;
color_rgb *ob_s_titlebut_focused_color;
color_rgb *ob_s_titlebut_unfocused_color;
/* style settings - fonts */
int ob_s_winfont_height;
int ob_s_winfont_shadow;
int ob_s_winfont_shadow_offset;
ObFont *ob_s_winfont;
/* style settings - masks */
pixmap_mask *ob_s_max_set_mask;
pixmap_mask *ob_s_max_unset_mask;
pixmap_mask *ob_s_iconify_mask;
pixmap_mask *ob_s_desk_set_mask;
pixmap_mask *ob_s_desk_unset_mask;
pixmap_mask *ob_s_close_mask;

/* global appearances */
Appearance *ob_a_focused_unpressed_max;
Appearance *ob_a_focused_pressed_max;
Appearance *ob_a_focused_pressed_set_max;
Appearance *ob_a_unfocused_unpressed_max;
Appearance *ob_a_unfocused_pressed_max;
Appearance *ob_a_unfocused_pressed_set_max;
Appearance *ob_a_focused_unpressed_close;
Appearance *ob_a_focused_pressed_close;
Appearance *ob_a_unfocused_unpressed_close;
Appearance *ob_a_unfocused_pressed_close;
Appearance *ob_a_focused_unpressed_desk;
Appearance *ob_a_focused_pressed_desk;
Appearance *ob_a_focused_pressed_set_desk;
Appearance *ob_a_unfocused_unpressed_desk;
Appearance *ob_a_unfocused_pressed_desk;
Appearance *ob_a_unfocused_pressed_set_desk;
Appearance *ob_a_focused_unpressed_iconify;
Appearance *ob_a_focused_pressed_iconify;
Appearance *ob_a_unfocused_unpressed_iconify;
Appearance *ob_a_unfocused_pressed_iconify;
Appearance *ob_a_focused_grip;
Appearance *ob_a_unfocused_grip;
Appearance *ob_a_focused_title;
Appearance *ob_a_unfocused_title;
Appearance *ob_a_focused_label;
Appearance *ob_a_unfocused_label;
Appearance *ob_a_icon; /* always parentrelative, so no focused/unfocused */
Appearance *ob_a_focused_handle;
Appearance *ob_a_unfocused_handle;

static void layout_title(ObFrame *self);
static void mouse_event(const ObEvent *e, ObFrame *self);

gboolean startup()
{
    char *path;

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

    /* create the ~/.openbox/themes/openbox dir */
    path = g_build_filename(g_get_home_dir(), ".openbox", "themes", "openbox",
                            NULL);
    mkdir(path, (S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP |
                 S_IROTH | S_IWOTH | S_IXOTH));
    g_free(path);

    ob_s_b_color = ob_s_cb_unfocused_color = ob_s_cb_focused_color = 
        ob_s_title_unfocused_color = ob_s_title_focused_color = 
        ob_s_titlebut_unfocused_color = ob_s_titlebut_focused_color = NULL;
    ob_s_winfont = NULL;
    ob_s_max_set_mask = ob_s_max_unset_mask = NULL;
    ob_s_desk_set_mask = ob_s_desk_unset_mask = NULL;
    ob_s_iconify_mask = ob_s_close_mask = NULL;

    ob_a_focused_unpressed_max = appearance_new(Surface_Planar, 1);
    ob_a_focused_pressed_max = appearance_new(Surface_Planar, 1);
    ob_a_focused_pressed_set_max = appearance_new(Surface_Planar, 1);
    ob_a_unfocused_unpressed_max = appearance_new(Surface_Planar, 1);
    ob_a_unfocused_pressed_max = appearance_new(Surface_Planar, 1);
    ob_a_unfocused_pressed_set_max = appearance_new(Surface_Planar, 1);
    ob_a_focused_unpressed_close = NULL;
    ob_a_focused_pressed_close = NULL;
    ob_a_unfocused_unpressed_close = NULL;
    ob_a_unfocused_pressed_close = NULL;
    ob_a_focused_unpressed_desk = NULL;
    ob_a_focused_pressed_desk = NULL;
    ob_a_focused_pressed_set_desk = NULL;
    ob_a_unfocused_unpressed_desk = NULL;
    ob_a_unfocused_pressed_desk = NULL;
    ob_a_unfocused_pressed_set_desk = NULL;
    ob_a_focused_unpressed_iconify = NULL;
    ob_a_focused_pressed_iconify = NULL;
    ob_a_unfocused_unpressed_iconify = NULL;
    ob_a_unfocused_pressed_iconify = NULL;
    ob_a_focused_grip = appearance_new(Surface_Planar, 0);
    ob_a_unfocused_grip = appearance_new(Surface_Planar, 0);
    ob_a_focused_title = appearance_new(Surface_Planar, 0);
    ob_a_unfocused_title = appearance_new(Surface_Planar, 0);
    ob_a_focused_label = appearance_new(Surface_Planar, 1);
    ob_a_unfocused_label = appearance_new(Surface_Planar, 1);
    ob_a_icon = appearance_new(Surface_Planar, 1);
    ob_a_focused_handle = appearance_new(Surface_Planar, 0);
    ob_a_unfocused_handle = appearance_new(Surface_Planar, 0);

    if (obtheme_load()) {
        RECT_SET(ob_a_focused_pressed_desk->area, 0, 0,
                 BUTTON_SIZE, BUTTON_SIZE);
        RECT_SET(ob_a_focused_pressed_set_desk->area, 0, 0,
                 BUTTON_SIZE, BUTTON_SIZE);
        RECT_SET(ob_a_focused_unpressed_desk->area, 0, 0,
                 BUTTON_SIZE, BUTTON_SIZE);
        RECT_SET(ob_a_unfocused_pressed_desk->area, 0, 0,
                 BUTTON_SIZE, BUTTON_SIZE);
        RECT_SET(ob_a_unfocused_pressed_set_desk->area, 0, 0,
                 BUTTON_SIZE, BUTTON_SIZE);
        RECT_SET(ob_a_unfocused_unpressed_desk->area, 0, 0,
                 BUTTON_SIZE, BUTTON_SIZE);
        RECT_SET(ob_a_focused_pressed_iconify->area, 0, 0,
                 BUTTON_SIZE, BUTTON_SIZE);
        RECT_SET(ob_a_focused_unpressed_iconify->area, 0, 0,
                 BUTTON_SIZE, BUTTON_SIZE);
        RECT_SET(ob_a_unfocused_pressed_iconify->area, 0, 0,
                 BUTTON_SIZE, BUTTON_SIZE);
        RECT_SET(ob_a_unfocused_unpressed_iconify->area, 0, 0,
                 BUTTON_SIZE, BUTTON_SIZE);
        RECT_SET(ob_a_unfocused_unpressed_iconify->area, 0, 0,
                 BUTTON_SIZE, BUTTON_SIZE);
        RECT_SET(ob_a_focused_pressed_max->area, 0, 0,
                 BUTTON_SIZE, BUTTON_SIZE);
        RECT_SET(ob_a_focused_pressed_set_max->area, 0, 0,
                 BUTTON_SIZE, BUTTON_SIZE);
        RECT_SET(ob_a_focused_unpressed_max->area, 0, 0,
                 BUTTON_SIZE, BUTTON_SIZE);
        RECT_SET(ob_a_unfocused_pressed_max->area, 0, 0,
                 BUTTON_SIZE, BUTTON_SIZE);
        RECT_SET(ob_a_unfocused_pressed_set_max->area, 0, 0,
                 BUTTON_SIZE, BUTTON_SIZE);
        RECT_SET(ob_a_unfocused_unpressed_max->area, 0, 0,
                 BUTTON_SIZE, BUTTON_SIZE);
        RECT_SET(ob_a_focused_pressed_close->area, 0, 0,
                 BUTTON_SIZE, BUTTON_SIZE);
        RECT_SET(ob_a_focused_unpressed_close->area, 0, 0,
                 BUTTON_SIZE, BUTTON_SIZE);
        RECT_SET(ob_a_unfocused_pressed_close->area, 0, 0,
                 BUTTON_SIZE, BUTTON_SIZE);
        RECT_SET(ob_a_unfocused_unpressed_close->area, 0, 0,
                 BUTTON_SIZE, BUTTON_SIZE);

        RECT_SET(ob_a_focused_grip->area, 0, 0,
                 GRIP_WIDTH, ob_s_handle_height);
        RECT_SET(ob_a_unfocused_grip->area, 0, 0,
                 GRIP_WIDTH, ob_s_handle_height);
        return TRUE;
    } else
        return FALSE;
}

void shutdown()
{
    if (ob_s_b_color != NULL) color_free(ob_s_b_color);
    if (ob_s_cb_unfocused_color != NULL) color_free(ob_s_cb_unfocused_color);
    if (ob_s_cb_focused_color != NULL) color_free(ob_s_cb_focused_color);
    if (ob_s_title_unfocused_color != NULL) color_free(ob_s_title_unfocused_color);
    if (ob_s_title_focused_color != NULL) color_free(ob_s_title_focused_color);
    if (ob_s_titlebut_unfocused_color != NULL)
        color_free(ob_s_titlebut_unfocused_color);
    if (ob_s_titlebut_focused_color != NULL)
        color_free(ob_s_titlebut_focused_color);

    if (ob_s_max_set_mask != NULL)
        pixmap_mask_free(ob_s_max_set_mask);
    if (ob_s_max_unset_mask != NULL)
        pixmap_mask_free(ob_s_max_unset_mask);
    if (ob_s_desk_set_mask != NULL)
        pixmap_mask_free(ob_s_desk_set_mask);
    if (ob_s_desk_unset_mask != NULL)
        pixmap_mask_free(ob_s_desk_unset_mask);
    if (ob_s_iconify_mask != NULL)
        pixmap_mask_free(ob_s_iconify_mask);
    if (ob_s_close_mask != NULL)
        pixmap_mask_free(ob_s_close_mask);

    if (ob_s_winfont != NULL) font_close(ob_s_winfont);

    appearance_free(ob_a_focused_unpressed_max);
    appearance_free(ob_a_focused_pressed_max);
    appearance_free(ob_a_focused_pressed_set_max);
    appearance_free(ob_a_unfocused_unpressed_max);
    appearance_free(ob_a_unfocused_pressed_max);
    appearance_free(ob_a_unfocused_pressed_set_max);
    if (ob_a_focused_unpressed_close != NULL)
	appearance_free(ob_a_focused_unpressed_close);
    if (ob_a_focused_pressed_close != NULL)
	appearance_free(ob_a_focused_pressed_close);
    if (ob_a_unfocused_unpressed_close != NULL)
	appearance_free(ob_a_unfocused_unpressed_close);
    if (ob_a_unfocused_pressed_close != NULL)
	appearance_free(ob_a_unfocused_pressed_close);
    if (ob_a_focused_unpressed_desk != NULL)
	appearance_free(ob_a_focused_unpressed_desk);
    if (ob_a_focused_pressed_desk != NULL)
	appearance_free(ob_a_focused_pressed_desk);
    if (ob_a_unfocused_unpressed_desk != NULL)
	appearance_free(ob_a_unfocused_unpressed_desk);
    if (ob_a_unfocused_pressed_desk != NULL)
	appearance_free(ob_a_unfocused_pressed_desk);
    if (ob_a_focused_unpressed_iconify != NULL)
	appearance_free(ob_a_focused_unpressed_iconify);
    if (ob_a_focused_pressed_iconify != NULL)
	appearance_free(ob_a_focused_pressed_iconify);
    if (ob_a_unfocused_unpressed_iconify != NULL)
	appearance_free(ob_a_unfocused_unpressed_iconify);
    if (ob_a_unfocused_pressed_iconify != NULL)
	appearance_free(ob_a_unfocused_pressed_iconify);
    appearance_free(ob_a_focused_grip);
    appearance_free(ob_a_unfocused_grip);
    appearance_free(ob_a_focused_title);
    appearance_free(ob_a_unfocused_title);
    appearance_free(ob_a_focused_label);
    appearance_free(ob_a_unfocused_label);
    appearance_free(ob_a_icon);
    appearance_free(ob_a_focused_handle);
    appearance_free(ob_a_unfocused_handle);
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
    attrib.event_mask = ELEMENT_EVENTMASK;
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
    XSetWindowBorder(ob_display, self->frame.window, ob_s_b_color->pixel);
    XSetWindowBorder(ob_display, self->label, ob_s_b_color->pixel);
    XSetWindowBorder(ob_display, self->rgrip, ob_s_b_color->pixel);
    XSetWindowBorder(ob_display, self->lgrip, ob_s_b_color->pixel);

    XResizeWindow(ob_display, self->max, BUTTON_SIZE, BUTTON_SIZE);
    XResizeWindow(ob_display, self->iconify, BUTTON_SIZE, BUTTON_SIZE);
    XResizeWindow(ob_display, self->icon, BUTTON_SIZE, BUTTON_SIZE);
    XResizeWindow(ob_display, self->close, BUTTON_SIZE, BUTTON_SIZE);
    XResizeWindow(ob_display, self->desk, BUTTON_SIZE, BUTTON_SIZE);
    XResizeWindow(ob_display, self->lgrip, GRIP_WIDTH, ob_s_handle_height);
    XResizeWindow(ob_display, self->rgrip, GRIP_WIDTH, ob_s_handle_height);

    /* set up the dynamic appearances */
    self->a_unfocused_title = appearance_copy(ob_a_unfocused_title);
    self->a_focused_title = appearance_copy(ob_a_focused_title);
    self->a_unfocused_label = appearance_copy(ob_a_unfocused_label);
    self->a_focused_label = appearance_copy(ob_a_focused_label);
    self->a_unfocused_handle = appearance_copy(ob_a_unfocused_handle);
    self->a_focused_handle = appearance_copy(ob_a_focused_handle);
    self->a_icon = appearance_copy(ob_a_icon);

    self->max_press = self->close_press = self->desk_press = 
	self->iconify_press = FALSE;

    dispatch_register(Event_X_ButtonPress | Event_X_ButtonRelease,
                      (EventHandler)mouse_event, self);

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

    dispatch_register(0, (EventHandler)mouse_event, self);

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
	    xrect[0].x = -ob_s_bevel;
	    xrect[0].y = -ob_s_bevel;
	    xrect[0].width = self->width + self->bwidth * 2;
	    xrect[0].height = TITLE_HEIGHT +
		self->bwidth * 2;
	    ++num;
	}

	if (self->frame.client->decorations & Decor_Handle) {
	    xrect[1].x = -ob_s_bevel;
	    xrect[1].y = HANDLE_Y(self);
	    xrect[1].width = self->width + self->bwidth * 2;
	    xrect[1].height = ob_s_handle_height +
		self->bwidth * 2;
	    ++num;
	}

	XShapeCombineRectangles(ob_display, self->frame.window,
				ShapeBounding, 0, 0, xrect, num,
				ShapeUnion, Unsorted);
    }
#endif
}

void frame_adjust_area(ObFrame *self, gboolean moved, gboolean resized)
{
    if (resized) {
        if (self->frame.client->decorations & Decor_Border) {
            self->bwidth = ob_s_bwidth;
            self->cbwidth = ob_s_cbwidth;
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

        /* they all default off, they're turned on in layout_title */
        self->icon_x = -1;
        self->desk_x = -1;
        self->icon_x = -1;
        self->label_x = -1;
        self->max_x = -1;
        self->close_x = -1;

        if (self->frame.client->decorations & Decor_Titlebar) {
            XMoveResizeWindow(ob_display, self->title,
                              -self->bwidth, -self->bwidth,
                              self->width, TITLE_HEIGHT);
            self->innersize.top += TITLE_HEIGHT + self->bwidth;
            XMapWindow(ob_display, self->title);

            RECT_SET(self->a_focused_title->area, 0, 0,
                     self->width, TITLE_HEIGHT);
            RECT_SET(self->a_unfocused_title->area, 0, 0,
                     self->width, TITLE_HEIGHT);

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
                              self->width, ob_s_handle_height);
            XMoveWindow(ob_display, self->lgrip,
                        -self->bwidth, -self->bwidth);
            XMoveWindow(ob_display, self->rgrip,
                        -self->bwidth + self->width -
                        GRIP_WIDTH, -self->bwidth);
            self->innersize.bottom += ob_s_handle_height +
                self->bwidth;
            XMapWindow(ob_display, self->handle);

            if (self->a_focused_handle->surface.data.planar.grad ==
                Background_ParentRelative)
                RECT_SET(self->a_focused_handle->area, 0, 0,
                         self->width, ob_s_handle_height);
            else
                RECT_SET(self->a_focused_handle->area,
                         GRIP_WIDTH + self->bwidth, 0,
                         self->width - (GRIP_WIDTH + self->bwidth) * 2,
                         ob_s_handle_height);
            if (self->a_unfocused_handle->surface.data.planar.grad ==
                Background_ParentRelative)
                RECT_SET(self->a_unfocused_handle->area, 0, 0,
                         self->width, ob_s_handle_height);
            else
                RECT_SET(self->a_unfocused_handle->area,
                         GRIP_WIDTH + self->bwidth, 0,
                         self->width - (GRIP_WIDTH + self->bwidth) * 2,
                         ob_s_handle_height);

        } else
            XUnmapWindow(ob_display, self->handle);
    }

    if (resized) {
        /* move and resize the plate */
        XMoveResizeWindow(ob_display, self->frame.plate,
                          self->innersize.left - self->cbwidth,
                          self->innersize.top - self->cbwidth,
                          self->frame.client->area.width,
                          self->frame.client->area.height);
        /* when the client has StaticGravity, it likes to move around. */
        XMoveWindow(ob_display, self->frame.client->window, 0, 0);
    }

    if (resized) {
        STRUT_SET(self->frame.size,
                  self->innersize.left + self->bwidth,
                  self->innersize.top + self->bwidth,
                  self->innersize.right + self->bwidth,
                  self->innersize.bottom + self->bwidth);
    }

    /* shading can change without being moved or resized */
    RECT_SET_SIZE(self->frame.area,
		  self->frame.client->area.width +
		  self->frame.size.left + self->frame.size.right,
		  (self->frame.client->shaded ? TITLE_HEIGHT + self->bwidth*2:
                   self->frame.client->area.height +
                   self->frame.size.top + self->frame.size.bottom));

    if (moved) {
        /* find the new coordinates, done after setting the frame.size, for
           frame_client_gravity. */
        self->frame.area.x = self->frame.client->area.x;
        self->frame.area.y = self->frame.client->area.y;
        frame_client_gravity((Frame*)self,
                             &self->frame.area.x, &self->frame.area.y);
    }

    /* move and resize the top level frame.
       shading can change without being moved or resized */
    XMoveResizeWindow(ob_display, self->frame.window,
                      self->frame.area.x, self->frame.area.y,
                      self->width,
                      self->frame.area.height - self->bwidth * 2);

    if (resized) {
        obrender_frame(self);

        frame_adjust_shape(self);
    }
}

void frame_adjust_state(ObFrame *self)
{
    obrender_frame(self);
}

void frame_adjust_focus(ObFrame *self)
{
    obrender_frame(self);
}

void frame_adjust_title(ObFrame *self)
{
    obrender_frame(self);
}

void frame_adjust_icon(ObFrame *self)
{
    obrender_frame(self);
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

    frame_adjust_area(self, TRUE, TRUE);

    /* set all the windows for the frame in the client_map */
    g_hash_table_insert(client_map, &self->frame.window, client);
    g_hash_table_insert(client_map, &self->frame.plate, client);
    g_hash_table_insert(client_map, &self->title, client);
    g_hash_table_insert(client_map, &self->label, client);
    g_hash_table_insert(client_map, &self->max, client);
    g_hash_table_insert(client_map, &self->close, client);
    g_hash_table_insert(client_map, &self->desk, client);
    g_hash_table_insert(client_map, &self->icon, client);
    g_hash_table_insert(client_map, &self->iconify, client);
    g_hash_table_insert(client_map, &self->handle, client);
    g_hash_table_insert(client_map, &self->lgrip, client);
    g_hash_table_insert(client_map, &self->rgrip, client);
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
    g_hash_table_remove(client_map, &self->frame.window);
    g_hash_table_remove(client_map, &self->frame.plate);
    g_hash_table_remove(client_map, &self->title);
    g_hash_table_remove(client_map, &self->label);
    g_hash_table_remove(client_map, &self->max);
    g_hash_table_remove(client_map, &self->close);
    g_hash_table_remove(client_map, &self->desk);
    g_hash_table_remove(client_map, &self->icon);
    g_hash_table_remove(client_map, &self->iconify);
    g_hash_table_remove(client_map, &self->handle);
    g_hash_table_remove(client_map, &self->lgrip);
    g_hash_table_remove(client_map, &self->rgrip);

    frame_free(self);
}

static void layout_title(ObFrame *self)
{
    char *lc;
    int x;
    gboolean n, d, i, l, m ,c;
    ConfigValue layout;

    n = d = i = l = m = c = FALSE;

    if (!config_get("titlebar.layout", Config_String, &layout)) {
        layout.string = "NDLIMC";
        config_set("titlebar.layout", Config_String, layout);
    }

    /* figure out whats being shown, and the width of the label */
    self->label_width = self->width - (ob_s_bevel + 1) * 2;
    for (lc = layout.string; *lc != '\0'; ++lc) {
	switch (*lc) {
	case 'N':
	    if (!(self->frame.client->decorations & Decor_Icon)) break;
            if (n) { *lc = ' '; break; } /* rm duplicates */
	    n = TRUE;
	    self->label_width -= BUTTON_SIZE + ob_s_bevel + 1;
	    break;
	case 'D':
	    if (!(self->frame.client->decorations & Decor_AllDesktops)) break;
            if (d) { *lc = ' '; break; } /* rm duplicates */
	    d = TRUE;
	    self->label_width -= BUTTON_SIZE + ob_s_bevel + 1;
	    break;
	case 'I':
	    if (!(self->frame.client->decorations & Decor_Iconify)) break;
            if (i) { *lc = ' '; break; } /* rm duplicates */
	    i = TRUE;
	    self->label_width -= BUTTON_SIZE + ob_s_bevel + 1;
	    break;
	case 'L':
            if (l) { *lc = ' '; break; } /* rm duplicates */
	    l = TRUE;
	    break;
	case 'M':
	    if (!(self->frame.client->decorations & Decor_Maximize)) break;
            if (m) { *lc = ' '; break; } /* rm duplicates */
	    m = TRUE;
	    self->label_width -= BUTTON_SIZE + ob_s_bevel + 1;
	    break;
	case 'C':
	    if (!(self->frame.client->decorations & Decor_Close)) break;
            if (c) { *lc = ' '; break; } /* rm duplicates */
	    c = TRUE;
	    self->label_width -= BUTTON_SIZE + ob_s_bevel + 1;
	    break;
	}
    }
    if (self->label_width < 1) self->label_width = 1;

    XResizeWindow(ob_display, self->label, self->label_width,
                  LABEL_HEIGHT);
  
    if (!n) XUnmapWindow(ob_display, self->icon);
    if (!d) XUnmapWindow(ob_display, self->desk);
    if (!i) XUnmapWindow(ob_display, self->iconify);
    if (!l) XUnmapWindow(ob_display, self->label);
    if (!m) XUnmapWindow(ob_display, self->max);
    if (!c) XUnmapWindow(ob_display, self->close);

    x = ob_s_bevel + 1;
    for (lc = layout.string; *lc != '\0'; ++lc) {
	switch (*lc) {
	case 'N':
	    if (!n) break;
	    self->icon_x = x;
            RECT_SET(self->a_icon->area, 0, 0, BUTTON_SIZE, BUTTON_SIZE);
	    XMapWindow(ob_display, self->icon);
	    XMoveWindow(ob_display, self->icon, x, ob_s_bevel + 1);
	    x += BUTTON_SIZE + ob_s_bevel + 1;
	    break;
	case 'D':
	    if (!d) break;
	    self->desk_x = x;
	    XMapWindow(ob_display, self->desk);
	    XMoveWindow(ob_display, self->desk, x, ob_s_bevel + 1);
	    x += BUTTON_SIZE + ob_s_bevel + 1;
	    break;
	case 'I':
	    if (!i) break;
	    self->iconify_x = x;
	    XMapWindow(ob_display, self->iconify);
	    XMoveWindow(ob_display, self->iconify, x, ob_s_bevel + 1);
	    x += BUTTON_SIZE + ob_s_bevel + 1;
	    break;
	case 'L':
	    if (!l) break;
	    self->label_x = x;
	    XMapWindow(ob_display, self->label);
	    XMoveWindow(ob_display, self->label, x, ob_s_bevel);
	    x += self->label_width + ob_s_bevel + 1;
	    break;
	case 'M':
	    if (!m) break;
	    self->max_x = x;
	    XMapWindow(ob_display, self->max);
	    XMoveWindow(ob_display, self->max, x, ob_s_bevel + 1);
	    x += BUTTON_SIZE + ob_s_bevel + 1;
	    break;
	case 'C':
	    if (!c) break;
	    self->close_x = x;
	    XMapWindow(ob_display, self->close);
	    XMoveWindow(ob_display, self->close, x, ob_s_bevel + 1);
	    x += BUTTON_SIZE + ob_s_bevel + 1;
	    break;
	}
    }

    RECT_SET(self->a_focused_label->area, 0, 0,
             self->label_width, LABEL_HEIGHT);
    RECT_SET(self->a_unfocused_label->area, 0, 0,
             self->label_width, LABEL_HEIGHT);
}

static void mouse_event(const ObEvent *e, ObFrame *self)
{
    Window win;
    gboolean press = e->type == Event_X_ButtonPress;

    win = e->data.x.e->xbutton.window;
    if (win == self->max) {
        self->max_press = press;
        obrender_frame(self);
    } else if (win == self->close) {
        self->close_press = press;
        obrender_frame(self);
    } else if (win == self->iconify) {
        self->iconify_press = press;
        obrender_frame(self);
    } else if (win == self->desk) { 
        self->desk_press = press;
        obrender_frame(self);
    }
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
