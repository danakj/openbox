#include "../../kernel/dispatch.h"
#include <glib.h>

void my_powerful_function() {}

static void event(ObEvent *e, void *foo)
{
    switch (e->type) {
    case Event_X_ButtonPress:
        break;
    case Event_X_ButtonRelease:
        break;
    case Event_X_MotionNotify:
        break;
    default:
        g_assert_not_reached();
    }
}

void plugin_startup()
{
    dispatch_register(Event_X_ButtonPress | Event_X_ButtonRelease |
                      Event_X_MotionNotify, (EventHandler)event, NULL);
}

void plugin_shutdown()
{
    dispatch_register(0, (EventHandler)event, NULL);
}
