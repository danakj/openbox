#include "../../kernel/dispatch.h"

static void press(ObEvent *e, void *foo)
{
}

void plugin_startup()
{
    dispatch_register(Event_X_KeyPress, (EventHandler)press, NULL);

    /* XXX parse config file! */
}

void plugin_shutdown()
{
    dispatch_register(0, (EventHandler)press, NULL);
}

