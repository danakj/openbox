#ifndef ob__keybaord_h
#define ob__keybaord_h

#include "keytree.h"
#include "frame.h"

#include <glib.h>
#include <X11/Xlib.h>

struct _ObClient;
struct _ObAction;

extern KeyBindingTree *keyboard_firstnode;

void keyboard_startup(gboolean reconfig);
void keyboard_shutdown(gboolean reconfig);

gboolean keyboard_bind(GList *keylist, ObAction *action);

void keyboard_event(struct _ObClient *client, const XEvent *e);
void keyboard_reset_chains();

void keyboard_interactive_grab(guint state, struct _ObClient *client,
                               ObFrameContext context,
                               struct _ObAction *action);
gboolean keyboard_process_interactive_grab(const XEvent *e,
                                           struct _ObClient **client,
                                           ObFrameContext *context);

void keyboard_grab_for_client(struct _ObClient *c, gboolean grab);

#endif
