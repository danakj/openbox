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

/*! Should new windows be focused */
extern gboolean focus_new;
/*! Focus windows when the mouse enters them */
extern gboolean focus_follow;

void focus_startup();
void focus_shutdown();

/*! Specify which client is currently focused, this doesn't actually
  send focus anywhere, its called by the Focus event handlers */
void focus_set_client(struct Client *client);

typedef enum {
    Fallback_Desktop,    /* switching desktops */
    Fallback_Unfocusing, /* forcefully remove focus from the curernt window */
    Fallback_NoFocus     /* nothing has focus for some reason */
} FallbackType;

/*! Call this when you need to focus something! */
void focus_fallback(FallbackType type);

/*! Cycle focus amongst windows
  Returns the Client to which focus has been cycled, or NULL if none. */
struct Client *focus_cycle(gboolean forward, gboolean linear, gboolean done,
                           gboolean cancel);

#endif
