#ifndef ob__mouse_h
#define ob__mouse_h

#include "action.h"
#include "frame.h"

#include <X11/Xlib.h>

typedef enum {
    MouseAction_Press,
    MouseAction_Release,
    MouseAction_Click,
    MouseAction_DClick,
    MouseAction_Motion,
    NUM_MOUSEACTION
} ObMouseAction;

void mouse_startup();
void mouse_shutdown();

gboolean mouse_bind(char *buttonstr, char *contextstr, ObMouseAction mact,
                    ObAction *action);

void mouse_event(struct _ObClient *client, ObFrameContext context, XEvent *e);

void mouse_grab_for_client(struct _ObClient *client, gboolean grab);

#endif
