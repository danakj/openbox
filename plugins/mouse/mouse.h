#ifndef __plugin_mouse_mouse_h
#define __plugin_mouse_mouse_h

#include "../../kernel/action.h"

typedef enum {
    MouseAction_Press,
    MouseAction_Release,
    MouseAction_Click,
    MouseAction_DClick,
    MouseAction_Motion,
    NUM_MOUSEACTION
} MouseAction;

typedef struct {
    guint state;
    guint button;
    GSList *actions[NUM_MOUSEACTION]; /* lists of Action pointers */
} MouseBinding;

extern gboolean mouse_lefthand;

gboolean mbind(char *buttonstr, char *contextstr, MouseAction mact,
               Action *action);

#endif
