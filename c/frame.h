#ifndef __frame_h
#define __frame_h

#include <X11/Xlib.h>
#include "geom.h"
#include "client.h"

/*! Varius geometry settings in the frame decorations */
typedef struct {
    int width; /* title and handle */
    int font_height;
/*  int title_height() { return font_height + bevel*2; } */
    int title_height;
    int label_width;
/*  int label_height() { return font_height; } */
    int handle_height; /* static, from the style */
    int icon_x;        /* x-position of the window icon button */
    int title_x;       /* x-position of the window title */
    int iconify_x;     /* x-position of the window iconify button */
    int desktop_x;     /* x-position of the window all-desktops button */
    int max_x;         /* x-position of the window maximize button */
    int close_x;       /* x-position of the window close button */
    int handle_y;
    int button_size;   /* static, from the style */
/*  int grip_width() { return button_size * 2; } */
    int grip_width;
    int bevel;         /* static, from the style */
    int bwidth;  /* frame elements' border width */
    int cbwidth; /* client border width */
} FrameGeometry;

typedef struct Frame {
    Window window;
    Window plate;
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

    Strut  size;
    Strut  innersize;
    Rect   area;
    FrameGeometry geom;

    Client *client;
    int decorations;

    gboolean visible;
} Frame;

Frame *frame_new(struct Client *client);
void frame_free(Frame *self);

void frame_grab_client(Frame *self);
void frame_release_client(Frame *self);

/*! Update the frame's size to match the client */
void frame_adjust_size(Frame *self);
/*! Update the frame's position to match the client */
void frame_adjust_position(Frame *self);
/*! Shape the frame window to the client window */
void frame_adjust_shape(Frame *self);
/*! Update the frame to match the client's new state (for things like toggle
  buttons, focus, and the title) XXX break this up */
void frame_adjust_state(Frame *self);
/*! Update the frame to match the client's focused state */
void frame_adjust_focus(Frame *self);
/*! Update the frame to display the client's current title */
void frame_adjust_title(Frame *self);
/*! Update the frame to display the client's current icon */
void frame_adjust_icon(Frame *self);

/*! Applies gravity to the client's position to find where the frame should
  be positioned.
  @return The proper coordinates for the frame, based on the client.
*/
void frame_client_gravity(Frame *self, int *x, int *y);

/*! Reversly applies gravity to the frame's position to find where the client
  should be positioned.
    @return The proper coordinates for the client, based on the frame.
*/
void frame_frame_gravity(Frame *self, int *x, int *y);

/*! Shows the frame */
void frame_show(Frame *self);
/*! Hides the frame */
void frame_hide(Frame *self);

/*! inits quarks - this will go in engines later */
void frame_startup(void);

GQuark frame_get_context(Client *client, Window win);

#endif
