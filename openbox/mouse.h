#ifndef ob__mouse_h
#define ob__mouse_h

#include "action.h"
#include "frame.h"
#include "misc.h"

#include <X11/Xlib.h>

void mouse_startup(gboolean reconfig);
void mouse_shutdown(gboolean reconfig);

gboolean mouse_bind(char *buttonstr, char *contextstr, ObMouseAction mact,
                    ObAction *action);

void mouse_event(struct _ObClient *client, XEvent *e);

void mouse_grab_for_client(struct _ObClient *client, gboolean grab);

#endif
