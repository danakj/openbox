#ifndef ob__mouse_h
#define ob__mouse_h

#include "action.h"
#include "frame.h"

#include <X11/Xlib.h>

typedef enum {
    OB_MOUSE_ACTION_PRESS,
    OB_MOUSE_ACTION_RELEASE,
    OB_MOUSE_ACTION_CLICK,
    OB_MOUSE_ACTION_DOUBLE_CLICK,
    OB_MOUSE_ACTION_MOTION,
    OB_MOUSE_NUM_ACTIONS
} ObMouseAction;

void mouse_startup(gboolean reconfig);
void mouse_shutdown(gboolean reconfig);

gboolean mouse_bind(char *buttonstr, char *contextstr, ObMouseAction mact,
                    ObAction *action);

void mouse_event(struct _ObClient *client, ObFrameContext context, XEvent *e);

void mouse_grab_for_client(struct _ObClient *client, gboolean grab);

#endif
