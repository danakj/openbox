#include "openbox/actions.h"
#include "openbox/dock.h"
#include "openbox/config.h"

static gboolean run_func(ObActionsData *data, gpointer options);

void action_dockautohide_startup()
{
    actions_register("ToggleDockAutoHide",
                     NULL, NULL,
                     run_func,
                     NULL, NULL);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionsData *data, gpointer options)
{
    config_dock_hide = !config_dock_hide;
    dock_configure();

    return FALSE;
}
