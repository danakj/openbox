#ifndef __focus_h
#define __focus_h

#include <X11/Xlib.h>
#include <glib.h>

struct Client;

/*! The window which gets focus when nothing else will be focused */
extern Window focus_backup;

/*! The client which is currently focused */
extern struct Client *focus_client;

/*! The recent focus order on each desktop */
extern GList **focus_order;

void focus_startup();
void focus_shutdown();

/*! Specify which client is currently focused, this doesn't actually
  send focus anywhere, its called by the Focus event handlers */
void focus_set_client(struct Client *client);

typedef enum {
    Fallback_Desktop,    /* switching desktops */
    Fallback_Unfocusing, /* forcefully remove focus from the current window */
    Fallback_NoFocus     /* nothing has focus for some reason */
} FallbackType;

/*! Call this when you need to focus something! */
void focus_fallback(FallbackType type);

/*! Cycle focus amongst windows
  Returns the Client to which focus has been cycled, or NULL if none. */
struct Client *focus_cycle(gboolean forward, gboolean linear, gboolean done,
                           gboolean cancel);

/*! Add a new client into the focus order */
void focus_order_add_new(struct Client *c);

/*! Remove a client from the focus order */
void focus_order_remove(struct Client *c);

/*! Move a client to the top of the focus order */
void focus_order_to_top(struct Client *c);

/*! Move a client to the bottom of the focus order (keeps iconic windows at the
  very bottom always though). */
void focus_order_to_bottom(struct Client *c);

#endif
