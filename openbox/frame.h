#ifndef __frame_h
#define __frame_h

#include "geom.h"
#include "client.h"
#include "render2/render.h"

typedef enum {
    Context_None,
    Context_Root,
    Context_Client,
    Context_Titlebar,
    Context_Handle,
    Context_Frame,
    Context_BLCorner,
    Context_BRCorner,
    Context_TLCorner,
    Context_TRCorner,
    Context_Maximize,
    Context_AllDesktops,
    Context_Shade,
    Context_Iconify,
    Context_Icon,
    Context_Close,
    NUM_CONTEXTS
} Context;

#define FRAME_HANDLE_Y(f) (f->size.top + f->client->area.height + \
		           f->cbwidth)

typedef struct Frame {
    Client *client;

    Window window;
    Window plate;

    Strut  size;
    Rect   area;
    gboolean visible;

    struct RrSurface *s_frame;
    struct RrSurface *s_title;
    struct RrSurface *s_label;
    struct RrSurface *s_max;
    struct RrSurface *s_close;
    struct RrSurface *s_desk;
    struct RrSurface *s_shade;
    struct RrSurface *s_iconify;
    struct RrSurface *s_icon;
    struct RrSurface *s_handle;
    struct RrSurface *s_lgrip;
    struct RrSurface *s_rgrip;

    Window w_title;
    Window w_label;
    Window w_max;
    Window w_close;
    Window w_desk;
    Window w_shade;
    Window w_iconify;
    Window w_icon;
    Window w_handle;
    Window w_lgrip;
    Window w_rgrip;

    GSList *clients;

    int width;         /* title and handle */
    int label_width;
    int icon_x;        /* x-position of the window icon button */
    int label_x;       /* x-position of the window title */
    int iconify_x;     /* x-position of the window iconify button */
    int desk_x;        /* x-position of the window all-desktops button */
    int shade_x;       /* x-position of the window shade button */
    int max_x;         /* x-position of the window maximize button */
    int close_x;       /* x-position of the window close button */
    int bwidth;        /* border width */
    int cbwidth;       /* client border width */

    gboolean max_press;
    gboolean close_press;
    gboolean desk_press;
    gboolean shade_press;
    gboolean iconify_press;

    gboolean focused;
} Frame;

void frame_startup();
void frame_shutdown();

Frame *frame_new();
void frame_show(Frame *self);
void frame_hide(Frame *self);
void frame_adjust_shape(Frame *self);
void frame_adjust_area(Frame *self, gboolean moved, gboolean resized);
void frame_adjust_state(Frame *self);
void frame_adjust_focus(Frame *self, gboolean hilite);
void frame_adjust_title(Frame *self);
void frame_adjust_icon(Frame *self);
void frame_grab_client(Frame *self, Client *client);
void frame_release_client(Frame *self, Client *client);

Context frame_context_from_string(char *name);

Context frame_context(Client *self, Window win);

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


#endif
