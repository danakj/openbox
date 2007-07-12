#include "openbox/actions.h"
#include "openbox/screen.h"

static gboolean run_func(ObActionsData *data, gpointer options);

void action_showdesktop_startup()
{
    actions_register("ShowDesktop",
                     NULL, NULL,
                     run_func,
                     NULL, NULL);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionsData *data, gpointer options)
{
    screen_show_desktop(!screen_showing_desktop, NULL);

    return FALSE;
}
