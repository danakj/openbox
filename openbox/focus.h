#ifndef __focus_h
#define __focus_h

#include <X11/Xlib.h>
#include <glib.h>

struct _ObClient;

/*! The client which is currently focused */
extern struct _ObClient *focus_client;

/*! The recent focus order on each desktop */
extern GList **focus_order;

void focus_startup();
void focus_shutdown();

/*! Specify which client is currently focused, this doesn't actually
  send focus anywhere, its called by the Focus event handlers */
void focus_set_client(struct _ObClient *client);

typedef enum {
    OB_FOCUS_FALLBACK_DESKTOP,    /*!< switching desktops */
    OB_FOCUS_FALLBACK_UNFOCUSING, /*!< forcefully remove focus from the
                                    current window */
    OB_FOCUS_FALLBACK_NOFOCUS     /*!< nothing has focus for some reason */
} ObFocusFallbackType;

/*! Call this when you need to focus something! */
void focus_fallback(ObFocusFallbackType type);

/*! Cycle focus amongst windows
  Returns the _ObClient to which focus has been cycled, or NULL if none. */
struct _ObClient *focus_cycle(gboolean forward, gboolean linear, gboolean done,
                           gboolean cancel);

/*! Add a new client into the focus order */
void focus_order_add_new(struct _ObClient *c);

/*! Remove a client from the focus order */
void focus_order_remove(struct _ObClient *c);

/*! Move a client to the top of the focus order */
void focus_order_to_top(struct _ObClient *c);

/*! Move a client to the bottom of the focus order (keeps iconic windows at the
  very bottom always though). */
void focus_order_to_bottom(struct _ObClient *c);

#endif
